

/*******************************************************************************

Static Exchange Evaluation and Static Table Evaluation.

*******************************************************************************/

#include <iostream>

#include "SEE.h"
#include "macros.h"
#include "bitboards.h"

//int ordering_tables[2][8][64];
/*
void initOrderingTables(short t) {
	for (int color = 0; color < 2; color++) {
		for (int square = 0; square < 64; square++) {
			ordering_tables[color][nPawn][square]	=	interpolate(nSquareValue[color][nPawn][square], nEndgamePawnValue[color][square], t);
			ordering_tables[color][nKing][square]	=	interpolate(nSquareValue[color][nKing][square], nEndgameKingValue[color][square], t);
			ordering_tables[color][nKnight][square] =	nSquareValue[color][nKnight][square];
			ordering_tables[color][nBishop][square] =	nSquareValue[color][nBishop][square];
			ordering_tables[color][nRook][square]	=	nSquareValue[color][nRook][square];
			ordering_tables[color][nQueen][square]	=	nSquareValue[color][nQueen][square];
		}
	}
}
*/

short STE(CHESSBOARD * b, MOVE m) {
	//return 0;
	short early_game = nSquareValue[m.color][m.active_piece_id][m.to] - nSquareValue[m.color][m.active_piece_id][m.from];
	return early_game;
	//return ordering_tables[m.color][m.active_piece_id][m.to] - ordering_tables[m.color][m.active_piece_id][m.from];
}

void findAttackingSquares(bitboard &attackers, CHESSBOARD *b, unsigned char square) {
	//identify all of the squares that threaten the square in question.
	//Then toggle those bits on in the 'attackers' bitboard
	//		The idea is to place yourself at the position of the square in question and pretend you are each of the pieces. 
	//		If you end up being able to move to a square containing that piece, then that piece is an attacker.
	
	bitboard pieceDB = 0;
	
	//First check for black attacking pawns:
	if(square < 48) {
		attackers = ( ((((bitboard)1) << (square+9)) & ~FileMask(h)) | ((((bitboard)1) << (square+7)) & ~FileMask(a)) ) & b->getPieceBB(BLACK,nPawn);
	}
	//Next check for white attacking pawns:
	if(square > 15) {
		attackers |= ( ((((bitboard)1) << (square-9)) & ~FileMask(a)) | ((((bitboard)1) << (square-7)) & ~FileMask(h)) ) & b->getPieceBB(WHITE,nPawn);
	}

	//Store the bits of diagonal attackers to xray through:
	bitboard xray =   b->getPieceBB(WHITE, nBishop) | b->getPieceBB(WHITE, nQueen) 
					| b->getPieceBB(BLACK, nBishop) | b->getPieceBB(BLACK, nQueen);

	//Diagonal attackers:  (Note the double use of 'attackers' in xray|attackers since we want to xray through any attacking pawns.
	attackers |= mask::MagicBishopMoves[square][(( ( b->getOccupiedBB(BOTH)^(xray/*|attackers*/) )&mask::MagicBishopMask[square] ) * mask::MagicBishopNumbers[square])>>mask::MagicBishopShift[square]] 
	& (xray); //In this case, xray also happens to be the pieces we are trying to find.
	
	//Straight sliders to xray through:
	xray = b->getPieceBB(WHITE, nRook) | b->getPieceBB(WHITE, nQueen) | b->getPieceBB(BLACK, nRook) | b->getPieceBB(BLACK, nQueen);

	//Straight attackers:
	attackers |= mask::MagicRookMoves[square][(( (b->getOccupiedBB(BOTH)^xray) & mask::MagicRookMask[square]) * mask::MagicRookNumbers[square])>>mask::MagicRookShift[square]] 
	& (xray); //In this case, xray also happens to be the pieces we are trying to find.

	//Knight attackers:
	attackers |= mask::knight_moves[square] & (b->getPieceBB(WHITE, nKnight) | b->getPieceBB(BLACK, nKnight) );

	//King Attackers:
	attackers |= mask::king_moves[square] & (b->getPieceBB(WHITE, nKing) | b->getPieceBB(BLACK, nKing));

}

short findSmallestAttacker(CHESSBOARD *b, bitboard &attackers, bool color ) {
	// _IN_		CHESSBOARD, attackers, color
	// _OUT_	attackers
	// Returns the value of the attacker (zero in the case that there is not an attacker afterall.
	// Then turns off the bit in the 'attackers' bitboard.

	unsigned char bit;
	//pawns:
	if( (bit = bitscan_msb(attackers & b->getPieceBB(color, nPawn))) != 64  ) {
		//turn off the bit:
		attackers ^= (((bitboard)1) << bit);
		//Return the piece value:
		return nPieceValue[nPawn];
	}

	//knights:
	if( (bit = bitscan_msb(attackers & b->getPieceBB(color, nKnight))) != 64  ) {
		//turn off the bit:
		attackers ^= (((bitboard)1) << bit);
		//Return the piece value:
		return nPieceValue[nKnight];
	}

	//bishops:
	if( (bit = bitscan_msb(attackers & b->getPieceBB(color, nBishop))) != 64  ) {
		//turn off the bit:
		attackers ^= (((bitboard)1) << bit);
		//Return the piece value:
		return nPieceValue[nBishop];
	}

	//rooks:
	if( (bit = bitscan_msb(attackers & b->getPieceBB(color, nRook))) != 64  ) {
		//turn off the bit:
		attackers ^= (((bitboard)1) << bit);
		//Return the piece value:
		return nPieceValue[nRook];
	}

	//queens:
	if( (bit = bitscan_msb(attackers & b->getPieceBB(color, nQueen))) != 64  ) {
		//turn off the bit:
		attackers ^= (((bitboard)1) << bit);
		//Return the piece value:
		return nPieceValue[nQueen];
	}
	
	//king:
	if( (bit = bitscan_msb(attackers & b->getPieceBB(color, nKing))) != 64  ) {
		//turn off the bit:
		attackers ^= (((bitboard)1) << bit);
		//Return the piece value:
		return nPieceValue[nKing];
	}

	return 0;
}

inline short max(short a, short b) {
	return (a>b)?a:b;
}

short SEE2(CHESSBOARD *b, MOVE m) {
	
	//check to see if this is already a winnin capture first. if so, we dont need to SEE.	
	if( nPieceValue[m.captured_piece] - nPieceValue[m.active_piece_id] > 0 ) {
		return nPieceValue[m.captured_piece];
	} else {
		short gain[32];
		unsigned char d = 0;
		//Set the values for the first iteration of the loop.
		//short attackers_value = nPieceValue[m.active_piece_id];
		gain[d] = nPieceValue[m.captured_piece];
		d++;
		gain[d] = nPieceValue[m.active_piece_id] - gain[d-1];

		//Make the move that we are scoring:
		b->quickCapture(m);
		bool toMove = !m.color; //since that color just went, it is the other colors turn.

		//Find all of the squares that are attacking the square that we just moved too. store them in a bitboard.
		bitboard attackers = 0;
		findAttackingSquares(attackers, b, m.to);
		
		//loop as long as there are attackers left of the appropriate color:
		while(attackers & b->getOccupiedBB(toMove)) {
			d++;
			gain[d] = findSmallestAttacker(b,attackers,toMove) - gain[d-1];//this will also toggle off the bit of the attacker found.
			if( max(-gain[d-1], gain[d]) < 0 ) break; //prune.
			//Toggle color:
			toMove = !toMove;
		} 
		
		//unmake the move we are scoring:
		b->quickUnCapture(m);
		//roll down the total score:
		while(--d) {
			gain[d-1] = -max(-gain[d-1] , gain[d]);
		}
		return gain[0];
	}

}

short SEE3_Loop(CHESSBOARD * b, bitboard &attackers, short defender, bool toMove) {
	//short value = findSmallestAttacker(b,attackers,toMove);
	short next_defender = findSmallestAttacker(b,attackers,toMove);
	if(attackers & b->getOccupiedBB(!toMove)) {
		return max(0, defender - SEE3_Loop(b,attackers, next_defender, !toMove));
	} else
		return defender;
}

short SEE3(CHESSBOARD *b, MOVE m) {
	
	//check to see if this is already a winnin capture first. if so, we dont need to SEE.	
	
	/*
	if( nPieceValue[m.captured_piece] - nPieceValue[m.active_piece_id] > 0 ) {
		//MVV/LVA: 16000 - (8 * captured_piece_value + capturing_piece_value)
		return (16000 - (8 * nPieceValue[m.captured_piece] + nPieceValue[m.active_piece_id]));
	} 
	else {
	*/
		//Make the move that we are scoring:
		b->quickCapture(m);
				
		//Find all of the squares that are attacking the square that we just moved too. store them in a bitboard.
		bitboard attackers = 0;
		
		findAttackingSquares(attackers, b, m.to);
		short value = nPieceValue[m.captured_piece];
		if(attackers & b->getOccupiedBB(!m.color)) {
			value = nPieceValue[m.captured_piece] - SEE3_Loop(b, attackers, nPieceValue[m.active_piece_id], !m.color );
		}

		//unmake the move we are scoring:
		b->quickUnCapture(m);
		return value;
	//}

}

short SEE1(CHESSBOARD * b, MOVE m) {
	//short value = nPieceValue[m.captured_piece];
	
	short value = nPieceValue[m.captured_piece] - nPieceValue[m.active_piece_id];
	if( value <= 0 ) {
		b->quickCapture(m);
		value = nPieceValue[m.captured_piece] - SEE1(b,m.to,!m.color);
		b->quickUnCapture(m);
	}
	else {
		value = nPieceValue[m.captured_piece];
	}
	return value;
}

short SEE1(CHESSBOARD * b, unsigned char square, bool toMove) {
	//Iterates until there is not an attacker.
	short value = 0;
	short attackee_value = nPieceValue[b->getBoard(square)];
	CAPTURER attacker = getSmallestAttacker(square, !toMove, b);
	if(attacker.from != 64) {
		//attacker.print();
		//std::cout << "->";
		b->quickCapture(attacker);
		value = std::max(0, attackee_value - SEE1(b, square, !toMove));
		b->quickUnCapture(attacker);
	}
	return value;
}

CAPTURER getSmallestAttacker( unsigned char toSquare, bool toMove, CHESSBOARD * b) {
	//TODO: -en passant is not covered here.
	//		-Avoid king captures king sort of stuff.
	
	bitboard pieceDB =0;
	//Pawn check:
	if(toMove == WHITE && toSquare < 48) {
		pieceDB = (((bitboard)1) << (toSquare+9)) & ~FileMask(h); //~mask::file[h]; 
		pieceDB|= (((bitboard)1) << (toSquare+7)) & ~FileMask(a); //~mask::file[a];
		pieceDB &= b->getPieceBB(!toMove,nPawn);
		if(pieceDB) {
			return(CAPTURER(nPawn, b->getBoard(toSquare), bitscan_msb(pieceDB), toSquare, !toMove));
		}
	}
	if(toMove == BLACK && toSquare > 15) {
		pieceDB = (((bitboard)1) << (toSquare-9)) & ~FileMask(a); //~mask::file[h];  
		pieceDB|= (((bitboard)1) << (toSquare-7)) & ~FileMask(h); //~mask::file[a];
		pieceDB &= b->getPieceBB(!toMove,nPawn);
		if(pieceDB) {
			return(CAPTURER(nPawn, b->getBoard(toSquare), bitscan_msb(pieceDB), toSquare, !toMove));
		}
	}

	//Knight Check:
	pieceDB = mask::knight_moves[toSquare] & b->getPieceBB(!toMove, nKnight);
	if(pieceDB) {
		return(CAPTURER(nKnight, b->getBoard(toSquare), bitscan_msb(pieceDB), toSquare, !toMove));
	}

	
	//Bishop attacks:
	if(pieceDB = (mask::MagicBishopMoves[toSquare][((b->getOccupiedBB(BOTH)&mask::MagicBishopMask[toSquare]) * mask::MagicBishopNumbers[toSquare])>>mask::MagicBishopShift[toSquare]] 
	& (b->getPieceBB(!toMove,nBishop))) ) {
		return (CAPTURER(nBishop, b->getBoard(toSquare), bitscan_msb(pieceDB),toSquare, !toMove));
	}
	
	// Rook attacks:
	if(pieceDB = (mask::MagicRookMoves[toSquare][((b->getOccupiedBB(BOTH) & mask::MagicRookMask[toSquare]) * mask::MagicRookNumbers[toSquare])>>mask::MagicRookShift[toSquare]] 
	& (b->getPieceBB(!toMove,nRook))) ) {
		return (CAPTURER(nRook, b->getBoard(toSquare), bitscan_msb(pieceDB),toSquare, !toMove));
	}
	
	//Queen attacks:
	if(pieceDB = (mask::MagicBishopMoves[toSquare][((b->getOccupiedBB(BOTH)&mask::MagicBishopMask[toSquare]) * mask::MagicBishopNumbers[toSquare])>>mask::MagicBishopShift[toSquare]] 
	& (b->getPieceBB(!toMove,nQueen))) ) {
		return (CAPTURER(nQueen, b->getBoard(toSquare), bitscan_msb(pieceDB),toSquare, !toMove));
	}

	if(pieceDB = (mask::MagicRookMoves[toSquare][((b->getOccupiedBB(BOTH) & mask::MagicRookMask[toSquare]) * mask::MagicRookNumbers[toSquare])>>mask::MagicRookShift[toSquare]] 
	& (b->getPieceBB(!toMove,nQueen)))) {
		return (CAPTURER(nQueen, b->getBoard(toSquare),bitscan_msb(pieceDB),toSquare, !toMove));
	}
	
	//King Check:
	if(pieceDB = mask::king_moves[toSquare] & b->getPieceBB(!toMove, nKing)) {
		return(CAPTURER(nKing, b->getBoard(toSquare), bitscan_msb(pieceDB), toSquare, !toMove));
	}

	//Bishop Check:
	
	/*
	if(pieceDB = mask::ne[toSquare] & b->getPieceBB(!toMove, nBishop)) {
		if(bitscan_lsb(pieceDB) == bitscan_lsb( mask::ne[toSquare] & b->getOccupiedBB(BOTH) ) ) {
			return(CAPTURER(nBishop, b->getBoard(toSquare), bitscan_lsb(pieceDB), toSquare, !toMove));
		}
	}
	if(pieceDB = mask::nw[toSquare] & b->getPieceBB(!toMove, nBishop)) {
		if(bitscan_lsb(pieceDB) == bitscan_lsb( mask::nw[toSquare] & b->getOccupiedBB(BOTH) ) ) {
			return(CAPTURER(nBishop, b->getBoard(toSquare), bitscan_lsb(pieceDB), toSquare, !toMove));
		}
	}
	if(pieceDB = mask::se[toSquare] & b->getPieceBB(!toMove, nBishop)) {
		if(bitscan_msb(pieceDB) == bitscan_msb( mask::se[toSquare] & b->getOccupiedBB(BOTH) ) ) {
			return(CAPTURER(nBishop, b->getBoard(toSquare), bitscan_msb(pieceDB), toSquare, !toMove));
		}
	}
	if(pieceDB = mask::sw[toSquare] & b->getPieceBB(!toMove, nBishop)) {
		if(bitscan_msb(pieceDB) == bitscan_msb( mask::sw[toSquare] & b->getOccupiedBB(BOTH) ) ) {
			return(CAPTURER(nBishop, b->getBoard(toSquare), bitscan_msb(pieceDB), toSquare, !toMove));
		}
	}
	

	//Rook Check:
	if(pieceDB = mask::north[toSquare] & b->getPieceBB(!toMove, nRook)) {
		if(bitscan_lsb(pieceDB) == bitscan_lsb( mask::north[toSquare] & b->getOccupiedBB(BOTH) ) ) {
			return(CAPTURER(nRook, b->getBoard(toSquare), bitscan_lsb(pieceDB), toSquare, !toMove));
		}
	}
	if(pieceDB = mask::west[toSquare] & b->getPieceBB(!toMove, nRook)) {
		if(bitscan_lsb(pieceDB) == bitscan_lsb( mask::west[toSquare] & b->getOccupiedBB(BOTH) ) ) {
			return(CAPTURER(nRook, b->getBoard(toSquare), bitscan_lsb(pieceDB), toSquare, !toMove));
		}
	}
	if(pieceDB = mask::south[toSquare] & b->getPieceBB(!toMove, nRook)) {
		if(bitscan_msb(pieceDB) == bitscan_msb( mask::south[toSquare] & b->getOccupiedBB(BOTH) ) ) {
			return(CAPTURER(nRook, b->getBoard(toSquare), bitscan_msb(pieceDB), toSquare, !toMove));
		}
	}
	if(pieceDB = mask::east[toSquare] & b->getPieceBB(!toMove, nRook)) {
		if(bitscan_msb(pieceDB) == bitscan_msb( mask::east[toSquare] & b->getOccupiedBB(BOTH) ) ) {
			return(CAPTURER(nRook, b->getBoard(toSquare), bitscan_msb(pieceDB), toSquare, !toMove));
		}
	}
	
	//Queen Check:
	if(pieceDB = mask::ne[toSquare] & b->getPieceBB(!toMove, nQueen)) {
		if(bitscan_lsb(pieceDB) == bitscan_lsb( mask::ne[toSquare] & b->getOccupiedBB(BOTH) ) ) {
			return(CAPTURER(nQueen, b->getBoard(toSquare), bitscan_lsb(pieceDB), toSquare, !toMove));
		}
	}
	if(pieceDB = mask::nw[toSquare] & b->getPieceBB(!toMove, nQueen)) {
		if(bitscan_lsb(pieceDB) == bitscan_lsb( mask::nw[toSquare] & b->getOccupiedBB(BOTH) ) ) {
			return(CAPTURER(nQueen, b->getBoard(toSquare), bitscan_lsb(pieceDB), toSquare, !toMove));
		}
	}
	if(pieceDB = mask::se[toSquare] & b->getPieceBB(!toMove, nQueen)) {
		if(bitscan_msb(pieceDB) == bitscan_msb( mask::se[toSquare] & b->getOccupiedBB(BOTH) ) ) {
			return(CAPTURER(nQueen, b->getBoard(toSquare), bitscan_msb(pieceDB), toSquare, !toMove));
		}
	}
	if(pieceDB = mask::sw[toSquare] & b->getPieceBB(!toMove, nQueen)) {
		if(bitscan_msb(pieceDB) == bitscan_msb( mask::sw[toSquare] & b->getOccupiedBB(BOTH) ) ) {
			return(CAPTURER(nQueen, b->getBoard(toSquare), bitscan_msb(pieceDB), toSquare, !toMove));
		}
	}
	if(pieceDB = mask::north[toSquare] & b->getPieceBB(!toMove, nQueen)) {
		if(bitscan_lsb(pieceDB) == bitscan_lsb( mask::north[toSquare] & b->getOccupiedBB(BOTH) ) ) {
			return(CAPTURER(nQueen, b->getBoard(toSquare), bitscan_lsb(pieceDB), toSquare, !toMove));
		}
	}
	if(pieceDB = mask::west[toSquare] & b->getPieceBB(!toMove, nQueen)) {
		if(bitscan_lsb(pieceDB) == bitscan_lsb( mask::west[toSquare] & b->getOccupiedBB(BOTH) ) ) {
			return(CAPTURER(nQueen, b->getBoard(toSquare), bitscan_lsb(pieceDB), toSquare, !toMove));
		}
	}
	if(pieceDB = mask::south[toSquare] & b->getPieceBB(!toMove, nQueen)) {
		if(bitscan_msb(pieceDB) == bitscan_msb( mask::south[toSquare] & b->getOccupiedBB(BOTH) ) ) {
			return(CAPTURER(nQueen, b->getBoard(toSquare), bitscan_msb(pieceDB), toSquare, !toMove));
		}
	}
	if(pieceDB = mask::east[toSquare] & b->getPieceBB(!toMove, nQueen)) {
		if(bitscan_msb(pieceDB) == bitscan_msb( mask::east[toSquare] & b->getOccupiedBB(BOTH) ) ) {
			return(CAPTURER(nQueen, b->getBoard(toSquare), bitscan_msb(pieceDB), toSquare, !toMove));
		}
	}
	*/
	
	
	return(CAPTURER(0,0,64,64,0));
}