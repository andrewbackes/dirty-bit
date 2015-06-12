#ifndef evaluate_h
#define evaluate_h

#include "board.h"

#define INFINITY 2147483647
#define WIN 2147483647
#define LOOSE -2147483647
#define MATE 16384
#define DRAW 0

#define ENDGAME 13

int contempt(CHESSBOARD * b);

int evaluate(CHESSBOARD * b);
int relative_eval(CHESSBOARD *b, int alpha= -INFINITY, int beta = INFINITY);

int materialScore(CHESSBOARD * b);



/*
const int INFINITY = 2147483647;
const int WIN = INFINITY;
const int LOOSE = -INFINITY;
const int MATE = 16384;
const int DRAW = 0;
*/


const short nPieceValue[9] = {0,PAWN_VALUE,KNIGHT_VALUE,KING_VALUE,0,BISHOP_VALUE,ROOK_VALUE,QUEEN_VALUE,0};

//See https://chessprogramming.wikispaces.com/Simplified+evaluation+function

const int nSquareValue[2][8][64] = { 
	//WHITE:
	{
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		//Pawns:
		{
			0,  0,  0,  0,  0,  0,  0,  0,
			5, 10, 10,-25,-25, 10, 10,  5,
			5, -5,-10,  0,  0,-10, -5,  5,
			0,  0,  0, 25, 25,  0,  0,  0,
			5,  5, 10, 27, 27, 10,  5,  5,
			10, 10, 20, 30, 30, 20, 10, 10,
			50, 50, 50, 50, 50, 50, 50, 50,
			0,  0,  0,  0,  0,  0,  0,  0
		},
		//Knights:
		{
			-50,-40,-30,-30,-30,-30,-40,-50,
			-40,-20,  0,  5,  5,  0,-20,-40,
			-30,  5, 10, 15, 15, 10,  5,-30,
			-30,  0, 15, 20, 20, 15,  0,-30,
			-30,  5, 15, 20, 20, 15,  5,-30,
			-30,  0, 10, 15, 15, 10,  0,-30,
			-40,-20,  0,  0,  0,  0,-20,-40,
			-50,-40,-20,-30,-30,-20,-40,-50
		},
		//King (Early/Mid Game):
		{
			20,  30,  10,   0,   0,  10,  30,  20,
			20,  20,   0,   0,   0,   0,  20,  20,
			-10, -20, -20, -20, -20, -20, -20, -10, 
			-20, -30, -30, -40, -40, -30, -30, -20,
			-30, -40, -40, -50, -50, -40, -40, -30,
			-30, -40, -40, -50, -50, -40, -40, -30,
			-30, -40, -40, -50, -50, -40, -40, -30,
			-30, -40, -40, -50, -50, -40, -40, -30	
		},

		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		//Bishops:
		{
			-20,-10,-40,-10,-10,-40,-10,-20,
			-10,  5,  0,  0,  0,  0,  5,-10,
			-10, 10, 10, 10, 10, 10, 10,-10,
			-10,  0, 10, 10, 10, 10,  0,-10,
			-10,  5,  5, 10, 10,  5,  5,-10,
			-10,  0,  5, 10, 10,  5,  0,-10,
			-10,  0,  0,  0,  0,  0,  0,-10,
			-20,-10,-10,-10,-10,-10,-10,-20
		},
		//Rooks:
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

		//Queens:
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	},
	//BLACK:
	{
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		//Pawns:
		{
			0,  0,  0,  0,  0,  0,  0,  0,
			50, 50, 50, 50, 50, 50, 50, 50,
			10, 10, 20, 30, 30, 20, 10, 10,
			5,  5, 10, 27, 27, 10,  5,  5,
			0,  0,  0, 25, 25,  0,  0,  0,
			5, -5,-10,  0,  0,-10, -5,  5,
			5, 10, 10,-25,-25, 10, 10,  5,
			0,  0,  0,  0,  0,  0,  0,  0
		},
		//Knights:
		{
			-50,-40,-30,-30,-30,-30,-40,-50,
			-40,-20,  0,  0,  0,  0,-20,-40,
			-30,  0, 10, 15, 15, 10,  0,-30,
			-30,  5, 15, 20, 20, 15,  5,-30,
			-30,  0, 15, 20, 20, 15,  0,-30,
			-30,  5, 10, 15, 15, 10,  5,-30,
			-40,-20,  0,  5,  5,  0,-20,-40,
			-50,-40,-20,-30,-30,-20,-40,-50
		},
		//King (Early/Mid Game):
		{
			-30, -40, -40, -50, -50, -40, -40, -30,
			-30, -40, -40, -50, -50, -40, -40, -30,
			-30, -40, -40, -50, -50, -40, -40, -30,
			-30, -40, -40, -50, -50, -40, -40, -30,
			-20, -30, -30, -40, -40, -30, -30, -20,
			-10, -20, -20, -20, -20, -20, -20, -10, 
			 20,  20,   0,   0,   0,   0,  20,  20,
			 20,  30,  10,   0,   0,  10,  30,  20
		},

		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		//Bishops:
		{
			-20,-10,-10,-10,-10,-10,-10,-20,
			-10,  0,  0,  0,  0,  0,  0,-10,
			-10,  0,  5, 10, 10,  5,  0,-10,
			-10,  5,  5, 10, 10,  5,  5,-10,
			-10,  0, 10, 10, 10, 10,  0,-10,
			-10, 10, 10, 10, 10, 10, 10,-10,
			-10,  5,  0,  0,  0,  0,  5,-10,
			-20,-10,-40,-10,-10,-40,-10,-20
		},
		//Rooks:
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

		//Queens:
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	}
};



const int nEndgameKingValue[2][64] =	{
		{
			-30,-18,-18,-18,-18,-18,-18,-30,
			-18,-18,  0,  0,  0,  0,-18,-18,
			-18,-6, 12, 18, 18, 12,-6,-18,
			-18,-6, 18, 24, 24, 18,-6,-18,
			-18,-6, 18, 24, 24, 18,-6,-18,
			-18,-6, 12, 18, 18, 12,-6,-18,
			-18,-12,-6,  0,  0,-6,-12,-18,
			-30,-24,-18,-12,-12,-18,-24,-30
		},
		{
			-30,-24,-18,-12,-12,-18,-24,-30,
			-18,-12,-6,  0,  0,-6,-12,-18,
			-18,-6, 12, 18, 18, 12,-6,-18,
			-18,-6, 18, 24, 24, 18,-6,-18,
			-18,-6, 18, 24, 24, 18,-6,-18,
			-18,-6, 12, 18, 18, 12,-6,-18,
			-18,-18,  0,  0,  0,  0,-18,-18,
			-30,-18,-18,-18,-18,-18,-18,-30
		} }; 
			/*
const int nSquareValue[2][8][64] = { 
	//WHITE:
	{
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		//Pawns:
		{
			0,  0,  0,  0,  0,  0,  0,  0,
			3, 6, 6,-15,-15, 6, 6,  3,
			3, -3,-6,  0,  0,-6, -3,  3,
			0,  0,  0, 15, 15,  0,  0,  0,
			3,  3, 6, 16, 16, 6,  3,  3,
			6, 6, 12, 18, 18, 12, 6, 6,
			30, 30, 30, 30, 30, 30, 30, 30,
			0,  0,  0,  0,  0,  0,  0,  0
		},
		//Knights:
		{
			-30,-24,-18,-18,-18,-18,-24,-30,
			-24,-12,  0,  3,  3,  0,-12,-24,
			-18,  3, 6, 9, 9, 6,  3,-18,
			-18,  0, 9, 12, 12, 9,  0,-18,
			-18,  3, 9, 12, 12, 9,  3,-18,
			-18,  0, 6, 9, 9, 6,  0,-18,
			-24,-12,  0,  0,  0,  0,-12,-24,
			-30,-24,-12,-18,-18,-12,-24,-30
		},
		//King (Early/Mid Game):
		{
			12,  18,  6,   0,   0,  6,  18,  12,
			12,  12,   0,   0,   0,   0,  12,  12,
			-6, -12, -12, -12, -12, -12, -12, -6, 
			-12, -18, -18, -24, -24, -18, -18, -12,
			-18, -24, -24, -30, -30, -24, -24, -18,
			-18, -24, -24, -30, -30, -24, -24, -18,
			-18, -24, -24, -30, -30, -24, -24, -18,
			-18, -24, -24, -30, -30, -24, -24, -18	
		},

		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		//Bishops:
		{
			-12,-6,-24,-6,-6,-24,-6,-12,
			-6,  3,  0,  0,  0,  0,  3,-6,
			-6, 6, 6, 6, 6, 6, 6,-6,
			-6,  0, 6, 6, 6, 6,  0,-6,
			-6,  3,  3, 6, 6,  3,  3,-6,
			-6,  0,  3, 6, 6,  3,  0,-6,
			-6,  0,  0,  0,  0,  0,  0,-6,
			-12,-6,-6,-6,-6,-6,-6,-12
		},
		//Rooks:
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

		//Queens:
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	},
	//BLACK:
	{
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		//Pawns:
		{
			0,  0,  0,  0,  0,  0,  0,  0,
			30, 30, 30, 30, 30, 30, 30, 30,
			6, 6, 12, 18, 18, 12, 6, 6,
			3,  3, 6, 16, 16, 6,  3,  3,
			0,  0,  0, 15, 15,  0,  0,  0,
			3, -3,-6,  0,  0,-6, -3,  3,
			3, 6, 6,-15,-15, 6, 6,  3,
			0,  0,  0,  0,  0,  0,  0,  0
		},
		//Knights:
		{
			-30,-24,-18,-18,-18,-18,-24,-30,
			-24,-12,  0,  0,  0,  0,-12,-24,
			-18,  0, 6, 9, 9, 6,  0,-18,
			-18,  3, 9, 12, 12, 9,  3,-18,
			-18,  0, 9, 12, 12, 9,  0,-18,
			-18,  3, 6, 9, 9, 6,  3,-18,
			-24,-12,  0,  3,  3,  0,-12,-24,
			-30,-24,-12,-18,-18,-12,-24,-30
		},
		//King (Early/Mid Game):
		{
			-18, -24, -24, -30, -30, -24, -24, -18,
			-18, -24, -24, -30, -30, -24, -24, -18,
			-18, -24, -24, -30, -30, -24, -24, -18,
			-18, -24, -24, -30, -30, -24, -24, -18,
			-12, -18, -18, -24, -24, -18, -18, -12,
			-6, -12, -12, -12, -12, -12, -12, -6, 
			 12,  12,   0,   0,   0,   0,  12,  12,
			 12,  18,  6,   0,   0,  6,  18,  12
		},

		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		//Bishops:
		{
			-12,-6,-6,-6,-6,-6,-6,-12,
			-6,  0,  0,  0,  0,  0,  0,-6,
			-6,  0,  3, 6, 6,  3,  0,-6,
			-6,  3,  3, 6, 6,  3,  3,-6,
			-6,  0, 6, 6, 6, 6,  0,-6,
			-6, 6, 6, 6, 6, 6, 6,-6,
			-6,  3,  0,  0,  0,  0,  3,-6,
			-12,-6,-24,-6,-6,-24,-6,-12
		},
		//Rooks:
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},

		//Queens:
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	}
};

*/
#endif