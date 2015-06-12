#include <iostream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <ctype.h>

#include "macros.h"
#include "bitboards.h"
#include "move.h"
#include "board.h"
#include "hash.h"

#include "evaluate.h"
#include "utilities.h"



using namespace std;

/*
void CHESSBOARD::quickCapture(MOVE attacker) {
	// Only update the bitboards and the mailbox.
	// Do not update the zobrist key, material value, etc.

	//Turn off the attackees bit:
	pieceBB[!attacker.color][attacker.captured_piece] ^= (((bitboard)1) << attacker.to);
	occupiedBB[!attacker.color] ^= (((bitboard)1) << attacker.to);

	//Swap the attackers bit:
	pieceBB[attacker.color][attacker.active_piece_id] ^= ( (((bitboard)1) << attacker.from) | (((bitboard)1) << attacker.to) );
	occupiedBB[attacker.color] ^= ( (((bitboard)1) << attacker.from) | (((bitboard)1) << attacker.to) );
	occupiedBB[BOTH] ^= (((bitboard)1) << attacker.from);

	//Adjust the mailbox board representation:
	board[attacker.to] = attacker.active_piece_id;
	board[attacker.from] = 0;

}
*/
void CHESSBOARD::quickCapture(CAPTURER attacker) {
	// Only update the bitboards and the mailbox.
	// Do not update the zobrist key, material value, etc.

	//Turn off the attackees bit:
	pieceBB[!attacker.color][attacker.captured_id] ^= (((bitboard)1) << attacker.to);
	occupiedBB[!attacker.color] ^= (((bitboard)1) << attacker.to);

	//Swap the attackers bit:
	pieceBB[attacker.color][attacker.piece_id] ^= ( (((bitboard)1) << attacker.from) | (((bitboard)1) << attacker.to) );
	occupiedBB[attacker.color] ^= ( (((bitboard)1) << attacker.from) | (((bitboard)1) << attacker.to) );
	occupiedBB[BOTH] ^= (((bitboard)1) << attacker.from);

	//Adjust the mailbox board representation:
	board[attacker.to] = attacker.piece_id;
	board[attacker.from] = 0;

}
/*
void CHESSBOARD::quickUnCapture(MOVE attacker) {
	// Only update the bitboards and the mailbox.
	// Do not update the zobrist key, material value, etc.

	//Adjust the mailbox board representation:
	board[attacker.to] = attacker.captured_piece;
	board[attacker.from] = attacker.active_piece_id;

	//Turn on the attackees bit:
	pieceBB[!attacker.color][attacker.captured_piece] ^= (((bitboard)1) << attacker.to);
	occupiedBB[!attacker.color] ^= (((bitboard)1) << attacker.to);

	//Swap the attackers bit:
	pieceBB[attacker.color][attacker.active_piece_id] ^= ( (((bitboard)1) << attacker.from) | (((bitboard)1) << attacker.to) );
	occupiedBB[attacker.color] ^= ( (((bitboard)1) << attacker.from) | (((bitboard)1) << attacker.to) );
	occupiedBB[BOTH] ^= (((bitboard)1) << attacker.from);

}
*/
void CHESSBOARD::quickUnCapture(CAPTURER attacker) {
	// Only update the bitboards and the mailbox.
	// Do not update the zobrist key, material value, etc.

	//Adjust the mailbox board representation:
	board[attacker.to] = attacker.captured_id;
	board[attacker.from] = attacker.piece_id;

	//Turn on the attackees bit:
	pieceBB[!attacker.color][attacker.captured_id] ^= (((bitboard)1) << attacker.to);
	occupiedBB[!attacker.color] ^= (((bitboard)1) << attacker.to);

	//Swap the attackers bit:
	pieceBB[attacker.color][attacker.piece_id] ^= ( (((bitboard)1) << attacker.from) | (((bitboard)1) << attacker.to) );
	occupiedBB[attacker.color] ^= ( (((bitboard)1) << attacker.from) | (((bitboard)1) << attacker.to) );
	occupiedBB[BOTH] ^= (((bitboard)1) << attacker.from);

}

bool CHESSBOARD::checkThreeFold() {
	//Decides if the current board state is a draw due to 3fold repitition.
	for(int i=current_ply-2; i>= current_ply-gamestate[current_ply].half_move_rule; i=i-2) {
		if( gamestate[i].zobrist_key == gamestate[current_ply].zobrist_key ) {
			for(int j=i; j >= current_ply-gamestate[current_ply].half_move_rule; j=j-2) {
				if( gamestate[j].zobrist_key == gamestate[current_ply].zobrist_key )
					return true;
			}
			break;
		}
	}
	return false;
}

bool CHESSBOARD::isAttacked(unsigned char square_index) {
	return isAttacked(active_player, square_index);
}

bool CHESSBOARD::isAttacked(bool player, unsigned char square_index) {
	//TODO hardcode the masks that do not require a dynamic index.

	bool opponent = !player;
	bitboard pieceDB =0;

	// Is in danger from pawns?:
	if(opponent == WHITE) {
		pieceDB =	((pieceBB[opponent][nPawn] & ~FileMask(a)) << 9)  
				  |	((pieceBB[opponent][nPawn] & ~FileMask(h)) << 7); 
		if(pieceDB & (((bitboard)1) << square_index) ) {
			return true;
		}
	}
	else {
		pieceDB =	((pieceBB[opponent][nPawn] & ~FileMask(h)/*~mask::file[h]*/) >> 9) 
				  |	((pieceBB[opponent][nPawn] & ~FileMask(a)/*~mask::file[a]*/) >> 7);
		if(pieceDB & (((bitboard)1) << square_index) ) {
			return true;
		}
	}
	
	//Knight attacks:
	if(mask::knight_moves[square_index] & pieceBB[opponent][nKnight]){
		return true;
	}

	//Diagonal attacks:
	if(mask::MagicBishopMoves[square_index][ ((occupiedBB[BOTH] & mask::MagicBishopMask[square_index]) * mask::MagicBishopNumbers[square_index]) >> mask::MagicBishopShift[square_index] ] 
			& (pieceBB[opponent][nBishop] | pieceBB[opponent][nQueen])) {
		return true;
	}
	
	// Horizontal and Vertical attacks:
	if(mask::MagicRookMoves[square_index][((occupiedBB[BOTH]&mask::MagicRookMask[square_index]) * mask::MagicRookNumbers[square_index])>>mask::MagicRookShift[square_index]] 
			& (pieceBB[opponent][nRook] | pieceBB[opponent][nQueen])) {
		return true;
	}
	
	// King attacks:
	if(mask::king_moves[square_index] & pieceBB[opponent][nKing]) {
		return true;
	}

	return false;

}

bool CHESSBOARD::isInCheck() {
	return isInCheck(active_player);
}

bool CHESSBOARD::isInCheck(bool player) {
	//Returns if the active player is in check.
	return isAttacked(player, bitscan_msb(pieceBB[player][nKing]));	
}

void CHESSBOARD::clearBoard() {
	
	current_ply = 0;

	for(int i=0; i<8;i++) {
		pieceBB[0][i] = 0;
		pieceBB[1][i] = 0;
	}
	for(int i=0; i<64;i++) {
		board[i] = 0;
	}
	occupiedBB[0] = 0;
	occupiedBB[1] = 0;
	occupiedBB[2] = 0;
	material_score[0] = 0;
	material_score[1] = 0;
	//position_score[0] = 0;
	//position_score[1] = 0;
	
	gamestate[0].en_passant_index = 64;
	gamestate[0].half_move_rule = 0;
	gamestate[0].castling_rights[0][0] = false;
	gamestate[0].castling_rights[1][0] = false;
	gamestate[0].castling_rights[0][1] = false;
	gamestate[0].castling_rights[1][1] = false;

	gamestate[0].zobrist_key = generateKey();
	gamestate[0].pawn_key = 0;
	
}

void CHESSBOARD::newgame() 
{
	position("startpos");
}

void CHESSBOARD::position(string fen)
{
	// FEN Format: 
	//		PEICES ACTIVECOLOR CASTLING ENPASSANT HALFMOVES FULLMOVES
	// Starting position:
	//		rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
	// Regular expression:
	//		\s*([rnbqkpRNBQKP1-8]+\/){7}([rnbqkpRNBQKP1-8]+)\s[bw-]\s(([a-hkqA-HKQ]{1,4})|(-))\s(([a-h][36])|(-))\s\d+\s\d+\s*

	clearBoard();
	if(fen == "startpos") {
		fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";	
	}

	stringstream ss(fen);

	string fen_board;
	string fen_active_color;
	string fen_castling;
	string fen_enPassantSquare;
	string fen_half_moves;
	string fen_full_moves;

	ss >> fen_board >> fen_active_color >> fen_castling >> fen_enPassantSquare >> fen_half_moves >> fen_full_moves;

	gamestate[0].half_move_rule = atoi(fen_half_moves.c_str());
	current_ply = 0;
	

	//Game status variables:
	if(fen_active_color == "w") {
		active_player = 0; 
		gamestate[0].zobrist_key ^= zobrist::wtm;
	}
	else {
		active_player = 1; 
		//current_ply++;
	}
	
	//enPassant:
	if(fen_enPassantSquare != "-") {
		gamestate[0].en_passant_index = alg_to_index(fen_enPassantSquare);
		gamestate[0].zobrist_key ^= zobrist::enPassant[ alg_to_index(fen_enPassantSquare) ];
	}

	//Castling:
	for(int i=0; i < fen_castling.length(); i++) {
		if(fen_castling.at(i) == 'K')  {
			//kingCastle[WHITE] = true;
			gamestate[0].castling_rights[WHITE][KING_SIDE] = true;
			gamestate[0].zobrist_key ^= zobrist::castling[WHITE][KING_SIDE];
		}
		else if(fen_castling.at(i) == 'k') {
			gamestate[0].castling_rights[BLACK][KING_SIDE] = true;
			gamestate[0].zobrist_key ^= zobrist::castling[BLACK][KING_SIDE];
		}
		else if(fen_castling.at(i) == 'Q') {
			gamestate[0].castling_rights[WHITE][QUEEN_SIDE] = true;
			gamestate[0].zobrist_key ^= zobrist::castling[WHITE][QUEEN_SIDE];
		}
		else if(fen_castling.at(i) == 'q') {
			gamestate[0].castling_rights[BLACK][QUEEN_SIDE] = true;
			gamestate[0].zobrist_key ^= zobrist::castling[BLACK][QUEEN_SIDE];
		}
	}

	//CHESSBOARD position variables:
	int index = 63;
	for(int pos=0; pos < fen_board.length(); pos++) {
		//pawn=1, knight=2, king=3, bishop=5, rook=6 and queen=7
		char piece = fen_board.at(pos);
		if(piece == 'P') {
			pieceBB[0][nPawn] |= (((bitboard)1) << index);
			board[index] = nPawn;
			material_score[WHITE] += nPieceValue[nPawn];
			gamestate[0].zobrist_key ^= zobrist::pieces[WHITE][nPawn][index];
			gamestate[0].pawn_key	 ^= (phash_lock)zobrist::pieces[WHITE][nPawn][index];
		}
		else if(piece == 'N') {
			pieceBB[0][nKnight] |= (((bitboard)1) << index);
			board[index] = nKnight;
			material_score[WHITE] += nPieceValue[nKnight];
			gamestate[0].zobrist_key ^= zobrist::pieces[WHITE][nKnight][index];
		}
		else if(piece == 'K') {
			pieceBB[0][nKing] |= (((bitboard)1) << index);
			board[index] = nKing;
			material_score[WHITE] += nPieceValue[nKing];
			gamestate[0].zobrist_key ^= zobrist::pieces[WHITE][nKing][index];
		}
		else if(piece == 'B') {
			pieceBB[0][nBishop] |= (((bitboard)1) << index);
			board[index] = nBishop;
			material_score[WHITE] += nPieceValue[nBishop];
			gamestate[0].zobrist_key ^= zobrist::pieces[WHITE][nBishop][index];
		}
		else if(piece == 'R') {
			pieceBB[0][nRook] |= (((bitboard)1) << index);
			board[index] = nRook;
			material_score[WHITE] += nPieceValue[nRook];
			gamestate[0].zobrist_key ^= zobrist::pieces[WHITE][nRook][index];
		}
		else if(piece == 'Q') {
			pieceBB[0][nQueen] |= (((bitboard)1) << index);
			board[index] = nQueen;
			material_score[WHITE] += nPieceValue[nQueen];
			gamestate[0].zobrist_key ^= zobrist::pieces[WHITE][nQueen][index];
		}
		else if(piece == 'p') {
			pieceBB[1][nPawn] |= (((bitboard)1) << index);
			board[index] = nPawn;
			material_score[BLACK] += nPieceValue[nPawn];
			gamestate[0].zobrist_key ^= zobrist::pieces[BLACK][nPawn][index];
			gamestate[0].pawn_key	 ^= (phash_lock)zobrist::pieces[BLACK][nPawn][index];
		}
		else if(piece == 'n') {
			pieceBB[1][nKnight] |= (((bitboard)1) << index);
			board[index] = nKnight;
			material_score[BLACK] += nPieceValue[nKnight];
			gamestate[0].zobrist_key ^= zobrist::pieces[BLACK][nKnight][index];
		}
		else if(piece == 'k') {
			pieceBB[1][nKing] |= (((bitboard)1) << index);
			board[index] = nKing;
			material_score[BLACK] += nPieceValue[nKing];
			gamestate[0].zobrist_key ^= zobrist::pieces[BLACK][nKing][index];
		}
		else if(piece == 'b') {
			pieceBB[1][nBishop] |= (((bitboard)1) << index);
			board[index] = nBishop;
			material_score[BLACK] += nPieceValue[nBishop];
			gamestate[0].zobrist_key ^= zobrist::pieces[BLACK][nBishop][index];
		}
		else if(piece == 'r') {
			pieceBB[1][nRook] |= (((bitboard)1) << index);
			board[index] = nRook;
			material_score[BLACK] += nPieceValue[nRook];
			gamestate[0].zobrist_key ^= zobrist::pieces[BLACK][nRook][index];
		}
		else if(piece == 'q') {
			pieceBB[1][nQueen] |= (((bitboard)1) << index);
			board[index] = nQueen;
			material_score[BLACK] += nPieceValue[nQueen];
			gamestate[0].zobrist_key ^= zobrist::pieces[BLACK][nQueen][index];
		}
		else if(piece == '/') 
			index++; //negates the -- to follow, so that ultimately the index will not change.
		else {
			int skip = (piece - '0'); // '0' = 48
			index -= (skip);
			index++; //negates the -- to follow, so that ultimately the index will not change.
		}
		index--;
	}

	occupiedBB[0] = pieceBB[0][1] | pieceBB[0][2]  | pieceBB[0][3] | pieceBB[0][5] | pieceBB[0][6] | pieceBB[0][7];
	occupiedBB[1] = pieceBB[1][1] | pieceBB[1][2]  | pieceBB[1][3] | pieceBB[1][5] | pieceBB[1][6] | pieceBB[1][7];
	occupiedBB[2] = occupiedBB[0] | occupiedBB[1];

	gamestate[0].zobrist_key = generateKey();
	gamestate[0].pawn_key = generatePawnKey();
}

void CHESSBOARD::print_bb() {
	std::cout << "White: " << endl;
	std::cout << "Pawn     Knight   Bishop   Rook     Queen    King     Occupied\n";
	bitboard whiteBB[7] = {pieceBB[WHITE][nPawn], pieceBB[WHITE][nKnight], pieceBB[WHITE][nBishop], pieceBB[WHITE][nRook], pieceBB[WHITE][nQueen], pieceBB[WHITE][nKing], occupiedBB[WHITE]};
	for(int i=8; i >= 1; i--) { //row control
		for(int j=0; j < 7; j++) { //pieceBB control
			for(int k=1; k <=8; k++) { //column control
				int index = (i*8) - k;
				if( bitcheck(whiteBB[j], index) )
					std::cout << "1";
				else
					std::cout << "0";
			}
			std::cout << " ";
		}
		std::cout << endl;
	}
	std::cout << endl;

	std::cout << "Black: " << endl;
	std::cout << "Pawn     Knight   Bishop   Rook     Queen    King     Occupied\n";
	bitboard blackBB[7] = {pieceBB[BLACK][nPawn], pieceBB[BLACK][nKnight], pieceBB[BLACK][nBishop], pieceBB[BLACK][nRook], pieceBB[BLACK][nQueen], pieceBB[BLACK][nKing], occupiedBB[BLACK]};
	
	for(int i=8; i >= 1; i--) { //row control
		for(int j=0; j < 7; j++) { //pieceBB control
			for(int k=1; k <=8; k++) { //column control
				int index = (i*8) - k;
				if( bitcheck(blackBB[j], index) )
					std::cout << "1";
				else
					std::cout << "0";
			}
			std::cout << " ";
		}
		std::cout << endl;
	}
	std::cout << endl;

}

void CHESSBOARD::print_mailbox() {
	for(int i =63; i >= 0; i--) {
		cout << board[i];
		if( i%8 == 0)
			cout << endl;
	}
}

void CHESSBOARD::print()
{
	cout << " +---+---+---+---+---+---+---+---+" << endl;
	//bitboard pos = 1;
	for(int i=63; i>=0; i--) 
	{
		cout << " |";
		if( (((bitboard)1) << i) & pieceBB[0][nPawn] )
			cout << " P";
		else if( (((bitboard)1) << i) & pieceBB[1][nPawn] )
			cout << " p";
		else if( (((bitboard)1) << i) & pieceBB[0][nKnight] )
			cout << " N";
		else if( (((bitboard)1) << i) & pieceBB[1][nKnight] )
			cout << " n";
		else if( (((bitboard)1) << i) & pieceBB[0][nKing] )
			cout << " K";
		else if( (((bitboard)1) << i) & pieceBB[1][nKing] )
			cout << " k";
		else if( (((bitboard)1) << i) & pieceBB[0][nBishop] )
			cout << " B";
		else if( (((bitboard)1) << i) & pieceBB[1][nBishop] )
			cout << " b";
		else if( (((bitboard)1) << i) & pieceBB[0][nRook] )
			cout << " R";
		else if( (((bitboard)1) << i) & pieceBB[1][nRook] )
			cout << " r";
		else if( (((bitboard)1) << i) & pieceBB[0][nQueen] )
			cout << " Q";
		else if( (((bitboard)1) << i) & pieceBB[1][nQueen] )
			cout << " q";
		else
			cout << "  ";


		if(i%8 == 0) {
			cout << " |";
			if(i/8 == 7) {
				if(active_player == 0) 
					cout << "   White to move.";
				else
					cout << "   Black to move.";
			}
			else if( i/8 == 6) {
				cout << "   Ply: \t\t" << current_ply;
			}
			else if( i/8 == 5) {
				cout <<  "   50 Move Rule: \t" << gamestate[current_ply].half_move_rule;
			}
			else if(i/8 == 4) {
				cout << "   En Passant: \t";
				if(gamestate[current_ply].en_passant_index != 64) 
					cout << index_to_alg(gamestate[current_ply].en_passant_index);
				else
					cout << "-";
			}
			else if(i/8 == 3) {
				cout << "   Wht Castle: ";
				if(gamestate[current_ply].castling_rights[WHITE][KING_SIDE])
					cout << "K-side ";
				if(gamestate[current_ply].castling_rights[WHITE][QUEEN_SIDE])
					cout << "Q-side";
			}
			else if(i/8 == 2) {
				cout << "   Blk Castle: ";
				if(gamestate[current_ply].castling_rights[BLACK][KING_SIDE])
					cout << "K-side ";
				if(gamestate[current_ply].castling_rights[BLACK][QUEEN_SIDE])
					cout << "Q-side";
			}
			else if( i/8 == 0) {
				cout << "   Key: " << gamestate[current_ply].zobrist_key;
			}
			cout << endl << " +---+---+---+---+---+---+---+---+" << endl;
		}
	}
	
}

void CHESSBOARD::toggleActivePlayer() {
	gamestate[current_ply].zobrist_key ^= zobrist::wtm; 
	active_player=!active_player;
}

bitboard CHESSBOARD::generateKey() {
	//Since this function is only used for debugging, optimization isn't necessary.
	bitboard key = 0;
	
	//Player to move:
	if(getActivePlayer() == WHITE)
		key ^= zobrist::wtm;

	//En passant:
	if(gamestate[current_ply].en_passant_index != 64)
		key ^= zobrist::enPassant[gamestate[current_ply].en_passant_index];

	//Castling:
	if(gamestate[current_ply].castling_rights[WHITE][KING_SIDE])
		key ^= zobrist::castling[WHITE][KING_SIDE];
	if(gamestate[current_ply].castling_rights[WHITE][QUEEN_SIDE])
		key ^= zobrist::castling[WHITE][QUEEN_SIDE];
	if(gamestate[current_ply].castling_rights[BLACK][KING_SIDE])
		key ^= zobrist::castling[BLACK][KING_SIDE];
	if(gamestate[current_ply].castling_rights[BLACK][QUEEN_SIDE])
		key ^= zobrist::castling[BLACK][QUEEN_SIDE];

	//Pieces:
	for(int i =0; i < 64; i++) {
		short piece = getBoard(i);
		if( piece ) {
			bool color = BLACK;
			if( getOccupiedBB(WHITE) & (((bitboard)1) << i) )
				color = WHITE;
			key ^= zobrist::pieces[color][piece][i];
		}
	}
	return key;
}

phash_lock CHESSBOARD::generatePawnKey() {

	//Since this function is only used for debugging
	phash_lock key = 0;

	for(int i =0; i < 64; i++) {
		short piece = getBoard(i);
		if( piece == nPawn ) {
			bool color = BLACK;
			if( getPieceBB(WHITE,nPawn) & (((bitboard)1) << i) )
				color = WHITE;
			key ^= (phash_lock)zobrist::pieces[color][nPawn][i];
		}
	}

	/*
	bitboard pieceDB = getPieceBB(WHITE,nPawn);
	while(pieceDB) {
		int square = bitscan_msb(pieceDB);
		key ^= zobrist::pieces[WHITE][nPawn][square];
		pieceDB ^= (((bitboard)1) << square);
	}
	pieceDB = getPieceBB(BLACK,nPawn);
	while(pieceDB) {
		int square = bitscan_msb(pieceDB);
		key ^= zobrist::pieces[BLACK][nPawn][square];
		pieceDB ^= (((bitboard)1) << square);
	}
	*/
	return key;
}