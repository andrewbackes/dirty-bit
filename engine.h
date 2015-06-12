#ifndef engine_h
#define engine_h

#include "hash.h"
#include "board.h"
#include "book.h"
#include "move.h"

#include "parameters.h"

#include <vector>
#include <ctime>

struct GAME_INFO {

	std::string				event_name;
	std::string				site;
	std::string				date;
	std::string				time;
	int						round;
	std::string				result;

	long					time_control;
	long					move_control;

	std::string				opponent_name;
	int						opponent_elo;
};

class ITERATION_INFO {
public:
	//Results:
	MOVE					move;
	short					score;
	short					depth;
	short					seldepth;
	long long				nodes;
	long long				nps() { return ((nodes / (time_taken==0?1:time_taken))*1000); }
	std::vector<MOVE>		pv;

	//Aspiration windows:
	int						alpha		= -INFTY;
	int						beta		= INFTY;
	bool					failed		= false;

	//Time control:
	long					time_allotted;
	long					time_taken;
	bool					timed_out	= false;

};

class SEARCH_INFO {
public:
	//Results:
	MOVE						best_move();
	
	short						best_score();
	short						max_depth();
	short						max_seldepth();
	long long					nodes();		// total nodes over all iterations.
	long						nps();			// total nps over all iterations.
	std::vector<MOVE>			pv();
	
	//Iteration details:
	std::vector<ITERATION_INFO>	iteration_details;
	short						moves_to_go;

	//Overall Time control:
	long						time_allowed;	// total time left in the game.
	long						time_allotted;	// out of the total time, how much can we use for this search.
	long						time_safety;	// if things get messy, what will be our new time limit.
	long						start_time;
	long						time_lapsed();
	long						time_remaining()	{ return (time_allotted - time_lapsed()); }
};

class ENGINE {
	/*
		Each instance of ENGINE will be responsible for one game at a time.
		The hash tables will be linked to that game only.
		The records are also only for the currently played game
	*/
public:
	ENGINE() { }
	
	void						initialize();
	void						reset();
	void						precompute();
	
	void						save_pgn();
	void						load_pgn();
	

	void						think(SEARCH_INFO & s);
	bool						abort() { return abort_flag; }

	//Game play data:
	HASHTABLE					hashtable;
	PAWNTABLE					pawntable;
	CHESSBOARD					game;
	//BOOK						book;

	//Game record data:
	GAME_INFO					game_info;
	std::vector<SEARCH_INFO>	play_info;

	//settings:
	long long					hashtable_size;
	long long					pawntable_size;
	bool						debug = false;
	bool						ponder = false;
	bool						ownbook = false;

	//UCI specific settings:
	bool						UCI_ShowCurrLine = false;
	bool						UCI_ShowRefutations = false;
	bool						UCI_AnalyseMode = false;

private:
	bool abort_flag = false;

};

#endif