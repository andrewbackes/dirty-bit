#ifndef utilities_h
#define utilities_h

#include <string>
#include <ctime>
#include <iostream>

#include "bitboards.h"
#include "board.h"
#include "move.h"

class TIMER {
public:
	void start()		{ start_time = clock()/(CLOCKS_PER_SEC/1000);	}
	void end()			{ end_time   = clock()/(CLOCKS_PER_SEC/1000);	}
	long time()			{ return end_time - start_time;					}
	//TIMER()			{ start();										}
private:
	long start_time;
	long end_time;
};

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

std::string index_to_alg(int index);
int alg_to_index(std::string alg);
int str_to_int(std::string s);

bitboard perft(CHESSBOARD * game, int depth);
//bitboard Perft(CHESSBOARD * game, int depth);

#endif