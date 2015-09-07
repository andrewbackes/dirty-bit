#include <vector>
#include <ctime>
#include <iostream>

#include "search.h"

#include "move.h"
#include "evaluate.h"
#include "movegen.h"
#include "utilities.h"
#include "nextmove.h"

#include "parameters.h"

using namespace std;



void SEARCH::setHasheTable(HASHTABLE * t) {
	hashtable = t;
}

bool SEARCH::isOutOfTime() {

	if( (clock()/(CLOCKS_PER_SEC/1000) - start_time >= allotted_time) || fTimeOut ) {
		#ifdef DEBUG_TIME
			cout << "\tSearch timed out."  
				 << "\tNow-Start: " 		<< clock()/(CLOCKS_PER_SEC/1000) - start_time
				 << "\tAlotted: " 			<< allotted_time << endl;
			cout << "\tNode: " << node_count << endl;
		#endif
		fTimeOut = true;
		return true;
	}
	return false;

}

void SEARCH::start(int alpha_bound, int beta_bound) {
	root_alpha = alpha_bound;
	root_beta = beta_bound;
	best_score = root_alpha - 1;
	selective_depth = 0;

	//for record keeping:
	start_time = clock()/(CLOCKS_PER_SEC/1000);
	node_count = 0;
	
	//Generate move ordering tables:
	//short gauge = phase_gauge(root_game);
	//initOrderingTables(gauge);
	
	//Search:
	//PVS_Root();
	Hard_PVS_Root();
	collectPV();

	//for record keeping:
	end_time = clock()/(CLOCKS_PER_SEC/1000);


}

void SEARCH::collectPV() {
	//This function gathers the PV info from the Hash Tables.
	// Output: Populates the pv vector.

	pv.clear();
	
	for(int d=0; d < root_depth; d++) {
		MOVE hashed_move = hashtable->getHashMove(root_game->getKey());
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

int SEARCH::Hard_qSearch(CHESSBOARD * game, int alpha, int beta, int qPly) {
	
	int value = evaluate(game, side_to_move);
	/*
	int value2 = evaluate_old(game, alpha, beta);
	if (abs(value - value2) > 1) {
		game->print();
		cout << "old: " << value << ". new: " << value2 << endl;
		system("PAUSE");
	}
	*/
	if(qPly > q_max_ply) {
		q_max_ply = qPly;
		
		if(qPly >= Q_CUTOFF) {
			return value;
		}
		
	}

	/*
	//Hashe table Check:
	int value = hashtable->SearchHash(game->getKey(),0,alpha,beta);
	if(value != HASH_UNKNOWN )
		return value;
	*/

	if (value >= beta) {
		return beta;	//hard
		//return value;	//soft
	}
	
	short delta = nPieceValue[nQueen];
	if (mask::pawns_spawn[!game->getActivePlayer()] & game->getPieceBB(game->getActivePlayer(), nPawn))
		delta += QUEEN_VALUE - PAWN_VALUE - PAWN_VALUE;
	
	if(value + delta < alpha)
		return value;
		
	if(value > alpha )
		alpha = value;

	//Get all of the capture moves:
	q_move_list[qPly].reset();

	//Exhaust all of the capture moves:
	while(q_move_list[qPly].NextQMove()) {
		game->qMakeMove(*q_move_list[qPly].move());
		if(!game->isInCheck(!game->getActivePlayer())) {
			node_count++;
			value = -Hard_qSearch(game, -beta, -alpha, qPly+1);
			game->qUnMakeMove(*q_move_list[qPly].move());
			if( value >= beta ) {
				return beta; //hard
				//return value; //soft
			}
			if( value > alpha )
				alpha = value;
		}
		else {
			game->qUnMakeMove(*q_move_list[qPly].move());
		}

		/*****************************************************
			Quit searching if the allotted time is up.
		*****************************************************/
		if ( (node_count & nodes_for_time_check_) == 0) {
			if (isOutOfTime())
				break;
		}

	}
	return alpha;
}

void SEARCH::Hard_PVS_Root() {
	int alpha = root_alpha;
	int beta = root_beta;
	int ply = 0;
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
	/*
	move_list[root_depth].reset();
	move_list[root_depth].addHash(hash_suggestion);
	move_list[root_depth].clearKillers();
	move_list[root_depth - 1].clearKillers();
	*/
	
	move_list[ply].reset();
	move_list[ply].addHash(hash_suggestion);
	move_list[ply].clearKillers();
	move_list[ply + 1].clearKillers();
	//short move_list_size = move_list[ply].PopulateCompleteList();
	


	while( move_list[ply].NextRootMove() ) {
	//#pragma omp for
	//for ( short i = 0 ; i < move_list_size ; i++) {
		//move_list[ply].NextRootMove(true);
		root_game->MakeMove(*move_list[ply].move());
		if(!root_game->isInCheck(!root_game->getActivePlayer())) {
			node_count++;
			short depth_adjustment = 0;
				
			#ifdef ENABLE_CHECK_EXTENSIONS			
			//Check extension:
			if(root_game->isInCheck(root_game->getActivePlayer())) {
				depth_adjustment = 1;			//INCREASE the search depth by 1. This also implies that when =1, then the opponent is in check.
			}
			#endif
				
			#ifdef ENABLE_ROOT_LMR
			//PVS with LMR
			if (move_count == 0)
				score = -Hard_PVS(root_depth - 1 + depth_adjustment, ply + 1, root_game, -beta, -alpha, false);
			else {
				score = alpha + 1; //recommended hack.

				if (	move_list[ply].phase() == NONCAPTURES			// Do not reduce or prune winning captures or killers
					&&	!checked										// Do not reduce or prune if in check
					&&	depth_adjustment == 0							// Do not reduce or prune if already extended aka oppenent in check
					&&	move_list[ply].move()->promote_piece_to == 0	// No promotions
					&&	move_count >= LMR_THRESHOLD						// Wait for LATE moves
					&&	root_depth >= LMR_HORIZON						// Do not reduce too close to the horizon
					&& (move_list[ply].move()->active_piece_id != PAWN_VALUE || //Do not reduce passed pawns
					(root_game->getPieceBB(!move_list[ply].move()->color, nPawn) & mask::passed_pawn[move_list[ply].move()->color][move_list[ply].move()->to]))
					){
						score = -Hard_PVS(root_depth - 1 - LMR_REDUCTION/*-((parent_moves_searched>=4&&root_depth>=3)?1:0)*/, ply + 1, root_game, -(alpha + 1), -alpha, false);
					
				}

				if (score > alpha) { //research
					score = -Hard_PVS(root_depth - 1 + depth_adjustment, ply + 1, root_game, -(alpha + 1), -alpha, false);
					if (score > alpha && score < beta) {
						score = -Hard_PVS(root_depth - 1 + depth_adjustment, ply + 1, root_game, -beta, -alpha, false);
					}
				}
			}
			#endif		

			#ifndef ENABLE_ROOT_LMR			
			//PVS:
			if(hashe_flag == HASH_EXACT && root_depth > PVS_HORIZON) { //equivalent to being a PV node.
				score = -Hard_PVS(root_depth -1 + depth_adjustment, ply+1, root_game, -alpha-1, -alpha);
				if ( (score > alpha) && (score < beta) ) {
					score = -Hard_PVS(root_depth-1 + depth_adjustment, ply+1, root_game, -beta, -alpha); }
			}
			else {
				score = -Hard_PVS(root_depth -1 + depth_adjustment, ply+1, root_game, -beta, -alpha); 
				//Research at normal depth if need be:
				if(depth_adjustment < 0 && score > alpha) {
					score = -Hard_PVS(root_depth -1, ply+1, root_game, -beta, -alpha); //should + depth_adjustment?
				}
			}
			#endif
			//Normal Alpha-Beta:
			move_count++;
			root_game->unMakeMove(*move_list[ply].move());
			if( score >= beta ) {
				#ifdef SEARCH_STATS
					cutoff_phase[move_list[depth].current_phase]++;
					cutoff_move[move_count]++;
				#endif
				hashtable->RecordHash(root_game->getKey(),root_depth, beta, HASH_BETA, move_list[ply].move());
//TODO: WTF IS BELOW THIS LINE!!!!!!!!!!				
				
				//best_move = *move_list[ply].move();
				hashe_flag = HASH_BETA;
				best_score = score;
				break; 
			}
			
			if(score > alpha) {
				alpha = score;
				hashe_flag = HASH_EXACT;
				best_move = *move_list[ply].move();
				best_score = score;
				
			}
		}
		else {
			root_game->unMakeMove(*move_list[ply].move()); 
		}

		/*****************************************************
			Quit searching if the allotted time is up.
		*****************************************************/
		if( (node_count & nodes_for_time_check_) == 0) {
			if (isOutOfTime()) {
				break;
			}
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
		hashtable->RecordHash(root_game->getKey(), root_depth, alpha, hashe_flag, &best_move); //should the score be alpha?
	}
}

int SEARCH::Hard_PVS(unsigned char depth, unsigned char ply, CHESSBOARD * game, int alpha, int beta, bool fNulled/*, unsigned char parent_moves_searched*/) {

	//Draw by 50 move rule or 3 fold repition:
	if(game->getHalfMovesRule() >=4) {
		//check for 3 fold and/or check for 50 move:
		if (game->checkThreeFold() || (game->getHalfMovesRule() >= 100))
			return contempt(game);
	}
	#ifdef INSUFFICIENT_MATERIAL_CHECK
	else {
		if(insufficientMaterial(game)) return DRAW;
	}
	#endif
	
	//Hashe table Check:
	MOVE hash_suggestion;	
	int score = hashtable->SearchHash(game->getKey(), depth, alpha, beta, &hash_suggestion);
	if(score != HASH_UNKNOWN )
		return score;

	//Final depth return:
	if(depth <= 0) {
		if(ply > selective_depth)
			selective_depth = ply;
		//int value = evaluate(game);
		int value = Hard_qSearch(game, alpha, beta,0);
		//hashtable->RecordHash( game->getKey(), 0, value, HASH_EXACT );
		return value; 
	}
	//move_list[ply + 1].clearKillers();
	bool checked = game->isInCheck();
	
	

/**************************************************************************************
	Null move if it is not a King/pawn game and the previous move want a null move.
***************************************************************************************/
	#ifdef ENABLE_NULLMOVE
	
	move_list[ply + 1].clearKillers();

	if(	!fNulled				//Do not null if we just nulled
	&&	!checked				//Do not null in check, thats stupid
	//&&	hash_score!=alpha	//Do not null if the hash table says that we might get a fail low. (BUG: that is NOT what the condition written actually does!)
	//Do not null in a King/Pawn game:
	&& ( (game->getPieceBB(game->getActivePlayer(), nPawn) | game->getPieceBB(game->getActivePlayer(), nKing)) 
		   != game->getOccupiedBB(game->getActivePlayer())) ) 
	{
		game->NullMove(); //make nullmove!
		if(depth - 1 - NULL_REDUCTION > 0) {
			
			score = -Hard_PVS(depth - 1 - NULL_REDUCTION, ply+1, game, -beta,-beta+1, true);
		}
		else {
			score = -Hard_qSearch(game, -beta, -beta+1, 0);
		}
		game->unNullMove(); //unmake nullmove!
		if (score >=beta) {
			//hashtable->RecordHash( game->getKey(),depth, beta, HASH_BETA );
			return beta; }
	}
	#endif

/**************************************************************************************
	IID: If there is no hash suggestion, try to generate one with a lower depth search.
***************************************************************************************/	
	move_list[ply].reset();
	
	#ifdef ENABLE_IID	
	if((hash_suggestion.from == 64) 
	&& (depth >= IID_HORIZON) 
	&& !fNulled 
	&& ply > 1
	&& ((ply&1) ? root_alpha : -root_beta) == alpha
	&& ((ply&1) ? root_beta  : -root_alpha)== beta
	//&& alpha != beta -1
	){
		
		score = Hard_PVS(depth-IID_REDUCTION, ply+1, game, alpha, beta, false);


		//if(t <= alpha) 
		//	cout << "alpha cutoff!\n";
		// Recheck the hash table to get what the lower depth search generated:
		//score = hashtable->SearchHash(game->getKey(), depth, -INFTY, beta, &hash_suggestion);
		hash_suggestion = hashtable->getHashMove(game->getKey());
		/*
		if(hash_suggestion.from == 64 )
			cout << "*IID didnt work!*\n";
		else
			cout << "IID worked!\n";
		*/
	}
	#endif
	move_list[ply].addHash(hash_suggestion);

/**************************************************************************************
	Search
***************************************************************************************/
	char hashe_flag = HASH_ALPHA;
	MOVE best_local_move;
	unsigned char move_count = 0;	
	
	#ifdef SEARCH_STATS
		short last_alpha = 0;
		short last_alpha_phase = 0;
	#endif
	
	move_list[ply + 1].clearKillers();
	
	while( move_list[ply].NextMove() ) {
		game->MakeMove(*move_list[ply].move());

		#ifdef DEBUG_ZOBRIST
		if (game->generateKey() != game->getKey()) {
			game->print();
			game->print_bb();
			game->print_mailbox();

			cout << "keys dont match. search: ";
			move_list[ply].move()->print(true);
			cout << "movelist phase: " << (int) move_list[ply].phase() << endl;
			
			for (int i = 1; i <= ply; i++) {
				cout << "killers ply " << i << ": (" << move_list[i].countKiller(0) << " ) ";
				move_list[i].getKiller(0).print(true);
				cout << endl;
				cout << "killers ply " << i << ": (" << move_list[i].countKiller(1) << " ) ";
				move_list[i].getKiller(1).print(true);
				cout << endl;
			}
			cout << "KEYS DONT MATCH!" << endl;
			system("PAUSE");
		}
		#endif

		if(!game->isInCheck(!game->getActivePlayer())) {
			node_count++;

			short depth_adjustment = 0;	
			#ifdef ENABLE_CHECK_EXTENSIONS
			//Check extension:
			if(game->isInCheck(game->getActivePlayer())) {
				depth_adjustment = 1; //INCREASE the search depth by 1. Implies that opponent is in check.
			}
			#endif

			#ifdef ENABLE_LMR
			//PVS with LMR
			if(move_count == 0)
				score = -Hard_PVS(depth-1 + depth_adjustment, ply+1, game, -beta, -alpha, false);
			else {
				score = alpha+1; //recommended hack.
				
				//LMR and Futility Conditions:
				if( move_list[ply].phase() == NONCAPTURES			// Do not reduce or prune winning captures or killers
				&&	!checked										// Do not reduce or prune if in check
				&&	depth_adjustment == 0							// Do not reduce or prune if already extended aka oppenent in check
				&&	(move_list[ply].move()->promote_piece_to==0)	// No promotions
				) {
					#ifdef ENABLE_FUTILITY_PRUNING
					//Futility Pruning:
					if (depth <= FUTILITY_DEPTH
						//&& (game->getMaterialValue(!game->getActivePlayer()) - game->getMaterialValue(game->getActivePlayer())) 
						&& -game->getRelativeMaterialValue()
							+ futility_margin[depth] < alpha	//Futility condition
						) {
						move_count++;
						game->unMakeMove(*move_list[ply].move());
						continue;
					}
					#endif					
					
					//LMR:
					if(	move_count >= LMR_THRESHOLD				// Wait for LATE moves
					&&	depth >= LMR_HORIZON					// Do not reduce too close to the horizon
					&& ( move_list[ply].move()->active_piece_id != PAWN_VALUE || //Do not reduce passed pawns
							(game->getPieceBB(!move_list[ply].move()->color,nPawn) & mask::passed_pawn[move_list[ply].move()->color][move_list[ply].move()->to]) )
					){
						score = -Hard_PVS(depth-1-LMR_REDUCTION/*-((parent_moves_searched>=4&&depth>=3)?1:0)*/, ply+1, game, -(alpha+1), -alpha, false);
					}
				}
					
				if(score > alpha) { //research
					score = -Hard_PVS(depth-1+depth_adjustment, ply+1, game, -(alpha+1), -alpha, false);
					if(score > alpha && score < beta) {
						score = -Hard_PVS(depth-1+depth_adjustment, ply+1, game, -beta, -alpha, false);
					}
				}
			}
			#endif

			#ifndef ENABLE_LMR
			//PVS:
			if(hashe_flag == HASH_EXACT && depth > PVS_HORIZON) {
				score = -Hard_PVS(depth-1 + depth_adjustment, ply+1, game, -alpha-1, -alpha, false);
				if ( (score > alpha) && (score < beta) ) {
					score = -Hard_PVS(depth-1 + depth_adjustment, ply+1, game, -beta, -alpha, false); }
			}
			else {
				score = -Hard_PVS(depth-1 + depth_adjustment, ply+1, game, -beta, -alpha, false); 
			}
			#endif

			move_count++;
			game->unMakeMove(*move_list[ply].move());
			
			//Normal Alpha-Beta:
			if( score >= beta ) {
				#ifdef ENABLE_KILLER_MOVES
				if (move_list[ply].phase() == NONCAPTURES && move_list[ply].move()->promote_piece_to == 0) {
					move_list[ply].addKiller(*move_list[ply].move());
				}
				#endif
				#ifdef SEARCH_STATS
					cutoff_phase[move_list[ply].current_phase ]++;
					cutoff_move[move_count]++;
				#endif
				hashtable->RecordHash( game->getKey(),depth, beta, HASH_BETA, move_list[ply].move() );
				return beta; 
			}
			if(score > alpha) {
				#ifdef SEARCH_STATS
					last_alpha = move_count;
					
					last_alpha_phase = move_list[ply].current_phase;
				#endif
				hashe_flag = HASH_EXACT;					
				alpha = score;
				best_local_move = *move_list[ply].move();
			}
		}
		else {
			game->unMakeMove(*move_list[ply].move()); }

		/*****************************************************
			Quit searching if the allotted time is up.
		*****************************************************/
		if ( (node_count & nodes_for_time_check_) == 0) {
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
		#ifdef SEARCH_STATS
			alpha_move[last_alpha]++;
			alpha_phase[last_alpha_phase]++;
		#endif
		return alpha;
	}

}

/*
void SEARCH::addKiller(MOVE killer, unsigned char depth) {
	//cout << "Added Killer: ";
	//killer.print(true);
	//cout << endl;
	if(move_list[depth].killer_counter[0] == 0) {
		move_list[depth].killer_move[0]= killer;
		move_list[depth].killer_counter[0]=1;
		return;
	}
	if(move_list[depth].killer_counter[1] == 0) {
		move_list[depth].killer_move[1] = killer;
		move_list[depth].killer_counter[1]=1;
		return;
	}
	
	if(move_list[depth].killer_move[0] == killer) {
		move_list[depth].killer_counter[0]++;
		return;
	}
	if(move_list[depth].killer_move[1] == killer) {
		move_list[depth].killer_counter[1]++;
		return;
	}

	if(move_list[depth].killer_counter[0] >= move_list[depth].killer_counter[1]) {
		move_list[depth].killer_move[1] = killer;
		move_list[depth].killer_counter[1]=1;
	}
	else {
		move_list[depth].killer_move[0]= killer;
		move_list[depth].killer_counter[0]=1;
	}
}
*/