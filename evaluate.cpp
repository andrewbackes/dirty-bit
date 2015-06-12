#include "evaluate.h"

#include "uci.h"
#include "bitboards.h"
#include "board.h"

using namespace std;

PAWNTABLE pawntable;

int contempt(CHESSBOARD * b) {
	return DRAW;
	
	//Disabled for now.
	if(b->getMaterialValue(b->getActivePlayer()) >= b->getMaterialValue(!b->getActivePlayer()))
		return -50;
	return DRAW;
}

bool insufficientMaterial(CHESSBOARD *b) {
	// TODO: a lot!
	// Currently only checks for kings only remaining.
	
	return(b->getMaterialValue(WHITE) == KING_VALUE && b->getMaterialValue(BLACK) == KING_VALUE);
}

int materialScore(CHESSBOARD * b) {
	//Only used for debugging!
	int score =
		bitcount(b->getPieceBB(WHITE,nPawn)) * nPieceValue[nPawn]
	+	bitcount(b->getPieceBB(WHITE,nKnight)) * nPieceValue[nKnight]
	+	bitcount(b->getPieceBB(WHITE,nBishop)) * nPieceValue[nBishop]
	+	bitcount(b->getPieceBB(WHITE,nRook)) * nPieceValue[nRook]
	+	bitcount(b->getPieceBB(WHITE,nQueen)) * nPieceValue[nQueen]
	+	bitcount(b->getPieceBB(WHITE,nKing)) * nPieceValue[nKing]
	-	bitcount(b->getPieceBB(BLACK,nPawn)) * nPieceValue[nPawn]
	-	bitcount(b->getPieceBB(BLACK,nKnight)) * nPieceValue[nKnight]
	-	bitcount(b->getPieceBB(BLACK,nBishop)) * nPieceValue[nBishop]
	-	bitcount(b->getPieceBB(BLACK,nRook)) * nPieceValue[nRook]
	-	bitcount(b->getPieceBB(BLACK,nQueen)) * nPieceValue[nQueen]
	-	bitcount(b->getPieceBB(BLACK,nKing)) * nPieceValue[nKing];

	return score;
}

int evaluate(CHESSBOARD * b) {
	
	short score = relative_eval(b);

	//Unmake it relative:
	if(b->getActivePlayer() == BLACK)
		score = -score;

	return score;
}

void DevelopementEval() {
/*	Taken from: http://chessprogramming.wikispaces.com/file/view/LittleChessEvaluationCompendium.pdf/388410128/LittleChessEvaluationCompendium.pdf

	Order of development 
	+20 for developing n before b 
	-30 for developing q before 2 minor pieces are developed 
	-50 for developing r before 2 minors are developed 
	+60 for castling to developing pieces on the other side 
	+50 for castling short to castling long if both possible 
 
	-35 for playing twice with the same piece in the opening
	 
	Repeated moves in the opening 
	Repeated moves with the same piece or pawn in the opening (up till move 6 or 7) are usually 
	bad, they are a waste of time and stand in the way of development. 
	-20cps for a repeated move with a piece 
	-10cps for a repeated move with a pawn 

*/


}

void PawnEval(CHESSBOARD *b, short &score, short &opening_bonus, short &endgame_bonus) {
	// This score is from the POV of white. 
	// That way we dont have to double count positions in the pawn hash table
		
	//	TODO:	-isolated (on open/closed files) 
	//			-backward (open/closed)
	//			-candidate passed pawn
	//			-unstoppable pawn bonus = queen - pawn (only in king/pawn endings?)


	//Keep local versions of score changes, so the hashing will be accurate:
	short pawn_opening_bonus = 0;
	short pawn_endgame_bonus = 0;
	
	#ifdef ENABLE_PAWN_HASH
	//Check the pawn hash table:
	if(pawntable.getScores(b->getPawnKey(),pawn_opening_bonus,pawn_endgame_bonus)) {
		opening_bonus += pawn_opening_bonus;
		endgame_bonus += pawn_endgame_bonus;
		return;
	}
	#endif

	//There is no hash hit, so continue with the evaluation:
	bitboard pieceDB = b->getPieceBB(WHITE,nPawn);
	while(pieceDB) {
		int square = bitscan_msb(pieceDB);
		//Square value:
		//score += nSquareValue[toMove][nPawn][square];
		pawn_opening_bonus += nSquareValue[WHITE][nPawn][square];
		pawn_endgame_bonus += nEndgamePawnValue[WHITE][square];

		//Passed pawn bonus:
		if(b->getPieceBB(BLACK,nPawn) & mask::passed_pawn[WHITE][square]) {
			//score+=PASSED_PAWN_VALUE;
			pawn_opening_bonus += PASSED_PAWN_EARLY_VALUE;
			pawn_endgame_bonus += PASSED_PAWN_LATE_VALUE;
		}
		//Isolated pawn penalty:
		if(b->getPieceBB(WHITE,nPawn) & mask::isolated_pawn[square]) {
			pawn_opening_bonus += -ISOLATED_PAWN_EARLY_PENALTY;
			pawn_endgame_bonus += -ISOLATED_PAWN_LATE_PENALTY;
		}
		//Doubled pawn penalty:
		if(b->getPieceBB(WHITE,nPawn) & mask::north[square] ) {//look ahead for white, behind for black. hopefully this doesnt make a difference
			pawn_opening_bonus += -DOUBLED_PAWN_EARLY_PENALTY;
			pawn_endgame_bonus += -DOUBLED_PAWN_LATE_PENALTY;
		}
		//Turn off the bit:
		pieceDB ^= (1i64 << square);
	}

	pieceDB = b->getPieceBB(BLACK,nPawn);
	while(pieceDB) {
		int square = bitscan_msb(pieceDB);
		//Square value:
		//score -= nSquareValue[!toMove][nPawn][square];
		pawn_opening_bonus -= nSquareValue[BLACK][nPawn][square];
		pawn_endgame_bonus -= nEndgamePawnValue[BLACK][square];
		//Passed pawn bonus:
		if(b->getPieceBB(WHITE,nPawn) & mask::passed_pawn[BLACK][square]) {
			pawn_opening_bonus -= PASSED_PAWN_EARLY_VALUE;
			pawn_endgame_bonus -= PASSED_PAWN_LATE_VALUE;
		}
		//Isolated pawn penalty:
		if(b->getPieceBB(BLACK,nPawn) & mask::isolated_pawn[square]) {
			pawn_opening_bonus -= -ISOLATED_PAWN_EARLY_PENALTY;
			pawn_endgame_bonus -= -ISOLATED_PAWN_LATE_PENALTY;
		}
		//Doubled pawn penalty:
		if(b->getPieceBB(BLACK,nPawn) & mask::south[square] ) {//look ahead for white, behind for black. hopefully this doesnt make a difference
			pawn_opening_bonus -= -DOUBLED_PAWN_EARLY_PENALTY;
			pawn_endgame_bonus -= -DOUBLED_PAWN_LATE_PENALTY;
		}

		//Turn off the bit:
		pieceDB ^= (1i64 << square);
	}
	#ifdef ENABLE_PAWN_HASH
	pawntable.saveScore(b->getPawnKey(),pawn_opening_bonus,pawn_endgame_bonus);
	#endif
	/*
	//DEBUG:
	if(b->getPawnKey() != b->generatePawnKey()) {
		cout << "EVAL: PAWN KEYS DONT MATCH " << endl;
		cout << "Pawn BBs:" << endl;
		bitprint(b->getPieceBB(WHITE,nPawn));
		bitprint(b->getPieceBB(BLACK,nPawn));
		system("PAUSE"); 
	}
	*/
	//Return values:
	opening_bonus += pawn_opening_bonus;
	endgame_bonus += pawn_endgame_bonus;
}

int relative_eval(CHESSBOARD * b, int alpha, int beta) {
//Returns game evaluation with respect to the player to move.

	//TODO:		
	//			-More mobility checks.
	//			-More positioning checks: passed pawns, doubled pawns.
	//			-King defense. 

	//bool toMove = b->getActivePlayer();
	short score = 0;

//Material score:
	score += b->getMaterialValue(WHITE) - b->getMaterialValue(BLACK);

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
		Kings:
	*******************************************************************************/

	square = bitscan_msb(b->getPieceBB(WHITE,nKing));
	opening_bonus += nSquareValue[WHITE][nKing][square];
	endgame_bonus += nEndgameKingValue[WHITE][square];
	
	#ifndef ENABLE_SIMPLE_EVAL
	bitboard white_king_circle = mask::king_moves[square];
	//Temporary Pawn Fortress bonus:
	if((1i64 << square) & 231i64) {
		opening_bonus += SHELTER_BONUS		 *	bitcount(white_king_circle & b->getPieceBB(WHITE,nPawn));
		opening_bonus += PIECE_SHELTER_BONUS *	bitcount(white_king_circle & b->getOccupiedBB(WHITE));
	}
	#endif
	
	square = bitscan_msb(b->getPieceBB(BLACK,nKing));
	opening_bonus -= nSquareValue[BLACK][nKing][square];
	endgame_bonus -= nEndgameKingValue[BLACK][square];
	
	#ifndef ENABLE_SIMPLE_EVAL
	bitboard black_king_circle = mask::king_moves[square];
	//Temporary Pawn Fortress bonus:
	if((1i64 << square) & 16645304222761353216i64)
		opening_bonus -= SHELTER_BONUS		 *	bitcount(black_king_circle & b->getPieceBB(BLACK,nPawn));
		opening_bonus -= PIECE_SHELTER_BONUS *	bitcount(black_king_circle & b->getOccupiedBB(BLACK));
	#endif

	/*******************************************************************************
		Knights:
	*******************************************************************************/
	bitboard pieceDB = b->getPieceBB(WHITE,nKnight);
	#ifndef ENABLE_SIMPLE_EVAL
	bitboard mobility;
	#endif
	while(pieceDB) {
		square = bitscan_msb(pieceDB);
		score += nSquareValue[WHITE][nKnight][square];
		
		#ifndef ENABLE_SIMPLE_EVAL
		// Mobility:
		mobility = mask::knight_moves[square];
		
		opening_bonus += KNIGHT_MOBILITY_EARLY_BONUS * bitcount(mobility & ~b->getOccupiedBB(WHITE));
		endgame_bonus += KNIGHT_MOBILITY_LATE_BONUS * bitcount(mobility & ~b->getOccupiedBB(WHITE));
		
		// King Threats:
		if(mobility&black_king_circle) {
			opening_bonus += KNIGHT_KING_THREAT_EARLY_BONUS * (  (mobility&black_king_circle)? 1 : 0  ); //bitcount(mobility&black_king_circle); //(  (mobility&black_king_circle)? 1 : 0  );
			endgame_bonus += KNIGHT_KING_THREAT_LATE_BONUS  * (  (mobility&black_king_circle)? 1 : 0  );//bitcount(mobility&black_king_circle); //(  (mobility&black_king_circle)? 1 : 0  );
		}		
		#endif

		//Reduce value in late game:
		endgame_bonus += -KNIGHT_LATE_PENALTY;
		
		pieceDB ^= (1i64 << square);
	}
	pieceDB = b->getPieceBB(BLACK,nKnight);
	while(pieceDB) {
		square = bitscan_msb(pieceDB);
		score -= nSquareValue[BLACK][nKnight][square];
		
		#ifndef ENABLE_SIMPLE_EVAL
		// Mobility:
		mobility = mask::knight_moves[square];
		
		opening_bonus -= KNIGHT_MOBILITY_EARLY_BONUS * bitcount(mobility & ~b->getOccupiedBB(BLACK));
		endgame_bonus -= KNIGHT_MOBILITY_LATE_BONUS  * bitcount(mobility & ~b->getOccupiedBB(BLACK));
		
		// King Threats:
		if(mobility&white_king_circle) {
			opening_bonus -= KNIGHT_KING_THREAT_EARLY_BONUS;// * (  (mobility&white_king_circle)? 1 : 0  );//bitcount(mobility&white_king_circle); //(  (mobility&white_king_circle)? 1 : 0  );
			endgame_bonus -= KNIGHT_KING_THREAT_LATE_BONUS;//  * (  (mobility&white_king_circle)? 1 : 0  );//bitcount(mobility&white_king_circle); //(  (mobility&white_king_circle)? 1 : 0  );
		}
		#endif

		//Reduce value in late game:
		endgame_bonus -= -KNIGHT_LATE_PENALTY;
		
		pieceDB ^= (1i64 << square);
	}

	/*******************************************************************************
		Bishops: (and Queens)
	*******************************************************************************/
	//	TODO:	-consider xray
	//			-Penalize "bad" bishops.
	
	pieceDB = b->getPieceBB(WHITE,nBishop);
	if(bitcount(pieceDB) > 1) {
		//Bishop Pair bonus:
		opening_bonus += BISHOP_PAIR_EARLY_BONUS;
		endgame_bonus += BISHOP_PAIR_LATE_BONUS;
	}
	#ifndef ENABLE_SIMPLE_EVAL
	//pieceDB |= b->getPieceBB(WHITE,nQueen);
	#endif
	while(pieceDB) {
		square = bitscan_msb(pieceDB);
		score += nSquareValue[WHITE][nBishop][square];

		#ifndef ENABLE_SIMPLE_EVAL
		//Mobility:
		mobility = mask::MagicBishopMoves[square][((b->getOccupiedBB(BOTH)&mask::MagicBishopMask[square]) * mask::MagicBishopNumbers[square])>>mask::MagicBishopShift[square]];
		opening_bonus += BISHOP_MOBILITY_EARLY_BONUS* bitcount(mobility & ~b->getOccupiedBB(WHITE));
		endgame_bonus += BISHOP_MOBILITY_LATE_BONUS * bitcount(mobility & ~b->getOccupiedBB(WHITE));
		
		// King Threats:
		if(mobility&black_king_circle) {
			opening_bonus += BISHOP_KING_THREAT_EARLY_BONUS;// * (  (mobility&black_king_circle)? 1 : 0  );//bitcount(mobility&black_king_circle); //(  (mobility&black_king_circle)? 1 : 0  );
			endgame_bonus += BISHOP_KING_THREAT_LATE_BONUS ;// * (  (mobility&black_king_circle)? 1 : 0  );//bitcount(mobility&black_king_circle); //(  (mobility&black_king_circle)? 1 : 0  );
		}
		#endif

		pieceDB ^= (1i64 << square);	
	}

	pieceDB = b->getPieceBB(BLACK,nBishop);
	if(bitcount(pieceDB) > 1) {
		//Bishop Pair bonus:
		opening_bonus -= BISHOP_PAIR_EARLY_BONUS;
		endgame_bonus -= BISHOP_PAIR_LATE_BONUS;
	}
	#ifndef ENABLE_SIMPLE_EVAL
	//pieceDB |= b->getPieceBB(BLACK,nQueen);
	#endif
	while(pieceDB) {
		square = bitscan_msb(pieceDB);
		score -= nSquareValue[BLACK][nBishop][square];

		#ifndef ENABLE_SIMPLE_EVAL
		//Mobility:
		mobility = mask::MagicBishopMoves[square][((b->getOccupiedBB(BOTH)&mask::MagicBishopMask[square]) * mask::MagicBishopNumbers[square])>>mask::MagicBishopShift[square]];
		opening_bonus -= BISHOP_MOBILITY_EARLY_BONUS * bitcount(mobility & ~b->getOccupiedBB(BLACK));
		endgame_bonus -= BISHOP_MOBILITY_LATE_BONUS * bitcount(mobility & ~b->getOccupiedBB(BLACK));
		
		// King Threats:
		if(mobility&white_king_circle) {
			opening_bonus -= BISHOP_KING_THREAT_EARLY_BONUS * (  (mobility&white_king_circle)? 1 : 0  );//bitcount(mobility&white_king_circle); //(  (mobility&white_king_circle)? 1 : 0  );
			endgame_bonus -= BISHOP_KING_THREAT_LATE_BONUS  * (  (mobility&white_king_circle)? 1 : 0  );//bitcount(mobility&white_king_circle); //(  (mobility&white_king_circle)? 1 : 0  );
		}
		#endif
		
		pieceDB ^= (1i64 << square);
	}
	
	
	
	/*******************************************************************************
		Rooks: 
	*******************************************************************************/
	//	TODO:	-make an open file bonus that is a function of number of pawns.
	//			-bonus for rooks not moving until a castle in early game.
	//			-seventh rank bonus
	
	#ifndef ENABLE_SIMPLE_EVAL
	pieceDB = b->getPieceBB(WHITE,nRook);// | b->getPieceBB(WHITE,nQueen);
	while(pieceDB) {
		square = bitscan_msb(pieceDB);

		// Mobility:
		mobility = mask::MagicRookMoves[square][((b->getOccupiedBB(BOTH) & mask::MagicRookMask[square]) * mask::MagicRookNumbers[square])>>mask::MagicRookShift[square]];
		opening_bonus += ROOK_MOBILITY_EARLY_BONUS * bitcount(mobility & ~b->getOccupiedBB(WHITE));
		endgame_bonus += ROOK_MOBILITY_LATE_BONUS * bitcount(mobility & ~b->getOccupiedBB(WHITE));

		// King Threats:
		if(mobility&black_king_circle) {
			opening_bonus += ROOK_KING_THREAT_EARLY_BONUS;// * (  (mobility&black_king_circle)? 1 : 0  );//bitcount(mobility&black_king_circle); //(  (mobility&black_king_circle)? 1 : 0  );
			endgame_bonus += ROOK_KING_THREAT_LATE_BONUS;//  * (  (mobility&black_king_circle)? 1 : 0  );//bitcount(mobility&black_king_circle); //(  (mobility&black_king_circle)? 1 : 0  );
		}
		
		// Semi-Open file bonus:
		if((mask::north[square] & (b->getPieceBB(WHITE,nPawn))) == 0 ) {
			opening_bonus += ROOK_SEMI_OPEN_FILE_EARLY_BONUS;
			endgame_bonus += ROOK_SEMI_OPEN_FILE_LATE_BONUS;
			// King Threats for semi open files:
			if(mobility&black_king_circle) {
				opening_bonus += SEMIOPEN_THREAT_EARLY_BONUS;// * (  (mobility&black_king_circle)? 1 : 0  );
				endgame_bonus += SEMIOPEN_THREAT_LATE_BONUS ;// * (  (mobility&black_king_circle)? 1 : 0  );
			}
			// Full-Open file bonus
			if((mask::north[square] & (b->getPieceBB(BLACK,nPawn))) == 0 ) {
				opening_bonus += ROOK_OPEN_FILE_EARLY_BONUS;
				endgame_bonus += ROOK_OPEN_FILE_LATE_BONUS;
				// King Threats for fully open files:
				if(mobility&black_king_circle) {
					opening_bonus += OPEN_THREAT_EARLY_BONUS;// * (  (mobility&black_king_circle)? 1 : 0  );
					endgame_bonus += OPEN_THREAT_LATE_BONUS;//  * (  (mobility&black_king_circle)? 1 : 0  );
				}
			}
			
		}
		
		pieceDB^= (1i64 << square);
	}

	pieceDB = b->getPieceBB(BLACK,nRook);// | b->getPieceBB(BLACK,nQueen);
	while(pieceDB) {
		square = bitscan_msb(pieceDB);
		// Mobility:
		mobility = mask::MagicRookMoves[square][((b->getOccupiedBB(BOTH) & mask::MagicRookMask[square]) * mask::MagicRookNumbers[square])>>mask::MagicRookShift[square]];
		opening_bonus -= ROOK_MOBILITY_EARLY_BONUS * bitcount(mobility & ~b->getOccupiedBB(BLACK));
		endgame_bonus -= ROOK_MOBILITY_LATE_BONUS * bitcount(mobility & ~b->getOccupiedBB(BLACK));

		// King Threats:
		if(mobility&white_king_circle) {
			opening_bonus -= ROOK_KING_THREAT_EARLY_BONUS;// * (  (mobility&white_king_circle)? 1 : 0  );//bitcount(mobility&white_king_circle);  //(  (mobility&white_king_circle)? 1 : 0  );
			endgame_bonus -= ROOK_KING_THREAT_LATE_BONUS ;// * (  (mobility&white_king_circle)? 1 : 0  );//bitcount(mobility&white_king_circle);  //(  (mobility&white_king_circle)? 1 : 0  );
		}
		square = bitscan_msb(pieceDB);
		// Semi-Open file bonus:
		if((mask::south[square] & (b->getPieceBB(BLACK,nPawn))) == 0 ) {
			opening_bonus -= ROOK_SEMI_OPEN_FILE_EARLY_BONUS;
			endgame_bonus -= ROOK_SEMI_OPEN_FILE_LATE_BONUS;
			// King Threats for semi open files:
			if(mobility&white_king_circle) {
				opening_bonus -= SEMIOPEN_THREAT_EARLY_BONUS;// * (  (mobility&white_king_circle)? 1 : 0  );
				endgame_bonus -= SEMIOPEN_THREAT_LATE_BONUS ;// * (  (mobility&white_king_circle)? 1 : 0  );
			}
			// Full-Open file bonus
			if((mask::south[square] & (b->getPieceBB(WHITE,nPawn))) == 0 ) {
				opening_bonus -= ROOK_OPEN_FILE_EARLY_BONUS;
				endgame_bonus -= ROOK_OPEN_FILE_LATE_BONUS;
				// King Threats for fully open files:
				if(mobility&white_king_circle) {
					opening_bonus -= OPEN_THREAT_EARLY_BONUS * (  (mobility&white_king_circle)? 1 : 0  );
					endgame_bonus -= OPEN_THREAT_LATE_BONUS  * (  (mobility&white_king_circle)? 1 : 0  );
				}
			}
		}
		
		pieceDB^= (1i64 << square);
	}
	#endif
	
	/*******************************************************************************
		Queens:
	*******************************************************************************/
	// TODO:	-bonus for not activating the queen early.
	
	
	//Additional mobility bonus for queens:
	#ifndef ENABLE_SIMPLE_EVAL
	pieceDB = b->getPieceBB(WHITE,nQueen);
	while(pieceDB) {
		square = bitscan_msb(pieceDB);

		// Diagonal Mobility:
		mobility = mask::MagicRookMoves[square][((b->getOccupiedBB(BOTH) & mask::MagicRookMask[square]) * mask::MagicRookNumbers[square])>>mask::MagicRookShift[square]];
		opening_bonus += QUEEN_MOBILITY_EARLY_BONUS * bitcount(mobility & ~b->getOccupiedBB(WHITE));
		endgame_bonus += QUEEN_MOBILITY_LATE_BONUS * bitcount(mobility & ~b->getOccupiedBB(WHITE));		
		// Diagonal King Threats:
		if(mobility&black_king_circle) {
			opening_bonus += QUEEN_KING_THREAT_EARLY_BONUS;// * (  (mobility&black_king_circle)? 1 : 0  );//bitcount(mobility&black_king_circle); //(  (mobility&black_king_circle)? 1 : 0  );
			endgame_bonus += QUEEN_KING_THREAT_LATE_BONUS ; //* (  (mobility&black_king_circle)? 1 : 0  );//bitcount(mobility&black_king_circle); //(  (mobility&black_king_circle)? 1 : 0  );
		}
		// Straight Mobility
		mobility = mask::MagicBishopMoves[square][((b->getOccupiedBB(BOTH)&mask::MagicBishopMask[square]) * mask::MagicBishopNumbers[square])>>mask::MagicBishopShift[square]];
		opening_bonus += QUEEN_MOBILITY_EARLY_BONUS* bitcount(mobility & ~b->getOccupiedBB(WHITE));
		endgame_bonus += QUEEN_MOBILITY_LATE_BONUS * bitcount(mobility & ~b->getOccupiedBB(WHITE));
		// Straight King Threats
		if(mobility&black_king_circle) {
			opening_bonus += QUEEN_KING_THREAT_EARLY_BONUS;// * (  (mobility&black_king_circle)? 1 : 0  );//bitcount(mobility&black_king_circle); //(  (mobility&black_king_circle)? 1 : 0  );
			endgame_bonus += QUEEN_KING_THREAT_LATE_BONUS;//  * (  (mobility&black_king_circle)? 1 : 0  );//bitcount(mobility&black_king_circle); //(  (mobility&black_king_circle)? 1 : 0  );
		}
		pieceDB^= (1i64 << square);
	}

	pieceDB = b->getPieceBB(BLACK,nQueen);
	while(pieceDB) {
		square = bitscan_msb(pieceDB);
		// Diagonal Mobility:
		mobility = mask::MagicRookMoves[square][((b->getOccupiedBB(BOTH) & mask::MagicRookMask[square]) * mask::MagicRookNumbers[square])>>mask::MagicRookShift[square]];
		opening_bonus -= QUEEN_MOBILITY_EARLY_BONUS * bitcount(mobility & ~b->getOccupiedBB(BLACK));
		endgame_bonus -= QUEEN_MOBILITY_LATE_BONUS * bitcount(mobility & ~b->getOccupiedBB(BLACK));
		// Diagonal King Threats:
		if(mobility&white_king_circle) {
			opening_bonus -= QUEEN_KING_THREAT_EARLY_BONUS;// * (  (mobility&white_king_circle)? 1 : 0  );//bitcount(mobility&black_king_circle); //(  (mobility&black_king_circle)? 1 : 0  );
			endgame_bonus -= QUEEN_KING_THREAT_LATE_BONUS;//  * (  (mobility&white_king_circle)? 1 : 0  );//bitcount(mobility&black_king_circle); //(  (mobility&black_king_circle)? 1 : 0  );
		}
		// Straight Mobility
		mobility = mask::MagicBishopMoves[square][((b->getOccupiedBB(BOTH)&mask::MagicBishopMask[square]) * mask::MagicBishopNumbers[square])>>mask::MagicBishopShift[square]];
		opening_bonus -= QUEEN_MOBILITY_EARLY_BONUS* bitcount(mobility & ~b->getOccupiedBB(BLACK));
		endgame_bonus -= QUEEN_MOBILITY_LATE_BONUS * bitcount(mobility & ~b->getOccupiedBB(BLACK));
		// Straight King Threats:
		if(mobility&white_king_circle) {
			opening_bonus -= QUEEN_KING_THREAT_EARLY_BONUS;// * (  (mobility&white_king_circle)? 1 : 0  );//bitcount(mobility&black_king_circle); //(  (mobility&black_king_circle)? 1 : 0  );
			endgame_bonus -= QUEEN_KING_THREAT_LATE_BONUS ;// * (  (mobility&white_king_circle)? 1 : 0  );//bitcount(mobility&black_king_circle); //(  (mobility&black_king_circle)? 1 : 0  );
		}
		pieceDB^= (1i64 << square);
	}
	#endif
	

	/*******************************************************************************
		Tapered Eval:
	*******************************************************************************/
	//13955 - 9900 = 4055
	//Endgame is at 1300 points. 4055 - 1300 = 2755
	// m-9900-1300. when m=0 it is endgame. when m=2755 it is opening
	// opening(m) +ending(2775 - m)
	
	short phase_gauge = ((b->getMaterialValue(WHITE) > b->getMaterialValue(BLACK)) ?
							b->getMaterialValue(WHITE) : b->getMaterialValue(BLACK)) 
						- 9900; //This will range from 0 to 4055
	phase_gauge = (phase_gauge < 1300 ) ? 0 : phase_gauge - 1300; //This will range from 0 to 2755

	score+= ((opening_bonus*( ((100000*phase_gauge)/2775 )) + (endgame_bonus*((100000) - ((100000*phase_gauge)/2775)))))/100000;
	
	//Side to move Bonus:
	score -= SIDE_TO_MOVE_BONUS;

	//Remember to flip it for black:
	if(b->getActivePlayer() == BLACK)
		return -score;

	return score;
}