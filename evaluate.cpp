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

void PawnEval(CHESSBOARD *b, short &score, short &opening_bonus, short &endgame_bonus) {
	bool toMove = b->getActivePlayer();
	//	TODO:	-doubled
	//			-isolated (on open/closed files) 
	//			-backward (open/closed)
	//			-passed and candidate passed pawn


	bitboard pieceDB = b->getPieceBB(toMove,nPawn);
	while(pieceDB) {
		int square = bitscan_msb(pieceDB);
		//Square value:
		score += nSquareValue[toMove][nPawn][square];
		//Passed pawn bonus:
		if(b->getPieceBB(!toMove,nPawn) & mask::passed_pawn[toMove][square])
			score+=PASSED_PAWN_VALUE;
		
		//Turn off the bit:
		pieceDB ^= (1i64 << square);
	}
	pieceDB = b->getPieceBB(!toMove,nPawn);
	while(pieceDB) {
		int square = bitscan_msb(pieceDB);
		//Square value:
		score -= nSquareValue[!toMove][nPawn][square];
		//Passed pawn bonus:
		if(b->getPieceBB(toMove,nPawn) & mask::passed_pawn[!toMove][square])
			score-=PASSED_PAWN_VALUE;

		//Turn off the bit:
		pieceDB ^= (1i64 << square);
	}

}

int relative_eval(CHESSBOARD * b, int alpha, int beta) {
//Returns game evaluation with respect to the player to move.

	//TODO:		
	//			-More mobility checks.
	//			-More positioning checks: passed pawns, doubled pawns.
	//			-King defense. 

	bool toMove = b->getActivePlayer();
	short score = 0;

//Material score:
	score += b->getMaterialValue(toMove) - b->getMaterialValue(!toMove);

	//if(score > beta + 100 || score < alpha - 100)
	//	return score;

//Phase:
	short endgame_bonus = 0;
	short opening_bonus = 0;

	
	unsigned char square;
	/*******************************************************************************
		Pawns:
	*******************************************************************************/

	PawnEval(b, score, opening_bonus, endgame_bonus);

	/*******************************************************************************
		Knights:
	*******************************************************************************/
	bitboard pieceDB = b->getPieceBB(toMove,nKnight);
	while(pieceDB) {
		square = bitscan_msb(pieceDB);
		score += nSquareValue[toMove][nKnight][square];
/*		
		//1 point for each controlled square:
		score += bitcnt(mask::knight_moves[square] & ~b->getPieceBB(toMove,nKnight));
*/	
		//Reduce value in late game:
		endgame_bonus-= KNIGHT_LATE_PENALTY;
		
		pieceDB ^= (1i64 << square);
	}
	pieceDB = b->getPieceBB(!toMove,nKnight);
	while(pieceDB) {
		square = bitscan_msb(pieceDB);
		score -= nSquareValue[!toMove][nKnight][square];
/*		
		//1 point for each controlled square:
		score -= bitcnt(mask::knight_moves[square] & ~b->getPieceBB(!toMove,nKnight));
*/		
		//Reduce value in late game:
		endgame_bonus+= KNIGHT_LATE_PENALTY;
		
		pieceDB ^= (1i64 << square);
	}

	/*******************************************************************************
		Bishops:
	*******************************************************************************/
	//	TODO:	-Inclease value in late game
	//			-Penalize "bad" bishops.
	pieceDB = b->getPieceBB(toMove,nBishop);
	bool two_bishops = false;
	while(pieceDB) {
		square = bitscan_msb(pieceDB);
		score += nSquareValue[toMove][nBishop][square];

		//Endgame bonus for two bishops:
		if(two_bishops)
			endgame_bonus += BISHOP_PAIR_LATE_BONUS;
		
		pieceDB ^= (1i64 << square);
		two_bishops = true;
	}
	two_bishops = false;
	pieceDB = b->getPieceBB(!toMove,nBishop);
	while(pieceDB) {
		square = bitscan_msb(pieceDB);
		score -= nSquareValue[!toMove][nBishop][square];
		//Endgame bonus for two bishops:
		if(two_bishops)
			endgame_bonus -= BISHOP_PAIR_LATE_BONUS;
		
		pieceDB ^= (1i64 << square);
		two_bishops = true;
	}

	/*******************************************************************************
		Rooks:
	*******************************************************************************/
	//	TODO:	-make an open file bonus that is a function of number of pawns.
	//			-bonus for rooks not moving until a castle in early game.



	/*******************************************************************************
		Queens:
	*******************************************************************************/
	// TODO:	-bonus for not activating the queen early.
	
	/*******************************************************************************
		Kings:
	*******************************************************************************/
	// TODO:	-protected by fortress in early game.
	
	square = bitscan_msb(b->getPieceBB(toMove,nKing));
	opening_bonus += nSquareValue[toMove][nKing][square];
	endgame_bonus += nEndgameKingValue[toMove][square];
	
	square = bitscan_msb(b->getPieceBB(!toMove,nKing));
	opening_bonus -= nSquareValue[!toMove][nKing][square];
	endgame_bonus -= nEndgameKingValue[!toMove][square];


	/*******************************************************************************
		Tapered Eval:
	*******************************************************************************/
	//13955 - 9900 = 4055
	//Endgame is at 1300 points. 4055 - 1300 = 2755
	// m-9900-1300. when m=0 it is endgame. when m=2755 it is opening
	// opening(m) +ending(2775 - m)
	
	short phase_gauge = ((b->getMaterialValue(toMove) > b->getMaterialValue(!toMove)) ? b->getMaterialValue(toMove) : b->getMaterialValue(!toMove)) - 9900; //This will range from 0 to 4055
	phase_gauge = (phase_gauge < 1300 ) ? 0 : phase_gauge - 1300; //This will range from 0 to 2755

	score+= ((opening_bonus*( ((100000*phase_gauge)/2775 )) + (endgame_bonus*((100000) - ((100000*phase_gauge)/2775)))))/100000;
	

	//Side to move Bonus:
	score -= SIDE_TO_MOVE_BONUS;
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