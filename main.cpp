// Andrew Backes
// December 2013
// DirtyBit - Chess Engine

#include <iostream>
#include <string>

using namespace std;

#include "dirtybit.h"
#include "uci.h"

bool DEBUG_FLAG = false;

int main(int argv, char* argc[]) 
{
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