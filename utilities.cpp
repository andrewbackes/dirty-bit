#include <vector>
#include <string>
#include <sstream>

#include "bitboards.h"
#include "board.h"
#include "movegen.h"
#include "evaluate.h"
#include "utilities.h"

using namespace std;

void PERFT::clear() {
	captures = 0;
	enpassants = 0;
	castles = 0;
	promotions = 0;
	checks = 0;
	checkmates = 0;
	nodes = 0;
}
void PERFT::go(CHESSBOARD * game, int depth) {
		clear();
		Perft(game, depth);
		cout << 
			"Nodes:\t\t" << nodes << endl <<
			"Captures:\t" << captures << endl <<
			"E.p:\t\t" << enpassants << endl <<
			"Castles:\t" << castles << endl <<
			"Promotions:\t" << promotions << endl <<
			"Checks:\t\t" << checks << endl <<
			"Checkmates:\t" << checkmates << endl;
	}

void PERFT::Divide(CHESSBOARD * game, int depth) {
	
	bitboard total = 0;
	int moves = 0;
	vector<MOVE> move_list = CaptureGen(game);
	for(int i =0; i < move_list.size(); i++) {
		game->MakeMove(move_list[i]);
		if(!game->isInCheck(!game->getActivePlayer())) {
			move_list[i].print();
			cout << " ";
			clear();
			Perft(game,depth-1);
			cout << nodes << endl;
			total+=nodes;
			moves++;
		}
		game->unMakeMove(move_list[i]);
	}

	move_list = nonCaptureGen(game);
	for(int i =0; i < move_list.size(); i++) {
		game->MakeMove(move_list[i]);
		if(!game->isInCheck(!game->getActivePlayer())) {
			move_list[i].print();
			cout << " ";
			clear();
			Perft(game,depth-1);
			cout << nodes << endl;
			total+=nodes;
			moves++;
		}
		game->unMakeMove(move_list[i]);
	}
	cout << "\nMoves:\t" << moves << endl;
	cout << "Nodes:\t" << total << endl;
}

void PERFT::Perft(CHESSBOARD * game, int depth) {

	if (depth == 0) {
		nodes++;	
		return;
	}
	bool fMate = true;

	vector<MOVE> move_list = CaptureGen(game);
	for(int i =0; i < move_list.size(); i++) {
		game->MakeMove(move_list[i]);
		if(!game->isInCheck(!game->getActivePlayer())) {
			fMate = false;
			if(move_list[i].promote_piece_to !=0) promotions++;
			if(move_list[i].captured_piece != 0) captures++;
			if(game->isInCheck()) checks++;
			if(move_list[i].active_piece_id == nPawn 
			&& move_list[i].captured_piece == 0
			&& move_list[i].promote_piece_to == 0
			&& move_list[i].static_value >= 50) {
				enpassants++; }

			Perft(game, depth -1);
		}
		game->unMakeMove(move_list[i]);
	}
	
	move_list = nonCaptureGen(game);
	for(int i =0; i < move_list.size(); i++) {
		game->MakeMove(move_list[i]);
		if(!game->isInCheck(!game->getActivePlayer())) {
			fMate = false;
			if(game->isInCheck()) checks++;
			Perft(game, depth -1);
		}
		game->unMakeMove(move_list[i]);
	}
	if(fMate) checkmates++;
	
}

bitboard perft(CHESSBOARD * game, int depth) {
	bitboard nodes = 0;
	if (depth == 0) {	
		return 1;
	}

	vector<MOVE> move_list = CaptureGen(game);
	for(int i =0; i < move_list.size(); i++) {
		game->MakeMove(move_list[i]);
		if(!game->isInCheck(!game->getActivePlayer())) {
			nodes += perft(game, depth -1);
		}
		game->unMakeMove(move_list[i]);
	}
	
	move_list = nonCaptureGen(game);
	for(int i =0; i < move_list.size(); i++) {
		game->MakeMove(move_list[i]);
		if(!game->isInCheck(!game->getActivePlayer())) {
			nodes+=perft(game, depth -1);
		}
		game->unMakeMove(move_list[i]);
	}
	
	return nodes;
}

string index_to_alg(int index) {
	char file =	97 + (7-(index%8));
	char rank = (index / 8) + 49;
	string s = "";
	s += file;
	s += rank;
	return(s);
}

int alg_to_index(string alg) {
	char file = alg.at(0);
	char rank = alg.at(1);
	int index = ((rank-48) * 8) - (file-96);
	return index;
}

int str_to_int(string str) {

	int numb;
	istringstream ( str ) >> numb;

	return numb;
}

/*
void backupMakeMove(MOVE m) {
	//	m.from	- index of piece location.
	//	m.to	- index of target square.

	// TODO:	-change enpassant m.to only 1 variariable instead of 2. 
	//			-does capture work correctly for enpassant?
	//			-pawn promotion!!!!
	//			-optimise the way castling and enpassant is done.
	//
	// --------->Put a counter for each condition in the if/else chain. order accordingly.
	//
	// Q: is ~ or ! faster?
	// Q: could bitboards in place of m.from/m.to be faster?

	//Move count:
	if(active_player == WHITE)
		full_moves++;
	//50 half move rule:
	half_moves++;

	//reset en passant possibility:
	short old_en_passant_index = en_passant_index;
	if(en_passant_index != 64 ) {
		//only XOR the zobrist key if it actually has a value.
		zobrist_key ^= zobrist::enPassant[en_passant_index];
		en_passant_index = 64;
	}

	//Adjust the mailbox board representation:
	board[m.to] = m.active_piece_id;
	board[m.from] = 0;

	//Adjust the bitboard:
	bitboard mask = ((i << m.to) | (i << m.from));
	pieceBB[active_player][m.active_piece_id] ^= mask;
	occupiedBB[active_player] ^= mask;
		
	//Update Zobrist Key:
	zobrist_key ^= zobrist::pieces[active_player][m.active_piece_id][m.from];
	zobrist_key ^= zobrist::pieces[active_player][m.active_piece_id][m.to];
	
	if (m.captured_piece != 0) {
		//adjust BBs for a capture occuring.
		pieceBB[!active_player][m.captured_piece] ^= (i << m.to);
		occupiedBB[!active_player] ^= (i << m.to);
		//Remove the captured piece from the Zobrist Key:
		zobrist_key ^= zobrist::pieces[!active_player][m.captured_piece][m.to];

		//Adjust material score:
		if(active_player == WHITE)
			material_score += nPieceValue[m.captured_piece];
		else
			material_score -= nPieceValue[m.captured_piece];

		//Reset the 50 half move rule:
		half_moves = 0;
		//clearHistory();
	}
	if(m.active_piece_id == nPawn) {
		//set enPassant if needed.
		if( ((i << m.from) & mask::pawns_spawn[active_player]) 
					&& ((i << m.to) & mask::pawn_double_jump[active_player]) ) {
			
			en_passant_index = bitscan_lsb(mask::pawn_enPassant[active_player][m.from]);
			zobrist_key ^= zobrist::enPassant[en_passant_index];
		}
		//check for an enPassant capture:
		else if (m.to == old_en_passant_index) {
			pieceBB[!active_player][nPawn] ^= mask::pawn_advances[!active_player][m.to];
			occupiedBB[!active_player] ^= mask::pawn_advances[!active_player][m.to];
			int removedPieceIndex = bitscan_lsb(mask::pawn_advances[!active_player][m.to]);
			board[removedPieceIndex] = 0;
			zobrist_key ^= zobrist::pieces[!active_player][nPawn][removedPieceIndex];
		}
		//Check for pawn promotion:
		else if (m.promote_piece_to != 0 ) {
			board[m.to] = m.promote_piece_to;
			pieceBB[active_player][nPawn] ^= (i << m.to);
			pieceBB[active_player][m.promote_piece_to] ^= (i << m.to);
			zobrist_key ^= zobrist::pieces[active_player][nPawn][m.to];
			zobrist_key ^= zobrist::pieces[active_player][m.promote_piece_to][m.to];

			//Adjust material score:
			if(active_player == WHITE)
				material_score += nPieceValue[m.promote_piece_to] - nPieceValue[nPawn];
			else
				material_score -= nPieceValue[m.promote_piece_to] + nPieceValue[nPawn];

		}
		//Reset the 50 half move rule:
		half_moves = 0;
		//clearHistory();
	}
	else if(m.active_piece_id == nRook) {
		//Cancel some castling posibilities:
		if(~occupiedBB[active_player] & mask::kRook_spawn[active_player]) {
			if(kingCastle[active_player] == true) { //this check is m.to prevent un-needed application of XOR'ing the key
				kingCastle[active_player] = false;
				zobrist_key ^= zobrist::castling[active_player][KING_SIDE];
			}
		}
		if(~occupiedBB[active_player] & mask::qRook_spawn[active_player]) {
			if(queenCastle[active_player] == true) { //this check is m.to prevent un-needed application of XOR'ing the key
				queenCastle[active_player] = false;
				zobrist_key ^= zobrist::castling[active_player][QUEEN_SIDE];
			}
		}
	}
	else if(m.active_piece_id == nKing) {
		//Cancel castling posibilities:
		if(kingCastle[active_player] == true ) { //this check is to prevent un-needed application of XOR'ing the key
			kingCastle[active_player] = false;
			zobrist_key ^= zobrist::castling[active_player][KING_SIDE];
		}
		if(queenCastle[active_player] == true ) { //this check is to prevent un-needed application of XOR'ing the key
			queenCastle[active_player] = false;
			zobrist_key ^= zobrist::castling[active_player][QUEEN_SIDE];
		}
		//Was this a Castle?	
		if(mask::king_spawn[active_player] & (i << m.from)) {
			if((mask::kCastle[active_player] & (i << m.to)) ) {
				//king side castle! So move the rook.
				bitboard mask = ( mask::kBishop_spawn[active_player] | mask::kRook_spawn[active_player] );
				board[bitscan_lsb(mask::kBishop_spawn[active_player])] = nRook;
				board[bitscan_lsb(mask::kRook_spawn[active_player])] = 0;
				pieceBB[active_player][nRook] ^= mask;
				occupiedBB[active_player] ^= mask;
				//update key:
				zobrist_key ^= zobrist::pieces[active_player][nRook][bitscan_lsb(mask)];
				zobrist_key ^= zobrist::pieces[active_player][nRook][bitscan_msb(mask)];
			}
			else if(mask::qCastle[active_player] & (i << m.to)) {
				//queen side castle! So move the rook.
				bitboard mask = ( mask::queen_spawn[active_player] | mask::qRook_spawn[active_player]  );
				board[bitscan_lsb(mask::queen_spawn[active_player])] = nRook;
				board[bitscan_lsb(mask::qRook_spawn[active_player])] = 0;
				pieceBB[active_player][nRook] ^= mask;
				occupiedBB[active_player] ^= mask;
				//update key:
				zobrist_key ^= zobrist::pieces[active_player][nRook][bitscan_lsb(mask)];
				zobrist_key ^= zobrist::pieces[active_player][nRook][bitscan_msb(mask)];
			}
		}
	}

	occupiedBB[2] = occupiedBB[0] | occupiedBB[1]; //done separately for the sake of castling.
	
	toggleActivePlayer();
	
	//For 3 fold repetition:
	//addHistory(zobrist_key);
}

*/