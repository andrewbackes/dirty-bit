#ifndef uci_h
#define uci_h

#include <string>

#include "board.h"
#include "hash.h"
#include "book.h"

#include "parameters.h"

//const double ID_VERSION = 1.0;
//const std::string ID_NAME = "DirtyBit";
//const std::string ID_AUTHOR = "Andrew Backes"

extern int previous_score;

void printMoves(CHESSBOARD * b);

int uci_go(CHESSBOARD game, std::string go_string, std::string move_history);
void uci_newgame(CHESSBOARD* b, HASHTABLE * h);
std::string uci_position(CHESSBOARD * b, std::string cmd_input) ;
int UCI_loop();

#endif