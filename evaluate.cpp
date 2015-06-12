#include "evaluate.h"

#include "uci.h"
#include "bitboards.h"
#include "board.h"

using namespace std;

int contempt(CHESSBOARD * b) {
	if(b->getMaterialValue(b->getActivePlayer()) == b->getMaterialValue(!b->getActivePlayer()))
		return -50;
	return DRAW;
}

int materialScore(CHESSBOARD * b) {
	//Only used for debugging!
	int score =
		bitcnt(b->getPieceBB(WHITE,nPawn)) * nPieceValue[nPawn]
	+	bitcnt(b->getPieceBB(WHITE,nKnight)) * nPieceValue[nKnight]
	+	bitcnt(b->getPieceBB(WHITE,nBishop)) * nPieceValue[nBishop]
	+	bitcnt(b->getPieceBB(WHITE,nRook)) * nPieceValue[nRook]
	+	bitcnt(b->getPieceBB(WHITE,nQueen)) * nPieceValue[nQueen]
	+	bitcnt(b->getPieceBB(WHITE,nKing)) * nPieceValue[nKing]
	-	bitcnt(b->getPieceBB(BLACK,nPawn)) * nPieceValue[nPawn]
	-	bitcnt(b->getPieceBB(BLACK,nKnight)) * nPieceValue[nKnight]
	-	bitcnt(b->getPieceBB(BLACK,nBishop)) * nPieceValue[nBishop]
	-	bitcnt(b->getPieceBB(BLACK,nRook)) * nPieceValue[nRook]
	-	bitcnt(b->getPieceBB(BLACK,nQueen)) * nPieceValue[nQueen]
	-	bitcnt(b->getPieceBB(BLACK,nKing)) * nPieceValue[nKing];

	return score;
}

int evaluate(CHESSBOARD * b) {
	
	short score = relative_eval(b);

	//Unmake it relative:
	if(b->getActivePlayer() == BLACK)
		score = -score;

	return score;
}


int relative_eval(CHESSBOARD * b, int alpha, int beta) {
//Returns game evaluation with respect to the player to move.

	//TODO:		
	//			-More mobility checks.
	//			-More positioning checks: passed pawns, doubled pawns.
	//			-King defense. 

	bool toMove = b->getActivePlayer();
	int score = 0;

//Material score:
	score += b->getMaterialValue(toMove) - b->getMaterialValue(!toMove);

	//if(score > beta + 100 || score < alpha - 100)
	//	return score;

//Phase:
	//short endgame_bonus = 0;
	short opening_bonus = 0;
	//bool endgame = false;
	//if((b->getMaterialValue(toMove) <= ENDGAME) && (b->getMaterialValue(!toMove) <= ENDGAME))
	//	endgame = true;
	
	/*******************************************************************************
		Pawns:
	*******************************************************************************/
	//	TODO:	- Add double/triple pawn penalty.
	//			- Add bonus for passed pawn.
	bitboard pieceDB = b->getPieceBB(toMove,nPawn);
	while(pieceDB) {
		int square = bitscan_msb(pieceDB);
		score += nSquareValue[toMove][nPawn][square];
		pieceDB ^= (1i64 << square);
	}
	pieceDB = b->getPieceBB(!toMove,nPawn);
	while(pieceDB) {
		int square = bitscan_msb(pieceDB);
		score -= nSquareValue[!toMove][nPawn][square];
		pieceDB ^= (1i64 << square);
	}

	/*******************************************************************************
		Knights:
	*******************************************************************************/
	pieceDB = b->getPieceBB(toMove,nKnight);
	while(pieceDB) {
		int square = bitscan_msb(pieceDB);
		score += nSquareValue[toMove][nKnight][square];
/*		
		//1 point for each controlled square:
		score += bitcnt(mask::knight_moves[square] & ~b->getPieceBB(toMove,nKnight));
*/	
		//Reduce value in late game:
		//endgame_bonus-= 10;
		
		pieceDB ^= (1i64 << square);
	}
	pieceDB = b->getPieceBB(!toMove,nKnight);
	while(pieceDB) {
		int square = bitscan_msb(pieceDB);
		score -= nSquareValue[!toMove][nKnight][square];
/*		
		//1 point for each controlled square:
		score -= bitcnt(mask::knight_moves[square] & ~b->getPieceBB(!toMove,nKnight));
*/		
		//Reduce value in late game:
		//endgame_bonus+= 10;
		
		pieceDB ^= (1i64 << square);
	}

	/*******************************************************************************
		Bishops:
	*******************************************************************************/
	//	TODO:	-Inclease value in late game
	//			-Penalize "bad" bishops.
	pieceDB = b->getPieceBB(toMove,nBishop);
	//bool two_bishops = false;
	while(pieceDB) {
		int square = bitscan_msb(pieceDB);
		score += nSquareValue[toMove][nBishop][square];
/*		
		//X-ray mobility bonus. stops at enemy piece or own pawn.
		
		//NW
		bitboard mobility = mask::nw[square] & (b->getOccupiedBB(!toMove) | b->getPieceBB(toMove,nPawn));
		short blocker = bitscan_lsb(mobility);
		mobility ^= mask::nw[blocker];
		mobility &= ~b->getPieceBB(toMove,nPawn);
		score += bitcnt(mobility);
		//NE
		mobility = mask::ne[square] & (b->getOccupiedBB(!toMove) | b->getPieceBB(toMove,nPawn));
		blocker = bitscan_lsb(mobility);
		mobility ^= mask::ne[blocker];
		mobility &= ~b->getPieceBB(toMove,nPawn);
		score += bitcnt(mobility);
		//SW
		mobility = mask::sw[square] & (b->getOccupiedBB(!toMove) | b->getPieceBB(toMove,nPawn));
		blocker = bitscan_msb(mobility);
		mobility ^= mask::sw[blocker];
		mobility &= ~b->getPieceBB(toMove,nPawn);
		score += bitcnt(mobility);
		//SE
		mobility = mask::se[square] & (b->getOccupiedBB(!toMove) | b->getPieceBB(toMove,nPawn));
		blocker = bitscan_msb(mobility);
		mobility ^= mask::se[blocker];
		mobility &= ~b->getPieceBB(toMove,nPawn);
		score += bitcnt(mobility);
*/		
		//Endgame bonus for two bishops:
		//if(two_bishops)
		//	endgame_bonus += 75;
		
		pieceDB ^= (1i64 << square);
		//two_bishops = true;
	}
	//two_bishops = false;
	pieceDB = b->getPieceBB(!toMove,nBishop);
	while(pieceDB) {
		int square = bitscan_msb(pieceDB);
		score -= nSquareValue[!toMove][nBishop][square];
/*
		//X-ray mobility bonus. stops at enemy piece or own pawn.
		
		//NW
		bitboard mobility = mask::nw[square] & (b->getOccupiedBB(toMove) | b->getPieceBB(!toMove,nPawn));
		short blocker = bitscan_lsb(mobility);
		mobility ^= mask::nw[blocker];
		mobility &= ~b->getPieceBB(!toMove,nPawn);
		score -= bitcnt(mobility);
		//NE
		mobility = mask::ne[square] & (b->getOccupiedBB(toMove) | b->getPieceBB(!toMove,nPawn));
		blocker = bitscan_lsb(mobility);
		mobility ^= mask::ne[blocker];
		mobility &= ~b->getPieceBB(!toMove,nPawn);
		score -= bitcnt(mobility);
		//SW
		mobility = mask::sw[square] & (b->getOccupiedBB(toMove) | b->getPieceBB(!toMove,nPawn));
		blocker = bitscan_msb(mobility);
		mobility ^= mask::sw[blocker];
		mobility &= ~b->getPieceBB(!toMove,nPawn);
		score -= bitcnt(mobility);
		//SE
		mobility = mask::se[square] & (b->getOccupiedBB(toMove) | b->getPieceBB(!toMove,nPawn));
		blocker = bitscan_msb(mobility);
		mobility ^= mask::se[blocker];
		mobility &= ~b->getPieceBB(!toMove,nPawn);
		score -= bitcnt(mobility);
*/
		//Endgame bonus for two bishops:
		//if(two_bishops)
		//	endgame_bonus -= 75;
		
		pieceDB ^= (1i64 << square);
		//two_bishops = true;
	}

	/*******************************************************************************
		Rooks:
	*******************************************************************************/
	//	TODO:	-make an open file bonus that is a function of number of pawns.
	//			-bonus for rooks not moving until a castle in early game.

/*
	pieceDB = b->getPieceBB(toMove,nRook);
	while(pieceDB) {
		short square = bitscan_lsb(pieceDB);

		//X-ray mobility bonus, with double verticle bonus.

		//North (2x bonus)
		bitboard mobility = mask::north[square] & (b->getOccupiedBB(!toMove) | b->getPieceBB(toMove,nPawn));
		short blocker = bitscan_lsb(mobility);
		mobility ^= mask::north[blocker];
		mobility &= ~b->getPieceBB(toMove,nPawn);
		score += 2 * bitcnt(mobility);
		//South (2x bonus)
		mobility = mask::south[square] & (b->getOccupiedBB(!toMove) | b->getPieceBB(toMove,nPawn));
		blocker = bitscan_msb(mobility);
		mobility ^= mask::south[blocker];
		mobility &= ~b->getPieceBB(toMove,nPawn);
		score += 2 * bitcnt(mobility);
		//East
		mobility = mask::east[square] & (b->getOccupiedBB(!toMove) | b->getPieceBB(toMove,nPawn));
		blocker = bitscan_lsb(mobility);
		mobility ^= mask::east[blocker];
		mobility &= ~b->getPieceBB(toMove,nPawn);
		score += bitcnt(mobility);
		//West
		mobility = mask::west[square] & (b->getOccupiedBB(!toMove) | b->getPieceBB(toMove,nPawn));
		blocker = bitscan_lsb(mobility);
		mobility ^= mask::west[blocker];
		mobility &= ~b->getPieceBB(toMove,nPawn);
		score += bitcnt(mobility);

		pieceDB ^= (1i64 << square);
	}
	pieceDB = b->getPieceBB(!toMove,nRook);
	while(pieceDB) {
		short square = bitscan_lsb(pieceDB);

		//X-ray mobility bonus, with double verticle bonus.

		//North (2x bonus)
		bitboard mobility = mask::north[square] & (b->getOccupiedBB(toMove) | b->getPieceBB(!toMove,nPawn));
		short blocker = bitscan_lsb(mobility);
		mobility ^= mask::north[blocker];
		mobility &= ~b->getPieceBB(!toMove,nPawn);
		score -= 2 * bitcnt(mobility);
		//South (2x bonus)
		mobility = mask::south[square] & (b->getOccupiedBB(toMove) | b->getPieceBB(!toMove,nPawn));
		blocker = bitscan_msb(mobility);
		mobility ^= mask::south[blocker];
		mobility &= ~b->getPieceBB(!toMove,nPawn);
		score -= 2 * bitcnt(mobility);
		//East
		mobility = mask::east[square] & (b->getOccupiedBB(toMove) | b->getPieceBB(!toMove,nPawn));
		blocker = bitscan_lsb(mobility);
		mobility ^= mask::east[blocker];
		mobility &= ~b->getPieceBB(!toMove,nPawn);
		score -= bitcnt(mobility);
		//West
		mobility = mask::west[square] & (b->getOccupiedBB(toMove) | b->getPieceBB(!toMove,nPawn));
		blocker = bitscan_lsb(mobility);
		mobility ^= mask::west[blocker];
		mobility &= ~b->getPieceBB(!toMove,nPawn);
		score -= bitcnt(mobility);

		pieceDB ^= (1i64 << square);
	}
*/

	/*******************************************************************************
		Queens:
	*******************************************************************************/
	// TODO:	-bonus for not activating the queen early.
	
	/*******************************************************************************
		Kings:
	*******************************************************************************/
	// TODO:	-protected by fortress in early game.
	
	pieceDB = b->getPieceBB(toMove,nKing);
	unsigned char square = bitscan_msb(pieceDB);
	score += nSquareValue[toMove][nKing][square];
	//opening_bonus += nSquareValue[toMove][nKing][square];
	//Possible endgame piece square tables:
	//endgame_bonus += nEndgameKingValue[toMove][square];
	
	pieceDB = b->getPieceBB(!toMove,nKing);
	square = bitscan_msb(pieceDB);
	score -= nSquareValue[toMove][nKing][square];
	//opening_bonus -= nSquareValue[!toMove][nKing][square];
	//Possible endgame piece square tables:
	//endgame_bonus -= nEndgameKingValue[!toMove][square];


	/*******************************************************************************
		Tapered Eval:
	*******************************************************************************/
	// TODO: taper!
	
	//if(endgame)
	//	score+=endgame_bonus;
	//else
	//	score+=opening_bonus;

	/*
	if(score != (b->getMaterialValue(toMove) + b->getPositionValue(toMove) - b->getMaterialValue(!toMove) - b->getPositionValue(!toMove)) ) {
		cout << "eval scores dont match!\n";
	}
	*/

	//Side to move Bonus:
	//score -= 11;
	/*
	int difference = abs(score - previous_score);
	if(difference < 100) {
		if(difference > 0) {
			score -= (difference/2 - 1);
		}
	}
	*/
	return score;
}

/*
bool isInCheck(CHESSBOARD *b) {
	//Returns if the active player is in check.
	
	// TODO:	-Re-arrange in order of most occurring if/else.
	
	bool player = b->getActivePlayer();
	bool opponent = !player;
	
	bitboard pieceDB =0;

	// Is in danger from pawns?:
	if(opponent == WHITE) {
		pieceDB =	((b->getPieceBB(opponent,nPawn) & ~mask::file[a]) << 9) 
				  |	((b->getPieceBB(opponent,nPawn) & ~mask::file[h]) << 7); //5 bit operations for this?
		if(pieceDB & b->getPieceBB(player,nKing) )
			return true;
	}
	else {
		pieceDB =	((b->getPieceBB(opponent,nPawn) & ~mask::file[h]) >> 9) 
				  |	((b->getPieceBB(opponent,nPawn) & ~mask::file[a]) >> 7); //5 bit operations for this?
		if(pieceDB & b->getPieceBB(player,nKing) )
			return true;
	}
	
	// Is in danger from knights? :
	pieceDB = b->getPieceBB(opponent,nKnight);
	while(pieceDB) {
		int square = bitscan_lsb(pieceDB);
		if(mask::knight_moves[square] & b->getPieceBB(player,nKing))
			return true;
		pieceDB ^= mask::square[square];
	}

	// In Danger from Diagonal Attacks? Bishops and Queens:
	pieceDB = b->getPieceBB(opponent,nBishop) | b->getPieceBB(opponent,nQueen);
	while(pieceDB) {
		int square = bitscan_lsb(pieceDB);
		
		//NE:
		if(mask::ne[square] & b->getPieceBB(player,nKing)) { 
			if(bitscan_lsb(b->getPieceBB(player,nKing)) == bitscan_lsb(mask::ne[square] & b->getOccupiedBB(BOTH)))
				return true;
		}
		//NW:
		if(mask::nw[square] & b->getPieceBB(player,nKing)) { 
			if(bitscan_lsb(b->getPieceBB(player,nKing)) == bitscan_lsb(mask::nw[square] & b->getOccupiedBB(BOTH)))
				return true;
		}
		//SE:
		if(mask::se[square] & b->getPieceBB(player,nKing)) { 
			if(bitscan_msb(b->getPieceBB(player,nKing)) == bitscan_msb(mask::se[square] & b->getOccupiedBB(BOTH)))
				return true;
		}
		//SW:
		if(mask::sw[square] & b->getPieceBB(player,nKing)) { 
			if(bitscan_msb(b->getPieceBB(player,nKing)) == bitscan_msb(mask::sw[square] & b->getOccupiedBB(BOTH)))
				return true;
		}
		pieceDB ^= mask::square[square];
	}

	// Is in danger from straight attacks? Rooks and Queens:
	pieceDB = b->getPieceBB(opponent,nRook) | b->getPieceBB(opponent,nQueen);
	while(pieceDB) {
		int square = bitscan_lsb(pieceDB);
		//North:
		if(mask::north[square] & b->getPieceBB(player,nKing)) { 
			if(bitscan_lsb(b->getPieceBB(player,nKing)) == bitscan_lsb(mask::north[square] & b->getOccupiedBB(BOTH)))
				return true;
		}
		//West:
		if(mask::west[square] & b->getPieceBB(player,nKing)) { 
			if(bitscan_lsb(b->getPieceBB(player,nKing)) == bitscan_lsb(mask::west[square] & b->getOccupiedBB(BOTH)))
				return true;
		}
		//South:
		if(mask::south[square] & b->getPieceBB(player,nKing)) { 
			if(bitscan_msb(b->getPieceBB(player,nKing)) == bitscan_msb(mask::south[square] & b->getOccupiedBB(BOTH)))
				return true;
		}
		//East:
		if(mask::east[square] & b->getPieceBB(player,nKing)) { 
			if(bitscan_msb(b->getPieceBB(player,nKing)) == bitscan_msb(mask::east[square] & b->getOccupiedBB(BOTH)))
				return true;
		}
		pieceDB ^= mask::square[square];
	}
	return false;
}
*/