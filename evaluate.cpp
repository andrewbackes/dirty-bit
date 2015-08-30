#include "evaluate.h"

#include "uci.h"
#include "bitboards.h"
#include "board.h"
#include "macros.h"

extern unsigned char	passed_pawn_value[8];
extern unsigned char	passed_pawn_late_value[8];
extern unsigned char	canidate_pp_value[8];
extern unsigned char	canidate_pp_late_value[8];
extern unsigned char	pp_self_blocking_penalty[8];
extern unsigned char	pp_unthreatened_path_bonus[8];
extern unsigned char	pp_clear_path_bonus[8];
extern unsigned char	BISHOP_PAIR_BONUS[17];

using namespace std;

PAWNTABLE pawntable;

int contempt(CHESSBOARD * b) {
	//return DRAW;
	
	if(b->getMaterialValue(b->getActivePlayer()) >= b->getMaterialValue(!b->getActivePlayer()))
		return -50;
	return DRAW;
}

bool insufficientMaterial(CHESSBOARD *b) {
	// FIDE draws:
	//			both sides have a bare king
	//			one side has a king and a minor piece against a bare king
	//			one side has two knights against the bare king
	// Todo:
	//			both sides have a king and a bishop, the bishops being the same color
	

	// White bare king:
	if (b->getMaterialValue(WHITE) == KING_VALUE) {
		switch (b->getMaterialValue(BLACK)) {
		// Two bare kings
		case KING_VALUE:
			return true;
			break;
		// Single minor piece:
		case KING_VALUE + BISHOP_VALUE:
			return true;
			break;
		//Two Knights:
		case KING_VALUE + KNIGHT_VALUE + KNIGHT_VALUE:
			if (!b->getPieceBB(BLACK, nBishop)) return true;
			break;
		}
	}

	// Black bare king:
	if (b->getMaterialValue(BLACK) == KING_VALUE) {
		switch (b->getMaterialValue(WHITE)) {
		// Single minor piece:
		case KING_VALUE + BISHOP_VALUE:
			return true;
			break;
		//Two Knights:
		case KING_VALUE + KNIGHT_VALUE + KNIGHT_VALUE:
			if (!b->getPieceBB(WHITE, nBishop)) return true;
			break;
		}
	}


	return false;
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

short PawnEval::Shelter(CHESSBOARD * b, unsigned char square) {

	return 0;
}

void PawnEval::Opening(CHESSBOARD *b, short &opening_bonus, short &endgame_bonus, PAWNSTRUCTURE & pawn_structure) {

	// This score is from the POV of white. 
	// That way we dont have to double count positions in the pawn hash table

	//	TODO:	-isolated (on open/closed files) 
	//			-backward (open/closed)
	//			-candidate passed pawn
	//			-unstoppable pawn bonus = queen - pawn (only in king/pawn endings?)


	//Keep local versions of score changes, so the hashing will be accurate:
	short pawn_opening_score = 0;
	short pawn_endgame_score = 0;
	
	#ifdef ENABLE_PAWN_HASH
	//Check the pawn hash table:
	if (pawntable.getScores(b->getPawnKey(), pawn_opening_score, pawn_endgame_score, pawn_structure)) {
		opening_bonus += pawn_opening_score;
		endgame_bonus += pawn_endgame_score;
		return;
	}
	#endif

	pawn_structure.short_shelter_penalty[WHITE] = 0; //[2] = { 0, 0 };
	pawn_structure.short_shelter_penalty[BLACK] = 0; //[2] = { 0, 0 };
	pawn_structure.long_shelter_penalty[WHITE] = 0; //[2] = { 0, 0 };
	pawn_structure.long_shelter_penalty[BLACK] = 0; //[2] = { 0, 0 };
	pawn_structure.passed_pawns[WHITE] = 0;
	pawn_structure.passed_pawns[BLACK] = 0;

	//There is no hash hit, so continue with the evaluation:
	
	bitboard pieceDB = b->getPieceBB(WHITE, nPawn);
	//Before turning bits off in the pieceDB, first adjust shelter score to prevent back row mates:
	if ( (pieceDB & mask::short_castle_pawns[WHITE]) == mask::short_castle_pawns[WHITE])
		pawn_structure.short_shelter_penalty[WHITE] = shelter_penalty[RankOf(a3) - 1];
	if ( (pieceDB & mask::long_castle_pawns[WHITE]) == mask::long_castle_pawns[WHITE])
		pawn_structure.long_shelter_penalty[WHITE] = shelter_penalty[RankOf(h3) - 1];
	if ( (pieceDB & mask::mid_castle_pawns[WHITE]) == mask::mid_castle_pawns[WHITE])
		pawn_structure.mid_shelter_penalty[WHITE] = shelter_penalty[RankOf(e4) - 1];

	while (pieceDB) {
		int square = bitscan_msb(pieceDB);
		bool open_file = !(b->getPieceBB(BLACK, nPawn) & mask::north[square]);
		/***********************************
			Piece Values:
		***********************************/
		//Square value:
		pawn_opening_score += nSquareValue[WHITE][nPawn][square];
		pawn_endgame_score += nEndgamePawnValue[WHITE][square];
		// Edge pawn penalty:
		if ((((bitboard)1) << square) & (FileMask(a) | FileMask(h)))
			pawn_opening_score += -EDGE_PAWN_PENALTY;
		/***********************************
			Passed/Canidates:
		***********************************/
		if (!(b->getPieceBB(BLACK, nPawn) & mask::passed_pawn[WHITE][square])) {
			pawn_structure.passed_pawns[WHITE] |= ((unsigned char)1 << FileOf(square));
			pawn_opening_score += passed_pawn_value[RankOf(square) - 1];
			pawn_endgame_score += passed_pawn_late_value[RankOf(square) - 1];
		}
		// Canidate:
		else if (open_file) {
			//caution: double use of the isolated pawn mask here.
			bitboard enemies = mask::isolated_pawn[square] & b->getPieceBB(BLACK, nPawn);
			bitboard friends = mask::isolated_pawn[square] & b->getPieceBB(WHITE, nPawn);
			if (bitcount(friends) >= bitcount(enemies)) {
				pawn_opening_score += canidate_pp_value[RankOf(square) - 1];
				pawn_endgame_score += canidate_pp_late_value[RankOf(square) - 1];
			}
		}
		/*********************************************
			Strength (how connected):
		*********************************************/
		//Pawn Duo
		if ((b->getPieceBB(WHITE, nPawn)&~FileMask(a))&(((bitboard)1) << (square - 1))) {//check the pawn to the right.
			pawn_opening_score += PAWN_DUO_EARLY_VALUE;
			pawn_endgame_score += PAWN_DUO_LATE_VALUE;
		}
		//Connected (supported by another pawn)
		else if (b->getPieceBB(WHITE, nPawn)&mask::pawn_captures[BLACK][square]) {//check the pawns one rank below
			pawn_opening_score += CONNECTED_PAWN_EARLY_VALUE;
			pawn_endgame_score += CONNECTED_PAWN_LATE_VALUE;
		}
		/*********************************************
			Weakness (Isolated/backward):
		*********************************************/
		else {
			//Isolated pawn penalty:
			if (!(b->getPieceBB(WHITE, nPawn) & mask::isolated_pawn[square])) {
				pawn_opening_score += -ISOLATED_PAWN_EARLY_PENALTY;
				pawn_endgame_score += -ISOLATED_PAWN_LATE_PENALTY;
				if (open_file) {
					pawn_opening_score += -ISOLATED_OPEN_FILE;
				}
			}
			//Backwards pawn penalty:
			else {
				unsigned char closest_friend = bitscan_lsb(mask::isolated_pawn[square] & b->getPieceBB(WHITE, nPawn));
				bool backwards = false;
				// Is this the furthest back pawn out of the adjacent files?
				if (RankOf(square) < RankOf(closest_friend)) {
					backwards = true;
					//is the closest friend 1 move away?
					if (RankOf(square + 8) >= RankOf(closest_friend)) {
						//is there something stopping us from getting there?
						bitboard enemy_control = b->getPieceBB(BLACK, nPawn)
							| ((b->getPieceBB(BLACK, nPawn) & ~FileMask(h)) >> 9)
							| ((b->getPieceBB(BLACK, nPawn) & ~FileMask(a)) >> 7);

						if (!(enemy_control & (((bitboard)1) << (square + 8)))) backwards = false;
					}
					//at spawn, is the friend 2 moves away?
					else if ((RankOf(square) == 2) && (RankOf(square + 16) >= RankOf(closest_friend))) {
						//is there something stopping us from getting there?
						bitboard enemy_control = b->getPieceBB(BLACK, nPawn)
							| ((b->getPieceBB(BLACK, nPawn) & ~FileMask(h)) >> 9)
							| ((b->getPieceBB(BLACK, nPawn) & ~FileMask(a)) >> 7);
						if (!(enemy_control & ((((bitboard)1) << (square + 8)) | (((bitboard)1) << (square + 16))))) backwards = false;
					}
				}
				if (backwards) {
					pawn_opening_score += -BACKWARD_PAWN_EARLY_PENALTY;
					pawn_endgame_score += -BACKWARD_PAWN_LATE_PENALTY;
					if (open_file)
						pawn_opening_score += -BACKWARD_PAWN_OPEN_FILE;
				}
			}
		}
		/*********************************************
			Doubled:
		*********************************************/
		//Doubled pawn penalty:
		if (b->getPieceBB(WHITE, nPawn) & mask::south[square]) {
		//look behind the pawn for another pawn.
			pawn_opening_score += -DOUBLED_PAWN_EARLY_PENALTY;
			pawn_endgame_score += -DOUBLED_PAWN_LATE_PENALTY;
			if (open_file) {
				pawn_opening_score += -DOUBLED_PAWN_OPEN_FILE;
				pawn_endgame_score += -DOUBLED_PAWN_OPEN_FILE;
			}
			if (RankOf( bitscan_msb(b->getPieceBB(WHITE, nPawn)&mask::south[square]) ) > 2){
				pawn_opening_score += -DOUBLED_PAWN_REAR_ADVANCED;
				pawn_endgame_score += -DOUBLED_PAWN_REAR_ADVANCED;
			}
		}
		/*********************************************
			Shelter:
		*********************************************/
		else {
		// if it isnt doubled, then it is the least advanced pawn on that file,
		// so we need to consider it for king shelter scoring.
			if (FileOf(square) >= 5) { //long
				pawn_structure.short_shelter_penalty[WHITE] += shelter_penalty[RankOf(square) - 1];
				pawn_structure.short_shelter_penalty[BLACK] += storm_penalty[RankOf(square) - 1];
			}
			else if (FileOf(square) <= 2) { //short
				pawn_structure.long_shelter_penalty[WHITE] += shelter_penalty[RankOf(square) - 1];
				pawn_structure.long_shelter_penalty[BLACK] += storm_penalty[RankOf(square) - 1];
			}
			else { //middle
				pawn_structure.mid_shelter_penalty[WHITE] += shelter_penalty[RankOf(square) - 1];
				pawn_structure.mid_shelter_penalty[BLACK] += storm_penalty[RankOf(square) - 1];
			}
		}
		//Turn off the bit:
		pieceDB ^= (((bitboard)1) << square);
		//cout << index_to_alg(square) << ": " << pawn_opening_score << "/" << pawn_endgame_score << endl;
	}
	
	pieceDB = b->getPieceBB(BLACK, nPawn);
	//Before turning bits off in the pieceDB, first adjust shelter score to prevent back row mates:
	if ( (pieceDB & mask::short_castle_pawns[BLACK]) == mask::short_castle_pawns[BLACK])
		pawn_structure.short_shelter_penalty[BLACK] = shelter_penalty[9 - RankOf(a6) - 1];
	if ( (pieceDB & mask::long_castle_pawns[BLACK]) == mask::long_castle_pawns[BLACK])
		pawn_structure.long_shelter_penalty[BLACK] = shelter_penalty[9 - RankOf(h6) - 1];
	if ( (pieceDB & mask::mid_castle_pawns[BLACK]) == mask::mid_castle_pawns[BLACK])
		pawn_structure.mid_shelter_penalty[BLACK] = shelter_penalty[9 - RankOf(e5) - 1];

	while (pieceDB) {
		int square = bitscan_msb(pieceDB);
		bool open_file = !(b->getPieceBB(WHITE, nPawn) & mask::south[square]);
		/*********************************************
			Piece Values:
		*********************************************/
		//Square value:
		pawn_opening_score -= nSquareValue[BLACK][nPawn][square];
		pawn_endgame_score -= nEndgamePawnValue[BLACK][square];
		//Edge pawn penalty
		if ((((bitboard)1) << square) & (FileMask(a) | FileMask(h)))
			pawn_opening_score -= -EDGE_PAWN_PENALTY;
		/*********************************************
			Passed/Canidates:
		*********************************************/
		//Passed pawn bonus:
		if (!(b->getPieceBB(WHITE, nPawn) & mask::passed_pawn[BLACK][square])) {
			pawn_structure.passed_pawns[BLACK] |= ((unsigned char)1 << FileOf(square));
			pawn_opening_score -= passed_pawn_value[9 - RankOf(square) - 1];
			pawn_endgame_score -= passed_pawn_late_value[9 - RankOf(square) - 1];
		}
		// Canidate:
		else if (open_file) {
			//double use of the isolated pawn mask here.
			bitboard enemies = mask::isolated_pawn[square] & b->getPieceBB(WHITE, nPawn);
			bitboard friends = mask::isolated_pawn[square] & b->getPieceBB(BLACK, nPawn);
			if (bitcount(friends) >= bitcount(enemies)) {
				pawn_opening_score -= canidate_pp_value[ 9 - RankOf(square) - 1];
				pawn_endgame_score -= canidate_pp_late_value[ 9 - RankOf(square) - 1];
			}
		}
		/*********************************************
			Strength (how connected):
		*********************************************/
		//Pawn Duo
		if ((b->getPieceBB(BLACK, nPawn)&~FileMask(h))&(((bitboard)1) << (square + 1))) {//check the pawn to the left.
			pawn_opening_score -= PAWN_DUO_EARLY_VALUE;
			pawn_endgame_score -= PAWN_DUO_LATE_VALUE;
		}
		//Connected (supported by another pawn)
		else if (b->getPieceBB(BLACK, nPawn)&mask::pawn_captures[WHITE][square]) { //check the pawns one rank below
			pawn_opening_score -= CONNECTED_PAWN_EARLY_VALUE;
			pawn_endgame_score -= CONNECTED_PAWN_LATE_VALUE;
		}
		/*********************************************
			Weakness (Isolated/backward):
		*********************************************/
		else {
			//Isolated pawn penalty:
			if (!(b->getPieceBB(BLACK, nPawn) & mask::isolated_pawn[square])) {
				pawn_opening_score -= -ISOLATED_PAWN_EARLY_PENALTY;
				pawn_endgame_score -= -ISOLATED_PAWN_LATE_PENALTY;
				if (open_file) {
					pawn_opening_score -= -ISOLATED_OPEN_FILE;
				}
			}
			//Backwards pawn penalty:
			else {
				unsigned char closest_friend = bitscan_msb(mask::isolated_pawn[square] & b->getPieceBB(BLACK, nPawn));
				bool backwards = false;
				// Is this the furthest back pawn out of the adjacent files?
				if (RankOf(square) > RankOf(closest_friend)) {
					backwards = true;
					//is the closest friend 1 move away?
					if (RankOf(square - 8) <= RankOf(closest_friend)) {
						//is there something stopping us from getting there?
						bitboard enemy_control = b->getPieceBB(WHITE, nPawn)
							| ((b->getPieceBB(WHITE, nPawn) & ~FileMask(a)) << 9)
							| ((b->getPieceBB(WHITE, nPawn) & ~FileMask(h)) << 7);
						if (!(enemy_control & (((bitboard)1) << (square - 8)))) backwards = false;
					}
					//at spawn, is the friend 2 moves away?
					else if ((RankOf(square) == 7) && (RankOf(square - 16) <= RankOf(closest_friend))) {
						//is there something stopping us from getting there?
						bitboard enemy_control = b->getPieceBB(WHITE, nPawn)
							| ((b->getPieceBB(WHITE, nPawn) & ~FileMask(a)) << 9)
							| ((b->getPieceBB(WHITE, nPawn) & ~FileMask(h)) << 7);
						if (!(enemy_control & ((((bitboard)1) << (square - 8)) | (((bitboard)1) << (square - 16))))) backwards = false;
					}
				}
				if (backwards) {
					pawn_opening_score -= -BACKWARD_PAWN_EARLY_PENALTY;
					pawn_endgame_score -= -BACKWARD_PAWN_LATE_PENALTY;
					if (open_file)
						pawn_opening_score -= -BACKWARD_PAWN_OPEN_FILE;
				}
			}
		}
		/*********************************************
			Doubled:
		*********************************************/
		//Doubled pawn penalty:
		if (b->getPieceBB(BLACK, nPawn) & mask::north[square]) {
			pawn_opening_score -= -DOUBLED_PAWN_EARLY_PENALTY;
			pawn_endgame_score -= -DOUBLED_PAWN_LATE_PENALTY;
			if (open_file) {
				pawn_opening_score -= -DOUBLED_PAWN_OPEN_FILE;
				pawn_endgame_score -= -DOUBLED_PAWN_OPEN_FILE;
			}
			if (RankOf(bitscan_lsb(b->getPieceBB(BLACK, nPawn)&mask::north[square])) < 7){
				pawn_opening_score -= -DOUBLED_PAWN_REAR_ADVANCED;
				pawn_endgame_score -= -DOUBLED_PAWN_REAR_ADVANCED;
			}
		}
		/*********************************************
			Shelter:
		*********************************************/
		else {
			// if it isnt doubled, then it is the least advanced pawn on that file,
			// so we need to consider it for king shelter scoring.
			if (FileOf(square) >= 5) { //long
				pawn_structure.short_shelter_penalty[BLACK] += shelter_penalty[9 - RankOf(square) - 1];
				pawn_structure.short_shelter_penalty[WHITE] += storm_penalty[9 - RankOf(square) - 1];
			}
			else if (FileOf(square) <= 2) { //short
				pawn_structure.long_shelter_penalty[BLACK] += shelter_penalty[9 - RankOf(square) - 1];
				pawn_structure.long_shelter_penalty[WHITE] += storm_penalty[9 - RankOf(square) - 1];
			}
			else { //middle
				pawn_structure.mid_shelter_penalty[BLACK] += shelter_penalty[9 - RankOf(square) - 1];
				pawn_structure.mid_shelter_penalty[WHITE] += storm_penalty[9 - RankOf(square) - 1];
			}
		}

		//Turn off the bit:
		pieceDB ^= (((bitboard)1) << square);
		//cout << index_to_alg(square) << ": " << pawn_opening_score << "/" << pawn_endgame_score << endl;
	}
	#ifdef ENABLE_PAWN_HASH
	pawntable.saveScore(b->getPawnKey(), pawn_opening_score, pawn_endgame_score, pawn_structure);
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
	opening_bonus += pawn_opening_score;
	endgame_bonus += pawn_endgame_score;
}

void evaluateMaterialTable(MaterialTable * m, short &opening_score, short &endgame_score) {
	//Bishop Pair Bonuses:
	if (m->bishops[WHITE] > 1) {
		opening_score += BISHOP_PAIR_EARLY_BONUS; //BISHOP_PAIR_BONUS[m->pawns[WHITE] + m->pawns[BLACK]];
		endgame_score += BISHOP_PAIR_LATE_BONUS;  //BISHOP_PAIR_BONUS[m->pawns[WHITE] + m->pawns[BLACK]];
	}
	if (m->bishops[BLACK] > 1) {
		opening_score -= BISHOP_PAIR_EARLY_BONUS; //BISHOP_PAIR_BONUS[m->pawns[WHITE] + m->pawns[BLACK]];
		endgame_score -= BISHOP_PAIR_LATE_BONUS;  //BISHOP_PAIR_BONUS[m->pawns[WHITE] + m->pawns[BLACK]];
	}
}

int evaluate(CHESSBOARD * b, bool side_to_move) {
	//Returns game evaluation with respect to the player to move.

	//Phase and flags:
	//short gauge = phase_gauge(b);
	short phase = 0;
	bool end_game[2] = { false, false };
	//bool no_pieces[2] = { true, true }; //will turn this off if pieces are found later in the eval.
	
	short endgame_score = 0;
	short opening_score = 0;
	MaterialTable material;

	//Side to move (tempo) bonus:
	if (side_to_move == WHITE) {
		opening_score += SIDE_TO_MOVE_EARLY_BONUS;
		endgame_score += SIDE_TO_MOVE_LATE_BONUS;
	}
	else {
		opening_score -= SIDE_TO_MOVE_EARLY_BONUS;
		endgame_score -= SIDE_TO_MOVE_LATE_BONUS;
	}

	unsigned char square;
	bitboard attacks[2] = { 0, 0 };

	short king_attackers[2] = { 0, 0 };
	short attackers_weight[2] = { 0, 0 };

	/*******************************************************************************
	Pawn Structure Evaluation (not including dynamic, situation specific things):
	*******************************************************************************/

	PAWNSTRUCTURE pawn_structure;
	PawnEval::Opening(b, opening_score, endgame_score, pawn_structure);
	
	material.pawns[WHITE] = bitcount(b->getPieceBB(WHITE, nPawn));
	material.pawns[BLACK] = bitcount(b->getPieceBB(BLACK, nPawn));
	phase += (material.pawns[WHITE] + material.pawns[BLACK])*PAWN_PHASE_VALUE;

	/*******************************************************************************
	Kings:
	*******************************************************************************/
	bitboard king_circle[2];

	square = bitscan_msb(b->getPieceBB(WHITE, nKing));
	opening_score += nSquareValue[WHITE][nKing][square];
	endgame_score += nEndgameKingValue[WHITE][square];
	king_circle[WHITE] = mask::king_moves[square];
	attacks[WHITE] = king_circle[WHITE];
	
	// King safety, Pawn Shelter, Pawn Storm:
	if (!end_game[WHITE]) {
		//Short Side:
		if ((((bitboard)1) << square) & mask::short_castle_area[WHITE]) {
			opening_score += -pawn_structure.short_shelter_penalty[WHITE];
		}
		//Long Side:
		else if ((((bitboard)1) << square) & mask::long_castle_area[WHITE]) {
			opening_score += -pawn_structure.long_shelter_penalty[WHITE];
		}
		
		//Middle Area:
		else {
			short penalty = pawn_structure.mid_shelter_penalty[WHITE];
			if (b->getCastlingRights(WHITE, KING_SIDE)) 
				penalty = pawn_structure.short_shelter_penalty[WHITE];
			if (b->getCastlingRights(WHITE, QUEEN_SIDE) && (pawn_structure.long_shelter_penalty < pawn_structure.short_shelter_penalty))
				penalty = pawn_structure.long_shelter_penalty[WHITE];
			opening_score += -((pawn_structure.mid_shelter_penalty[WHITE] + penalty) / 2);
		}
		
	}
	
	square = bitscan_msb(b->getPieceBB(BLACK, nKing));
	opening_score -= nSquareValue[BLACK][nKing][square];
	endgame_score -= nEndgameKingValue[BLACK][square];
	king_circle[BLACK] = mask::king_moves[square];
	attacks[BLACK] = king_circle[BLACK];
	
	if (!end_game[BLACK]) {
		//Short Side:
		if ((((bitboard)1) << square) & mask::short_castle_area[BLACK]) {
			opening_score -= -pawn_structure.short_shelter_penalty[BLACK];
		}
		//Long Side:
		else if ((((bitboard)1) << square) & mask::long_castle_area[BLACK]) {
			opening_score -= -pawn_structure.long_shelter_penalty[BLACK];
		}
		//Middle Area:
		
		else {
			
			short penalty = pawn_structure.mid_shelter_penalty[BLACK];
			if (b->getCastlingRights(BLACK, KING_SIDE))
				penalty = pawn_structure.short_shelter_penalty[BLACK];
			if (b->getCastlingRights(BLACK, QUEEN_SIDE) && (pawn_structure.long_shelter_penalty > pawn_structure.short_shelter_penalty))
				penalty = pawn_structure.long_shelter_penalty[BLACK];
			opening_score -= -((pawn_structure.mid_shelter_penalty[BLACK] + penalty) / 2);
			
		}
		
	}
	
	/*******************************************************************************
	Knights:
	*******************************************************************************/
	bitboard pieceDB = b->getPieceBB(WHITE, nKnight);
	
	bitboard mobility;
	
	while (pieceDB) {
		phase += KNIGHT_PHASE_VALUE;
		material.knights[WHITE]++;
		//no_pieces[WHITE] = false;
		square = bitscan_msb(pieceDB);
		opening_score += nSquareValue[WHITE][nKnight][square];
		endgame_score += nSquareValue[WHITE][nKnight][square];
		// Mobility:
		mobility = mask::knight_moves[square];
		attacks[WHITE] |= (mobility & ~b->getOccupiedBB(WHITE));

		opening_score += knight_mobility[bitcount(mobility)];
		endgame_score += knight_mobility[bitcount(mobility)];

		// King Threats:
		if (mobility&king_circle[BLACK]) {
			king_attackers[WHITE]++;
			attackers_weight[WHITE] += KNIGHT_THREAT_WEIGHT;
		}
		//Reduce value in late game:
		endgame_score += -KNIGHT_LATE_PENALTY;
		pieceDB ^= (((bitboard)1) << square);
	}

	pieceDB = b->getPieceBB(BLACK, nKnight);
	while (pieceDB) {
		phase += KNIGHT_PHASE_VALUE;
		material.knights[BLACK]++;
		//no_pieces[BLACK] = false;
		square = bitscan_msb(pieceDB);
		opening_score -= nSquareValue[BLACK][nKnight][square];
		endgame_score -= nSquareValue[BLACK][nKnight][square];
		// Mobility:
		mobility = mask::knight_moves[square];
		attacks[BLACK] |= (mobility & ~b->getOccupiedBB(BLACK));
		opening_score -= knight_mobility[bitcount(mobility)];
		endgame_score -= knight_mobility[bitcount(mobility)];
		// King Threats:
		if (mobility&king_circle[WHITE]) {
			king_attackers[BLACK]++;
			attackers_weight[BLACK] += KNIGHT_THREAT_WEIGHT;
		}
		//Reduce value in late game:
		endgame_score -= -KNIGHT_LATE_PENALTY;

		pieceDB ^= (((bitboard)1) << square);
	}
	
	/*******************************************************************************
	Bishops:
	*******************************************************************************/
	//	TODO:	-consider xray
	//			-Penalize "bad" bishops.
	
	pieceDB = b->getPieceBB(WHITE, nBishop);
	//Trapped Bishop Pattern check:
	#ifdef ENABLE_PATTERNS
	if (((pieceDB >> 7)&(b->getPieceBB(BLACK, nPawn))&(mask::trapped_bishop_long[WHITE])) ||
		((pieceDB >> 9)&(b->getPieceBB(BLACK, nPawn))&(mask::trapped_bishop_short[WHITE])) ){
		opening_score += -TRAPPED_BISHOP;
		endgame_score += -TRAPPED_BISHOP;
	}
	#endif
	while (pieceDB) {
		phase += BISHOP_PHASE_VALUE;
		material.bishops[WHITE]++;
		//no_pieces[WHITE] = false;
		square = bitscan_msb(pieceDB);
		opening_score += nSquareValue[WHITE][nBishop][square];
		endgame_score += nSquareValue[WHITE][nBishop][square];
		//Mobility:
		mobility = (mask::MagicBishopMoves[square][((b->getOccupiedBB(BOTH)&mask::MagicBishopMask[square]) * mask::MagicBishopNumbers[square]) >> mask::MagicBishopShift[square]] & ~b->getOccupiedBB(WHITE));
		attacks[WHITE] |= mobility;
		opening_score += bishop_mobility[bitcount(mobility)];
		endgame_score += bishop_mobility[bitcount(mobility)];

		// King Threats:
		if (mobility&king_circle[BLACK]) {
			king_attackers[WHITE]++;
			attackers_weight[WHITE] += BISHOP_THREAT_WEIGHT;
		}

		pieceDB ^= (((bitboard)1) << square);
	}

	pieceDB = b->getPieceBB(BLACK, nBishop);
	//Trapped Bishop Pattern check:
	#ifdef ENABLE_PATTERNS
	if (((pieceDB << 7)&(b->getPieceBB(WHITE, nPawn))&(mask::trapped_bishop_long[BLACK])) ||
		((pieceDB << 9)&(b->getPieceBB(WHITE, nPawn))&(mask::trapped_bishop_short[BLACK]))){
		opening_score -= -TRAPPED_BISHOP;
		endgame_score -= -TRAPPED_BISHOP;
	}
	#endif
	while (pieceDB) {
		phase += BISHOP_PHASE_VALUE;
		material.bishops[BLACK]++;
		//no_pieces[BLACK] = false;
		square = bitscan_msb(pieceDB);
		opening_score -= nSquareValue[BLACK][nBishop][square];
		endgame_score -= nSquareValue[BLACK][nBishop][square];
		//Mobility:
		mobility = (mask::MagicBishopMoves[square][((b->getOccupiedBB(BOTH)&mask::MagicBishopMask[square]) * mask::MagicBishopNumbers[square]) >> mask::MagicBishopShift[square]] & ~b->getOccupiedBB(BLACK));
		attacks[BLACK] |= mobility;
		opening_score -= bishop_mobility[bitcount(mobility)];
		endgame_score -= bishop_mobility[bitcount(mobility)];

		// King Threats:
		if (mobility&king_circle[WHITE]) {
			king_attackers[BLACK]++;
			attackers_weight[BLACK] += BISHOP_THREAT_WEIGHT;
		}
		pieceDB ^= (((bitboard)1) << square);
	}
	
	/*******************************************************************************
	Rooks:
	*******************************************************************************/
	//	TODO:	-make an open file bonus that is a function of number of pawns.
	//			-bonus for rooks not moving until a castle in early game.
	//			-seventh rank bonus
	//			-supporting passed pawns bonus

	pieceDB = b->getPieceBB(WHITE, nRook);
	while (pieceDB) {
		square = bitscan_msb(pieceDB);
		phase += ROOK_PHASE_VALUE;
		material.rooks[WHITE]++;
		//Adjust value:
		opening_score += ROOK_ADJUSTMENT[material.total_pawns()];
		endgame_score += ROOK_ADJUSTMENT[material.total_pawns()];
		// Mobility:
		mobility = (mask::MagicRookMoves[square][((b->getOccupiedBB(BOTH) & mask::MagicRookMask[square]) * mask::MagicRookNumbers[square]) >> mask::MagicRookShift[square]] & ~b->getOccupiedBB(WHITE));
		attacks[WHITE] |= mobility;
		opening_score += rook_mobility[bitcount(mobility)];
		endgame_score += rook_mobility[bitcount(mobility)];
		//Trapped Rook Pattern:		
		#ifdef ENABLE_PATTERNS
		if (RankOf(square) == 1) {
			if (FileOf(square) < 2) {
				if (b->getPieceBB(WHITE, nKing)&mask::trapped_rook_short_king[WHITE])
					opening_score += -TRAPPED_BISHOP; }
			else if (FileOf(square) > 5) {
				if (b->getPieceBB(WHITE, nKing)&mask::trapped_rook_long_king[WHITE])
					opening_score += -TRAPPED_BISHOP; }
		}
		#endif

		// King Threats:
		if (mobility&king_circle[BLACK]) {
			king_attackers[WHITE]++;
			attackers_weight[WHITE] += ROOK_THREAT_WEIGHT;
		}

		// 7th rank bonus
		if (RankOf(square) == 7) {
			opening_score += ROOK_7TH_FILE_EARLY_BONUS;
			endgame_score += ROOK_7TH_FILE_LATE_BONUS;
		}

		//opening_score -= ROOK_OPEN_FILE_EARLY_BONUS / 2;
		//endgame_score -= ROOK_OPEN_FILE_LATE_BONUS / 2;
		// Semi-Open file bonus:
		if ((mask::north[square] & (b->getPieceBB(WHITE, nPawn))) == 0) {
			opening_score += ROOK_SEMI_OPEN_FILE_EARLY_BONUS;
			endgame_score += ROOK_SEMI_OPEN_FILE_LATE_BONUS;
			// King Threats for semi open files:
			/*
			if (mobility&king_circle[BLACK]) {
				opening_score += SEMIOPEN_THREAT_EARLY_BONUS; // * (  (mobility&king_circle[BLACK])? 1 : 0  );
				endgame_score += SEMIOPEN_THREAT_LATE_BONUS;  // * (  (mobility&king_circle[BLACK])? 1 : 0  );
			}
			*/
			// Full-Open file bonus
			if ((mask::north[square] & (b->getPieceBB(BLACK, nPawn))) == 0) {
				opening_score += ROOK_OPEN_FILE_EARLY_BONUS - ROOK_SEMI_OPEN_FILE_EARLY_BONUS;
				endgame_score += ROOK_OPEN_FILE_LATE_BONUS - ROOK_SEMI_OPEN_FILE_LATE_BONUS;
				// King Threats for fully open files:
				/*
				if (mobility&king_circle[BLACK]) {
					opening_score += OPEN_THREAT_EARLY_BONUS; // * (  (mobility&king_circle[BLACK])? 1 : 0  );
					endgame_score += OPEN_THREAT_LATE_BONUS;  //  * (  (mobility&king_circle[BLACK])? 1 : 0  );
				}
				*/
			}
		}
		
		pieceDB ^= (((bitboard)1) << square);
	}

	pieceDB = b->getPieceBB(BLACK, nRook);
	while (pieceDB) {
		square = bitscan_msb(pieceDB);
		phase += ROOK_PHASE_VALUE;
		material.rooks[BLACK]++;
		//Adjust value:
		opening_score -= ROOK_ADJUSTMENT[material.total_pawns()];
		endgame_score -= ROOK_ADJUSTMENT[material.total_pawns()];
		
		// Mobility:
		mobility = (mask::MagicRookMoves[square][((b->getOccupiedBB(BOTH) & mask::MagicRookMask[square]) * mask::MagicRookNumbers[square]) >> mask::MagicRookShift[square]] & ~b->getOccupiedBB(BLACK) );
		attacks[BLACK] |= mobility;
		opening_score -= rook_mobility[bitcount(mobility)];
		endgame_score -= rook_mobility[bitcount(mobility)];
		
		//Trapped Rook Pattern:
		#ifdef ENABLE_PATTERNS
		if (RankOf(square) == 8) {
			if (FileOf(square) > 2) {
				if (b->getPieceBB(BLACK, nKing)&mask::trapped_rook_short_king[BLACK])
					opening_score -= -TRAPPED_BISHOP; }
			else if (FileOf(square) < 5) {
				if (b->getPieceBB(BLACK, nKing)&mask::trapped_rook_long_king[BLACK])
					opening_score -= -TRAPPED_BISHOP; }
		}
		#endif

		// King Threats:
		if (mobility&king_circle[WHITE]) {
			king_attackers[BLACK]++;
			attackers_weight[BLACK] += ROOK_THREAT_WEIGHT;
		}
		
		// 7th rank bonus
		if (RankOf(square) == 2) {
			opening_score -= ROOK_7TH_FILE_EARLY_BONUS;
			endgame_score -= ROOK_7TH_FILE_LATE_BONUS;
		}

		//opening_score += ROOK_OPEN_FILE_EARLY_BONUS / 2;
		//endgame_score += ROOK_OPEN_FILE_LATE_BONUS / 2;
		// Semi-Open file bonus:
		if ((mask::south[square] & (b->getPieceBB(BLACK, nPawn))) == 0) {
			opening_score -= ROOK_SEMI_OPEN_FILE_EARLY_BONUS;
			endgame_score -= ROOK_SEMI_OPEN_FILE_LATE_BONUS;
			// King Threats for semi open files:
			/*
			if (mobility&king_circle[WHITE]) {
				opening_score -= SEMIOPEN_THREAT_EARLY_BONUS;// * (  (mobility&king_circle[WHITE])? 1 : 0  );
				endgame_score -= SEMIOPEN_THREAT_LATE_BONUS;// * (  (mobility&king_circle[WHITE])? 1 : 0  );
			}
			*/
			// Full-Open file bonus
			if ((mask::south[square] & (b->getPieceBB(WHITE, nPawn))) == 0) {
				opening_score -= ROOK_OPEN_FILE_EARLY_BONUS;
				endgame_score -= ROOK_OPEN_FILE_LATE_BONUS;
				// King Threats for fully open files:
				/*
				if (mobility&king_circle[WHITE]) {
					opening_score -= OPEN_THREAT_EARLY_BONUS * ((mobility&king_circle[WHITE]) ? 1 : 0);
					endgame_score -= OPEN_THREAT_LATE_BONUS  * ((mobility&king_circle[WHITE]) ? 1 : 0);
				}
				*/
			}
		}
		pieceDB ^= (((bitboard)1) << square);
	}

	
	/*******************************************************************************
	Queens:
	*******************************************************************************/
	// TODO:	-penalty for activating the queen early.

	pieceDB = b->getPieceBB(WHITE, nQueen);
	while (pieceDB) {
		phase += QUEEN_PHASE_VALUE;
		material.queens[WHITE]++;
		square = bitscan_msb(pieceDB);
		#ifdef ENABLE_QUEEN_PST
		opening_score += nSquareValue[WHITE][nQueen][square];
		endgame_score += nSquareValue[WHITE][nQueen][square];
		#endif
		// Diagonal Mobility:
		mobility = (mask::MagicRookMoves[square][((b->getOccupiedBB(BOTH) & mask::MagicRookMask[square]) * mask::MagicRookNumbers[square]) >> mask::MagicRookShift[square]] & ~b->getOccupiedBB(WHITE) );
		mobility |= (mask::MagicBishopMoves[square][((b->getOccupiedBB(BOTH)&mask::MagicBishopMask[square]) * mask::MagicBishopNumbers[square]) >> mask::MagicBishopShift[square]] & ~b->getOccupiedBB(WHITE));
		attacks[WHITE] |= mobility;

		opening_score += queen_mobility[bitcount(mobility)];
		endgame_score += queen_mobility[bitcount(mobility)];
		
		if (mobility&king_circle[BLACK]) {
			king_attackers[WHITE]++;
			attackers_weight[WHITE] += QUEEN_THREAT_WEIGHT;
		}

		pieceDB ^= (((bitboard)1) << square);
	}

	pieceDB = b->getPieceBB(BLACK, nQueen);
	while (pieceDB) {
		phase += QUEEN_PHASE_VALUE;
		material.queens[BLACK]++;
		square = bitscan_msb(pieceDB);
		#ifdef ENABLE_QUEEN_PST
		opening_score -= nSquareValue[BLACK][nQueen][square];
		endgame_score -= nSquareValue[BLACK][nQueen][square];
		#endif
		// Diagonal Mobility:
		mobility = ( mask::MagicRookMoves[square][((b->getOccupiedBB(BOTH) & mask::MagicRookMask[square]) * mask::MagicRookNumbers[square]) >> mask::MagicRookShift[square]] & ~b->getOccupiedBB(BLACK));
		mobility |= (mask::MagicBishopMoves[square][((b->getOccupiedBB(BOTH)&mask::MagicBishopMask[square]) * mask::MagicBishopNumbers[square]) >> mask::MagicBishopShift[square]] & ~b->getOccupiedBB(BLACK));
		attacks[BLACK] |= mobility;
		opening_score -= queen_mobility[bitcount(mobility)];
		endgame_score -= queen_mobility[bitcount(mobility)];
		if (mobility&king_circle[WHITE]) {
			king_attackers[BLACK]++;
			attackers_weight[BLACK] += QUEEN_THREAT_WEIGHT;
		}
		pieceDB ^= (((bitboard)1) << square);
	}

	/*******************************************************************************
	Dynamic Pawn Eval:
	*******************************************************************************/
	// -unstoppable passer bonus if the opponent doesnt have any pieces
	//		- not blocked by own piece and outside of king square.			
	//		- promotion square and pawn is supported by king
	//		when this is the case, it is pretty much worth a queen, or some fraction of a queen based on distance
	//	-check if the path is dangerous, if so, is it also defended?
	
	//WHITE:
	while (pawn_structure.passed_pawns[WHITE]) {
		unsigned char file = bitscan_msb(pawn_structure.passed_pawns[WHITE]);
		square =bitscan_msb(b->getPieceBB(WHITE,nPawn) & FileMask(file));
		//Rook support bonus:
		if (mask::south[square] & b->getPieceBB(WHITE,nRook) ) {
			opening_score += ROOK_SUPPORTING_PP_EARLY_BONUS;
			endgame_score += ROOK_SUPPORTING_PP_LATE_BONUS;
		}
		//Pawn Ending:
		if (material.no_pieces(BLACK)) {
			//check the rule of the square or if the king is supporting:
			if (!(mask::rule_of_the_square[WHITE][square] & b->getPieceBB(BLACK, nKing))
			|| ((king_circle[WHITE] & ((((bitboard)1) << square)) && (king_circle[WHITE]&RankMask(8)))) 
			){
				endgame_score += (GUARANTEED_PROMOTION_VALUE) / ((9) - RankOf(square));			
			}
			// penalize for having a piece in the path:
			bitboard blocker = (mask::north[square] & b->getOccupiedBB(WHITE));
			if (blocker)
				endgame_score += -pp_self_blocking_penalty[RankOf(bitscan_lsb(blocker)) - 2];
		}
		else {
			//Check if the path is blocked:
			bitboard blockers = mask::north[square] & (b->getOccupiedBB(BLACK));
			unsigned char nearest = bitscan_lsb(blockers);
			if (nearest == 64)
				endgame_score += pp_clear_path_bonus[7]; //full bonus
			else
				endgame_score += pp_clear_path_bonus[RankOf(nearest) - 2];
			//check if the path is threatened
			bitboard threats = mask::north[square] & (attacks[BLACK] ^ (attacks[WHITE] & attacks[BLACK]));
			nearest = bitscan_lsb(threats);
			if (nearest == 64)
				endgame_score += pp_unthreatened_path_bonus[7];
			else
				endgame_score += pp_unthreatened_path_bonus[RankOf(nearest) - 2];
			//TODO: add a bonus if our king can get to the square in front of the pawn
			//		faster than the enemy king.
		}
		pawn_structure.passed_pawns[WHITE] ^= (1 << file);
	}
	//BLACK:
	while (pawn_structure.passed_pawns[BLACK]) {
		unsigned char file = bitscan_lsb(pawn_structure.passed_pawns[BLACK]);
		pawn_structure.passed_pawns[BLACK] ^= (1 << file);
		square = bitscan_lsb(b->getPieceBB(BLACK, nPawn) & FileMask(file));
		//Rook support bonus:
		if (mask::north[square] & b->getPieceBB(BLACK, nRook)) {
			opening_score -= ROOK_SUPPORTING_PP_EARLY_BONUS;
			endgame_score -= ROOK_SUPPORTING_PP_LATE_BONUS;
		}
		//Pawn Ending:
		if (material.no_pieces(WHITE)) {
			// check the rule of the square or if the king is supporting:
			if (!(mask::rule_of_the_square[BLACK][square] & b->getPieceBB(WHITE, nKing))
			|| ((king_circle[BLACK] & (((bitboard)1) << square))&&(king_circle[BLACK] &RankMask(1))) 
			){
				endgame_score -= (GUARANTEED_PROMOTION_VALUE) / (RankOf(square));
			}
			// penalize for having a piece in the path:
			bitboard blocker = (mask::south[square] & b->getOccupiedBB(WHITE));
			if (blocker)
				endgame_score -= -pp_self_blocking_penalty[9 - RankOf((bitscan_msb(blocker))) - 2];
		}
		else {
			
			//Check if the path is blocked:
			bitboard blockers = mask::south[square] & (b->getOccupiedBB(WHITE));
			unsigned char nearest = bitscan_msb(blockers);
			if (nearest == 64)
				endgame_score -= pp_clear_path_bonus[7]; //full bonus
			else
				endgame_score -= pp_clear_path_bonus[9 - RankOf(nearest) - 2];
			
			//check if the path is threatened
			bitboard threats = mask::south[square] & (attacks[WHITE] ^ (attacks[BLACK] & attacks[WHITE]));
			nearest = bitscan_msb(threats);
			if (nearest == 64)
				endgame_score -= pp_unthreatened_path_bonus[7];
			else
				endgame_score -= pp_unthreatened_path_bonus[RankOf(nearest) - 2];
			
			//TODO: add a bonus if our king can get to the square in front of the pawn
			//		faster than the enemy king.
		}
	}
	
	/*******************************************************************************
	King Threat Score:
	*******************************************************************************/
	opening_score += ((attackers_weight[WHITE]) * threat_score[king_attackers[WHITE]]) / THREAT_RATIO;
	//endgame_score += ((attackers_weight[WHITE]) * threat_score[king_attackers[WHITE]]) / THREAT_RATIO;

	opening_score -= ((attackers_weight[BLACK]) * threat_score[king_attackers[BLACK]]) / THREAT_RATIO;
	//endgame_score -= ((attackers_weight[BLACK]) * threat_score[king_attackers[BLACK]]) / THREAT_RATIO;
	

	/*******************************************************************************
	Material Balance:
	*******************************************************************************/
	opening_score += b->getMaterialValue(WHITE) - b->getMaterialValue(BLACK);
	endgame_score += b->getMaterialValue(WHITE) - b->getMaterialValue(BLACK);
	evaluateMaterialTable(&material, opening_score, endgame_score);

	/*******************************************************************************
	Tapered Eval:
	*******************************************************************************/

	phase = TOTAL_PHASE - phase;
	phase = (phase * 256 + (TOTAL_PHASE / 2)) / TOTAL_PHASE;

	if (b->getActivePlayer() == BLACK)
		return -((opening_score*(256 - phase)) + (endgame_score*phase)) / 256;
	else
		return ((opening_score*(256 - phase)) + (endgame_score*phase)) / 256;

}

#ifdef UNUSED_CODE
void PawnEval_loop(CHESSBOARD *b, short &opening_score, short &endgame_score) {
	// This score is from the POV of white. 
	// That way we dont have to double count positions in the pawn hash table
		
	//	TODO:	-isolated (on open/closed files) 
	//			-backward (open/closed)
	//			-candidate passed pawn
	//			-unstoppable pawn bonus = queen - pawn (only in king/pawn endings?)


	//Keep local versions of score changes, so the hashing will be accurate:
	short pawn_opening_score[2] = { 0, 0 };
	short pawn_endgame_score[2] = { 0, 0 };
	
	#ifdef ENABLE_PAWN_HASH
	//Check the pawn hash table:
	if(pawntable.getScores(b->getPawnKey(),pawn_opening_score[WHITE],pawn_endgame_score[WHITE])) {
		opening_score += pawn_opening_score[WHITE];
		endgame_score += pawn_endgame_score[WHITE];
		return;
	}
	#endif

	//There is no hash hit, so continue with the evaluation:
	
	for (bool color : {WHITE, BLACK}) {

		bitboard pieceDB = b->getPieceBB(color, nPawn);
		while (pieceDB) {
			int square = bitscan_msb(pieceDB);
			//Square value:
			//score += nSquareValue[toMove][nPawn][square];
			pawn_opening_score[color] += nSquareValue[color][nPawn][square];
			pawn_endgame_score[color] += nEndgamePawnValue[color][square];

			//Passed pawn bonus:
			if (b->getPieceBB(!color, nPawn) & mask::passed_pawn[color][square]) {
				//score+=PASSED_PAWN_VALUE;
				//pawn_opening_score[color] += PASSED_PAWN_EARLY_VALUE;
				//pawn_endgame_score[color] += PASSED_PAWN_LATE_VALUE;
				pawn_opening_score[color] += passed_pawn_pst[color][square];
				pawn_endgame_score[color] += passed_pawn_pst[color][square];
				//pawn_endgame_score[color] += passed_pawn_pst_endgame[color][square];
			}
			//Isolated pawn penalty:
			if (b->getPieceBB(color, nPawn) & mask::isolated_pawn[square]) {
				pawn_opening_score[color] += -ISOLATED_PAWN_EARLY_PENALTY;
				pawn_endgame_score[color] += -ISOLATED_PAWN_LATE_PENALTY;
			}
			//Doubled pawn penalty:
			if (b->getPieceBB(color, nPawn) & (color?mask::south[square]:mask::north[square])) {
				pawn_opening_score[color] += -DOUBLED_PAWN_EARLY_PENALTY;
				pawn_endgame_score[color] += -DOUBLED_PAWN_LATE_PENALTY;
			}
			//Turn off the bit:
			pieceDB ^= (((bitboard)1) << square);
		}

	}
	
	// Roll what we have up into the white scores and just use the white scores from now on:
	pawn_opening_score[WHITE] -= pawn_opening_score[BLACK];
	pawn_endgame_score[WHITE] -= pawn_endgame_score[BLACK];


	#ifdef ENABLE_PAWN_HASH
	pawntable.saveScore(b->getPawnKey(), pawn_opening_score[WHITE], pawn_endgame_score[WHITE]);
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
	opening_score += pawn_opening_score[WHITE];
	endgame_score += pawn_endgame_score[WHITE];
}
#endif
#ifdef UNUSED_CODE
int evaluate_loop(CHESSBOARD * b, int alpha, int beta) {
//Returns game evaluation with respect to the player to move.
	
	short endgame_score[2] = { 0, 0 };
	short opening_score[2] = { 0, 0 };

//Material score:
	opening_score[WHITE] = b->getMaterialValue(WHITE);
	endgame_score[WHITE] = b->getMaterialValue(WHITE);
	opening_score[BLACK] = b->getMaterialValue(BLACK);
	endgame_score[BLACK] = b->getMaterialValue(BLACK);


	unsigned char square;
	/*******************************************************************************
		Pawns:
	*******************************************************************************/

	// Notice that it does not matter if we subtract black's score in Pawn Eval
	// or at the end of the evaluation. So we can save passing more.
	PawnEval(b, opening_score[WHITE], endgame_score[WHITE]);

	/*******************************************************************************
		Kings:
	*******************************************************************************/
	bitboard king_circle[2];
	for (bool color : {WHITE, BLACK}) {

		square = bitscan_msb(b->getPieceBB((bool)color, nKing));

		opening_score[color] += nSquareValue[color][nKing][square];
		endgame_score[color] += nEndgameKingValue[color][square];

		#ifndef ENABLE_SIMPLE_EVAL
		king_circle[color] = mask::king_moves[square];
		
		//Temporary Pawn Fortress bonus:
		if ((((bitboard)1) << square) & (color == WHITE ? 23((bitboard)1) : 16645304222761353216i64)) {
			opening_score[color] += SHELTER_BONUS		 *	bitcount(king_circle[color] & b->getPieceBB((bool)color, nPawn));
			opening_score[color] += PIECE_SHELTER_BONUS  *	bitcount(king_circle[color] & b->getOccupiedBB(color));
		}
		#endif
	}

	bitboard pieceDB;
	bitboard mobility;
	for (bool color : {WHITE, BLACK} ) {

		/*******************************************************************************
			Knights:
			*******************************************************************************/




		pieceDB = b->getPieceBB((bool)color, nKnight);

		while (pieceDB) {
			square = bitscan_msb(pieceDB);
			opening_score[color] += nSquareValue[color][nKnight][square];
			endgame_score[color] += nSquareValue[color][nKnight][square];

			// Mobility:
			mobility = mask::knight_moves[square];

			opening_score[color] += KNIGHT_MOBILITY_EARLY_BONUS * bitcount(mobility & ~b->getOccupiedBB(color));
			endgame_score[color] += KNIGHT_MOBILITY_LATE_BONUS * bitcount(mobility & ~b->getOccupiedBB(color));

			// King Threats:
			if (mobility & king_circle[!color]) {
				opening_score[color] += KNIGHT_KING_THREAT_EARLY_BONUS;
				endgame_score[color] += KNIGHT_KING_THREAT_LATE_BONUS;
			}

			//Reduce value in late game:
			endgame_score[color] += -KNIGHT_LATE_PENALTY;

			pieceDB ^= (((bitboard)1) << square);
		}


		/*******************************************************************************
			Bishops:
			*******************************************************************************/
		//	TODO:	-consider xray
		//			-Penalize "bad" bishops.

		pieceDB = b->getPieceBB((bool)color, nBishop);
		if (bitcount(pieceDB) > 1) {
			//Bishop Pair bonus:
			opening_score[color] += BISHOP_PAIR_EARLY_BONUS;
			endgame_score[color] += BISHOP_PAIR_LATE_BONUS;
		}

		while (pieceDB) {
			square = bitscan_msb(pieceDB);
			opening_score[color] += nSquareValue[color][nBishop][square];
			endgame_score[color] += nSquareValue[color][nBishop][square];

			//Mobility:
			mobility = mask::MagicBishopMoves[square][((b->getOccupiedBB(BOTH)&mask::MagicBishopMask[square]) * mask::MagicBishopNumbers[square]) >> mask::MagicBishopShift[square]];
			opening_score[color] += BISHOP_MOBILITY_EARLY_BONUS * bitcount(mobility & ~b->getOccupiedBB(color));
			endgame_score[color] += BISHOP_MOBILITY_LATE_BONUS  * bitcount(mobility & ~b->getOccupiedBB(color));

			// King Threats:
			if (mobility&king_circle[!color]) {
				opening_score[color] += BISHOP_KING_THREAT_EARLY_BONUS;// * (  (mobility&king_circle[!color])? 1 : 0  );//bitcount(mobility&king_circle[!color]); //(  (mobility&king_circle[!color])? 1 : 0  );
				endgame_score[color] += BISHOP_KING_THREAT_LATE_BONUS;// * (  (mobility&king_circle[!color])? 1 : 0  );//bitcount(mobility&king_circle[!color]); //(  (mobility&king_circle[!color])? 1 : 0  );
			}

			pieceDB ^= (((bitboard)1) << square);
		}

		/*******************************************************************************
			Rooks:
			*******************************************************************************/
		//	TODO:	-make an open file bonus that is a function of number of pawns.
		//			-bonus for rooks not moving until a castle in early game.
		//			-seventh rank bonus
		//			-award semi/open file bonus for passed pawns also

		pieceDB = b->getPieceBB((bool)color, nRook);
		while (pieceDB) {
			square = bitscan_msb(pieceDB);

			// Mobility:
			mobility = mask::MagicRookMoves[square][((b->getOccupiedBB(BOTH) & mask::MagicRookMask[square]) * mask::MagicRookNumbers[square]) >> mask::MagicRookShift[square]];
			opening_score[color] += ROOK_MOBILITY_EARLY_BONUS * bitcount(mobility & ~b->getOccupiedBB(color));
			endgame_score[color] += ROOK_MOBILITY_LATE_BONUS * bitcount(mobility & ~b->getOccupiedBB(color));

			// King Threats:
			if (mobility&king_circle[!color]) {
				opening_score[color] += ROOK_KING_THREAT_EARLY_BONUS;
				endgame_score[color] += ROOK_KING_THREAT_LATE_BONUS;
			}

			// Semi-Open file bonus:
			if (((color == WHITE ? mask::north[square] : mask::south[square]) & (b->getPieceBB((bool)color, nPawn))) == 0) {
				opening_score[color] += ROOK_SEMI_OPEN_FILE_EARLY_BONUS;
				endgame_score[color] += ROOK_SEMI_OPEN_FILE_LATE_BONUS;
				// King Threats for semi open files:
				if (mobility&king_circle[!color]) {
					opening_score[color] += SEMIOPEN_THREAT_EARLY_BONUS;
					endgame_score[color] += SEMIOPEN_THREAT_LATE_BONUS;
				}
				// Full-Open file bonus
				if (((color == WHITE ? mask::north[square] : mask::south[square]) & (b->getPieceBB(!color, nPawn))) == 0) { //(FileMaskSq(square)
					opening_score[color] += ROOK_OPEN_FILE_EARLY_BONUS;
					endgame_score[color] += ROOK_OPEN_FILE_LATE_BONUS;
					// King Threats for fully open files:
					if (mobility&king_circle[!color]) {
						opening_score[color] += OPEN_THREAT_EARLY_BONUS;// * (  (mobility&king_circle[!color])? 1 : 0  );
						endgame_score[color] += OPEN_THREAT_LATE_BONUS;//  * (  (mobility&king_circle[!color])? 1 : 0  );
					}
				}
			}
			pieceDB ^= (((bitboard)1) << square);
		}

		/*******************************************************************************
			Queens:
			*******************************************************************************/
		// TODO:	-bonus for not activating the queen early.


		//Additional mobility bonus for queens:


		pieceDB = b->getPieceBB((bool)color, nQueen);
		while (pieceDB) {
			square = bitscan_msb(pieceDB);

#ifdef ENABLE_QUEEN_PST
			opening_score[color] += nSquareValue[color][nQueen][square];
			endgame_score[color] += nSquareValue[color][nQueen][square];
#endif

			// Straight Mobility:
			mobility = (mask::MagicRookMoves[square][((b->getOccupiedBB(BOTH) & mask::MagicRookMask[square]) * mask::MagicRookNumbers[square]) >> mask::MagicRookShift[square]]);
			opening_score[color] += QUEEN_MOBILITY_EARLY_BONUS * bitcount(mobility& ~b->getOccupiedBB(color));
			endgame_score[color] += QUEEN_MOBILITY_LATE_BONUS * bitcount(mobility& ~b->getOccupiedBB(color));
			// Straight King Threats:
			if (mobility & king_circle[!color]) {
				opening_score[color] += QUEEN_KING_THREAT_EARLY_BONUS;// * (  (mobility&king_circle[!color])? 1 : 0  );//bitcount(mobility&king_circle[!color]); //(  (mobility&king_circle[!color])? 1 : 0  );
				endgame_score[color] += QUEEN_KING_THREAT_LATE_BONUS; //* (  (mobility&king_circle[!color])? 1 : 0  );//bitcount(mobility&king_circle[!color]); //(  (mobility&king_circle[!color])? 1 : 0  );
			}
			// Diagonal Mobility
			mobility = (mask::MagicBishopMoves[square][((b->getOccupiedBB(BOTH)&mask::MagicBishopMask[square]) * mask::MagicBishopNumbers[square]) >> mask::MagicBishopShift[square]]);
			opening_score[color] += QUEEN_MOBILITY_EARLY_BONUS* bitcount(mobility& ~b->getOccupiedBB(color));
			endgame_score[color] += QUEEN_MOBILITY_LATE_BONUS * bitcount(mobility& ~b->getOccupiedBB(color));
			// Diagonal King Threats
			if (mobility & king_circle[!color]) {
				opening_score[color] += QUEEN_KING_THREAT_EARLY_BONUS;// * (  (mobility&king_circle[!color])? 1 : 0  );//bitcount(mobility&king_circle[!color]); //(  (mobility&king_circle[!color])? 1 : 0  );
				endgame_score[color] += QUEEN_KING_THREAT_LATE_BONUS;//  * (  (mobility&king_circle[!color])? 1 : 0  );//bitcount(mobility&king_circle[!color]); //(  (mobility&king_circle[!color])? 1 : 0  );
			}
			pieceDB ^= (((bitboard)1) << square);
		}


	}
	/*******************************************************************************
		Tapered Eval:
	*******************************************************************************/
	//13955 - 9900 = 4055
	//Endgame is at 1300 points. 4055 - 1300 = 2755
	// m-9900-1300. when m=0 it is endgame. when m=2755 it is opening
	// opening(m) +ending(2775 - m)
	
	short gauge = phase_gauge(b);
	
	// Roll up the scores into white's scores:
	opening_score[WHITE] -= opening_score[BLACK];
	endgame_score[WHITE] -= endgame_score[BLACK];
	
	if (b->getActivePlayer() == BLACK)
		return -interpolate(opening_score[WHITE], endgame_score[WHITE], gauge);
	else
		return interpolate(opening_score[WHITE], endgame_score[WHITE], gauge);
	
}
#endif


inline short interpolate(short a, short b, short t) {
	int resolution = 100000;
	return ((a*(((resolution * t) / 2775)) + (b*((resolution)-((resolution * t) / 2775))))) / resolution;
}

inline short phase_gauge(CHESSBOARD * b) {
	short gauge = ((b->getMaterialValue(WHITE) > b->getMaterialValue(BLACK)) ?
		b->getMaterialValue(WHITE) : b->getMaterialValue(BLACK))
		- KING_VALUE; //This will range from 0 to 4055
	gauge = (gauge < 1300) ? 0 : gauge - 1300; //This will range from 0 to 2755
	return gauge;
}