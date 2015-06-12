#ifndef uci_h
#define uci_h

#include <string>

#include "board.h"
#include "hash.h"
#include "book.h"

//#define DEBUG_TIME

extern int previous_score;

const double ID_VERSION = 1.0;
const std::string ID_NAME = "DirtyBit";
const std::string ID_AUTHOR = "Andrew Backes";

const int ASPIRATION_WINDOW = 50;
const int A_LOT = 100;
const int A_LITTLE = 15;

void printMoves(CHESSBOARD * b);

void uci_go(CHESSBOARD * game, std::string go_string, BOOK * book, std::string move_history);
void uci_newgame(CHESSBOARD* b, HASHTABLE * h);
std::string uci_position(CHESSBOARD * b, std::string cmd_input) ;
int UCI_loop();

#endif