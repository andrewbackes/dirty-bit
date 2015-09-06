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
	bitboard mask1 = (((bitboard)1) << b5) | (((bitboard)1) << b6) | (((bitboard)1) << c7);
	bitprint(mask1);
	bitboard mask2 = (((bitboard)1) << b4) | (((bitboard)1) << b3) | (((bitboard)1) << c2);
	bitprint(mask2);
	cout << "trapped_bishop_long[] = {" << mask1 << ", " << mask2 <<endl;

	bitboard mask3 = (((bitboard)1) << g5) | (((bitboard)1) << g6) | (((bitboard)1) << f7);
	bitprint(mask3);
	bitboard mask4 = (((bitboard)1) << g4) | (((bitboard)1) << g3) | (((bitboard)1) << f2);
	bitprint(mask4);
	cout << "trapped_bishop_short[] = {" << mask3 << ", " << mask4 << endl;
	*/
	/*
	bitboard mask1 = (((bitboard)1) << a1) | (((bitboard)1) << a2) | (((bitboard)1) << b1);
	bitprint(mask1);
	bitboard mask2 = (((bitboard)1) << a8) | (((bitboard)1) << a7) | (((bitboard)1) << b8);
	bitprint(mask2);
	cout << "trapped_rook_long[] = {" << mask1 << ", " << mask2 << " };" << endl;
	mask1 = (((bitboard)1) << b1) | (((bitboard)1) << c1);
	bitprint(mask1);
	mask2 = (((bitboard)1) << b8) | (((bitboard)1) << c8);
	bitprint(mask2);
	cout << "trapped_rook_long_king[] = { " << mask1 << ", " << mask2 << " };" << endl;

	mask1 = (((bitboard)1) << h1) | (((bitboard)1) << h2) | (((bitboard)1) << g1);
	bitprint(mask1);
	mask2 = (((bitboard)1) << h8) | (((bitboard)1) << h7) | (((bitboard)1) << g8);
	bitprint(mask2);
	cout << "trapped_rook_short[] = {" << mask1 << ", " << mask2 << " };" << endl;
	mask1 = (((bitboard)1) << f1) | (((bitboard)1) << g1);
	bitprint(mask1);
	mask2 = (((bitboard)1) << f8) | (((bitboard)1) << g8);
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