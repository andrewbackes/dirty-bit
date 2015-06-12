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
	bitboard rook_masks[65];
	bitboard bishop_masks[65];
	int l;

	for(int i = 0; i <= 7; i++){
        for(int j = 0; j <= 7; j++){
            rook_masks[(8*j) + i] = 0;
            for(int k = 1; k <= 6; k++){
                if(k != i) rook_masks[(8*j) + i] += (1i64 << ((8*j) + k));  // Horizontal
                if(k != j) rook_masks[(8*j) + i] += (1i64 << ((8*k) + i));  // Vertical
            }

			bishop_masks[(8*j) + i] = 0;
            for(int k = i+1, l=j+1; max(k,l) <= 6; k++, l++) bishop_masks[(8*j)+i]+=(1i64<<((8*l)+k));        // Up, left
            for(int k = i+1, l=j-1; (k <= 6) & (l >= 1); k++, l--) bishop_masks[(8*j)+i]+=(1i64<<((8*l)+k));  // Down, left
            for(int k = i-1, l=j+1; (k >= 1) & (l <= 6); k--, l++) bishop_masks[(8*j)+i]+=(1i64<<((8*l)+k));  // Up, right
            for(int k = i-1, l=j-1; min(k, l) >= 1; k--, l--) bishop_masks[(8*j)+i]+=(1i64<<((8*l)+k));       // Down, right
            
            
        }
    }

	ofstream myfile;
	myfile.open ("output.txt");
	bitboard border = mask::file[a] | mask::file[h] | mask::rank[1] | mask::rank[8];

	myfile << "const bitboard MagicBishopMask[65] = {";
	for(int i =0; i < 64 ; i++) {
		myfile << bishop_masks[i] << ", ";
	}
	myfile << "0000000000000000000000000000000000000000000000000000000000000000";
	myfile << "};\n\n";

	myfile << "const bitboard MagicRookMask[65] = {";
	for(int i =0; i < 64 ; i++) {
		myfile << rook_masks[i] << ", ";
	}
	myfile << "0000000000000000000000000000000000000000000000000000000000000000";
	myfile << "};\n\n";
	
	myfile.close();
	//-----------------------------------------------------------
	*/

	/*
	cout << "const bitboard passed_pawn[2][64] = {";
	cout << "{\n";
	for(int i=0; i < 64; i++) {
		bitboard mask = mask::north[i];
		if( i % 8 < 7)
			mask |= mask::north[i+1];
		if( i % 8 > 0)
			mask |= mask::north[i-1];
		//bitprint(mask);
		cout << mask;
		if(i<63) cout << ",";
		if(i%3 == 0) cout << endl;
	}
	cout << "\n},\n{\n";
	for(int i=0; i < 64; i++) {
		bitboard mask = mask::south[i];
		if( i % 8 < 7)
			mask |= mask::south[i+1];
		if( i % 8 > 0)
			mask |= mask::south[i-1];
		//bitprint(mask);
		cout << mask;
		if(i<63) cout << ",";
		if(i%3 == 0) cout << endl;
	}
	cout << "};\n";
	*/

	//initializeMagicBishops();
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