#ifndef uci_h
#define uci_h

#include <string>

#include "parameters.h"

extern int previous_score;

//void printMoves(CHESSBOARD * b);

//int UCI_loop();

class ITERATION_INFO;
class SEARCH_INFO;
class ENGINE;

namespace uci {
	
	void				send(std::string text);
	void				send(ITERATION_INFO iteration);
	void				send(SEARCH_INFO s);
	void				send_str(std::string text);

	int					uci(ENGINE* engine, std::string cmd_input);
	int					isready(ENGINE* engine, std::string cmd_input);
	
	int					go(ENGINE* engine, std::string cmd_input);
	int					newgame(ENGINE* engine, std::string cmd_input);
	int					position(ENGINE* engine, std::string cmd_input);
	int					setoption(ENGINE* engine, std::string cmd_input);
	
	int					debug(ENGINE* engine, std::string cmd_input);
	int					stop(ENGINE* engine, std::string cmd_input);
	int					ponderhit(ENGINE* engine, std::string cmd_input);
	
	int					move(ENGINE* engine, std::string cmd_input);
	int					unmove(ENGINE* engine, std::string cmd_input);

	static int			known_commands = 9;
	static std::string	command[] = { "uci", "isready", "go", "ucinewgame", "position", "setoption", "debug", "stop", "ponderhit" };
	static int			(*funtion[])(ENGINE* engine, std::string cmd_input) = { uci, isready, go, newgame, position, setoption, debug, stop, ponderhit };
	
};


#endif