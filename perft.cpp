
#include "bitboards.h"
#include "perft.h"
#include "move.h"
#include "board.h"
#include "nextmove.h"
#include "movegen.h"

using namespace std;




void PERFT::clear() {
	captures = 0;
	enpassants = 0;
	castles = 0;
	promotions = 0;
	checks = 0;
	checkmates = 0;
	nodes = 0;
}
void PERFT::go(CHESSBOARD * game, int depth) {
		clear();
		Perft(game, depth);
		cout << 
			"Nodes:\t\t" << nodes << endl <<
			"Captures:\t" << captures << endl <<
			"E.p:\t\t" << enpassants << endl <<
			"Castles:\t" << castles << endl <<
			"Promotions:\t" << promotions << endl <<
			"Checks:\t\t" << checks << endl <<
			"Checkmates:\t" << checkmates << endl;
	}

void PERFT::Divide(CHESSBOARD * game, int depth) {
	MOVELIST moves;
	moves.linkGame(game);
		
	bitboard total = 0;
	int movecount = 0;
	
	//vector<MOVE> move_list = CaptureGen(game);
	
	while(moves.NextMove()) {
		//game->MakeMove(move_list[i]);
		game->MakeMove(*moves.move());
		if(!game->isInCheck(!game->getActivePlayer())) {
			moves.move()->print();
			cout << " ";
			clear();
			Perft(game,depth-1);
			cout << nodes << endl;
			total+=nodes;
			movecount++;
		}
		game->unMakeMove(*moves.move());
	}

	cout << "\nMoves:\t" << movecount << endl;
	cout << "Nodes:\t" << total << endl;
	
}

void PERFT::Perft(CHESSBOARD * game, int depth) {

	if (depth == 0) {
		nodes++;	
		return;
	}
	bool fMate = true;
	MOVELIST moves;
	moves.linkGame(game);

	//vector<MOVE> move_list = CaptureGen(game);
	//for(int i =0; i < move_list.size(); i++) {
	while(moves.NextMove()) {
		game->MakeMove(*moves.move());
		//game->MakeMove(move_list[i]);
		if(!game->isInCheck(!game->getActivePlayer())) {
			fMate = false;
			if(moves.move()->promote_piece_to !=0) promotions++;
			if(moves.move()->captured_piece != 0) captures++;
			if(game->isInCheck()) checks++;
			if(moves.move()->active_piece_id == nPawn 
			&& moves.move()->captured_piece == 0
			&& moves.move()->promote_piece_to == 0
			&& moves.move()->static_value >= 50) {
				enpassants++; }

			Perft(game, depth -1);
		}
		game->unMakeMove(*moves.move());
	}
	/*
	move_list = nonCaptureGen(game);
	for(int i =0; i < move_list.size(); i++) {
		game->MakeMove(move_list[i]);
		if(!game->isInCheck(!game->getActivePlayer())) {
			fMate = false;
			if(game->isInCheck()) checks++;
			Perft(game, depth -1);
		}
		game->unMakeMove(move_list[i]);
	}
	*/
	if(fMate) checkmates++;
	
}

bitboard perft(CHESSBOARD * game, int depth) {
	bitboard nodes = 0;
	if (depth == 0) {	
		return 1;
	}

	MOVELIST moves;
	moves.linkGame(game);

	//vector<MOVE> move_list = CaptureGen(game);
	while(moves.NextMove()) {
		game->MakeMove(*moves.move());
		if(!game->isInCheck(!game->getActivePlayer())) {
			nodes += perft(game, depth -1);
		}
		game->unMakeMove(*moves.move());
	}
	return nodes;
	
	/*
	for(int i =0; i < move_list.size(); i++) {
		game->MakeMove(move_list[i]);
		if(!game->isInCheck(!game->getActivePlayer())) {
			nodes += perft(game, depth -1);
		}
		game->unMakeMove(move_list[i]);
	}
	
	move_list = nonCaptureGen(game);
	for(int i =0; i < move_list.size(); i++) {
		game->MakeMove(move_list[i]);
		if(!game->isInCheck(!game->getActivePlayer())) {
			nodes+=perft(game, depth -1);
		}
		game->unMakeMove(move_list[i]);
	}
	*/
	return nodes;
}
