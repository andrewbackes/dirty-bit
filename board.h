/*
63 62 61 60 59 58 57 56
55 54 53 52 51 50 49 48
47 46 45 44 43 42 41 40
39 38 37 36 35 34 33 32
31 30 29 28 27 26 25 24
23 22 21 20 19 18 17 16
15 14 13 12 11 10 9  8
7  6  5  4  3  2  1  0
*/

#ifndef board_h
#define board_h

#include <string>
#include <vector>
#include <iostream>

#include "bitboards.h"
#include "move.h"

#include "parameters.h"
/*
const short nPawn = 1;
const short nKnight = 2;
const short nKing = 3;
const short nBishop = 5;
const short nRook = 6;
const short nQueen = 7;

const short WHITE = 0;
const short BLACK = 1;
const short BOTH = 2;

const short KING_SIDE = 0;
const short QUEEN_SIDE = 1;

*/

class GAMESTATE {
public:
	GAMESTATE() {}
	//{zobrist_key = 0; half_move_rule = 0; en_passant_index=64; kCastleWhite =false;  qCastleWhite=false; kCastleBlack=false; qCastleBlack=false; }
	GAMESTATE(bitboard key, short half_moves, short en_passant, bool kCastleWhite, bool qCastleWhite, bool kCastleBlack, bool qCastleBlack) {
		std::cout << "start GAMESTATE()" << std::endl;
		zobrist_key = key; 
		half_move_rule = half_moves; 
		en_passant_index = en_passant;
		castling_rights[WHITE][KING_SIDE] =		kCastleWhite;
		castling_rights[BLACK][KING_SIDE] =		kCastleBlack;
		castling_rights[WHITE][QUEEN_SIDE] =	qCastleWhite;
		castling_rights[BLACK][QUEEN_SIDE] =	qCastleBlack;
	}

	bitboard		zobrist_key;
	//bitboard		pawn_key;
	phash_lock		pawn_key;
	short			half_move_rule;
	short			en_passant_index;
	bool			castling_rights[2][2];
	
};

class CHESSBOARD{

public: 
	CHESSBOARD()
		{ clearBoard(); }
	~CHESSBOARD()
		{  }
	
	bitboard	getPieceBB(bool player_index, int piece_id) {return pieceBB[player_index][piece_id];}
	void		setPieceBB(bool player_index, int piece_id, bitboard value){pieceBB[player_index][piece_id] = value;}
	
	bitboard	getOccupiedBB(int who)				{return occupiedBB[who];}

	int			getBoard(int index)					{return board[index];}
	void		setBoard(int index, int piece_id)	{board[index]=piece_id;}

	short		getRelativeMaterialValue() {return material_score[active_player] - material_score[!active_player];}
	short		getMaterialValue(bool player) {return material_score[player];}
	//short		getPositionValue(bool player) {return position_score[player];}
	void		resetValue();
	
	bool		isInCheck(bool player);
	bool		isInCheck();
	bool		NewisInCheck(bool player);
	bool		NewisInCheck();
	bool		isAttacked(bool player, unsigned char square_index);
	bool		isAttacked(unsigned char square_index);

	bool		checkThreeFold();
			
	int			getMoveCount()						{return current_ply/2;}
	int			getPly()							{return current_ply;}
	bitboard	getKey()							{return gamestate[current_ply].zobrist_key;}
	phash_lock	getPawnKey()						{return gamestate[current_ply].pawn_key;}
	short		getHalfMovesRule()					{return gamestate[current_ply].half_move_rule;}
	short		getEnPassantIndex()					{return gamestate[current_ply].en_passant_index;}
	bool		getCastlingRights(bool p, bool s)	{return gamestate[current_ply].castling_rights[p][s];}
	bool		getActivePlayer()					{return active_player;}
	
	void		MakeMove(MOVE m);
	void		unMakeMove(MOVE m);
	void		qMakeMove(MOVE m);
	void		qUnMakeMove(MOVE m);
	void		NullMove();
	void		unNullMove();

	void		quickCapture(CAPTURER attacker);
	void		quickCapture(MOVE attacker) {quickCapture(CAPTURER(attacker));}

	void		quickUnCapture(CAPTURER attacker);
	void		quickUnCapture(MOVE attacker) {quickUnCapture(CAPTURER(attacker));}

	void		print();
	void		print_mailbox() ;
	void		print_bb();

	void		newgame(); //reset the CHESSBOARD.
	void		position(std::string fen); //passes a FEN string.
	void		clearBoard();
	bitboard	generateKey();
	phash_lock  generatePawnKey();
	void		toggleActivePlayer();

private:
	bitboard	pieceBB[2][8]; //pawn=1, knight=2, king=3, bishop=5, rook=6 and queen=7
	bitboard	occupiedBB[3]; //0=white,1=black,2=both.
	GAMESTATE	gamestate[500];
	short		material_score[2];
	//short		position_score[2];
	short		board[64]; //pawn=1, knight=2, king=3, bishop=5, rook=6 and queen=7.
	short		current_ply;
	bool		active_player;
};

inline unsigned char getMostAdvanced(bool toMove, bitboard * pieceDB) {
	return (toMove==WHITE)? bitscan_msb(*pieceDB) : bitscan_lsb(*pieceDB); }

inline unsigned char getLeastAdvanced(bool toMove, bitboard * pieceDB) {
	return (toMove==BLACK)? bitscan_msb(*pieceDB) : bitscan_lsb(*pieceDB); }

#endif