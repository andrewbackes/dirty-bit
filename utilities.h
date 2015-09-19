#ifndef utilities_h
#define utilities_h

#include <string>
#include <ctime>
#include <iostream>

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

std::string index_to_alg(int index);
int alg_to_index(std::string alg);
int str_to_int(std::string s);
unsigned long long floor_2n(unsigned long long l);

//bitboard Perft(CHESSBOARD * game, int depth);

#endif