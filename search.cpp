#include <vector>
#include <ctime>
#include <iostream>

#include "search.h"

#include "move.h"
#include "evaluate.h"
#include "movegen.h"
#include "utilities.h"
#include "nextmove.h"

using namespace std;

void SEARCH::setHasheTable(HASHTABLE * t) {
	hashtable = t;
}

bool SEARCH::isOutOfTime() {

	if(clock()/(CLOCKS_PER_SEC/1000) - start_time >= allotted_time ) {
		fTimeOut = true;
		return true;
	}
	return false;

}

void SEARCH::start(int alpha_bound, int beta_bound) {
	alpha = alpha_bound;
	beta = beta_bound;
	best_score = alpha - 1;

	#ifdef SEARCH_STATS
		for(int i=0; i <255; i++) {
			beta_cutoffs[i] = 0;
			alpha_cutoffs[i] = 0;
			killer_cutoffs[i] = 0;
		}
	#endif

	//for record keeping:
	start_time = clock()/(CLOCKS_PER_SEC/1000);
	node_count = 0;
	
	//Search:
	//PVS_Root();
	Hard_PVS_Root();
	collectPV();

	//for record keeping:
	end_time = clock()/(CLOCKS_PER_SEC/1000);

	#ifdef SEARCH_STATS
		print_stats();
	#endif
}

void SEARCH::collectPV() {
	//This function gathers the PV info from the Hash Tables.
	// Output: Populates the pv vector.

	pv.clear();

	for(int d=0; d < root_depth; d++) {
		MOVE hashed_move = hashtable->getHashMove(root_game->getKey(),root_depth-d);
		if(hashed_move.from != 64 ) {
			pv.push_back(hashed_move);
			root_game->MakeMove(hashed_move);
		}
		else
			break;
	}
	
	//Now undo all the moves that were just done:
	for(int i = (int) pv.size() -1; i >=0; i--)
		root_game->unMakeMove(pv.at(i));
}

int SEARCH::Hard_qSearch(CHESSBOARD * game, int alpha, int beta, int qDepth) {
	
	if(qDepth > q_depth)
		q_depth = qDepth;

	/*
	//Hashe table Check:
	int value = hashtable->SearchHash(game->getKey(),0,alpha,beta);
	if(value != HASH_UNKNOWN )
		return value;
	*/

	int value = relative_eval(game,alpha, beta);

	if (value >= beta) {
		return beta;	//hard
		//return value;	//soft
	}
	
	short delta = nPieceValue[nQueen];
	if(mask::pawns_spawn[!game->getActivePlayer()] & game->getPieceBB(game->getActivePlayer(),nPawn) )
		delta += nPieceValue[nQueen] - nPieceValue[nPawn]- nPieceValue[nPawn];
	
	if(value + delta < alpha)
		return value;
		
	if(value > alpha )
		alpha = value;

	//Get all of the capture moves:
	vector<MOVE> capture_list = CaptureGen(game);
	MoveSort(capture_list);
	int capture_count = (int) capture_list.size();

	//Exhaust all of the capture moves:
	for (int i=0; i < capture_count; i++) {
		node_count++;
		game->qMakeMove(capture_list.at(i));
		if(!game->isInCheck(!game->getActivePlayer())) {
			value = -qSearch(game, -beta, -alpha, qDepth++);
			if( value >= beta ) {
				game->qUnMakeMove(capture_list.at(i));
				return beta; //hard
				//return value; //soft
			}
			if( value > alpha )
				alpha = value;
		}
		game->qUnMakeMove(capture_list.at(i));
	}
	return alpha;
}

int SEARCH::qSearch(CHESSBOARD * game, int alpha, int beta, int qDepth) {
	
	if(qDepth > q_depth)
		q_depth = qDepth;

	/*
	//Hashe table Check:
	int value = hashtable->SearchHash(game->getKey(),0,alpha,beta);
	if(value != HASH_UNKNOWN )
		return value;
	*/

	int value = relative_eval(game,alpha, beta);

	if (value >= beta) {
		//return beta;	//hard
		return value;	//soft
	}
	
	short delta = nPieceValue[nQueen];
	if(mask::pawns_spawn[!game->getActivePlayer()] & game->getPieceBB(game->getActivePlayer(),nPawn) )
		delta += nPieceValue[nQueen] - nPieceValue[nPawn]- nPieceValue[nPawn];
	
	if(value + delta < alpha)
		return value;
		
	if(value > alpha )
		alpha = value;

	//Get all of the capture moves:
	vector<MOVE> capture_list = CaptureGen(game);
	MoveSort(capture_list);
	int capture_count = (int) capture_list.size();

	//Exhaust all of the capture moves:
	for (int i=0; i < capture_count; i++) {
		node_count++;
		game->qMakeMove(capture_list.at(i));
		if(!game->isInCheck(!game->getActivePlayer())) {
			value = -qSearch(game, -beta, -alpha, qDepth++);
			if( value >= beta ) {
				game->qUnMakeMove(capture_list.at(i));
				//return beta; //hard
				return value; //soft
			}
			if( value > alpha )
				alpha = value;
		}
		game->qUnMakeMove(capture_list.at(i));
	}
	return alpha;
}
 

void SEARCH::PVS_Root() {

	int score = 0;
	bool foundPV = false;
	node_count++;

/***************************************************
	Check the Hash Table:
***************************************************/
	MOVE hash_suggestion;
	int hashe_value = hashtable->SearchHash(root_game->getKey(),root_depth,alpha,beta, &hash_suggestion);

	if(hashe_value != HASH_UNKNOWN ) {
		 score = hashe_value;
		 //Just in case, check to make sure there is actually a move here. not sure if this is actually needed or not.
		 if( hash_suggestion.from != 64 ) { 
			 best_score = score;
			 best_move = hash_suggestion;
			 end_time = clock()/(CLOCKS_PER_SEC/1000);
			 return;
		 }
	}
	char hashe_flag = HASH_ALPHA;

/****************************************************************
	Search:
*****************************************************************/

	//Start searching:
	short move_count = 0;
	bool checked = root_game->isInCheck();
	clearKillers(root_depth-1);
	MOVE next_move;
	MOVELIST move_list;
	
	while( move_list.NextRootMove(&next_move, root_game, hash_suggestion) ) {
		root_game->MakeMove(next_move);
		if(!root_game->isInCheck(!root_game->getActivePlayer())) {
			node_count++;
			move_count++;
			best_score = -PVS(root_depth -1, root_game, -beta, -alpha);
			best_move = next_move;
			root_game->unMakeMove(next_move);

			if(best_score > alpha) {
				if(best_score >= beta) {
					#ifdef SEARCH_STATS
						beta_cutoffs[move_count]++;
					#endif
					#ifdef DEBUG_SEARCH
						cout << "\n" << root_depth << ". beta: " << beta << ". best_score = " << best_score << endl;
					#endif
					hashtable->RecordHash(root_game->getKey(),root_depth, best_score, HASH_BETA, &next_move);
					return;
				}
				alpha = best_score;
				hashe_flag = HASH_EXACT;
			}
		}
		else {
			root_game->unMakeMove(next_move); }
		
		if( move_count == 1) break;
	}

	while( move_list.NextRootMove(&next_move, root_game, hash_suggestion) ) {
		root_game->MakeMove(next_move);
		if(!root_game->isInCheck(!root_game->getActivePlayer())) {
			node_count++;
			short depth_adjustment = 0;

			//LMR:
			if( move_list.phase() == NONCAPTURES	// Only reduce noncaptures
			&&	move_count >= LMR_THRESHOLD			// Wait for LATE moves
			//&& !foundPV							// Do not reduce PV nodes
			&&	root_depth >= LMR_HORIZON			// Do not reduce too close to the horizon
			&&	!checked							// Do not reduce if in check
			&&	depth_adjustment == 0				// Do not reduce if already extended (also makes sure opponent is not in check)
			) {
				depth_adjustment = -1;
			}
			
			move_count++;
	
			score = -PVS( (root_depth-1) + depth_adjustment, root_game, -alpha-1, -alpha, false);
			if(score > alpha && score < beta) {
				score = -PVS( (root_depth-1) + depth_adjustment, root_game, -beta, -alpha, false);
				if(score > alpha) {
					alpha = score;
					hashe_flag = HASH_EXACT;
				}
			}
			
			//Research at full depth if need be:
			if(depth_adjustment > 0 && score > alpha)
				score = -PVS(root_depth-1, root_game, -beta, -alpha, false); 
			root_game->unMakeMove(next_move);

			if(score > best_score) {
				best_score = score;
				if( score >= beta) {
					#ifdef SEARCH_STATS
						beta_cutoffs[move_count]++;
					#endif
					#ifdef DEBUG_SEARCH
						cout << "\n" << root_depth << ". beta: " << beta << ". score = " << score << endl;
					#endif
					hashtable->RecordHash(root_game->getKey(),root_depth, score, HASH_BETA, &next_move);
					return; 
				}
				best_move = next_move;
				//hashe_flag = HASH_EXACT;
			}
		}
		else {
			root_game->unMakeMove(next_move); }

		/*****************************************************
			Quit searching if the allotted time is up.
		*****************************************************/
		if(node_count % 2000000 == 0) {
			if(isOutOfTime())
				break;
		}
	}

	//if(fMate) {
	if( move_count == 0 ) {
		//Check for a stalemate:
		if(!root_game->isInCheck())
			best_score = contempt(root_game);
		//Return checkmate score:
		else {
			best_score = -MATE;
		}
	}
	else {
		hashtable->RecordHash(root_game->getKey(), root_depth, best_score, hashe_flag, &best_move);
	}

}

int SEARCH::PVS(int depth, CHESSBOARD * game, int alpha, int beta, bool fNulled) {

	//Draw by 50 move rule or 3 fold repition:
	if(game->getHalfMovesRule() >=4) {
		//check for 3 fold:
		if(game->checkThreeFold())
			return contempt(game);
		//check for 50 move:
		if(game->getHalfMovesRule() >= 50)
			return contempt(game);
	}

	//Hashe table Check:
	MOVE hash_suggestion;
	int score = hashtable->SearchHash(game->getKey(),depth,alpha,beta, &hash_suggestion);
	if(score != HASH_UNKNOWN )
		return score;
	
	char hashe_flag = HASH_ALPHA;
	
	//Final depth return:
	if(depth <= 0) {
		//int value = relative_eval(game);
		int value = qSearch(game, alpha, beta,1);
		//hashtable->RecordHash( game->getKey(), 0, value, HASH_EXACT );
		return value; 
	}

	bool checked = game->isInCheck();


/**************************************************************************************
	Null move if it is not a King/pawn game and the previous move want a null move.
***************************************************************************************/
	
	if(	!fNulled			//Do not null if we just nulled
	&&	!checked			//Do not null in check, thats stupid
	&&	(score>=alpha)		//Do not null if the hash table says that we might get a fail low.
	&& ( (game->getPieceBB(game->getActivePlayer(), nPawn) | game->getPieceBB(game->getActivePlayer(), nKing)) 
		   != game->getOccupiedBB(game->getActivePlayer())) ) 
	{
		game->NullMove(); //make nullmove!
		score = -PVS(depth - 1 - 2, game, -beta,-beta+1, true);
		game->unNullMove(); //unmake nullmove!
		if (score >=beta) {
			return beta; }
	}

/**************************************************************************************
	
***************************************************************************************/	

	MOVE best_local_move;
	bool foundPV = false;
	short move_count = 0;
	int best_local_score = -INFINITY;
	clearKillers(depth-1);
	MOVE next_move;
	MOVELIST move_list;

	//For fail-soft purposes, test the first move seperately:
	while( move_list.NextRootMove(&next_move, game, hash_suggestion) ) {
	//while( move_list.NextMove(&next_move, this, depth, game, hash_suggestion) ) {
		game->MakeMove(next_move);
		if(!game->isInCheck(!game->getActivePlayer())) {
			move_count++;
			node_count++;
			best_local_score = -PVS(depth-1, game, -beta, -alpha, false);
			best_local_move = next_move;
			game->unMakeMove(next_move);

			if(best_local_score > alpha) {
				if(best_local_score >= beta) {
					#ifdef SEARCH_STATS
						beta_cutoffs[move_count]++;
					#endif
					if(move_list.phase() == NONCAPTURES) {
						addKiller(depth, next_move);
					}
					hashtable->RecordHash( game->getKey(),depth, best_local_score, HASH_BETA, &best_local_move );
					return best_local_score;
				}
				alpha = best_local_score;
				hashe_flag = HASH_EXACT;
			}
		}
		else {
			game->unMakeMove(next_move);
		}
		if(move_count == 1) break;
	}

	//while( move_list.NextRootMove(&next_move, game, hash_suggestion) ) {
	while( move_list.NextMove(&next_move, this, depth, game, hash_suggestion) ) {
		game->MakeMove(next_move);
		if(!game->isInCheck(!game->getActivePlayer())) {
			node_count++;
			short depth_adjustment = 0;

			//LMR:
			if( move_list.phase() == NONCAPTURES	// Only reduce noncaptures
			&&	move_count >= LMR_THRESHOLD			// Wait for LATE moves
			&& hashe_flag != HASH_EXACT				// Do not reduce PV nodes
			&&	depth >= LMR_HORIZON				// Do not reduce too close to the horizon
			&&	!checked							// Do not reduce if in check
			&&	depth_adjustment == 0				// Do not reduce if already extended (also makes sure opponent is not in check)
			) {
				depth_adjustment = -1;
			}
			move_count++;

			score = -PVS( (depth-1)+depth_adjustment, game, -alpha-1, -alpha, false);
			if(score > alpha && score < beta) {
				score = -PVS( (depth-1)+depth_adjustment, game, -beta, -alpha, false);
				if( score > alpha ) {
					alpha = score;
					hashe_flag = HASH_EXACT;
				}
			}

			//Research at full depth if need be:
			if(depth_adjustment > 0 && score > alpha)
				score = -PVS(depth-1, game, -beta, -alpha, false); 

			game->unMakeMove(next_move);

			if( score > best_local_score ) {
				if( score >= beta ) {
					#ifdef SEARCH_STATS
						beta_cutoffs[move_count]++;
					#endif
					if(move_list.phase() == NONCAPTURES) {
						addKiller(depth, next_move);
					}
					hashtable->RecordHash( game->getKey(), depth, score, HASH_BETA, &next_move );
					return score;
				}
				best_local_score = score;
				best_local_move = next_move;
			}
		}
		else {
			game->unMakeMove(next_move);
		}
		
		/*****************************************************
			Quit searching if the allotted time is up.
		*****************************************************/
		if(node_count % 2000000 == 0) {
			if(isOutOfTime())
				break;
		}
	}

	//Return scores:
	if( move_count == 0 ) {
		if(!checked)
			return contempt(game); //stalemate
		else{
			return (-MATE + (root_depth - depth)); //checkmate
		}
	}
	else {
		hashtable->RecordHash(game->getKey(), depth, best_local_score, hashe_flag, &best_local_move);
		return best_local_score;
	}
	
}

void SEARCH::Hard_PVS_Root() {

	int score = 0;
	node_count++;

/***************************************************
	Check the Hash Table:
***************************************************/
	MOVE hash_suggestion;
	int hashe_value = hashtable->SearchHash(root_game->getKey(),root_depth,alpha,beta, &hash_suggestion);

	if(hashe_value != HASH_UNKNOWN ) {
		 score = hashe_value;
		 //Just in case, check to make sure there is actually a move here. not sure if this is actually needed or not.
		 if( hash_suggestion.from != 64 ) { 
			 alpha = score;
			 best_score = score;
			 best_move = hash_suggestion;
			 //end_time = clock()/(CLOCKS_PER_SEC/1000);
			 return;
		 }
	}

/****************************************************************
	Search:
*****************************************************************/

	//Start searching:
	char hashe_flag = HASH_ALPHA;
	short move_count = 0;
	bool checked = root_game->isInCheck();
	clearKillers(root_depth-1);
	MOVE next_move;
	MOVELIST move_list;
	
	while( move_list.NextRootMove(&next_move, root_game, hash_suggestion) ) {
		root_game->MakeMove(next_move);
		if(!root_game->isInCheck(!root_game->getActivePlayer())) {
			node_count++;
			short depth_adjustment = 0;
				
			/*
			//Check extension:
			if(root_game->isInCheck(root_game->getActivePlayer())) {
				depth_adjustment = 1;			//INCREASE the search depth by 1
			}
			*/
				
			
			//LMR:
			if( move_list.phase() == NONCAPTURES		// Only reduce noncaptures
				&& move_count >= LMR_THRESHOLD			// Wait for LATE moves
				&& hashe_flag != HASH_EXACT				// Do not reduce PV nodes
				&& root_depth >= LMR_HORIZON			// Do not reduce too close to the horizon
				&& !checked								// Do not reduce if in check
				&& depth_adjustment == 0				// Do not reduce if already extended (also makes sure opponent is not in check)
				) {
				depth_adjustment = -1;
			}
			
			move_count++;
				
			//PVS:
			if(hashe_flag == HASH_EXACT && root_depth > 2) { //equivalent to being a PV node.
				score = -Hard_PVS(root_depth -1 + depth_adjustment, root_game, -alpha-1, -alpha);
				if ( (score > alpha) && (score < beta) ) {
					score = -Hard_PVS(root_depth-1 + depth_adjustment, root_game, -beta, -alpha); }
			}
			else {
				score = -Hard_PVS(root_depth -1 + depth_adjustment, root_game, -beta, -alpha); 
			}
				
			//Research at normal depth if need be:
			//Todo: include a PVS here?
			if(depth_adjustment > 0 && score > alpha) {
				score = -Hard_PVS(root_depth -1, root_game, -beta, -alpha);
			}

			//Normal Alpha-Beta:
			root_game->unMakeMove(next_move);
			if( score >= beta ) {
				hashtable->RecordHash(root_game->getKey(),root_depth, beta, HASH_BETA, &next_move);
				best_move = next_move;
				best_score = score;
				break; 
			}
			if(score > alpha) {
				alpha = score;
				hashe_flag = HASH_EXACT;
				best_move = next_move;
				best_score = score;
			}
		}
		else {
			root_game->unMakeMove(next_move); }

		/*****************************************************
			Quit searching if the allotted time is up.
		*****************************************************/
		if(node_count % 2000000 == 0) {
			if(isOutOfTime())
				break;
		}
	}

	if(move_count == 0) {
		//Check for a stalemate:
		if(!root_game->isInCheck())
			best_score = contempt(root_game);
		//Return checkmate score:
		else {
			best_score = -MATE;
		}
	}
	else {
		hashtable->RecordHash(root_game->getKey(), root_depth, alpha, hashe_flag, &best_move);
	}
}

int SEARCH::Hard_PVS(int depth, CHESSBOARD * game, int alpha, int beta, bool fNulled) {

	//Draw by 50 move rule or 3 fold repition:
	if(game->getHalfMovesRule() >=4) {
		//check for 3 fold:
		if(game->checkThreeFold())
			return contempt(game);
		//check for 50 move:
		if(game->getHalfMovesRule() >= 50)
			return contempt(game);
	}

	//Hashe table Check:
	MOVE hash_suggestion;
	int score = hashtable->SearchHash(game->getKey(),depth,alpha,beta, &hash_suggestion);
	if(score != HASH_UNKNOWN )
		return score;
	
	//Final depth return:
	if(depth <= 0) {
		//int value = relative_eval(game);
		int value = Hard_qSearch(game, alpha, beta,1);
		//hashtable->RecordHash( game->getKey(), 0, value, HASH_EXACT );
		return value; 
	}

	bool checked = game->isInCheck();


/**************************************************************************************
	Null move if it is not a King/pawn game and the previous move want a null move.
***************************************************************************************/
	
	if(	!fNulled			//Do not null if we just nulled
	&&	!checked			//Do not null in check, thats stupid
	&&	(score>=alpha)		//Do not null if the hash table says that we might get a fail low.
							//Do not null in a King/Pawn game:
	&& ( (game->getPieceBB(game->getActivePlayer(), nPawn) | game->getPieceBB(game->getActivePlayer(), nKing)) 
		   != game->getOccupiedBB(game->getActivePlayer())) ) 
	{
		game->NullMove(); //make nullmove!
		score = -Hard_PVS(depth - 1 - 2, game, -beta,-beta+1, true);
		game->unNullMove(); //unmake nullmove!
		if (score >=beta) {
			return beta; }
	}

/**************************************************************************************
	
***************************************************************************************/	
	char hashe_flag = HASH_ALPHA;
	MOVE best_local_move;
	short move_count = 0;
	clearKillers(depth-1);
	MOVE next_move;
	MOVELIST move_list;

	while( move_list.NextMove(&next_move, this, depth, game, hash_suggestion) ) {
		game->MakeMove(next_move);
		if(!game->isInCheck(!game->getActivePlayer())) {
			node_count++;
			short depth_adjustment = 0;
				
			/*
			//Check extension:
			if(game->isInCheck(game->getActivePlayer())) {
				depth_adjustment = 1; //INCREASE the search depth by 1
			}
			*/

			//LMR:
			if( move_list.phase() == NONCAPTURES	// Do not reduce captures
				&& move_count >= LMR_THRESHOLD		// Wait for LATE moves
				&& hashe_flag != HASH_EXACT			// Do not reduce PV nodes
				&& depth >= LMR_HORIZON				// Do not reduce too close to the horizon
				&& !checked							// Do not reduce if in check
				&& depth_adjustment == 0			// Do not reduce if already extended (also makes sure opponent is not in check)
			) {
				depth_adjustment = -1;
			}
			
			move_count++;
			//PVS:
			if(hashe_flag == HASH_EXACT && depth > 2) {
				score = -Hard_PVS(depth-1 + depth_adjustment, game, -alpha-1, -alpha, false);
				if ( (score > alpha) && (score < beta) ) {
					score = -Hard_PVS(depth-1 + depth_adjustment, game, -beta, -alpha, false); }
			}
			else
				score = -Hard_PVS(depth-1 + depth_adjustment, game, -beta, -alpha, false); 
				
			//Research at full depth if need be:
			if(depth_adjustment > 0 && score > alpha)
				score = -PVS(depth-1, game, -beta, -alpha, false); 
			game->unMakeMove(next_move);
			//Normal Alpha-Beta:
			if( score >= beta ) {
				if(move_list.phase() == NONCAPTURES) {
					addKiller(depth, next_move);
				}
				#ifdef SEARCH_STATS
					if(move_list.phase() == KILLER_MOVE_1 || move_list.phase() == KILLER_MOVE_2) {
						killer_cutoffs[move_count]++;
					}
				#endif
				hashtable->RecordHash( game->getKey(),depth, beta, HASH_BETA, &next_move );
				return beta; }
			if(score > alpha) {
				hashe_flag = HASH_EXACT;					
				alpha = score;
				best_local_move = next_move;
			}
		}
		else {
			game->unMakeMove(next_move); }

		/*****************************************************
			Quit searching if the allotted time is up.
		*****************************************************/
		if(node_count % 2000000 == 0) {
			if(isOutOfTime())
				break;
		}
	}

	//Return scores:
	if(move_count == 0) {
		if(!game->isInCheck())
			return contempt(game); //stalemate
		else
			return (-MATE + (root_depth - depth)); //checkmate
	}
	else {
		hashtable->RecordHash(game->getKey(), depth, alpha, hashe_flag, &best_local_move);
		return alpha;
	}

}

/*
int qSearch(CHESSBOARD game, int alpha, int beta) {
	
	int value = relative_eval(&game);

	//Checkmate?
	if(value == LOOSE || value == WIN)
		return value;

	if (value >= beta)
		return beta;
	if(value > alpha )
		alpha = value;
	
	//Get all of the capture moves:
	vector<MOVE> capture_list = CaptureGen(&game);
	MoveSort(&capture_list);

	int capture_count = (int) capture_list.size();
	
	//Exhaust all of the capture moves:
	for (int i=0; i < capture_count; i++) {

		CHESSBOARD subgame = game;
		subgame.MakeMove(capture_list.at(i));
		value = -qSearch(subgame, -beta, -alpha);
		if( value >= beta )
			return beta;
		if( value > alpha )
			alpha = value;
	}
	
	return alpha;
}
*/
int NegaMax(int depth, CHESSBOARD * game) {
	if(depth == 0) {
		int eval = relative_eval(game);
		//cout << "player: " << game.getActivePlayer() << " d0: " << eval << endl;
		//int eval = qSearch(game, -10000,10000);
		return eval; }
	
	int max = -INFINITY;
	
	vector<MOVE> move_list = MoveGen(game);	
	for (int i=0;  i < move_list.size(); i++) {	
		//CHESSBOARD subgame = game;
		game->MakeMove(move_list.at(i));
		//NODES++;
		int score = -NegaMax(depth-1, game);
		if(score > max)
			max = score;
		game->unMakeMove(move_list.at(i));
	}
	
	return max;
}

void SEARCH::addKiller(short depth, MOVE killer) {
	//cout << "Added Killer: ";
	//killer.print(true);
	//cout << endl;
	if(killer_counter[0][depth] == 0) {
		killer_move[0][depth] = killer;
		killer_counter[0][depth]=1;
		return;
	}
	if(killer_counter[1][depth] == 0) {
		killer_move[1][depth] = killer;
		killer_counter[1][depth]=1;
		return;
	}
	
	if(killer_move[0][depth] == killer) {
		killer_counter[0][depth]++;
		return;
	}
	if(killer_move[1][depth] == killer) {
		killer_counter[1][depth]++;
		return;
	}

	if(killer_counter[0][depth] >= killer_counter[1][depth]) {
		killer_move[1][depth] = killer;
		killer_counter[1][depth]=1;
	}
	else {
		killer_move[0][depth] = killer;
		killer_counter[0][depth]=1;
	}
}

void SEARCH::clearKillers(short depth) {
	killer_move[0][depth].from = 64;
	killer_move[1][depth].from = 64;
	killer_counter[0][depth] = 0;
	killer_counter[1][depth] = 0;	
}

short SEARCH::countKillers() {
	if(killer_counter[1] > 0)
		return 2;
	if(killer_counter[0] > 0)
		return 1;
	return 0;
}