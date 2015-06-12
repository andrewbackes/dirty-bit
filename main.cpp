// Andrew Backes
// December 2013
// DirtyBit - Chess Engine

#include <iostream>
#include <string>
#include <fstream>

#include "dirtybit.h"

using namespace std;

bool DEBUG_FLAG = false;

int main(int argv, char* argc[]) 
{

	
	/*
	bitboard mask1 = (1i64 << b5) | (1i64 << b6) | (1i64 << c7);
	bitprint(mask1);
	bitboard mask2 = (1i64 << b4) | (1i64 << b3) | (1i64 << c2);
	bitprint(mask2);
	cout << "trapped_bishop_long[] = {" << mask1 << ", " << mask2 <<endl;

	bitboard mask3 = (1i64 << g5) | (1i64 << g6) | (1i64 << f7);
	bitprint(mask3);
	bitboard mask4 = (1i64 << g4) | (1i64 << g3) | (1i64 << f2);
	bitprint(mask4);
	cout << "trapped_bishop_short[] = {" << mask3 << ", " << mask4 << endl;
	*/
	/*
	bitboard mask1 = (1i64 << a1) | (1i64 << a2) | (1i64 << b1);
	bitprint(mask1);
	bitboard mask2 = (1i64 << a8) | (1i64 << a7) | (1i64 << b8);
	bitprint(mask2);
	cout << "trapped_rook_long[] = {" << mask1 << ", " << mask2 << " };" << endl;
	mask1 = (1i64 << b1) | (1i64 << c1);
	bitprint(mask1);
	mask2 = (1i64 << b8) | (1i64 << c8);
	bitprint(mask2);
	cout << "trapped_rook_long_king[] = { " << mask1 << ", " << mask2 << " };" << endl;

	mask1 = (1i64 << h1) | (1i64 << h2) | (1i64 << g1);
	bitprint(mask1);
	mask2 = (1i64 << h8) | (1i64 << h7) | (1i64 << g8);
	bitprint(mask2);
	cout << "trapped_rook_short[] = {" << mask1 << ", " << mask2 << " };" << endl;
	mask1 = (1i64 << f1) | (1i64 << g1);
	bitprint(mask1);
	mask2 = (1i64 << f8) | (1i64 << g8);
	bitprint(mask2);
	cout << "trapped_rook_short_king[] = { " << mask1 << ", " << mask2 << " };" << endl;
	*/

	precompute::rays();
	precompute::shelter_scores();
	precompute::rule_of_the_squares();
	precompute::passed_pawns();
	precompute::material_adjustments();
	//precompute::castle_areas();
	//precompute::castle_pawns();
	populateMagicBishopArray(); 
	populateMagicRookArray();

	
	string user_input;
	//UCI_loop();

	while(user_input != "q" && user_input != "quit") 
	{
		cout << "Command? (q to quit, h for help): ";
		cin >> user_input;
		if(user_input == "uci")
		{
			//specify to use UCI. Advance to the UCI_loop
			if( UCI_loop() == 0)
				return(0);
		}
		else if(user_input == "h" || user_input == "help")
		{
			cout << "\nSupported commands:" << endl;
			cout << "	uci" << endl;
			cout << "	quit" << endl;
			cout << endl;
		}
	}
	return(0);
}