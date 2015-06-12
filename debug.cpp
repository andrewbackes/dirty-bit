
#include "dirtybit.h"
#include "perft.h"
#include "board.h"
#include "debug.h"

#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

extern unsigned char	passed_pawn_value[8];
extern unsigned char	passed_pawn_late_value[8];
extern unsigned char	canidate_pp_value[8];
extern unsigned char	canidate_pp_late_value[8];
extern unsigned char	pp_self_blocking_penalty[8];
extern unsigned char	pp_unthreatened_path_bonus[8];
extern unsigned char	pp_clear_path_bonus[8];
extern unsigned char	BISHOP_PAIR_BONUS[17];
using namespace std;

/*
void test::perftsuite() {

	cout << "Perft Suite:\n";
	string filename = "perftsuite_results.txt";
	ofstream file_out;
	file_out.open(filename);

	ifstream file_in;
	file_in.open("perftsuite.epd");

	CHESSBOARD test_board;
	int record_num = 0;

	while (file_in) {
		bool passed = true;
		record_num++;
		cout << record_num << ": ";
		file_out << record_num << ": ";

		string line;
		if (!getline(file_in, line)) break;

		istringstream line_stream(line);
		vector<string> line_data;

		while (line_stream) {
			string entry;
			if (!getline(line_stream, entry, ';')) break;
			line_data.push_back(entry);
		}

		test_board.position(line_data[0]);
		for (int i = 1; i < line_data.size(); i++) {
			istringstream depth_record(line_data[i]);
			bitboard node_goal;
			string depth;
			depth_record >> depth >> node_goal;
			bitboard nodes = perft(&test_board, i);
			if (nodes != node_goal) {
				passed = false;
				cout << depth << "- FAILED! (" << nodes << "!=" << node_goal << "). ";
				file_out << depth << "- FAILED! (" << nodes << "!=" << node_goal << "). ";
				break;
			}
		}
		if (passed == true) {
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
void test::pawneval() {
	string positions[] = {
		"8/2P3k1/8/8/8/8/6K1/8 w - - 0 1", //7th rank pp
		"8/2p1p1k1/8/8/3P4/8/2P1P1K1/8 w - - 0 1", //???blockade
		"8/2p1p1k1/5p2/8/3P4/8/2P1P1K1/8 w - - 0 1", //blackade 3v3
		"8/2p1p1k1/5p2/8/3P4/8/2P1PPK1/8 w - - 0 1", //blockade 4v3
		"4k3/ppp2ppp/8/1P2p1P1/4P3/P7/2P1P2P/4K3 w - - 0 1", //white horizontal dislocation. better for black
		"4k3/pp2pppp/3p4/2p5/4P3/3P4/PPP2PPP/4K3 w - - 0 1", //???commander pawns (more advanced pawn of a closed position) 
		"4k3/pp3ppp/4p3/2p1P3/3p1P2/3P4/PPP3PP/4K3 w - - 0 1", //harder to defend commander pawns
		"4k3/ppp3pp/4p3/3p1p2/3P1P2/4P3/PPP3PP/4K3 w - - 0 1", //stonewall
		"4k3/pp3ppp/8/8/8/8/PPP3PP/4K3 w - - 0 1", //2 pawn islands. offset majorities.
		"4k3/8/8/4p3/p6p/P2P2p1/1P4P1/4K3 w - - 0 1", //???a bunch of backwardness
		//Passed pawn tests:
		"1k6/pppp4/8/8/8/6P1/PPP5/1K6 w - - 0 1", //rule of the square 1
		"1k6/pppp4/8/8/6P1/8/PPP5/1K6 w - - 0 1", //rule of the square 2
		"1k6/pppp4/8/6P1/8/8/PPP5/1K6 w - - 0 1", //rule of the square 3
		"1k6/pppp4/1n4N1/6P1/8/8/PPP5/1K6 w - - 0 1", //friendly blocker
		"1k4N1/pppp4/1n6/6P1/8/8/PPP5/1K6 w - - 0 1", //friendly distant blocker
		"1k6/pppp4/6n1/6P1/8/1N6/PPP5/1K6 w - - 0 1", //enemy blocker
		"1k4n1/pppp4/8/6P1/8/1N6/PPP5/1K6 w - - 0 1", //enemy distant blocker
		"1kn5/pppp4/8/3b2P1/8/8/PPP5/1KN5 w - - 0 1", //distant attacker
		"1kn5/pppp3B/8/3b2P1/8/8/PPP5/1KN5 w - - 0 1", //distant attacked&defended
		"1kn5/pppp4/7B/6P1/3b4/8/PPP5/1KN5 w - - 0 1", //not as distant attacked&defended
		"1kn5/pppp4/8/6PB/4b3/8/PPP5/1KN5 w - - 0 1", //immediately attacked&defended
		"1kn5/pppp4/8/6P1/7B/4b3/PPP5/1KN5 w - - 0 1", //current square attacked&defended
		"1knr4/pppp4/8/6P1/7B/4b3/PPP5/1KN3R1 w - - 0 1", //supporting rook
		"1kn5/pppp4/8/6P1/8/8/PPP5/1KN3R1 w - - 0 1", //only supporting rook
	};
	int count = 24;
	CHESSBOARD game;
	for (int i = 0; i < count; i++) {
		if (i == 10) cout << "*********************************************\nPASSED PAWN TESTS:\n*********************************************\n";
		game.newgame();
		game.position(positions[i]);
		game.print();
		cout << "evaluation:                " << evaluate(&game, WHITE) << endl;
		cout << "evaluation (w/o material): " << evaluate(&game, WHITE) - game.getRelativeMaterialValue() << endl;
	}

}
void test::symmetry() {
	
	//Should be even:
	string positions[8] = {
		"r1bqkbnr/pppp1ppp/2n5/4p3/4P3/2N5/PPPP1PPP/R1BQKBNR w KQkq - 2 3",
		"r1bqk1nr/pppp1ppp/2n5/1B2p3/1b2P3/2N5/PPPP1PPP/R1BQK1NR w KQkq - 4 4",
		"r1b1k1nr/pppp1ppp/2n5/1B2p2Q/1b2P2q/2N5/PPPP1PPP/R1B1K1NR w KQkq - 6 5",
		"r1bq1rk1/pppp1ppp/2n2n2/1B2p3/1b2P3/2N2N2/PPPP1PPP/R1BQ1RK1 w - - 8 6",
		"r1bq1rk1/pppp1pp1/2n2n1p/1B2p3/1b2P3/2N2N1P/PPPP1PP1/R1BQ1RK1 w - - 0 7",
		"r1bq1rk1/pppp1p2/2n2n1p/1B2p1p1/1b2P1P1/2N2N1P/PPPP1P2/R1BQ1RK1 w - g6 0 8",
		"r1b2rk1/ppppqp2/2n2n1p/1B2p1p1/1b2P1P1/2N2N1P/PPPPQP2/R1B2RK1 w - - 2 9",
		"3r1rk1/p1pb1p2/1pnpqn1p/1B2p1p1/1b2P1P1/1PNPQN1P/P1PB1P2/3R1RK1 w - - 0 14"
	};
	for (int i = 0; i < 8; i++) {
		cout << "Even position " << i << ": ";
		game->position(positions[i]);
		int score = evaluate(game, game->getActivePlayer());
		if (score != 0) {
			cout << "\nScore: " << score << endl;
			game->print();
			cout << "Not an even score!" << endl;
			system("PAUSE");
		}
		cout << "complete.\n";
	}

	// Mirrored positions:
	string positions_w[2] = {
		"r4kr1/pppqbp2/2n1bn2/3pp2p/1P1PP1p1/P1N2N1P/2PQ1PP1/R1B2RK1 w - - 0 1",
		"r1bqk1nr/pppp1ppp/2n5/2b1p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq -"
		//"8/6k1/p2P1p1p/1p4p1/6P1/1PP2P1P/6K1/8 w - - 0 1"
	};
	string positions_b[2] = {
		"1kr2b1r/1pp1qp2/p1n2n1p/1P1pp1p1/P2PP3/2NB1N2/2PBQPPP/1RK4R b - - 0 1",
		"rnbqk2r/pppp1ppp/5n2/2b1p3/2B1P3/2N5/PPPP1PPP/R1BQK1NR b KQkq -"
		//"8/1k6/p1p2pp1/1p6/1P4P1/P1P1p2P/1K6/8 b - - 0 1"
	};
	for (int i = 0; i < 2; i++) {
		cout << "Mirrored position " << i << ": ";
		game->position(positions_w[i]);
		int score_w = evaluate(game, game->getActivePlayer());
		game->position(positions_b[i]);
		int score_b = evaluate(game, game->getActivePlayer());
		if (score_w != score_b) {
			cout << "\nScore: " << score_w << ", " << score_b << endl;
			game->print();
			cout << "Not a symmetric score!" << endl;
			system("PAUSE");
		}
		cout << "complete.\n";
	}
	cout << "Test complete!" << endl;
}

void test::bench() {
	BOOK b;
	TIMER t1;

	t1.start();
	cout << "STARTING POSITION:\n";
	
	game->newgame();
	game->print();
	int d1 = uci::go(game, "", &b, "");
	t1.end();
	TIMER t2;

	t2.start();
	cout << "\n \nGAME OF THE CENTURY QUEEN SACRIFICE:\n";
	game->position("r3r1k1/pp3pbp/1qp3p1/2B5/2BP2b1/Q1n2N2/P4PPP/3R1K1R b - - 3 17");
	game->print();
	int d2 = uci::go(game, "", &b, "");
	t2.end();

	TIMER t3;
	t3.start();
	cout << "\n \nGAME OF THE CENTURY (MATE IN 15?):\n";
	game->position("4r1k1/1p3pbp/1Qp3p1/8/r1b5/2n2N2/P4PPP/3R2KR b - - 0 25");
	game->print();
	int d3 = uci::go(game, "", &b, "");
	t3.end();

	TIMER t4;
	t4.start();
	cout << "\n\nRETI ENDGAME POSITION:\n";
	game->position("7K/8/k1P5/7p/8/8/8/8 w - -");
	game->print();
	int d4 = uci::go(game, "", &b, "");
	t4.end();
	cout << "Position               Depth\tTime" << endl;
	cout << "-------------------------------------------------\n";
	cout << "Start   		" << d1 << "\t" << t1.time() / 1000.0 << endl;
	cout << "Midgame 1		" << d2 << "\t" << t2.time() / 1000.0 << endl;
	cout << "Midgame 2		" << d3 << "\t" << t3.time() / 1000.0 << endl;
	cout << "King-Pawn		" << d4 << "\t" << t4.time() / 1000.0 << endl;
}


void print::SEE(CHESSBOARD * b) {
	MOVELIST cap_list;
	cap_list.reset();
	CaptureGen(b, &cap_list);
	for (int i = 0; i < cap_list.capture_size(); i++) {
		cap_list.capture_at(i).print();
		cout << " SEE1: ";
		cout << SEE1(b, cap_list.capture_at(i));
		cout << ". SEE2: ";
		cout << SEE2(b, cap_list.capture_at(i));
		cout << ". SEE3: ";
		cout << SEE3(b, cap_list.capture_at(i));
		cout << endl;
	}
}
*/
void print::params() {

	int width = 38;

	cout << "*******************************************************************************\n";
	cout << "\n   PARAMETERS:\n\n";
	cout << "*******************************************************************************\n\n";

	cout << endl;
	cout << setw(width) << "PAWN_VALUE " << PAWN_VALUE << endl;
	cout << setw(width) << "KNIGHT_VALUE " << KNIGHT_VALUE << endl;
	cout << setw(width) << "BISHOP_VALUE " << BISHOP_VALUE << endl;
	cout << setw(width) << "ROOK_VALUE " << ROOK_VALUE << endl;
	cout << setw(width) << "QUEEN_VALUE " << QUEEN_VALUE << endl;
	cout << setw(width) << "KING_VALUE " << KING_VALUE << endl;

	// Adjust piece values as a function of pawns:
	//#define ENABLE_PIECE_VALUE_ADJUSTMENTS
	/*
	const char KNIGHT_ADJUSTMENT[] =		{ -31, -25, -19, -13, -6, 0, 6, 13, 19};
	const char BISHOP_ADJUSTMENT[] =		{ 38, 31, 25, 19, 13, 0, -13, -19, -25 };
	const char ROOK_ADJUSTMENT[] =			{ 62, 50, 38, 25, 13, 0, -13, -25, -38 };
	*/
	cout << endl;
	cout << setw(width) << "KNIGHT_ADJUSTMENT[] " << "{ ";
	for (int i = 0; i < 8; i++) cout << (int)KNIGHT_ADJUSTMENT[i] << ", ";
	cout << (int)KNIGHT_ADJUSTMENT[8] << " }" << endl;
	cout << setw(width) << "BISHOP_ADJUSTMENT[] " << "{ ";
	for (int i = 0; i < 8; i++) cout << (int)BISHOP_ADJUSTMENT[i] << ", ";
	cout << (int)BISHOP_ADJUSTMENT[8] << " }" << endl;
	cout << setw(width) << "ROOK_ADJUSTMENT[] " << "{ ";
	for (int i = 0; i < 8; i++) cout << (int)ROOK_ADJUSTMENT[i] << ", ";
	cout << (int)ROOK_ADJUSTMENT[8] << " }" << endl;

	cout << endl;
	cout << setw(width) << "SIDE_TO_MOVE_EARLY_BONUS " << SIDE_TO_MOVE_EARLY_BONUS << endl;
	cout << setw(width) << "SIDE_TO_MOVE_LATE_BONUS " << SIDE_TO_MOVE_LATE_BONUS << endl;

	// PST
	//cout << setw(width) << "//#define ENABLE_QUEEN_PST" << endl;
	//cout << setw(width) << "NEW_PSTS
	//cout << setw(width) << "#define OLD_PSTS

	// Pawn evaluation:
	cout << endl;
	cout << setw(width) << "PASSED_PAWN_EARLY_MIN_VALUE " << PASSED_PAWN_EARLY_MIN_VALUE << endl;
	cout << setw(width) << "PASSED_PAWN_EARLY_MAX_VALUE " << PASSED_PAWN_EARLY_MAX_VALUE << endl;
	cout << setw(width) << "PASSED_PAWN_LATE_MIN_VALUE " << PASSED_PAWN_LATE_MIN_VALUE << endl;
	cout << setw(width) << "PASSED_PAWN_LATE_MAX_VALUE " << PASSED_PAWN_LATE_MAX_VALUE << endl;
	cout << setw(width) << "CANIDATE_EARLY_MIN_VALUE " << CANIDATE_EARLY_MIN_VALUE << endl;
	cout << setw(width) << "CANIDATE_EARLY_MAX_VALUE " << CANIDATE_EARLY_MAX_VALUE << endl;
	cout << setw(width) << "CANIDATE_LATE_MIN_VALUE " << CANIDATE_LATE_MIN_VALUE << endl;
	cout << setw(width) << "CANIDATE_LATE_MAX_VALUE " << CANIDATE_LATE_MAX_VALUE << endl;
	cout << setw(width) << "GUARANTEED_PROMOTION_VALUE " << GUARANTEED_PROMOTION_VALUE << endl;
	//Late only. From self:
	cout << setw(width) << "SELF_BLOCKING_MIN_PENALTY " << SELF_BLOCKING_MIN_PENALTY << endl;
	cout << setw(width) << "SELF_BLOCKING_MAX_PENALTY " << SELF_BLOCKING_MAX_PENALTY << endl;
	//Late only. From Enemies:
	cout << setw(width) << "UNTHREATENED_PATH_MIN_BONUS " << UNTHREATENED_PATH_MIN_BONUS << endl;
	cout << setw(width) << "UNTHREATENED_PATH_MAX_BONUS " << UNTHREATENED_PATH_MAX_BONUS << endl;
	cout << setw(width) << "CLEAR_PATH_MIN_BONUS " << CLEAR_PATH_MIN_BONUS << endl;
	cout << setw(width) << "CLEAR_PATH_MAX_BONUS " << CLEAR_PATH_MAX_BONUS << endl;
	cout << endl;
	cout << setw(width) << "ISOLATED_PAWN_EARLY_PENALTY " << ISOLATED_PAWN_EARLY_PENALTY << endl;
	cout << setw(width) << "ISOLATED_PAWN_LATE_PENALTY " << ISOLATED_PAWN_LATE_PENALTY << endl;
	cout << setw(width) << "ISOLATED_OPEN_FILE " << ISOLATED_OPEN_FILE << endl;
	cout << endl;
	cout << setw(width) << "DOUBLED_PAWN_EARLY_PENALTY " << DOUBLED_PAWN_EARLY_PENALTY << endl;
	cout << setw(width) << "DOUBLED_PAWN_LATE_PENALTY " << DOUBLED_PAWN_LATE_PENALTY << endl;
	cout << setw(width) << "DOUBLED_PAWN_OPEN_FILE " << DOUBLED_PAWN_OPEN_FILE << endl;
	cout << setw(width) << "DOUBLED_PAWN_REAR_ADVANCED " << DOUBLED_PAWN_REAR_ADVANCED << endl;
	cout << endl;
	cout << setw(width) << "BACKWARD_PAWN_EARLY_PENALTY " << BACKWARD_PAWN_EARLY_PENALTY << endl;
	cout << setw(width) << "BACKWARD_PAWN_LATE_PENALTY " << BACKWARD_PAWN_LATE_PENALTY << endl;
	cout << setw(width) << "BACKWARD_PAWN_OPEN_FILE " << BACKWARD_PAWN_OPEN_FILE << endl;
	cout << setw(width) << "EDGE_PAWN_PENALTY " << EDGE_PAWN_PENALTY << endl;
	//cout << setw(width) << "CONNECTED_PAWN_EARLY_VALUE	5
	//cout << setw(width) << "CONNECTED_PAWN_LATE_VALUE		10

	// Pawn Shelter:
	cout << endl;
	/*
	const unsigned char
	shelter_penalty[] =						{ 0, 0, 1, 2, 4, 4, 4, 4 };//{ 0, 0, 11, 20, 27, 32, 34, 36 };
	const unsigned char
	storm_penalty[] =						{ 0, 0, 0, 0, 0, 0, 0, 0 };// { 0, 0, 0, 9, 27, 54, 0, 0 };
	*/
	cout << setw(width) << "shelter_penalty[] " << "{ ";
	for (int i = 0; i < 7; i++) cout << (int)shelter_penalty[i] << ", ";
	cout << (int)shelter_penalty[7] << " }" << endl;
	cout << setw(width) << "storm_penalty[] " << "{ ";
	for (int i = 0; i < 7; i++) cout << (int)storm_penalty[i] << ", ";
	cout << (int)storm_penalty[7] << " }" << endl;

	// King Safety:
	cout << endl;
	/*
	const unsigned char
	threat_score[] =						//{ 0, 50, 75, 88, 93, 96, 98, 100, 100, 100, 100, 100, 100, 100, 100, 100 };
	{ 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 };
	*/
	cout << setw(width) << "threat_score[] " << "{ ";
	for (int i = 0; i < 16; i++) cout << (int)threat_score[i] << ", ";
	cout << (int)threat_score[16] << " }" << endl;

	cout << setw(width) << "THREAT_RATIO " << THREAT_RATIO << endl;
	cout << setw(width) << "KNIGHT_THREAT_WEIGHT " << KNIGHT_THREAT_WEIGHT << endl;
	cout << setw(width) << "BISHOP_THREAT_WEIGHT " << BISHOP_THREAT_WEIGHT << endl;
	cout << setw(width) << "ROOK_THREAT_WEIGHT " << ROOK_THREAT_WEIGHT << endl;
	cout << setw(width) << "QUEEN_THREAT_WEIGHT " << QUEEN_THREAT_WEIGHT << endl;

	cout << setw(width) << "SEMIOPEN_THREAT_EARLY_BONUS " << SEMIOPEN_THREAT_EARLY_BONUS << endl;
	cout << setw(width) << "SEMIOPEN_THREAT_LATE_BONUS " << SEMIOPEN_THREAT_LATE_BONUS << endl;
	cout << setw(width) << "OPEN_THREAT_EARLY_BONUS " << OPEN_THREAT_EARLY_BONUS << endl;
	cout << setw(width) << "OPEN_THREAT_LATE_BONUS " << OPEN_THREAT_LATE_BONUS << endl;

	// Bishop and Knight:
	cout << endl;
	cout << setw(width) << "BISHOP_PAIR_EARLY_BONUS " << BISHOP_PAIR_EARLY_BONUS << endl;
	cout << setw(width) << "BISHOP_PAIR_LATE_BONU S" << BISHOP_PAIR_LATE_BONUS << endl;
	/*
	const unsigned char
	BISHOP_PAIR_BONUS[] =			//{ 75, 75, 72, 69, 66, 63, 59, 56, 53, 47, 44, 41, 38, 34, 31, 28, 25, 0 };
	{ 75, 75, 70, 65, 60, 55, 50, 45, 40, 35, width, 25, 20, 15, 10, 5,0 };
	*/
	cout << setw(width) << "BISHOP_PAIR_BONUS[] " << "{ ";
	for (int i = 0; i < 16; i++) cout << (int)BISHOP_PAIR_BONUS[i] << ", ";
	cout << (int)BISHOP_PAIR_BONUS[16] << " }" << endl;

	cout << setw(width) << "KNIGHT_LATE_PENALTY " << KNIGHT_LATE_PENALTY << endl;

	// Mobility:
	cout << endl;
	cout << setw(width) << "QUEEN_MOBILITY_EARLY_BONUS " << QUEEN_MOBILITY_EARLY_BONUS << endl;
	cout << setw(width) << "QUEEN_MOBILITY_LATE_BONUS " << QUEEN_MOBILITY_LATE_BONUS << endl;
	cout << endl;
	cout << setw(width) << "ROOK_SEMI_OPEN_FILE_EARLY_BONUS " << ROOK_SEMI_OPEN_FILE_EARLY_BONUS << endl;
	cout << setw(width) << "ROOK_SEMI_OPEN_FILE_LATE_BONUS " << ROOK_SEMI_OPEN_FILE_LATE_BONUS << endl;
	cout << setw(width) << "ROOK_OPEN_FILE_EARLY_BONUS " << ROOK_OPEN_FILE_EARLY_BONUS << endl;
	cout << setw(width) << "ROOK_OPEN_FILE_LATE_BONUS " << ROOK_OPEN_FILE_LATE_BONUS << endl;
	cout << setw(width) << "ROOK_MOBILITY_EARLY_BONUS " << ROOK_MOBILITY_EARLY_BONUS << endl;
	cout << setw(width) << "ROOK_MOBILITY_LATE_BONUS " << ROOK_MOBILITY_LATE_BONUS << endl;
	cout << setw(width) << "ROOK_SUPPORTING_PP_EARLY_BONUS " << ROOK_SUPPORTING_PP_EARLY_BONUS << endl;
	cout << setw(width) << "ROOK_SUPPORTING_PP_LATE_BONUS " << ROOK_SUPPORTING_PP_LATE_BONUS << endl;
	cout << setw(width) << "ROOK_7TH_FILE_EARLY_BONUS " << ROOK_7TH_FILE_EARLY_BONUS << endl;
	cout << setw(width) << "ROOK_7TH_FILE_LATE_BONUS " << ROOK_7TH_FILE_LATE_BONUS << endl;
	cout << endl;
	cout << setw(width) << "BISHOP_MOBILITY_EARLY_BONUS " << BISHOP_MOBILITY_EARLY_BONUS << endl;
	cout << setw(width) << "BISHOP_MOBILITY_LATE_BONUS " << BISHOP_MOBILITY_LATE_BONUS << endl;
	cout << endl;
	cout << setw(width) << "KNIGHT_MOBILITY_EARLY_BONUS " << KNIGHT_MOBILITY_EARLY_BONUS << endl;
	cout << setw(width) << "KNIGHT_MOBILITY_LATE_BONUS " << KNIGHT_MOBILITY_LATE_BONUS << endl;

	//Patterns:
	cout << endl;
	cout << setw(width) << "TRAPPED_BISHOP " << TRAPPED_BISHOP << endl;
	cout << setw(width) << "TRAPPED_ROOK " << TRAPPED_ROOK << endl;

	cout << "\n*******************************************************************************\n";
	cout << "\n   PRECOMPUTED:\n\n";
	cout << "*******************************************************************************\n\n";
	cout << setw(width) << "passed_pawn_value[] " << "{ ";
	for (int i = 0; i < 7; i++) cout << (int)passed_pawn_value[i] << ", ";
	cout << (int)passed_pawn_value[7] << " }" << endl;
	cout << setw(width) << "passed_pawn_late_value[] " << "{ ";
	for (int i = 0; i < 7; i++) cout << (int)passed_pawn_late_value[i] << ", ";
	cout << (int)passed_pawn_late_value[7] << " }" << endl;
	cout << setw(width) << "canidate_pp_value[] " << "{ ";
	for (int i = 0; i < 7; i++) cout << (int)canidate_pp_value[i] << ", ";
	cout << (int)canidate_pp_value[7] << " }" << endl;
	cout << setw(width) << "canidate_pp_late_value[] " << "{ ";
	for (int i = 0; i < 7; i++) cout << (int)canidate_pp_late_value[i] << ", ";
	cout << (int)canidate_pp_late_value[7] << " }" << endl;
	cout << setw(width) << "pp_unthreatened_path_bonus[] " << "{ ";
	for (int i = 0; i < 7; i++) cout << (int)pp_unthreatened_path_bonus[i] << ", ";
	cout << (int)pp_unthreatened_path_bonus[7] << " }" << endl;
	cout << setw(width) << "pp_clear_path_bonus[] " << "{ ";
	for (int i = 0; i < 7; i++) cout << (int)pp_clear_path_bonus[i] << ", ";
	cout << (int)pp_clear_path_bonus[7] << " }" << endl;
	cout << setw(width) << "pp_self_blocking_penalty[] " << "{ ";
	for (int i = 0; i < 7; i++) cout << (int)pp_self_blocking_penalty[i] << ", ";
	cout << (int)pp_self_blocking_penalty[7] << " }" << endl;
	cout << setw(width) << "BISHOP_PAIR_BONUS[] " << "{ ";
	for (int i = 0; i < 16; i++) cout << (int)BISHOP_PAIR_BONUS[i] << ", ";
	cout << (int)BISHOP_PAIR_BONUS[16] << " }" << endl;

}
/*
void print::captures(CHESSBOARD * b) {

	
	vector<MOVE> capture_list = CaptureGen(b);
	MoveSort(capture_list);
	cout << "Captures: \n";
	//for (list<MOVE>::iterator it=move_list.begin(); it != move_list.end(); ++it) {
	for (int i=0; i < capture_list.size(); i++) {
	capture_list.at(i).print() ;
	//cout << "(" << capture_list.at(i).static_value << "). ";
	}
	cout << endl;
	
}
*/
/*
void print::moves(CHESSBOARD * b) {
	MOVELIST moves;
	moves.linkGame(b);
	bool toMove = b->getActivePlayer();
	//unsigned char old_phase = 0;
	//int counter = 0;
	//if(moves.hash_suggestion.from == 64) old_phase = 1;
	while (moves.NextMove()) {

		b->MakeMove(*moves.move());
		if (!b->isInCheck(toMove)) {
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
 */
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
*/
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
/*
}

*/