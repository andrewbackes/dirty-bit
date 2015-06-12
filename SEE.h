#ifndef see_h
#define see_h

#include <algorithm> //For max

#include "board.h"
#include "move.h"
#include "evaluate.h"

extern int ordering_tables[2][8][64];
void initOrderingTables(short t);

CAPTURER getSmallestAttacker( unsigned char toSquare, bool toMove, CHESSBOARD * b);

void findAttackingSquares(bitboard &attackers, CHESSBOARD *b, unsigned char square);

short SEE1(CHESSBOARD * b, MOVE m);
short SEE1(CHESSBOARD * b, unsigned char square, bool toMove);
short SEE2(CHESSBOARD *b, MOVE m);
short SEE3(CHESSBOARD *b, MOVE m);

short STE(CHESSBOARD * b, MOVE m);

#endif
