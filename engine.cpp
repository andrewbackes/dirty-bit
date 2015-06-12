
#include "uci.h"
#include "engine.h"
#include "precompute.h"

#include <cmath>


short SEARCH_INFO::best_score() {
	short score = 0;
	for (int i = 0; i < iteration_details.size(); i++) {
		if (!iteration_details[i].failed  && !iteration_details[i].timed_out) {
			score = iteration_details[i].score;
		}
	}
	return score;
}

short SEARCH_INFO::max_depth() {
	short depth = 0;
	for (int i = 0; i < iteration_details.size(); i++) {
		if (iteration_details[i].depth > depth && !iteration_details[i].failed && !iteration_details[i].timed_out) {
			depth = iteration_details[i].depth;
		}
	}
	return depth;
}

short SEARCH_INFO::max_seldepth() {
	short seldepth = 0;
	for (int i = 0; i < iteration_details.size(); i++) {
		if (iteration_details[i].seldepth > seldepth && !iteration_details[i].failed && !iteration_details[i].timed_out) {
			seldepth = iteration_details[i].seldepth;
		}
	}
	return seldepth;
}
long long SEARCH_INFO::nodes() {
	long long nodes = 0;
	for (int i = 0; i < iteration_details.size(); i++) {
		nodes += iteration_details[i].nodes;
	}
	return nodes;
}

long SEARCH_INFO::time_lapsed() {
	long lapsed = (std::clock() / (CLOCKS_PER_SEC / 1000)) - start_time;
	return((lapsed < 1) ? 1 : lapsed);
}

long SEARCH_INFO::nps() {
	//std::cout << "nodes " << nodes() << std::endl;
	//std::cout << "time_taken " << time_taken() << std::endl;
	
	return ( (nodes() / time_lapsed()) * 1000);
}

MOVE SEARCH_INFO::best_move() {
	MOVE best_move;
	for (int i = 0; i < iteration_details.size(); i++) {
		if (!iteration_details[i].failed && !iteration_details[i].timed_out)
			best_move = iteration_details[i].move;
	}
	return best_move;
}

std::vector<MOVE> SEARCH_INFO::pv() {
	std::vector<MOVE> pv;
	for (int i = 0; i < iteration_details.size(); i++) {
		if (!iteration_details[i].failed)
			pv = iteration_details[i].pv;
	}
	return pv;
}

double EBF(SEARCH_INFO * s) {
	int count = 0;
	if (s->max_depth() > 1) {
		double sum = 0;
		double BF = 0;
		for (int i = 1; i < s->iteration_details.size(); i++) {
			BF = ((s->iteration_details[i].nodes * 1.0) / s->iteration_details[i - 1].nodes);
			if (BF <= 5) {
				sum += BF;
				count++;
			}
		}
		BF = (sum / count);
		uci::send_str("EBF: " + std::to_string(BF) + "\n");
		return BF;
	}
	else
		return DEFAULT_EBF;
}

long time_needed( SEARCH_INFO * s) {
	// long time_needed = (long)ceil(((average_EBF * (double)previous_node_count) / (double)running_nps) * 1000);
	// time_needed = (time_needed > 0) ? time_needed : 1;

	double average_EBF = EBF(s);

	// get the nodes of the previous successful iteration
	long long nodes = 0;
	for (size_t i = s->iteration_details.size() - 1; i >= 0; i--) {
		if (!s->iteration_details[i].failed) {
			nodes = s->iteration_details[i].nodes;
			break;
		}
	}

	long time_needed = (long)std::ceil(((average_EBF * nodes) / s->nps()) * 1000);
	if (time_needed < 0) time_needed = 1;
	uci::send_str("Time needed for next iteration: " + std::to_string(time_needed) + "\n");
	return time_needed;
}

void set_time(ITERATION_INFO & iteration, SEARCH_INFO * s) {
	// TODO: super safety time for when there are several fails in a row.
	
	//********** adapted from 0.39.5 t5:
	if (s->iteration_details.size() > 1) {
		ITERATION_INFO last_nonfail = s->iteration_details[s->iteration_details.size() - 1];
		// search back for a non failed iteration:
		for (int i = 1; i <= s->iteration_details.size() - 1; i++) {
			if (!s->iteration_details[s->iteration_details.size() - i].failed) {
				last_nonfail = s->iteration_details[s->iteration_details.size() - i];
				break;
			}
		}
		long time_needed = (long)std::ceil(((DEFAULT_EBF * (double)last_nonfail.nodes) / s->nps()) * 1000);

		iteration.time_allotted = time_needed;
	}
	//*********

	// from 0.40 :
	/*
	iteration.time_allotted = s->time_safety - s->time_lapsed(); // 0.40
	
	// Safety checks:
	if (s->iteration_details.size() > 1) {
		ITERATION_INFO prev = s->iteration_details[s->iteration_details.size() - 1];
		bool failed = (prev.failed || s->iteration_details[s->iteration_details.size() - 2].failed);
		if (failed && prev.timed_out) {
			iteration.time_allotted = s->time_safety - s->time_lapsed();
			uci::send_str("Assigned safety time.\n");
		}
	}
	*/
	if (iteration.time_allotted < 0) iteration.time_allotted = 0;
	uci::send_str("Time allotted for iteration: " + std::to_string(iteration.time_allotted) + "\n");
}

void set_window(ITERATION_INFO & iteration, SEARCH_INFO * s) {
	// Analyses previous search data to determine the 
	// depth and aspiration window of the next search
	
	// for searches under 6 plies, use a full window:
	if (s->max_depth() <= 6) {
		iteration.alpha = -INFTY;
		iteration.beta = INFTY;
		iteration.depth = s->max_depth() + 1;
		return;
	}
	
	// get the score of the previous successful iteration
	int target = 0;
	for (size_t i = s->iteration_details.size() - 1; i >= 0; i--) {
		if (!s->iteration_details[i].failed) {
			target = s->iteration_details[i].score;
			break;
		}
	}

	ITERATION_INFO prev = s->iteration_details[s->iteration_details.size()-1];
	// if the last iteration didnt fail, set up the next depth window:
	if (!prev.failed) {
		iteration.alpha = target - ASPIRATION_WINDOW;
		iteration.beta = target + ASPIRATION_WINDOW;
		iteration.depth = prev.depth + 1;
	}
	// failed low, so adjust the bottom window.
	else if (prev.score <= prev.alpha) {
		int last_window = target - prev.alpha;
		iteration.alpha = target - (last_window * 2);
		iteration.beta = prev.beta;
		iteration.depth = prev.depth;
	}
	else if (prev.score >= prev.beta) {
		int last_window = prev.beta - target;
		iteration.alpha = prev.alpha;
		iteration.beta = prev.beta + (last_window*2);
		iteration.depth = prev.depth;
	}
	uci::send_str("Aspiration window [" + std::to_string(iteration.alpha) + ", " + std::to_string(iteration.beta) + "]\n");

}

void ENGINE::think(SEARCH_INFO & s) {
	
	// Set overall search time:
	s.start_time = std::clock() / (CLOCKS_PER_SEC / 1000) ;

	//***** 0.40 :
	//long time_factor = 120; // 0.40
	//if (s.moves_to_go <= 2) time_factor = 75; // 0.40
	//long limit = s.time_allowed - (s.time_allowed * (s.moves_to_go - 1) / 40); // 0.40 //absolute limit to think.
	//limit = std::min((limit * 95) / 100, limit - 60); // 0.40
	//s.time_allotted = s.time_allowed / s.moves_to_go; // 0.40
	//s.time_allotted = std::min((s.time_allotted * time_factor) / 100, limit); // 0.40
	//s.time_safety = std::min(limit, s.time_allotted * 4); // 0.40
	//*****

	//***** 0.39.5 t5 :
	int move_num = std::min(this->game.getMoveCount(), 10); // 0.39.5 t5
	long time_factor = 2 - move_num / 10; // 0.39.5 t5
	
	double time_target = s.time_allowed / s.moves_to_go;
	s.time_allotted = (long)(time_factor * time_target);
	if (s.moves_to_go == 1) {
		s.time_allotted = (long)(s.time_allotted * 0.75);
	}
	s.time_safety = s.time_allowed / 5;

	//*****

	uci::send_str("Time allotted for whole search: " + std::to_string(s.time_allotted) + "\n");

	// Iterative Deepening:
	bool abort = false; // flag
	while (s.max_depth() < MAX_PLY && !abort) {
		ITERATION_INFO iteration;
		set_time(iteration, &s);

		//********** 0.39.5 t5 :
		// if the time needed is too much, lets bail:
		if (s.time_lapsed() + iteration.time_allotted >= s.time_allowed) {
			if ((s.iteration_details.size() > 0) && (!s.iteration_details[s.iteration_details.size() - 1].failed)) {
				// if there arent any fails, then we can safely quit
				abort = true;
				break;
			}
			else {
				// when there are fails, we have to figure out if there is enough time left:
				if (s.time_allowed/5 > iteration.time_allotted) {

				}
			}
		}
		//**********

		set_window(iteration, &s);
		
		SEARCH search(this, &iteration);
		search.start();
		s.iteration_details.push_back(iteration); //save the iteration results

		// If the search timed out, figure out what to do:
		if (iteration.timed_out) {
			//abort = true; // 0.40
			uci::send("info string Search timed out (Depth: " + std::to_string(iteration.depth) + ". Time: " + std::to_string(iteration.time_taken) + ")\n"); //tell the gui.
			// If the previous search failed, then something fishy is going on
			// so we should search some more to figure it out.
			//if(!iteration.failed) abort = true;
			
			// But if we are too close to time control, dont risk it.
			if (s.moves_to_go <= 2) abort = true; // 0.39.5 t5
		}
		// Search did not time out:
		else {
			uci::send(iteration); //tell the gui
			//s.iteration_details.push_back(iteration); //save the iteration results
				
			// Determine if we should do another iteration:
			//if ( (time_needed(&s)) >= s.time_remaining()) abort = true;
			//if (s.time_lapsed()  >= s.time_allotted / 2) abort = true; // 0.40
		}
	}
}

void ENGINE::precompute() {

	precompute::rays();
	precompute::shelter_scores();
	precompute::rule_of_the_squares();
	precompute::passed_pawns();
	precompute::material_adjustments();
	//precompute::castle_areas();
	//precompute::castle_pawns();
	populateMagicBishopArray();
	populateMagicRookArray();

}

void ENGINE::initialize() {
	debug = false;
	ponder = false;

	precompute();
	hashtable_size = HASH_TABLE_SIZE;
	hashtable.initialize(hashtable_size);
	//pawntable.initialize(pawntable_size);
}

void ENGINE::reset() {


}

void ENGINE::save_pgn() {


}

void ENGINE::load_pgn() {


}

/*
	bitboard mask1 = (1i64 << b5) | (1i64 << b6) | (1i64 << c7);
	bitprint(mask1);
	bitboard mask2 = (1i64 << b4) | (1i64 << b3) | (1i64 << c2);
	bitprint(mask2);
	cout << "trapped_bishop_long[] = {" << mask1 << ", " << mask2 <<endl;

	bitboard mask3 = (1i64 << g5) | (1i64 << g6) | (1i64 << f7);
	bitprint(mask3);
	bitboard mask4 = (1i64 << g4) | (1i64 << g3) | (1i64 << f2);
	bitprint(mask4);
	cout << "trapped_bishop_short[] = {" << mask3 << ", " << mask4 << endl;
	*/
	/*
	bitboard mask1 = (1i64 << a1) | (1i64 << a2) | (1i64 << b1);
	bitprint(mask1);
	bitboard mask2 = (1i64 << a8) | (1i64 << a7) | (1i64 << b8);
	bitprint(mask2);
	cout << "trapped_rook_long[] = {" << mask1 << ", " << mask2 << " };" << endl;
	mask1 = (1i64 << b1) | (1i64 << c1);
	bitprint(mask1);
	mask2 = (1i64 << b8) | (1i64 << c8);
	bitprint(mask2);
	cout << "trapped_rook_long_king[] = { " << mask1 << ", " << mask2 << " };" << endl;

	mask1 = (1i64 << h1) | (1i64 << h2) | (1i64 << g1);
	bitprint(mask1);
	mask2 = (1i64 << h8) | (1i64 << h7) | (1i64 << g8);
	bitprint(mask2);
	cout << "trapped_rook_short[] = {" << mask1 << ", " << mask2 << " };" << endl;
	mask1 = (1i64 << f1) | (1i64 << g1);
	bitprint(mask1);
	mask2 = (1i64 << f8) | (1i64 << g8);
	bitprint(mask2);
	cout << "trapped_rook_short_king[] = { " << mask1 << ", " << mask2 << " };" << endl;
	*/