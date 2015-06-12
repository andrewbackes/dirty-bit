#ifndef engine_h
#define engine_h

#include "dirtybit.h"
#include "hash.h"
#include "board.h"
#include "book.h"
#include "move.h"

#include "parameters.h"

#include <vector>

class ENGINE {
	/*
		Each instance of ENGINE will be responsible for one game at a time.
		The hash tables will be linked to that game only.
		The records are also only for the currently played game
	*/
public:
	ENGINE() { }

	//Game play data:
	HASHTABLE		hashtable;
	PAWNTABLE		pawntable;
	CHESSBOARD		active_game;
	BOOK			active_book;

	//Game record data:
	long			time_control;
	long			move_control;

	vector<MOVE>	move_record;
	vector<short>	evaluation_record;
	vector<short>	depth_record;
	string			opponent;

};

#endif