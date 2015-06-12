/*
#include <vector>
#include <ctime>
#include <iomanip>
#include <algorithm>
#include <list>

#include "engine.h"
#include "bitboards.h"
#include "board.h"
#include "move.h"

using namespace std;

/*
void AddToList(list<MOVE> *l, MOVE m) {
	for (list<MOVE>::iterator it=l->begin() ; it != l->end(); ++it) {
		if(m.static_value > it->static_value ) {
			l->insert(it, m);
			return;
		}
	}
	list<MOVE>::iterator it=l->end();
	l->insert(it, m);
}
*/





/*
MOVE UCI_NegaMaxInit(int depth, CHESSBOARD game) {
// TODO:
//			-By only checking legal moves at the root, the search can go down a psuedo-legal path and think it can do something that it can't.


	//for record keeping:
	double timer = clock()/(CLOCKS_PER_SEC/1000);
	NODES = 0;

	list<MOVE> move_list = MoveGen(&game);	
	MOVE best_move; 
	int max = -INFINITY;

	//Only consider legal moves:
	vector<MOVE> legal_move_list;
	bool inCheck = isInCheck(game); // <---- this is a problem because the search will think that castling is legal and then later be told that it is not.
	for (int i=0; i < move_list.size(); i++) {
		if( isMoveLegal(game, move_list.at(i), inCheck) ) {
			best_move = move_list.at(i); //to prevent no move being chosen.
			legal_move_list.push_back( move_list.at(i) );
		}
	}


	for (int i=0; i < legal_move_list.size(); i++) {
		//Go through everything on the movelist:
		CHESSBOARD subgame = game;
		subgame.makemove(legal_move_list.at(i));
		NODES++;
		int score = -UCI_NegaMax (depth - 1, subgame);
		if (score > max) {
			max = score;
			best_move = legal_move_list.at(i);
		}
	}

	//UCI Info:
	game.makemove(best_move);
	int score = relative_eval(game);
	timer = clock()/(CLOCKS_PER_SEC/1000) - timer;
	int nps = (int)(NODES/timer)*1000;
	cout << "info depth " << depth << " nodes " << NODES << " score cp " << score << " time " << timer << " nps " << nps << "\n";

	return best_move;
}
int UCI_NegaMax(int depth, CHESSBOARD game) {
	if(depth == 0) {
		return relative_eval(game); }
	
	vector<MOVE> move_list = MoveGen(&game);	
	int max = -INFINITY;
	
	//Go through the legal move list:
	for (int i=0;  i < move_list.size(); i++) {
		CHESSBOARD subgame = game;
		subgame.makemove(move_list.at(i));
		NODES++;
		int score = -UCI_NegaMax(depth -1, subgame);
		if(score > max) {
			max = score;
		}
	}
	return max;
	
}
*/





/*
int AlphaBeta(int depth, CHESSBOARD game, int alpha, int beta) {
	if(depth == 0) {
		int eval = relative_eval(game);
		//cout << "player: " << game.getActivePlayer() << " d0: " << eval << endl;
		return eval; }
	
	vector<MOVE> move_list = MoveGen(&game);	
	bool foundPV = false;

	for (int i=0;  i < move_list.size(); i++) {
		CHESSBOARD subgame = game;
		subgame.makemove(move_list.at(i));
		NODES++;
		int score =0;
		//PVS Addition:
		if(foundPV) {
			score = -UCI_AlphaBeta(depth -1, subgame, -alpha-1, -alpha);
			if ( (score > alpha) && (score < beta) ) {
				score = -UCI_AlphaBeta(depth-1, subgame, -beta, -alpha); }
		}
		else {
			score = -UCI_AlphaBeta(depth -1, subgame, -beta, -alpha); }
		if( score >= beta ) {
			return beta; }
		if(score > alpha) {
			alpha = score;
		}
	}
	return alpha;
}
*/

