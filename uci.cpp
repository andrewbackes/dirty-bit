// Code for allowing the engine to interact with the UCI protocol.

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <vector>
#include <list>
#include <ctime>
#include <cmath>
#include <functional>

#include "uci.h"

#include "bitboards.h"
#include "board.h"
#include "move.h"
#include "utilities.h"
#include "search.h"
#include "movegen.h"
#include "book.h"
#include "SEE.h"
#include "perft.h"
#include "evaluate.h"
#include "precompute.h"
#include "debug.h"

using namespace std;

int previous_score;

long hashetable_size; //in mb
HASHTABLE hashtable;

/*
bool fexists(string filename) {
	//Returns true if file exists.
	ifstream ifile(filename);
	return ( (bool) ifile );
}
*/



void benchmark(CHESSBOARD b, int depth, string label) {
/*
	//Filename by number:
	string filename;
	for(int i = 1; i <=1000; i++) {
		ostringstream convert;
		convert << i;

		filename = "benchmark_";
		filename.append(convert.str());
		filename.append(".txt");
		if(! fexists(filename) )
			break;
	}

	//Create the file.
	ofstream file_out;
	file_out.open(filename);
	file_out << label << endl;
	file_out.close();

	while(true) {
		file_out.open(filename, ios::out|ios::app); //append the file.

		SEARCH search = SEARCH(depth,&b,&hashtable);
		search.start();

		if(b.getActivePlayer() == WHITE) {
			cout << "White moves ";
			file_out << "White moves ";}
		else {
			cout << "Black moves ";
			file_out << "Black moves "; }
		search.getBestMove().print();
		cout << endl;
		file_out << index_to_alg(search.getBestMove().from) << index_to_alg(search.getBestMove().to) << " ";

		b.MakeMove(search.getBestMove());
		int score = -evaluate(&b);

		//console:
		cout << "--> info depth " << search.getDepth() << " nodes " << search.getNodes() << " score cp " << score << " time " << search.getRunTime() << " nps " << search.getNPS() << " pv " ;
		//search.getBestMove().print();
		/*
		for(int i=0; i < depth; i++) {
			if( search.pv[i].from == 64 ) {
				cout << "(HT) ";
				break;
			}
			else
				search.pv[i].print();
		}
		*/
	/*
		cout << endl << endl;

		//file:
		file_out << "--> info depth " << search.getDepth() << " nodes " << left << setw(10) << search.getNodes() << "score cp " << setw(10) << score << "time " << setw(10) << search.getRunTime() << "nps " << setw(10) << search.getNPS() << "pv " ;
		//file_out << index_to_alg(search.getBestMove().from) << index_to_alg(search.getBestMove().to) << " ";
		/*
		for(int i=0; i < depth; i++) {
			if( search.pv[i].from == 64 ) {
				file_out << "(HT) ";
				break;
			}
			else
				file_out << index_to_alg(search.pv[i].from) << index_to_alg(search.pv[i].to) << " ";
		}
		*/
	/*
		file_out << endl;

		file_out.close();

		if(search.getBestMove().from == 64)
			break;
	}
*/
};


void uci_setoption(string cmd_input) {
	//ex: setoption name Hash value 32

	stringstream command_stream(cmd_input);
	string parameter;
	string name;
	string value;

	command_stream >> parameter;
	command_stream >> parameter;
	if(parameter == "name")
		command_stream >> name;

	command_stream >> parameter;
	if(parameter == "value")
		command_stream >> value;

	//Go through the supported options:
	if(name == "Hash" || name == "hash" || name == "HASH") {
		hashetable_size = atoi(value.c_str());
		cout << "Set " << name << " to " << hashetable_size << endl;
	}
}

void parse_go_command( string go_string, bool player,
	bool *ponder,
	long *time_on_clock, long *time_increment, long *moves_to_go) {

	//Parse the info from the string:
	stringstream command_stream(go_string);
	string command;
	string value;
	command_stream >> command; //this should say "go"
	while( command_stream >> command ) {
		if(command == "ponder") {
			*ponder = true;
		} else {
			command_stream >> value;
		}

		if ( command == "wtime" && player == WHITE ){
			*time_on_clock = str_to_int(value);
		}
		else if(command == "btime" && player == BLACK ) {
			*time_on_clock = str_to_int(value);
		}
		else if(command == "winc" && player == WHITE ) {
			*time_increment = str_to_int(value);
		}
		else if(command == "binc" && player == BLACK ) {
			*time_increment = str_to_int(value);
		}
		else if(command == "movestogo") {
			*moves_to_go = str_to_int(value);
		}
		else if(command == "infinite" || command == "INFINITE") {
			//give 1 hour for each move.
			*time_on_clock = (long)3600000;
		}
	}
	#ifdef DEBUG_TIME
		cout << "\tTime on clock: " << *time_on_clock << endl;
	#endif
}

long get_time_for_move( long time_on_clock, long moves_to_go, long moves_completed ) {

	// Crude time management:
	double time_factor = 2 -  moves_completed / 10; // TODO: change this to moves out of book.
	double time_target = time_on_clock  / moves_to_go;
	long time_allotted   = (long)( time_factor * time_target ) - 10;
	time_allotted = min(time_allotted, time_on_clock - 10 );

	//Safety hack so not to use all of the remaining time if there is only 1 move left:
	if (moves_to_go == 1)
		time_allotted = (long)(time_allotted * 0.75);

	#ifdef DEBUG_TIME
		cout << "\tTime allotted for move: " << time_allotted << endl;
	#endif
	
	return time_allotted;
}

uint64 get_check_count_on_node( long start_time, long time_for_move, uint64 total_node_count ) {
	long time_lapsed = clock()/(CLOCKS_PER_SEC/1000) - start_time;
	time_lapsed = time_lapsed>0 ? time_lapsed : 1;
	
	long running_nps = (long)floor((double)total_node_count / ((double)time_lapsed/1000));
	running_nps = (running_nps>0)? running_nps : DEFAULT_NPS;
	
	long time_left = time_for_move - time_lapsed;
	time_left = time_left > 1 ? time_left : 1; 
	
	uint64 check_time_on_node = floor_2n( ((time_left) * running_nps/1000) )/8 - 1;
	if(check_time_on_node <= 1) check_time_on_node = 1024 - 1;
	#ifdef DEBUG_TIME
		cout << "\tCheck time every " << check_time_on_node << " nodes.";
		cout << "\tRunning NPS: " << running_nps;
		cout << endl;
	#endif
	return check_time_on_node;
}

void uci_print_search( const SEARCH & search, int alpha_window, int beta_window ) {
	vector<MOVE> pv;
	//Print the UCI info:
	cout 	<< "info depth " << search.getDepth()
			<< " seldepth " << search.getSelDepth() //search.getDepth() + search.getQDepth()
			<< " nodes " << search.getNodes()
			<< " score";
	//Print Mate in X:
	if (search.getScore() >= MATE - 1000) {
		cout << " mate " << (MATE - search.getScore() + 1) / 2;
	} else if (search.getScore() <= -MATE + 1000) {
		cout << " mate -" << (MATE + search.getScore()) / 2;
	} else {
		cout << " cp " << search.getScore(); 
	}
	
	if ( search.getScore() <= alpha_window ) {
		cout << " lowerbound ";
	} else if ( search.getScore() >= beta_window ) {
		cout << " upperbound ";
	}
	
	cout << " time " << search.getRunTime() <<
			" nps " << search.getNPS() <<
			" pv " ;
	//Print the PV:
	pv = search.getPV();
	for(int i=0; i < pv.size(); i++) {
		if( pv.at(i).from != 64 ) {
			pv.at(i).print();
		}
		else
			break;
	}
	cout << endl;
}

void set_aspiration_window( int previous_depth_score, int fail_low_count, int fail_high_count, 
	int * alpha_window, int * beta_window ) {
	
	switch ( fail_low_count ) {
		case 0:
			*alpha_window = previous_depth_score - ASPIRATION_WINDOW;
			break;
		case 1:
			*alpha_window = previous_depth_score - ASPIRATION_WINDOW - A_LOT;
			break;
		default:
			*alpha_window = -INFTY; 
			break;
	}
	switch ( fail_high_count ) {
		case 0:
			*beta_window = previous_depth_score + ASPIRATION_WINDOW;
			break;
		case 1:
			*beta_window = previous_depth_score + ASPIRATION_WINDOW + A_LOT;
			break;
		default:
			*beta_window = INFTY; 
			break;
	}
	#ifdef DEBUG_TIME
		cout << "\tAspiration window: [" << *alpha_window << ", " << *beta_window << "]" << endl;
	#endif
}

bool terminated_normally( const SEARCH & search, int fail_low_count, int fail_high_count ) {
	return ( !search.TimedOut() && !fail_low_count && !fail_high_count );
}

bool enough_time_left( long average_EBF, long previous_node_count, long running_node_count, long start_time, long time_for_move ) {
	long time_lapsed = clock()/(CLOCKS_PER_SEC/1000) - start_time;
	time_lapsed = time_lapsed>0 ? time_lapsed : 1;
	
	long running_nps = (long)floor((double)running_node_count / ((double)time_lapsed/1000));
	running_nps = (running_nps>0)? running_nps : DEFAULT_NPS;
	
	long time_needed = (long) ceil( (average_EBF * (double)previous_node_count ) / (double)running_nps * 1000 );
	time_needed = (time_needed > 0) ? time_needed : 1;
	
	return ( time_lapsed + time_needed < time_for_move );
}

void update_move_choice( const SEARCH & search, int * best_score, MOVE * best_move, MOVE * ponder_move) {
	*best_move = search.getBestMove();
	*best_score = search.getScore();
	vector<MOVE> pv = search.getPV();
	if( pv.size() >= 2 && pv[1].from != 64)
		*ponder_move = pv[1];
}

void uci_print_time_stats( long start_time, long time_for_move, long total_node_count ) {
	long time_lapsed = clock()/(CLOCKS_PER_SEC/1000) - start_time;
	cout << "\tTime Lapsed: " << time_lapsed << " of " << time_for_move << endl;
	cout << "\tTotal Node Count:  " << total_node_count << endl;
	long running_nps = (long)floor((double)total_node_count / ((double)time_lapsed/1000)); 
	cout << "\tRunning NPS: " << running_nps << endl;
}

void uci_print_bestmove( MOVE best_move, MOVE ponder_move ) {
	cout << "bestmove ";
	best_move.print();
	if(ENABLE_PONDERING && ponder_move.from != 64) {
		cout << "ponder ";
		ponder_move.print();
	}
	cout << endl;
}

bool can_force_research( bool failed, long fail_low_count, long fail_high_count,
	long start_time, long time_for_move, long time_on_clock  ) {
	
	long time_lapsed = clock()/(CLOCKS_PER_SEC/1000) - start_time;
	time_lapsed = time_lapsed>0 ? time_lapsed : 1;
	
	bool timed_out = (time_lapsed >= time_for_move);
	
	if( !timed_out && failed ) {
		return true;
	}
	
	// out of allotted time but failed high and low. this is a risky situation,
	// so check if we have time left on the clock.
	if ( timed_out && fail_low_count && fail_high_count ) {
		if ( time_on_clock/5 > time_lapsed  ) {
			#ifdef DEBUG_TIME
				cout << "\tForcing research." << endl;
			#endif
			return true;
		} else {
			#ifdef DEBUG_TIME
				cout << "\tNot enough time to force research." << endl;
			#endif
			return false;
		}
	}
	return false;
}

void update_iteration_stats( const SEARCH & search, int alpha_window, int beta_window, 
		long * total_node_count, int * fail_low_count, int * fail_high_count ) {
	
	*total_node_count += search.getNodes();
	if ( search.getScore() <= alpha_window ) {
		(*fail_low_count)++;
	} else if ( search.getScore() >= beta_window ) {
		(*fail_high_count)++;
	}
}

bool search_failed( const SEARCH & search, int alpha_window, int beta_window ) {
	return (search.getScore() <= alpha_window ) || ( search.getScore() >= beta_window );
}

long get_allowed_search_time( long time_on_clock, long time_for_move, long start_time, long moves_to_go,
		int fail_low_count, int fail_high_count ) {
	
	long time_lapsed  = clock()/(CLOCKS_PER_SEC/1000) - start_time;
	long time_left = time_for_move - time_lapsed;
	bool out_of_time = (time_left <= 0);
	
	long time_allotted = 1;
	time_allotted = (long)floor(time_left * ((moves_to_go == 1)? 1:TIME_WIGGLE_ROOM));
	// failing both high and low is risky, so maybe we should search longer.
	if( fail_low_count && fail_low_count ) {
		time_allotted = (time_on_clock - time_lapsed) / 5;	
	}
	
	return time_allotted;
}

void uci_print_resign() {
	cout << "bestmove 0000" << endl;
}

int uci_go(CHESSBOARD * game, string go_string, BOOK * book, string move_history) {
	long start_time = clock()/(CLOCKS_PER_SEC/1000);
    
	// constraints:
	bool ponder = false;
	long time_on_clock = 10, time_increment = 0, moves_to_go = 30;

	parse_go_command( go_string, game->getActivePlayer(), &ponder, &time_on_clock, &time_increment, &moves_to_go );

	// figure out how much time should be allowed for this move:
	long time_for_move = get_time_for_move( time_on_clock, moves_to_go, game->getMoveCount() );
	
	// Stats:
	double average_EBF = DEFAULT_EBF;
	long total_node_count = 1, previous_node_count = 1, previous_depth_score = 0;
	
	// Do depth 1 seperately:
	SEARCH shallow_search( 1, game, &hashtable, time_on_clock - 10, 1023 );
	shallow_search.start( -INFTY, INFTY );
	MOVE best_move = shallow_search.getBestMove();
	MOVE ponder_move; // maybe not so good to not explicitly initialize this.
	int best_score = shallow_search.getScore();
	
	// check if the game is over:
	if ( best_move.from == 64 && best_move.to == 64 ) {
		uci_print_resign();
		return 0;
	}
	
	//Iterative Deepening:
	int depth = 2;
	for ( ; depth < DEPTH_CUTOFF; depth++) {
		bool search_complete = false;
		// Check if we have time to do this iteration
		if ( ! enough_time_left( average_EBF, previous_node_count, total_node_count, start_time, time_for_move ) 
		|| ((clock()/(CLOCKS_PER_SEC/1000) - start_time) >= time_for_move) ) {
			break;
		}
		// Search this depth until we have not failed high or low:
		int fail_high_count = 0, fail_low_count = 0;
		while ( ! search_complete ) {
			int alpha_window = -INFTY, beta_window = INFTY;
			// Pick aspiration window:
			set_aspiration_window( previous_depth_score, fail_low_count, fail_high_count, 
				&alpha_window, &beta_window );
			long allowed_search_time = get_allowed_search_time( time_on_clock, time_for_move, start_time, moves_to_go,
				fail_low_count, fail_high_count );
			uint64 check_time_on_node = get_check_count_on_node( start_time, time_for_move, (uint64)total_node_count );
			// Do the search:
			SEARCH search( depth, game, &hashtable, allowed_search_time, check_time_on_node );
			search.start( alpha_window, beta_window );
			// Update stats required for iterative deepening:
			update_iteration_stats( search, alpha_window, beta_window, &total_node_count, &fail_low_count, &fail_high_count );
			// Print search results to the screen:
			if ( !search.TimedOut() ) uci_print_search( search, alpha_window, beta_window );
			// Handle a possible failed search:
			bool failed = search_failed( search, alpha_window, beta_window );
			if ( !search.TimedOut() && !failed ) {
				// search finished normally
				search_complete = true;
				update_move_choice( search, &best_score, &best_move, &ponder_move );
				previous_depth_score = search.getScore();
			} else if ( ! can_force_research( failed, fail_low_count, fail_high_count, start_time, time_for_move, time_on_clock ) ) {
				search_complete = true;
			}
		}
	}
	#ifdef DEBUG_TIME
		uci_print_time_stats( start_time, time_for_move, total_node_count );
	#endif
	uci_print_bestmove( best_move, ponder_move );
	return depth;
}

/*
// TODO: make more descriptive variable names:
//			- time_on_clock
//			- time_for_move
//			- time_for_iteration
int uci_go(CHESSBOARD * game, string go_string, BOOK * book, string move_history) {
// Uses Iterative Deepening to pick a best move.
// Then prints it out in UCI format.

//		TODO:	- Change the time allocation to something like: time_left / (n - kx/(x^2  +400)), x moves so far, k the fraction
//				  --> see graph: http://chessprogramming.wikispaces.com/Time+Management
//						{(1,1),(2,1),(3,1),(4,1),(5,1),(6,1),(7,1),(8,1),(9,2),(10,2),(11,3),(12,3),(13,4),(14,5),(15,6),(16,9),(17,12),(18,16),(19,16),(20,15),(21,14),(22,13),(23,12), (24,11), (25,9),(26,5),(27,4),(28,4),(29,4),(30,3),(31,3),(32,3),(33,3),(34,3),(35,3),(36,3),(37,2),(38,2),(39,2),(40,2),(41,2),(42,2),(43,2),(44,2),(45,1),(46,1),(47,1),(48,1),(49,1),(50,1),(51,1),(52,1),(53,1),(54,1),(55,1),(56,1)}


	long start_time = clock()/(CLOCKS_PER_SEC/1000);

	// Hard cutoff to prevent needless searches that just eat up all of the time:
	int depth_cutoff = 100;

	// constraints:
	bool ponder = false;
	long time_on_clock = 0, long time_increment = 0, long moves_to_go = 30;

	parse_go_command( go_string, game->getActivePlayer(), &ponder, &time_on_clock, &time_increment, &moves_to_go );

	// figure out how much time should be allowed for this move:
	long time_for_move = get_time_for_move( time_on_clock, , moves_to_go, game->getMoveCount() );
	cout << "Allotted time: " << time_for_move << endl;
	long time_left = time_for_move, long time_lapsed = 0;

	//Begin deciding which move to make:

	//Iterative Deepening:
	MOVE best_move;
	MOVE ponder_move;
	short best_score;

	int alpha_window = -INFTY;
	int beta_window = INFTY;
	int window_range = ASPIRATION_WINDOW;

	int fail_high_count = 0;
	int fail_low_count = 0;
	bool fQuitOnNext = false;

	int depth = 1;
	double * EBF = new double[depth_cutoff+1];
	double average_EBF = DEFAULT_EBF;
	int * node_count = new int[depth_cutoff+1];
	long running_nps = DEFAULT_NPS;
	long running_node_count = 0;
	long previous_node_count = 0;

	previous_score = 0;

	while( depth <= depth_cutoff ) {
		vector<MOVE> pv;
		bool fFailHigh = false;
		bool fFailLow = false;

		uint64 check_time_on_node = get_check_count_on_node( (uint64)time_left, (uint64)running_nps );
		
		SEARCH search( depth, game, &hashtable, (long)floor(time_left * ((moves_till_time_control == 1)? 1:TIME_WIGGLE_ROOM)), check_time_on_node );
		//Aspiration window:
		string fFailed = "";
		#ifdef DEBUG_TIME
			cout << "Next window: [" << alpha_window <<", " << beta_window << "]" << endl;
		#endif
		search.start(alpha_window, beta_window);

		if(search.getScore() <= alpha_window) {
			//failed low:
			fail_low_count++;
			fFailLow = true;
			if(fail_low_count < 2) {
				alpha_window = alpha_window - A_LOT;
			}
			else {
				alpha_window = -INFTY;
			}
			//alpha_window = -INFTY;
			//beta_window = search.getScore() + A_LITTLE;
			fFailed = " lowerbound ";
		}
		else if(search.getScore() >= beta_window) {
			//failed high:
			fail_high_count++;
			fFailHigh = true;
			//beta_window = INFTY;
			if(fail_high_count < 2) {
				beta_window = beta_window + A_LOT;
			}
			else {
				beta_window = INFTY;
			}
			//alpha_window = search.getScore() - A_LITTLE;
			fFailed = " upperbound ";
		}
		else {
			previous_node_count = search.getNodes();
			node_count[depth] = search.getNodes();
		}
		running_node_count += search.getNodes();

		if(!search.TimedOut()) {
			//Print the UCI info:
			std::cout	<< "info depth " << search.getDepth()
						<< " seldepth " << search.getSelDepth() //search.getDepth() + search.getQDepth()
						<< " nodes " << search.getNodes()
						<< " score";
			//Print Mate in X:
			if (search.getScore() >= MATE - 1000)
				cout << " mate " << (MATE - search.getScore() + 1) / 2;
			else if (search.getScore() <= -MATE + 1000)
				cout << " mate -" << (MATE + search.getScore()) / 2;
			else
				cout << " cp " << search.getScore();
			cout << fFailed <<
					" time " << search.getRunTime() <<
					" nps " << search.getNPS() <<
					" pv " ;
			//Print the PV:
			pv = search.getPV();
			for(int i=0; i < pv.size(); i++) {
				if( pv.at(i).from != 64 ) {
					pv.at(i).print();
				}
				else
					break;
			}
			cout << endl;
			//Save the results if it didnt time out or fail
			if(!fFailHigh && !fFailLow) {
				best_move = search.getBestMove();
				best_score = search.getScore();
				if(pv[1].from != 64)
					ponder_move = pv[1];
				if(fQuitOnNext)
					break;
			}
		}
		else {
			//Stop immediately because it is too dangerous to continue.
			if(moves_till_time_control <= 2)
				break;

			#ifdef DEBUG_TIME
				cout << "\tSearch timed out." << endl;
			#endif
		}

		time_lapsed = clock()/(CLOCKS_PER_SEC/1000) - start_time;
		time_lapsed = (time_lapsed>0) ? time_lapsed : 1;
		running_nps = (long)floor((double)running_node_count / ((double)time_lapsed/1000));
		running_nps = (running_nps>0)? running_nps : DEFAULT_NPS;

		long time_needed = (long) ceil( ( (average_EBF * (double)previous_node_count ) / (double)running_nps) * 1000 );
		time_needed = (time_needed > 0) ? time_needed : 1;
		#ifdef DEBUG_TIME
			cout << "\tEstimated time needed: " << time_needed << "/" << time_allotted - time_lapsed << ". ";
		#endif
		time_left = time_allotted - time_lapsed;

		if( time_lapsed + time_needed >= time_allotted ) {
			//Out of time but we have a good best move.
			if(!fFailHigh && !fFailLow)
			{
				#ifdef DEBUG_TIME
					cout << "\n\tNot enough time to search deeper. But did not fail high or low." << endl;
				#endif
				break;
			}
			//Out of time but the last search was a fail.
			else {
				if(time_remaining[game->getActivePlayer()]/5 > time_needed) {
					alpha_window = -INFTY;
					beta_window = INFTY;
					time_left = time_remaining[game->getActivePlayer()]/5;
					fQuitOnNext = true;
				}
				else {
					#ifdef DEBUG_TIME
						cout << "\tCan not give more time." << endl;
					#endif
					break;
				}
			}
		}

		//Adjust the aspiration window:
		if(!fFailHigh && !fFailLow) {
			depth++;
			alpha_window = best_score - window_range;
			beta_window = best_score +  window_range;
			fail_low_count = 0;
			fail_high_count = 0;
			fQuitOnNext = false;
			previous_score = search.getScore();
		}
	}
	cout << "\tTime Lapsed: " << time_lapsed << "/" << time_allotted << endl;
	#ifdef DEBUG_TIME
	cout << "\tNode Count:  " << running_node_count << endl;
	cout << "\tRunning NPS: " << running_nps << endl;
	#endif
	//Finally print the move chosen:
	std::cout << "bestmove ";
	best_move.print();
	if(ENABLE_PONDERING && ponder_move.from != 64) {
		cout << "ponder ";
		ponder_move.print();
	}
	std::cout << endl;

	return depth;

}
*/

void uci_search(CHESSBOARD * game, int depth) {
	//Specific depth search.

	SEARCH search(depth, game, &hashtable, INFTY, NODES_FOR_TIME_CHECK);
	search.start();

	//Print the UCI info:
	std::cout	<< "info depth " << search.getDepth()
				<< " nodes " << search.getNodes()
				<< " score cp " << search.getScore();
		//Print Mate in X:
	if (search.getScore() >= MATE - 1000)
		cout << " mate " << (MATE - search.getScore() + 1) / 2;
	else if (search.getScore() <= -MATE + 1000)
		cout << " mate -" << (MATE + search.getScore()) / 2;
	std::cout << " time " << search.getRunTime() <<
			" nps " << search.getNPS() <<
			" pv " ;
	//Print the PV:
	vector<MOVE> pv = search.getPV();
	for(int i=0; i < pv.size(); i++) {
		if( pv.at(i).from != 64 ) {
			pv.at(i).print();
		}
		else
			break;
	}
	//Finally print the move chosen:
	cout << "\nbestmove ";
	search.getBestMove().print();
	cout << "\n";

}

void uci_iterativeDeepening(CHESSBOARD * game, int depth) {

	long start_time = clock()/(CLOCKS_PER_SEC/1000);

	MOVE best_move;
	int best_score = -INFTY;

	int alpha_window = -INFTY;
	int beta_window = INFTY;
	int window_range = ASPIRATION_WINDOW;

	int d=1;

	previous_score = 0;
	while(d <= depth)
	{
		SEARCH search(d, game, &hashtable, INFTY, NODES_FOR_TIME_CHECK);

		//Aspiration window:
		string fFailed = "";
		bool fFailedLow = false;
		bool fFailedHigh = false;
		search.start(alpha_window, beta_window);
		if(search.getScore() <= alpha_window) {
			//failed low:
			fFailedLow = true;
			alpha_window = -INFTY;
			fFailed = " lowerbound ";
		}
		else if(search.getScore() >= beta_window) {
			//failed high:
			fFailedHigh = true;
			beta_window = INFTY;
			fFailed = " upperbound ";
		}

		//Print the UCI info:
		std::cout	<< "info depth " << search.getDepth()
					<< " seldepth " << search.getSelDepth() //search.getDepth() + search.getQDepth()
					<< " nodes " << search.getNodes()
					<< " score cp " << search.getScore();
		//Print Mate in X:
		if (search.getScore() >= MATE - 1000)
			cout << " mate " << (MATE - search.getScore() + 1) / 2;
		else if (search.getScore() <= -MATE + 1000)
			cout << " mate " << (MATE + search.getScore()) / 2;
		cout << fFailed  <<
				" time " << search.getRunTime() <<
				" nps "  << search.getNPS() <<
				" pv " ;
		//Print the PV:
		vector<MOVE> pv = search.getPV();
		for(int i=0; i < pv.size(); i++) {
			pv.at(i).print();
		}
		cout << endl;

		//Adjust the aspiration window:
		if(!fFailedLow && !fFailedHigh) {
			d++;
			best_move = search.getBestMove();
			best_score = search.getScore();
			alpha_window = best_score - window_range;
			beta_window = best_score +  window_range;
			previous_score = search.getScore();
		}

	}
	cout << "total time: " << (clock()/(CLOCKS_PER_SEC/1000)) - start_time << endl;
	//Finally print the move chosen:
	cout << "bestmove ";
	best_move.print();
	cout << "\n";
}

void uci_newgame(CHESSBOARD* b, HASHTABLE * h) {
	b->newgame();
	h->reinitialize(hashetable_size);
}

string uci_position(CHESSBOARD * b, string cmd_input) {
	stringstream command_stream(cmd_input);
	string parameter;
	string move_history = "";

	while( command_stream >> parameter) {
		if(parameter == "fen"){
			string fen = "";
			string fen_param;
			command_stream >> fen;
			int i=0;
			while(command_stream >> fen_param && i<=5) {
				fen.append(" ");
				fen.append(fen_param);
				i++;
			}
			b->position(fen);
		}
		else if(parameter == "startpos") {
			string startpos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
			b->position(startpos);
		}
		else if(parameter == "moves") {
			string move;
			while(command_stream >> move) {
				if(move_history != "")
					move_history.append(" ");
				move_history.append(move.substr(0,4));

				int move_from = alg_to_index(move.substr(0,2));
				int move_to = alg_to_index(move.substr(2,2));

				string promote_to = move.substr(4,1);
				int promote_piece_id = 0;
				if(promote_to == "q" || promote_to == "Q")
					promote_piece_id = nQueen;
				else if(promote_to == "r" || promote_to == "R")
					promote_piece_id = nRook;
				else if(promote_to == "b" || promote_to == "B")
					promote_piece_id = nBishop;
				else if(promote_to == "n" || promote_to == "N")
					promote_piece_id = nKnight;

				//Finally make the move:
				b->MakeMove(MOVE(
					move_from,
					move_to,
					b->getActivePlayer(),
					b->getBoard(move_from),
					b->getBoard(move_to),
					promote_piece_id					));
			}
		}
	}
	return move_history;
}



void uci_move(CHESSBOARD * b, string cmd_input) {
	stringstream command_stream(cmd_input);
	string move;

	command_stream >> move; //should say "move"
	command_stream >> move; //should be something like "e2e4"

	int move_from = alg_to_index(move.substr(0,2));
	int move_to = alg_to_index(move.substr(2,2));

	string promote_to = move.substr(4,1);
	int promote_piece_id = 0;
	if(promote_to == "q" || promote_to == "Q")
		promote_piece_id = nQueen;
	else if(promote_to == "r" || promote_to == "R")
		promote_piece_id = nRook;
	else if(promote_to == "b" || promote_to == "B")
		promote_piece_id = nBishop;
	else if(promote_to == "n" || promote_to == "N")
		promote_piece_id = nKnight;

	//Finally make the move:
	b->MakeMove(MOVE(
		move_from,
		move_to,
		b->getActivePlayer(),
		b->getBoard(move_from),
		b->getBoard(move_to),
		promote_piece_id					));
}

void uci_unmove(CHESSBOARD * b, string cmd_input) {
	//Will not undo captures!
	stringstream command_stream(cmd_input);
	string move;

	command_stream >> move; //should say "unmove"
	command_stream >> move; //should be something like "e2e4"

	int move_from = alg_to_index(move.substr(0,2));
	int move_to = alg_to_index(move.substr(2,2));

	string promote_to = move.substr(4,1);
	int promote_piece_id = 0;
	if(promote_to == "q" || promote_to == "Q")
		promote_piece_id = nQueen;
	else if(promote_to == "r" || promote_to == "R")
		promote_piece_id = nRook;
	else if(promote_to == "b" || promote_to == "B")
		promote_piece_id = nBishop;
	else if(promote_to == "n" || promote_to == "N")
		promote_piece_id = nKnight;

	//Finally make the move:
	b->unMakeMove(MOVE(
		move_from,
		move_to,
		!b->getActivePlayer(),
		b->getBoard(move_to),
		0,
		promote_piece_id					));
}



int UCI_loop()
{
	//Once in UCI mode, will only accept UCI commands.
	string cmd_input;

	//Start initializing the engine:
	cout << "id name " << ID_NAME << " v" << ID_VERSION << "\n";
	cout << "id author " << ID_AUTHOR << "\n";
	//cout << "option name Ponder type check default " << ENABLE_PONDERING << "\n";
	//cout << "option name OwnBook type check default true\n";
	//cout << "option name Hash type spin default " << HASH_TABLE_SIZE << " min 1000 max 1000\n";

	int depth = 12;
	hashetable_size = HASH_TABLE_SIZE; //in mb

	CHESSBOARD active_game; //will store the active game to search.
	string active_game_history = "";
	uci_newgame(&active_game,&hashtable);
	BOOK active_book;

	//active_book.load();
	cout << "uciok\n";
	//uci_position(&active_game, "position startpos moves b1c3 g8f6 e2e4 e7e5 g1f3 b8c6 f1c4 f6e4 c3e4 d7d5 c4d3 d5e4 d3e4 f8d6 e1g1 c8g4 f1e1 e8g8 h2h3 g4f3 e4f3 a8b8 c2c3 d8f6 d2d3 d6c5 c1d2 a7a6 d1c2 f8e8 e1e2 c5b6 a1e1 e8e6 f3e4 h7h6 d2c1 c6e7 b2b4 e7g6 e4g6 f7g6 e2e4 b8e8 e1e2 g6g5 c2b3 g8h7 c1a3 f6f5 d3d4 c7c6 f2f3 f5f6 e2e1 h7h8 g1h1 b6c7 e1e2 f6f7 a3c1 f7f5 e2e1 c7d6 f3f4 g5f4 c1f4 e6e7 d4e5 d6e5 b3d1 h8g8 f4h2 e5f6 e4e7 e8e7 d1d8 g8f7 e1e7 f6e7 d8c7 f5f1 h2g1 f1e1 c7d7 e1c3 d7b7 c3b4 b7c6 b4d6 c6c4 d6e6 c4a4 e7f6 a4a5 e6c4 a5a3 f7g6 a3g3 g6f5 a2a3 f6e5 g3f2 f5e6 f2f8 c4d3 a3a4 d3d6 f8f3 d6d5 f3e2 d5c6 e2d1 c6c3 d1b1 c3c4 b1g6 e6e7 g6h5 c4c3 h5h4 e7e6 h4g4 e6f6 g4e2 a6a5 e2f1 f6e6 f1f8 c3e1 f8f3 e6d6 f3f7 e5f6 f7f8 d6d7 f8a8 f6c3 a8b7 d7e6 b7c7 e1c1 c7c8 e6e7 c8b7 e7d8 b7a8 d8c7 a8a7 c7c6 a7b8 c1a1 b8d8 a1e1 d8c8 c6d6 c8f8 d6d7 f8f4 c3f6 f4c4 f6e5 c4f7 d7d8 f7f8 d8d7 f8f5 d7e7 f5d3 e1b4 d3e2 e7e6 e2c2 e5f6 c2d1 b4c3 d1h5 f6e5 h5e8 e6d5 e8d7 d5e4 d7d8 e5f6 d8d1 e4f5 d1g4 f5e5 g4d7");

	//uci_position(&active_game, "position fen r1bqkbnr/ppp1pppQ/2n5/8/4p3/3B4/PPPP1PPP/RNB1K1NR b KQkq - 0 1");

	//SEE test 2 (involving xray of pawns or not):
	//uci_position(&active_game, "position fen r1bqkb1r/pppp1ppp/2n2B2/4p3/3P4/N7/PPP1PPPP/R2QKBNR b KQkq - 0 4");

	//bitboard attackers;
	//findAttackingSquares(attackers, &active_game, 20);
	//bitprint(attackers);

	//SEE test:
	//uci_position(&active_game, "position fen 1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - -");

	//uci_position(&active_game,"position fen 6K1/5rp1/1b1r3p/5P2/pP1p2P1/P5kP/1BP5/N2Rn2q b - - 0 1");
	//uci_position(&active_game,"position fen 5rk1/6p1/1b1r3p/5P2/pPnp2P1/P5KP/1BP5/N2R3Q w - - 0 1");

	//Illegal Move (cannot take black king):
	//uci_position(&active_game,"position startpos moves b1c3 e7e5 g1f3 d7d5 f3e5 d5d4 c3b1 f8d6 e5f3 c7c5 e2e4 c8g4 f1e2 g4f3 e2f3 b8c6 d2d3 g8e7 b1d2 c6e5 f3e2 f7f5 e1g1 e8g8 f2f4 e5c6 d2b3 c5c4 d3c4 f5e4 e2g4 e7f5 d1e2 d8e7 c4c5 d6c7 c1d2 f8f6 a1d1 a8d8 a2a3 h7h6 e2c4 e7f7 g4e2 f7c4 e2c4 g8f8 d1e1 e4e3 d2c1 a7a5 c4b5 c6a7 b5d3 a5a4 b3a1 a7c6 b2b4 b7b6 g2g4 f5h4 c5b6 c7b6 c1b2 c6e7 h2h3 h4g6 f4f5 g6e5 d3e2 e7d5 g1g2 d8c8 e2a6 c8a8 a6b5 f8g8 e1d1 e3e2 b5e2 d5e3 g2g3 e3f1 e2f1 f6d6 f1g2 a8f8 g2h1 e5c4");

	//Should not take with King:
	//uci_position(&active_game,"position startpos moves b1c3 e7e5 g1f3 d7d5 f3e5 d5d4 c3b1 g8f6 e2e3 f8c5 b1a3 c8e6 a3b5 b8c6 e5c6 b7c6 b5d4 c5d4 e3d4 d8d4 d2d3 f6g4 d1f3 e8g8 f1e2 d4b4 e1f1 b4c5 e2d1 c5b6 f3g3 g4f6 b2b3 f6e4 g3f4 e4c3 d1f3 c3d5 f4e5 a8e8 e5g3 b6d4 a1b1 d4c5 c1b2 d5c3 a2a3 a7a5 b1e1 e6d5 e1e5 g7g6 h2h4 c3e2 e5e2 e8e2");

	//Illegal Move:
	//uci_position(&active_game,"position startpos moves e2e4 e7e5 g1f3 b8c6 f1b5 g8f6 d2d3 f8d6 c1g5 a7a6 b5a4 b7b5 a4b3 c6a5 d3d4 a5b3 a2b3 d8e7 b1c3 c8b7 d4e5 d6e5 f3e5 e7e5 f2f4 e5c5 d1d3 h8g8 e1c1 d7d6 g5f6 g7f6 c3d5 g8g2 b3b4 c5c6 d1g1 g2g6 f4f5 g6g1 h1g1 e8d7 g1g7 a8f8 d5f6 d7e7 f6h7 c6e4 f5f6");

	//Moves Bishop into corner again, then makes an illegal move:
	//uci_position(&active_game,"position fen 4rk1r/2R2R1p/p4p2/8/Np3P2/1P6/1PP4P/2K4b b - - 0 23");

	//Sacrafices bishop for no reason:
	//uci_position(&active_game, "position startpos moves e2e4 e7e5 g1f3 b8c6 f1b5 g8f6 b1c3 a7a6 b5c6 d7c6 f3e5 f6e4 c3e4 d8d4 d1e2 d4e5 d2d4 e5e7 e1g1 c8f5 f1e1 e8c8 c1g5 f7f6 e2f3 e7d7 e4f6 g7f6 g5f6 h8g8 f6d8 c8d8 e1e2 d7d5 g1h1 d5f3 g2f3 d8d7 a1g1 f8g7 g1g3 f5g6 h1g1 c6c5 d4c5 g7b2 f3f4 g8b8 g3f3 d7c6 f4f5 g6f7 e2e7 f7a2 e7h7 a2d5 f3g3 b2e5 h7h6 c6c5 g3a3 b8g8 h6g6 g8f8 a3a5 b7b5 a5a6 f8f5 c2c4");

	//uci_position(&active_game, "position fen r1bqk1nr/pppp1ppp/2n5/2b1p3/3PP3/2P2N2/PP3PPP/RNBQKB1R b KQkq d3 0 4");
	//uci_position(&active_game, "position fen rnbqkb1r/ppp2ppp/5n2/3pp1B1/3PP3/5N2/PPP2PPP/RN1QKB1R b KQkq - 1 4");
	//uci_position(&active_game, "position startpos moves e2e4 e7e5 g1f3 b8c6 f1b5 a7a6 b5c6 d7c6 f3e5 d8d4 e5f3 d4e4 e1f1 c8g4 d2d4 g4f3 g2f3 e4d4 d1d4");
	//uci_position(&active_game, "position startpos moves e2e4 e7e5 g1f3 g8f6 d2d4 f6e4 d4e5 d7d5");
	//uci_position(&active_game, "position startpos moves g1f3 d7d5 e2e3 e7e6 c2c4 g8f6 f1e2 b8c6 d1a4 c8d7 c4d5 e6d5 b1c3 f8b4 a4b3 b4c3 d2c3 a8b8 b3a3");
	//uci_position(&active_game,"position startpos moves e2e4 g8f6 e4e5 f6d5 d2d4 d7d6 f1c4 d5b6 c4b5 c8d7 b1c3 d7b5 c3b5 d6e5 d4e5 d8d1 e1d1 b8a6 g1f3 c7c6 b5d4 a8d8 d1e2 e7e6 c1g5 d8d5 g5f4 f8e7 f4g3 e8g8 e2f1 f8d8 c2c3 b6a4 f3e1 e7c5 e1c2 a4b2 a1b1 b2a4 b1b7 a4c3 d4f3 d5d1 c2e1 c3a2 g3h4 d8b8 b7b8 a6b8 h4g5 c5b4 g5f4 b8d7 f1e2 a2c3 e2f1 c3d5 f4g3 c6c5 f1e2 d1b1 h2h4 c5c4 h4h5 b1b3 h1h4 d7b6 f3d4 b4e1");
	//uci_position(&active_game,"position startpos moves e2e4 g8f6 e4e5 f6e4 d2d3 e4c5 d3d4 c5e6 g1f3 d7d6 c1e3 c7c6 b1c3 d6d5 f1d3 c8d7 c3e2 d8b6 c2c4 b6b2 c4c5 b7b6 d1c1 b2c1 a1c1 b6c5 d4c5 h7h5 e1g1 h5h4 c1e1 h4h3 g1h1 h3g2 h1g2 g7g5 f3g5 e6g5 e3g5 d7h3 g2g1 b8d7 e2d4 d7c5 d3b1 h8h5 d4c6 h5g5 g1h1 h3g2 h1g1");
	//uci_position(&active_game,"position startpos moves e2e4 b7b5 e4e5 d7d5");
	//uci_position(&active_game,"position fen 1nbqkbnr/P3pP1p/3p4/8/8/5Q2/pPpPP1PP/1NB1KBNR b Kk - 0 1");
	//uci_position(&active_game,"position startpos moves d2d4 d7d5 c2c4 e7e6 c4d5 e6d5 b1c3 f8b4 g1f3 b8c6 c1f4 g8f6 f3e5 f6e4 e5c6 b7c6 d1c1 c8f5 f2f3 e4c3 b2c3 b4d6 e1f2 d8h4 g2g3 d6f4 c1f4 h4f4 g3f4 a8b8 h1g1 b8b2 g1g7 f5g6 f4f5 e8e7 f5g6 h7g6 a2a4 e7f8 g7f7 f8f7 h2h3 f7g8 a1c1 h8h4 a4a5 h4h7 a5a6 b2a2 c1b1 a2a6 b1b8 g8g7 b8b7 g7g8 e2e4 a6a3 e4e5 h7e7 f1e2 c6c5 d4c5 a3c3 b7b8 g8g7 b8c8 c3c5 f3f4 g6g5 f2e3 g5f4 e3f4 d5d4 e2g4 c5e5 c8c7 e5e4 f4f5");
	//uci_position(&active_game, "position fen rnbqkbnr/pp1pp2p/5p2/2N1Q3/3p4/6p1/PPPPPPPP/R1B1KB1R w KQkq - 0 1");
	//uci_position(&active_game, "position startpos moves e2e4 e7e5 d2d4 e5d4 d1d4 g8f6 c1g5 d8e7 b1c3 b8c6 d4c4 a7a5 g1f3 c6b4 c4c7 b4c2");
	//uci_position(&active_game, "position startpos moves e2e4 e7e5 b1c3 g8f6 g1f3 f8c5 f3e5 e8g8 f1c4 g8h8 e5f7");
	//uci_position(&active_game, "position startpos moves e2e4 g8f6 b1c3 f6e4 c3e4 d7d5 e4c3 d8d6 d2d4 d6b6 g1f3 b6b2 c1b2 c8h3 g2h3 b8c6 f1b5 e8d8 c3d5 c6d4 d1d4 e7e5 d4e5 f8d6 e5g5 d6e7 d5e7 f7f6 g5e3 c7c6 b5c4 h8e8 b2a3 e8e7 a3e7 d8d7 e1g1 a8e8 f1d1");

	while(cmd_input != "quit" && cmd_input != "q")
	{
		getline(cin, cmd_input);
		stringstream command_stream(cmd_input);
		string command;
		command_stream >> command;

		if(command == "isready")
		{
			//GUI is done setting options. time to initialize the engine.
			cout << "readyok\n";
		}
		else if(command == "ucinewgame")
		{
			//Engine is to search on a game that is hasn't searched on before.
			uci_newgame(&active_game,&hashtable);
		}
		else if(command == "position") {
			active_game_history = uci_position(&active_game, cmd_input);
		}
		else if(command == "go") {
			uci_go(&active_game,cmd_input, &active_book, active_game_history);
		}
		else if(command == "setoption") {
			uci_setoption(cmd_input);
		}
		else if(command == "print") {
			active_game.print();
		}
		else if(command == "printm") {
			active_game.print_mailbox();
		}
		else if(command == "printbb") {
			active_game.print_bb();
		}
		else if(command == "printMoves" || command == "movegen" || command == "moves") {
			print::moves(&active_game);
		}
		else if(command == "captures" || command == "movegencaptures") {
			print::captures(&active_game);
		}
		else if(command == "SEE" || command == "see") {
			print::SEE(&active_game);
		}
		else if (command == "ray") {
			int from, to;
			command_stream >> from >> to;
			bitprint(mask::ray[from][to]);
		}
		else if(command == "divide") {

			command_stream >> command;
			PERFT p;
			p.Divide(&active_game, str_to_int(command));

		}
		else if(command == "perft") {

			command_stream >> command;
			PERFT p;
			p.go(&active_game,str_to_int(command));
			//cout << "Nodes: " << Perft(&active_game, str_to_int(command)) << endl;

		}
		else if(command == "perftsuite") {
			test::perftsuite();
		}
		else if(command == "t" || command == "test") {
			test::bench(&active_game);
		}
		else if (command == "symmetric") {
			test::symmetry(&active_game);
		}
		else if (command == "ptest") {
			test::pawneval();
		}
		else if(command == "eval") {

			bool toMove = active_game.getActivePlayer();
			//short phase_gauge = ((active_game.getMaterialValue(toMove) > active_game.getMaterialValue(!toMove)) ? active_game.getMaterialValue(toMove) : active_game.getMaterialValue(!toMove)) - 9900; //This will range from 0 to 4055
			//phase_gauge = (phase_gauge < 1300 ) ? 0 : phase_gauge - 1300; //This will range from 0 to 2775
			cout << "Relative Evaluation: "  << evaluate(&active_game, active_game.getActivePlayer()) << endl;
			//cout << "Phase Gauge:         " << phase_gauge << endl;
			//cout << "Formula:             " << "opening_bonus*(" << ((double)phase_gauge)/2775 <<") + endgame_bonus*(" << (double)1 - ((double)phase_gauge)/2775 << ")\n";
			//cout << "Formula 2:           ";
		}
		else if(command == "pawneval") {
			short s=0, o=0, e=0;
			//PawnEval(&active_game, o, e);
			cout << "White Opening: " << o << "\nWhite Ending:  " << e << endl;
		}
		else if(command == "id" || command == "i" ) {
			uci_iterativeDeepening(&active_game, depth);
		}
		else if(command == "s" || command == "search") {
			uci_search(&active_game,depth);
		}
		else if(command == "echo") {
			while(command_stream >> command) {
				cout << command << " ";
			}
			cout << "\n";
		}
		else if(command == "depth")  {
			command_stream >> depth; }
		else if(command == "value") {
			cout << "Material Value: " << active_game.getRelativeMaterialValue() << endl; }
		else if(command == "key") {
			cout << "Zobrist Key: " << active_game.generateKey() << endl; }
		else if(command == "check") {
			cout << active_game.isInCheck() << endl; }
		else if(command == "attacked") {
			for (int i=0; i < 64;i++)
				if(active_game.getOccupiedBB(active_game.getActivePlayer()) & (((bitboard)1)<<i))
					cout << i << ":" << (active_game.isAttacked(active_game.getActivePlayer(), i)) << ". ";
		}
		else if(command == "skip") {
			active_game.NullMove(); }
		else if( command == "move") {
			uci_move(&active_game, cmd_input);
		}
		else if( command == "unmove" ) {
			uci_unmove(&active_game, cmd_input);
		}
		else if(command == "benchmark" ) {
			benchmark(active_game,depth, cmd_input); }
		else if (command == "parameters" || command == "param") {
			print::params();		}

	}

	return(0);
}