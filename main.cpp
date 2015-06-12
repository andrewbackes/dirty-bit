// Andrew Backes
// December 2013
// DirtyBit - Chess Engine

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "parameters.h"
#include "uci.h"
#include "engine.h"

using namespace std;

bool DEBUG_FLAG = false;

int main(int argv, char* argc[]) 
{
	ENGINE engine;
	engine.initialize();
	engine.game.newgame();

	string user_input;
	string prompt = "Dirty-Bit> ";
	//Command lists:
	
	while(user_input != "q" && user_input != "quit") 
	{
		cout << prompt;
		getline(cin, user_input);
		stringstream command_stream(user_input);
		string command;
		command_stream >> command;

		//Check UCI commands:
		for (int i = 0; i < uci::known_commands; i++) {
			if (command == uci::command[i]) {
				//prompt = "UCI> ";
				prompt = "";
				int code = uci::funtion[i](&engine, user_input);
				break;
			}
		}
		//Check print commands:

		//Check test commands:

		//Help:
		if(user_input == "h" || user_input == "help")
		{
			cout << "\nSupported commands:" << endl;
			cout << " UCI Commands:\n";
			for (int i = 0; i < uci::known_commands; i++) cout << "   " << uci::command[i] << endl;
		}
	}
	return(0);
}