#include <iostream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <ctype.h>

#include "bitboards.h"
#include "board.h"
#include "hash.h"
#include "move.h"
#include "evaluate.h"
#include "utilities.h"

using namespace std;

void CHESSBOARD::MakeMove(MOVE m) {
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
	current_ply++;

	gamestate[current_ply] = gamestate[current_ply-1];
	gamestate[current_ply].half_move_rule++;

	//Handle en passant:
	gamestate[current_ply].en_passant_index = 64;
	if(gamestate[current_ply - 1].en_passant_index != 64)
		gamestate[current_ply].zobrist_key ^= zobrist::enPassant[gamestate[current_ply - 1].en_passant_index];

	//Adjust the mailbox board representation:
	board[m.to] = m.active_piece_id;
	board[m.from] = 0;

	//Adjust the bitboard:
	bitboard mask = ((1i64 << m.to) | (1i64 << m.from));
	pieceBB[active_player][m.active_piece_id] ^= mask;
	occupiedBB[active_player] ^= mask;
	
	//Update Zobrist Key:
	gamestate[current_ply].zobrist_key ^= zobrist::pieces[active_player][m.active_piece_id][m.from];
	gamestate[current_ply].zobrist_key ^= zobrist::pieces[active_player][m.active_piece_id][m.to];
	
	//Adjust the incremental position score:
	//position_score[active_player] += nSquareValue[active_player][m.active_piece_id][m.to] - nSquareValue[active_player][m.active_piece_id][m.from];

	if (m.captured_piece != 0) {
		//adjust BBs for a capture occuring.
		pieceBB[!active_player][m.captured_piece] ^= (1i64 << m.to);
		occupiedBB[!active_player] ^= (1i64 << m.to);
		
		//Remove the captured piece from the Zobrist Key:
		gamestate[current_ply].zobrist_key ^= zobrist::pieces[!active_player][m.captured_piece][m.to];
		//Remove a captured pawn from the pawn key:
		if(m.captured_piece == nPawn)
			gamestate[current_ply].pawn_key ^= (phash_lock)zobrist::pieces[!active_player][nPawn][m.to];

		//Adjust material score:
		material_score[!active_player] -= nPieceValue[m.captured_piece];
		//Adjust position score:
		//position_score[!active_player] -= nSquareValue[!active_player][m.captured_piece][m.to];

		//Reset the 50 half move rule:
		gamestate[current_ply].half_move_rule = 0;
	}
	if(m.active_piece_id == nPawn) {
		//Adjust the pawn key:
		gamestate[current_ply].pawn_key ^= (phash_lock)zobrist::pieces[active_player][nPawn][m.to];
		gamestate[current_ply].pawn_key ^= (phash_lock)zobrist::pieces[active_player][nPawn][m.from];
		
		//set enPassant if needed.
		if( ((1i64 << m.from) & mask::pawns_spawn[active_player]) 
					&& ((1i64 << m.to) & mask::pawn_double_jump[active_player]) ) {
			
			gamestate[current_ply].en_passant_index = bitscan_msb(mask::pawn_enPassant[active_player][m.from]);
			gamestate[current_ply].zobrist_key ^= zobrist::enPassant[gamestate[current_ply].en_passant_index];
		}
		//check for an enPassant capture:
		else if (m.to == gamestate[current_ply-1].en_passant_index) {
			pieceBB[!active_player][nPawn] ^= mask::pawn_advances[!active_player][m.to];
			occupiedBB[!active_player] ^= mask::pawn_advances[!active_player][m.to];
			unsigned char removedPieceIndex = bitscan_msb(mask::pawn_advances[!active_player][m.to]);
			board[removedPieceIndex] = 0;
			gamestate[current_ply].zobrist_key ^= zobrist::pieces[!active_player][nPawn][removedPieceIndex];
			//Adjust pawn key:
			gamestate[current_ply].pawn_key ^= (phash_lock)zobrist::pieces[!active_player][nPawn][removedPieceIndex];

			//Adjust Material score:
			material_score[!active_player] -= nPieceValue[nPawn];
			//Adjust position score:
			//position_score[!active_player] -= nSquareValue[!active_player][nPawn][removedPieceIndex];
		}
		//Check for pawn promotion:
		else if (m.promote_piece_to != 0 ) {
			board[m.to] = m.promote_piece_to;
			pieceBB[active_player][nPawn] ^= (1i64 << m.to);
			pieceBB[active_player][m.promote_piece_to] ^= (1i64 << m.to);
			gamestate[current_ply].zobrist_key ^= zobrist::pieces[active_player][nPawn][m.to];
			gamestate[current_ply].zobrist_key ^= zobrist::pieces[active_player][m.promote_piece_to][m.to];
			//Remove the promoted pawn from the pawn key:
			gamestate[current_ply].pawn_key ^= (phash_lock)zobrist::pieces[active_player][nPawn][m.to];
			//Adjust material score:
			material_score[active_player] += (nPieceValue[m.promote_piece_to] - nPieceValue[nPawn]);
			//Adjust position score:
			//position_score[active_player] -= nSquareValue[active_player][nPawn][m.to];
			//position_score[active_player] += nSquareValue[active_player][m.promote_piece_to][m.to];
			
		}
		//Reset the 50 half move rule:
		gamestate[current_ply].half_move_rule = 0;
	}
	else if(m.active_piece_id == nRook) {
		//Cancel some castling posibilities:
		if(~occupiedBB[active_player] & mask::kRook_spawn[active_player]) {
			if(gamestate[current_ply-1].castling_rights[active_player][KING_SIDE] == true) { //this check is to prevent un-needed application of XOR'ing the key
				gamestate[current_ply].castling_rights[active_player][KING_SIDE] = false;
				gamestate[current_ply].zobrist_key ^= zobrist::castling[active_player][KING_SIDE];
			}
		}
		if(~occupiedBB[active_player] & mask::qRook_spawn[active_player]) {
			if(gamestate[current_ply -1].castling_rights[active_player][QUEEN_SIDE] == true) { //this check is to prevent un-needed application of XOR'ing the key
				gamestate[current_ply].castling_rights[active_player][QUEEN_SIDE] = false;
				gamestate[current_ply].zobrist_key ^= zobrist::castling[active_player][QUEEN_SIDE];
			}
		}
	}
	else if(m.active_piece_id == nKing) {
		//Cancel castling posibilities:
		if(gamestate[current_ply-1].castling_rights[active_player][KING_SIDE] == true) { //this check is to prevent un-needed application of XOR'ing the key
			gamestate[current_ply].castling_rights[active_player][KING_SIDE] = false;
			gamestate[current_ply].zobrist_key ^= zobrist::castling[active_player][KING_SIDE];
		}
		if(gamestate[current_ply -1].castling_rights[active_player][QUEEN_SIDE] == true) { //this check is to prevent un-needed application of XOR'ing the key
			gamestate[current_ply].castling_rights[active_player][QUEEN_SIDE] = false;
			gamestate[current_ply].zobrist_key ^= zobrist::castling[active_player][QUEEN_SIDE];
		}
		//Was this a Castle?	
		if(mask::king_spawn[active_player] & (1i64 << m.from)) {
			if((mask::kCastle[active_player] & (1i64 << m.to)) ) {
				//king side castle! So move the rook.
				bitboard mask = ( mask::kBishop_spawn[active_player] | mask::kRook_spawn[active_player] );
				board[bitscan_msb(mask::kBishop_spawn[active_player])] = nRook;
				board[bitscan_msb(mask::kRook_spawn[active_player])] = 0;
				pieceBB[active_player][nRook] ^= mask;
				occupiedBB[active_player] ^= mask;
				//update key:
				gamestate[current_ply].zobrist_key ^= zobrist::pieces[active_player][nRook][bitscan_lsb(mask)];
				gamestate[current_ply].zobrist_key ^= zobrist::pieces[active_player][nRook][bitscan_msb(mask)];
			}
			else if(mask::qCastle[active_player] & (1i64 << m.to)) {
				//queen side castle! So move the rook.
				bitboard mask = ( mask::queen_spawn[active_player] | mask::qRook_spawn[active_player]  );
				board[bitscan_msb(mask::queen_spawn[active_player])] = nRook;
				board[bitscan_msb(mask::qRook_spawn[active_player])] = 0;
				pieceBB[active_player][nRook] ^= mask;
				occupiedBB[active_player] ^= mask;
				//update key:
				gamestate[current_ply].zobrist_key ^= zobrist::pieces[active_player][nRook][bitscan_lsb(mask)];
				gamestate[current_ply].zobrist_key ^= zobrist::pieces[active_player][nRook][bitscan_msb(mask)];
			}
		}
	}

	occupiedBB[2] = occupiedBB[0] | occupiedBB[1]; //done separately for the sake of castling.
	
	/*
	//DEBUG:
	if(getPawnKey() != generatePawnKey()) {
		cout << "MAKEMOVE: PAWN KEYS DONT MATCH " << endl;
		cout << "Pawn BBs:" << endl;
		bitprint(getPieceBB(WHITE,nPawn));
		bitprint(getPieceBB(BLACK,nPawn));
		system("PAUSE"); 
	}
	*/
	/*
	//Debugging:
	if(materialScore(this) != material_score[WHITE] - material_score[BLACK]) {
		cout << "*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~" << endl;
		cout << "*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~" << endl;
		cout << "*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~" << endl;
		cout << "*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~" << endl;
		cout << "            MATERIAL SCORES DONT MATCH!!!"  << endl;
		cout << "*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~" << endl;
		cout << "*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~" << endl;
		cout << "*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~" << endl;
		cout << "*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~" << endl;
	}
	*/
	/*
	#ifdef DEBUG_ZOBRIST
	if(generateKey() != gamestate[current_ply].zobrist_key) {
		print();
		print_bb();
		print_mailbox();
		
		cout << "Material is suppose to be: " << materialScore(this) << ". But is: " << material_score[WHITE] - material_score[BLACK] << endl;

		//for(int i=0; i < move_history.size(); i++) {
		cout << "keys dont match. makemove: ";
		m.print(true);
		//	cout << endl;
		//}
		

		cout << "KEYS DONT MATCH!" << endl;
		system("PAUSE");
	}
	#endif
	*/
	/*
	for(int i =0; i < 64; i++) {
		if( !(pieceBB[active_player][board[i]] & mask::square[i]) && !(pieceBB[!active_player][board[i]] & mask::square[i]) ) {
			cout << "in makemove()\n";
			m.print(true);
			print_bb();
			print_mailbox();
		}
	}
	*/

	toggleActivePlayer();

}

void CHESSBOARD::unMakeMove(MOVE m) {

	//Change active player without adjusting the zobrist key:
	active_player = !active_player;
	
	//Move count:
	current_ply--;
	
	//Mailbox representation update:
	board[m.from] = m.active_piece_id;
	board[m.to] = m.captured_piece; //0 for no capture anyways.

	//Adjust the bitboard:
	bitboard mask = ((1i64 << m.to) | (1i64 << m.from));
	pieceBB[active_player][m.active_piece_id] ^= mask;
	occupiedBB[active_player] ^= mask;
	
	//Adjust the position score:
	//position_score[active_player] += nSquareValue[active_player][m.active_piece_id][m.from] - nSquareValue[active_player][m.active_piece_id][m.to];

	//Undo en passant capture:
	if( (m.active_piece_id == nPawn) && (m.to == gamestate[current_ply].en_passant_index) ) {
		if(active_player == WHITE) {
			//this means a black pawn was captured.
			pieceBB[BLACK][nPawn] ^= mask::pawn_advances[BLACK][m.to];
			occupiedBB[BLACK] ^= mask::pawn_advances[BLACK][m.to];
			board[m.to - 8] = nPawn;
		}
		else {
			//this means a black pawn was captured.
			pieceBB[WHITE][nPawn] ^= mask::pawn_advances[WHITE][m.to];
			occupiedBB[WHITE] ^= mask::pawn_advances[WHITE][m.to];
			board[m.to + 8] = nPawn;
		}
		//Adjust material score:
		material_score[!active_player] += nPieceValue[nPawn];
		
	}
	//Undo a capture:
	else if(m.captured_piece != 0) {
		//Only need to adjust the bitboards:
		pieceBB[!active_player][m.captured_piece] ^= (1i64 << m.to);
		occupiedBB[!active_player] ^= (1i64 << m.to);

		//Adjust material score:
		material_score[!active_player] += nPieceValue[m.captured_piece];
	}
	//Undo Pawn promotion:
	if(m.promote_piece_to != 0) {
		//Only need to adjust the bitboards:
		pieceBB[active_player][nPawn] ^= (1i64 << m.to); //undo the mask done above.
		pieceBB[active_player][m.promote_piece_to] ^= (1i64 << m.to);
		//Adjust material score:
		material_score[active_player] = material_score[active_player] - nPieceValue[m.promote_piece_to] + nPieceValue[nPawn];
	}
	//Undo Castle:
	else if (m.active_piece_id == nKing) {
		//Move the rooks back:
		if(m.from == e1) {
			if(m.to == g1) {
				board[h1] = nRook;
				board[f1] = 0;
				mask = (1i64 << h1) | (1i64 << f1);
				pieceBB[WHITE][nRook] ^= mask;
				occupiedBB[WHITE] ^= mask;
			}
			else if(m.to == c1) {
				board[a1] = nRook;
				board[d1] = 0;
				mask = (1i64 << a1) | (1i64 << d1);
				pieceBB[WHITE][nRook] ^= mask;
				occupiedBB[WHITE] ^= mask;
			}
		}
		else if(m.from == e8) {
			if(m.to == g8) {
				board[h8] = nRook;
				board[f8] = 0;
				mask = (1i64 << h8) | (1i64 << f8);
				pieceBB[BLACK][nRook] ^= mask;
				occupiedBB[BLACK] ^= mask;
			}
			else if(m.to == c8) {
				board[a8] = nRook;
				board[d8] = 0;
				mask = (1i64 << a8) | (1i64 << d8);
				pieceBB[BLACK][nRook] ^= mask;
				occupiedBB[BLACK] ^= mask;
			}
		}
	}

	occupiedBB[BOTH] = occupiedBB[WHITE] | occupiedBB[BLACK];

}

void CHESSBOARD::NullMove() {
//Return the en Passant square before nullmove;
	
	//Move count:
	current_ply++;

	gamestate[current_ply] = gamestate[current_ply -1];
	
	//reset en passant possibility:
	gamestate[current_ply].en_passant_index = 64;
	if( gamestate[current_ply-1].en_passant_index != 64 )
		gamestate[current_ply].zobrist_key ^= zobrist::enPassant[gamestate[current_ply-1].en_passant_index];
	
	toggleActivePlayer();
}

void CHESSBOARD::unNullMove() {
	
	//Move count:
	current_ply--;
	
	//Change the player without changing the zobrist key.
	active_player = !active_player;

}

void CHESSBOARD::qMakeMove(MOVE m) {
	// Queit Make Move - Does not keep track of irreversable gamestate data nor zobrist keys.
	// Intended to be used with qSearch.

	//Move count:
	current_ply++;
	#ifdef ENABLE_PAWN_HASH
	gamestate[current_ply].pawn_key = gamestate[current_ply-1].pawn_key;
	#endif
	
	//Handle en passant b/c it is a type of capture.
	gamestate[current_ply].en_passant_index = 64;
	
	//Adjust the mailbox board representation:
	board[m.to] = m.active_piece_id;
	board[m.from] = 0;

	//Adjust the bitboard:
	bitboard mask = ((1i64 << m.to) | (1i64 << m.from));
	pieceBB[active_player][m.active_piece_id] ^= mask;
	occupiedBB[active_player] ^= mask;
		
	if (m.captured_piece != 0) {
		//adjust BBs for a capture occuring.
		pieceBB[!active_player][m.captured_piece] ^= (1i64 << m.to);
		occupiedBB[!active_player] ^= (1i64 << m.to);

		//Adjust material score:
		material_score[!active_player] -= nPieceValue[m.captured_piece];
		
		//Adjust the pawn key:
		if(m.captured_piece == nPawn)
			gamestate[current_ply].pawn_key ^= (phash_lock)zobrist::pieces[!active_player][nPawn][m.to];
			
	}
	if(m.active_piece_id == nPawn) {
		//Adjust the pawn key:
		gamestate[current_ply].pawn_key ^=(phash_lock) zobrist::pieces[active_player][nPawn][m.from];
		gamestate[current_ply].pawn_key ^=(phash_lock) zobrist::pieces[active_player][nPawn][m.to];

		//check for an enPassant capture:
		if (m.to == gamestate[current_ply-1].en_passant_index) {
			pieceBB[!active_player][nPawn] ^= mask::pawn_advances[!active_player][m.to];
			occupiedBB[!active_player] ^= mask::pawn_advances[!active_player][m.to];
			unsigned char removedPieceIndex = bitscan_msb(mask::pawn_advances[!active_player][m.to]);
			board[removedPieceIndex] = 0;
			//gamestate[current_ply].zobrist_key ^= zobrist::pieces[!active_player][nPawn][removedPieceIndex];
			//Adjust Material score:
			material_score[!active_player] -= nPieceValue[nPawn];
			//Adjust the pawn key:
			gamestate[current_ply].pawn_key ^= (phash_lock)zobrist::pieces[!active_player][nPawn][removedPieceIndex];
		}
		/*
		//Check for pawn promotion:
		else if (m.promote_piece_to != 0 ) {
			board[m.to] = m.promote_piece_to;
			pieceBB[active_player][nPawn] ^= (1i64 << m.to);
			pieceBB[active_player][m.promote_piece_to] ^= (1i64 << m.to);
			gamestate[current_ply].zobrist_key ^= zobrist::pieces[active_player][nPawn][m.to];
			gamestate[current_ply].zobrist_key ^= zobrist::pieces[active_player][m.promote_piece_to][m.to];

			//Adjust material score:
			if(active_player == WHITE)
				material_score += nPieceValue[m.promote_piece_to] - nPieceValue[nPawn];
			else
				material_score -= nPieceValue[m.promote_piece_to] - nPieceValue[nPawn];

		}
		*/
	}
	occupiedBB[2] = occupiedBB[0] | occupiedBB[1]; //done separately for the sake of castling.
	toggleActivePlayer();
}


void CHESSBOARD::qUnMakeMove(MOVE m) {

	//Change active player without adjusting the zobrist key:
	active_player = !active_player;
	
	//Move count:
	current_ply--;
	
	//Mailbox representation update:
	board[m.from] = m.active_piece_id;
	board[m.to] = m.captured_piece; //0 for no capture anyways.

	//Adjust the bitboard:
	bitboard mask = ((1i64 << m.to) | (1i64 << m.from));
	pieceBB[active_player][m.active_piece_id] ^= mask;
	occupiedBB[active_player] ^= mask;
	
	//Undo en passant capture:
	if( (m.to == gamestate[current_ply].en_passant_index) && (m.active_piece_id == nPawn)) {
		//cout << ".";
		if(active_player == WHITE) {
			//this means a black pawn was captured.
			pieceBB[BLACK][nPawn] ^= mask::pawn_advances[BLACK][m.to];
			occupiedBB[BLACK] ^= mask::pawn_advances[BLACK][m.to];
			board[m.to - 8] = nPawn;
		}
		else {
			//this means a black pawn was captured.
			pieceBB[WHITE][nPawn] ^= mask::pawn_advances[WHITE][m.to];
			occupiedBB[WHITE] ^= mask::pawn_advances[WHITE][m.to];
			board[m.to + 8] = nPawn;
		}
		//Adjust material score:
		material_score[!active_player] += nPieceValue[nPawn];
	}
	//Undo a capture:
	else if(m.captured_piece != 0) {
		//Only need to adjust the bitboards:
		pieceBB[!active_player][m.captured_piece] ^= (1i64 << m.to);
		occupiedBB[!active_player] ^= (1i64 << m.to);

		//Adjust material score:
		material_score[!active_player] += nPieceValue[m.captured_piece];
	}
	/*
	//Undo Pawn promotion:
	if(m.promote_piece_to != 0) {
		//Only need to adjust the bitboards:
		pieceBB[active_player][nPawn] ^= (1i64 << m.to); //undo the mask done above.
		pieceBB[active_player][m.promote_piece_to] ^= (1i64 << m.to);
		//Adjust material score:
		if(active_player == WHITE)
			material_score -= (nPieceValue[m.promote_piece_to] - nPieceValue[nPawn]);
		else
			material_score += (nPieceValue[m.promote_piece_to] - nPieceValue[nPawn]);
	}
	*/
	occupiedBB[BOTH] = occupiedBB[WHITE] | occupiedBB[BLACK];

}