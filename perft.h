#ifndef perft_h
#define perft_h

#include "board.h"

class PERFT {
public:
	bitboard captures;
	bitboard enpassants;
	bitboard castles;
	bitboard promotions;
	bitboard checks;
	bitboard checkmates;
	bitboard nodes;

	void Perft(CHESSBOARD * game, int depth);
	void Divide(CHESSBOARD * game, int depth);
	void go(CHESSBOARD * game, int depth);
	void clear();
};

bitboard perft(CHESSBOARD * game, int depth);

#endif

/*

Perft Suite:
1: D5- FAILED! (4865612!=4865609).
2: D3- FAILED! (97977!=97862).

105: D2- FAILED! (21!=22).
106: D4- FAILED! (636!=637).
107: D2- FAILED! (21!=22).
108: D4- FAILED! (621!=637).

112: D3- FAILED! (137!=139).
113: D5- FAILED! (4350!=4354).
114: D3- FAILED! (129!=139).
115: D3- FAILED! (99!=101).

120: D3- FAILED! (7469!=7421).
121: D1- FAILED! (19!=18).
122: D3- FAILED! (9547!=9483).

124: D4- FAILED! (125198!=124608).
125: D2- FAILED! (282!=270).
126: D4- FAILED! (183637!=182838).

*/