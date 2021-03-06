#ifndef debug_h
#define debug_h

#include "dirtybit.h"

#include <string>

namespace test {

	void perftsuite();
	void pawneval();
	void symmetry(CHESSBOARD * game);
	void bench(CHESSBOARD * game);

};

namespace print {

	void SEE(CHESSBOARD * b);
	void params();
	void captures(CHESSBOARD * b);
	void moves(CHESSBOARD * b);

};
/*
std::string debug_commands[] = { "perftsuite", "pawneval", "symmetry", "bench" };
void (*debug_functions[])() = { test::perftsuite, test::pawneval };
*/

#endif