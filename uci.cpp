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
		int score = -relative_eval(&b);
	
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

void printMoves(CHESSBOARD * b) {
	MOVELIST moves;
	moves.linkGame(b);
	bool toMove = b->getActivePlayer();
	//unsigned char old_phase = 0;
	//int counter = 0;
	//if(moves.hash_suggestion.from == 64) old_phase = 1;
	while(moves.NextMove()) {
			
		b->MakeMove(*moves.move());
		if(!b->isInCheck(toMove)) {
			//if(counter == 0 || (moves.phase() > 1 && old_phase == 0) )
			//	cout << "Movegen Phase " << (int)old_phase << ": ";
			moves.move()->print();
			//counter++;
			//if(old_phase != moves.phase()) {
			//	cout << endl;
			//	old_phase = moves.phase();
			//	counter =0;
			//}

		}
		b->unMakeMove(*moves.move());
	}
	cout << endl;
	/*
	vector<MOVE> move_list = MoveGen(b);
	MoveSort(move_list);

	cout << "Psuedolegal Moves: \n";
	for (int i=0; i < move_list.size(); i++) {
		move_list.at(i).print() ;
		//cout << "(" << move_list.at(i).static_value << ")";
	}		
	cout << " (" << move_list.size() << ")" << endl;
	cout << "Lazy Psuedolegal Moves: \n";

	typedef vector<MOVE> (*fn)(CHESSBOARD * g);
	static fn MoveGenFunctions[] = {CaptureGen, nonCaptureGen};
	
	int counter =0;
	for(int j=0; j <2; j++) {
		move_list = MoveGenFunctions[j](b);
		for (int i=0; i < move_list.size(); i++) {
			move_list.at(i).print() ;
			counter++;
		}
	}
	cout << " (" << counter << ")" << endl << endl;
	
	/*
	cout << endl << endl;
	cout << "Legal Moves: \n";
	for (int i=0; i < move_list.size(); i++) {
		bool isLegal = isMoveLegal(b, move_list.at(i));
		if( isLegal ) {
			move_list.at(i).print() ;
			//cout << "(" << move_list.at(i).static_value << ")";
		}
	}		
	cout << endl;
	*/
}

void printCaptures(CHESSBOARD * b) {
	
	/*
	vector<MOVE> capture_list = CaptureGen(b);	
	MoveSort(capture_list);
	cout << "Captures: \n";
	//for (list<MOVE>::iterator it=move_list.begin(); it != move_list.end(); ++it) {
	for (int i=0; i < capture_list.size(); i++) {
		capture_list.at(i).print() ;
		//cout << "(" << capture_list.at(i).static_value << "). ";
	}		
	cout << endl;
	*/
}

void printSEE(CHESSBOARD * b) {
	MOVELIST cap_list;
	cap_list.reset();
	CaptureGen(b,&cap_list);
	for(int i = 0; i < cap_list.capture_size(); i++) {
		cap_list.capture_at(i).print();
		cout << " SEE1: ";
		cout << SEE1(b,cap_list.capture_at(i));
		cout << ". SEE2: ";
		cout << SEE2(b,cap_list.capture_at(i));
		cout << ". SEE3: ";
		cout << SEE3(b,cap_list.capture_at(i));
		cout << endl;
	}
}

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

int uci_go(CHESSBOARD * game, string go_string, BOOK * book, string move_history) {
// Uses Iterative Deepening to pick a best move. 
// Then prints it out in UCI format.

//		TODO:	- Change the time allocation to something like: time_left / (n - kx/(x^2  +400)), x moves so far, k the fraction 
//				  --> see graph: http://chessprogramming.wikispaces.com/Time+Management
//						{(1,1),(2,1),(3,1),(4,1),(5,1),(6,1),(7,1),(8,1),(9,2),(10,2),(11,3),(12,3),(13,4),(14,5),(15,6),(16,9),(17,12),(18,16),(19,16),(20,15),(21,14),(22,13),(23,12), (24,11), (25,9),(26,5),(27,4),(28,4),(29,4),(30,3),(31,3),(32,3),(33,3),(34,3),(35,3),(36,3),(37,2),(38,2),(39,2),(40,2),(41,2),(42,2),(43,2),(44,2),(45,1),(46,1),(47,1),(48,1),(49,1),(50,1),(51,1),(52,1),(53,1),(54,1),(55,1),(56,1)}


	long start_time = clock()/(CLOCKS_PER_SEC/1000);
	int depth_cutoff = 100;
	long time_remaining[2] = {300000,300000};
	int time_increment[2] = {0,0};
	int moves_till_time_control = 30; //first value used was 35
	int move_num = min( game->getMoveCount(), 10 ); //TODO: change move count to moves out of the book.
	double time_factor = 2 -  move_num / 10;
	bool ponder = false;
	
	//30 second default time for moves:
	//time_remaining[WHITE] =  (long) floor((30000 * moves_till_time_control) / time_factor);
	//time_remaining[BLACK] = (long) floor((30000 * moves_till_time_control) / time_factor);

	//Parse the info from the string:
	stringstream command_stream(go_string);
	string command;
	string value;
	command_stream >> command; //this should say "go"
	while( command_stream >> command ) {
		if(command == "ponder") {
			ponder = true;
		} else {
			command_stream >> value;
		}

		if(command == "wtime"){
			time_remaining[WHITE] = str_to_int(value);
		}
		else if(command == "btime") {
			time_remaining[BLACK] = str_to_int(value);
		}
		else if(command == "winc") {
			time_increment[WHITE] = str_to_int(value);
		}
		else if(command == "binc") {
			time_increment[BLACK] = str_to_int(value);
		}
		else if(command == "infinite" || command == "INFINITE") {
			//give 3 min for each move.
			time_remaining[WHITE] = (long) floor((180000 * moves_till_time_control) / time_factor);
			time_remaining[BLACK] = (long) floor((180000 * moves_till_time_control) / time_factor);
		}
	}

	// Crude time management:
	double time_target = time_remaining[game->getActivePlayer()]  / moves_till_time_control;
	long time_allotted   = (long)( time_factor * time_target );
	cout << "Allotted time: " << time_allotted << endl;
	long time_left = time_allotted;
	long time_lapsed = 0;

	//Begin deciding which move to make:
	
	/*
	//Check the Book:
	if(!book->OutOfBook()) {
		string book_move = book->recommendedMove(game, move_history);
		if(book_move != "") {
			cout << "bestmove ";
			cout << book_move;
			cout << "\n";
			return;
		}
	}
	*/

	//Iterative Deepening:
	MOVE best_move;
	MOVE ponder_move;
	short best_score;
	
	int alpha_window = -INFINITY;
	int beta_window = INFINITY;
	int window_range = ASPIRATION_WINDOW;
	
	int fail_high_count = 0;
	int fail_low_count = 0;
	bool fQuitOnNext = false;

	int depth = 1;
	//int old_node_count = 1;
	//double branch_factor = 3.0;
	double * EBF = new double[depth_cutoff+1];
	double average_EBF = DEFAULT_EBF;
	int * node_count = new int[depth_cutoff+1];
	//long * nps = new long[depth_cutoff+1];
	long running_nps = DEFAULT_NPS;
	long running_node_count = 0;
	long previous_node_count = 0;

	//node_count[0] = 1;
	previous_score = 0;
	/*
	cout << "--- Before Search: ---\n";
	printMoves(game);
	cout << "--- --- Search --- ---\n";
	*/
	while( depth <= depth_cutoff ) {
		vector<MOVE> pv;
		bool fFailHigh = false;
		bool fFailLow = false;
		SEARCH search(depth, game, &hashtable, (long)floor(time_left * TIME_WIGGLE_ROOM));
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
				alpha_window = -INFINITY;
			}
			//alpha_window = -INFINITY;
			//beta_window = search.getScore() + A_LITTLE;
			fFailed = " lowerbound ";
		}
		else if(search.getScore() >= beta_window) {
			//failed high:
			fail_high_count++;
			fFailHigh = true;
			//beta_window = INFINITY;
			if(fail_high_count < 2) {
				beta_window = beta_window + A_LOT;
			}
			else {
				beta_window = INFINITY;
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
					alpha_window = -INFINITY;
					beta_window = INFINITY;
					//alpha_window -= A_LOT;
					//beta_window += A_LOT;
					//time_allotted = time_remaining[game->getActivePlayer()]/5;
					time_left = time_remaining[game->getActivePlayer()]/5;
					fQuitOnNext = true;
					#ifdef DEBUG_TIME
						//cout << "\tTime change. Allotted: " << time_allotted << endl;
						cout << "\tForcing one more search.\n";
					#endif
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
			
			/*
			if( search.getScore() - previous_score < 100 && search.getScore() - previous_score > -100 ) {
				alpha_window = previous_score - window_range;
				beta_window = previous_score +  window_range;
			}
			else {
				alpha_window = best_score - window_range;
				beta_window = best_score +  window_range;
			}
			*/
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
	
	/*
	cout << "--- After Search: ---\n";
	printMoves(game);
	cout << "--- --- --- --- --- ---\n";
	*/
	return depth;

}

void uci_search(CHESSBOARD * game, int depth) {
	//Specific depth search.
	
	SEARCH search(depth, game, &hashtable, INFINITY);
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
	int best_score = -INFINITY;

	int alpha_window = -INFINITY;
	int beta_window = INFINITY;
	int window_range = ASPIRATION_WINDOW;

	int d=1;
	
	previous_score = 0;
	while(d <= depth)
	{
		SEARCH search(d, game, &hashtable, INFINITY);
		
		//Aspiration window:
		string fFailed = "";
		bool fFailedLow = false;
		bool fFailedHigh = false;
		search.start(alpha_window, beta_window);
		if(search.getScore() <= alpha_window) {
			//failed low:
			fFailedLow = true;
			alpha_window = -INFINITY;
			fFailed = " lowerbound ";
		}
		else if(search.getScore() >= beta_window) {
			//failed high:
			fFailedHigh = true;
			beta_window = INFINITY;
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

void uci_test(CHESSBOARD * game) {
	BOOK b;
	TIMER t1;

	t1.start();
	cout << "STARTING POSITION:\n";
	game->newgame();
	int d1 = uci_go(game,"", &b, "");
	t1.end();
	TIMER t2;
	
	t2.start();
	cout << "\n \nGAME OF THE CENTURY QUEEN SACRIFICE:\n";
	game->position("r3r1k1/pp3pbp/1qp3p1/2B5/2BP2b1/Q1n2N2/P4PPP/3R1K1R b - - 3 17");
	int d2 = uci_go(game,"", &b, "");
	t2.end();
	
	TIMER t3;
	t3.start();
	cout << "\n \nGAME OF THE CENTURY (MATE IN 15?):\n";
	game->position("4r1k1/1p3pbp/1Qp3p1/8/r1b5/2n2N2/P4PPP/3R2KR b - - 0 25");
	int d3 = uci_go(game,"", &b, "");
	t3.end();
	
	TIMER t4;
	t4.start();
	cout << "\n\nRETI ENDGAME POSITION:\n";
	game->position("7K/8/k1P5/7p/8/8/8/8 w - -");
	int d4 = uci_go(game,"", &b, "");
	t4.end();
	cout << "Position               Depth\tTime" << endl;
	cout << "-------------------------------------------------\n";
	cout << "Start   		" << d1 << "\t" << t1.time()/1000.0 << endl;
	cout << "Midgame 1		" << d2 << "\t" << t2.time()/1000.0 << endl;
	cout << "Midgame 2		" << d3 << "\t" << t3.time()/1000.0 << endl;
	cout << "King-Pawn		" << d4 << "\t" << t4.time()/1000.0 << endl;
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

void uci_perftsuite() {
	
	cout << "Perft Suite:\n";
	string filename = "perftsuite_results.txt";
	ofstream file_out;
	file_out.open(filename);
	
	ifstream file_in;
	file_in.open("perftsuite.epd");

	CHESSBOARD test_board;
	int record_num = 0;
	
	while(file_in) {
		bool passed = true;
		record_num++;
		cout << record_num << ": ";
		file_out << record_num << ": ";
		
		string line;
		if(!getline(file_in, line)) break;

		istringstream line_stream( line );
		vector<string> line_data;

		while(line_stream) {
			string entry;
			if(!getline( line_stream, entry, ';')) break;
			line_data.push_back(entry);
		}

		test_board.position(line_data[0]);
		for(int i =1; i < line_data.size() ; i++) {
			istringstream depth_record( line_data[i] );
			bitboard node_goal;
			string depth;
			depth_record >> depth >> node_goal;
			bitboard nodes = perft(&test_board,i);
			if( nodes != node_goal) {
				passed = false;
				cout << depth << "- FAILED! (" << nodes << "!=" << node_goal << "). ";
				file_out << depth << "- FAILED! (" << nodes << "!=" << node_goal << "). ";
				break;
			}
		}
		if(passed == true) {
			cout << "passed."; 
			file_out << "passed.";
		}
		cout << endl;
	}

	cout << "Completed.\n";
	file_out << "Completed.\n";

	file_in.close();
	file_out.close();
	
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
			printMoves(&active_game);
		}
		else if(command == "captures" || command == "movegencaptures") {
			printCaptures(&active_game);
		}
		else if(command == "SEE" || command == "see") {
			printSEE(&active_game);
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
			uci_perftsuite();
		}
		else if(command == "t" || command == "test") {
			uci_test(&active_game);
		}
		else if(command == "eval") {
			
			bool toMove = active_game.getActivePlayer();
			short phase_gauge = ((active_game.getMaterialValue(toMove) > active_game.getMaterialValue(!toMove)) ? active_game.getMaterialValue(toMove) : active_game.getMaterialValue(!toMove)) - 9900; //This will range from 0 to 4055
			phase_gauge = (phase_gauge < 1300 ) ? 0 : phase_gauge - 1300; //This will range from 0 to 2775
			cout << "Relative Evaluation: "  << relative_eval(&active_game) << endl;
			cout << "Phase Gauge:         " << phase_gauge << endl;
			cout << "Formula:             " << "opening_bonus*(" << ((double)phase_gauge)/2775 <<") + endgame_bonus*(" << (double)1 - ((double)phase_gauge)/2775 << ")\n";
			cout << "Formula 2:           ";
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
				if(active_game.getOccupiedBB(active_game.getActivePlayer()) & (1i64<<i))
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

	}

	return(0);
}