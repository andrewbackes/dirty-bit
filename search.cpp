#include <vector>
#include <ctime>
#include <iostream>

#include "search.h"

#include "move.h"
#include "evaluate.h"
#include "movegen.h"
#include "utilities.h"
#include "nextmove.h"
#include "engine.h"
#include "parameters.h"

using namespace std;

SEARCH::SEARCH(ENGINE * e, ITERATION_INFO * id) {
	iteration = id;
	root_depth = id->depth; 
	root_game = &(e->game);
	q_max_ply = 0;
	selective_depth = 0;
	start_time = 0; end_time = 0; allotted_time = id->time_allotted; fTimeOut = false;
	node_count = 0; best_score = -INFTY;
	hashtable = &(e->hashtable);
	side_to_move = e->game.getActivePlayer();
	for (int i = 0; i < MAX_PLY; i++) {
		move_list[i].linkGame(root_game);
	}
	for (int i = 0; i < Q_CUTOFF; i++) {
		q_move_list[i].linkGame(root_game);
	}

}

SEARCH::SEARCH(int depth, CHESSBOARD * game, HASHTABLE * h, long time)
{
	root_depth = depth; root_game = game; q_max_ply = 0; selective_depth = 0;
	start_time = 0; end_time = 0; allotted_time = time; fTimeOut = false;
	node_count = 0; best_score = -INFTY;
	hashtable = h; side_to_move = game->getActivePlayer();
	//move_list = new MOVELIST[root_depth+1];
	//for(int i =0; i<=root_depth; i++) {
	for (int i = 0; i < MAX_PLY; i++) {
		move_list[i].linkGame(root_game);
	}
	for (int i = 0; i < Q_CUTOFF; i++) {
		q_move_list[i].linkGame(root_game);
	}
	#ifdef SEARCH_STATS
	for (int i = 0; i <= BAD_CAPTURES; i++)
		cutoff_phase[i] = 0;
	for (int i = 0; i < 256; i++) {
		cutoff_move[i] = 0;
		alpha_move[i] = 0;
	}
	#endif
}
SEARCH::~SEARCH()
{
	//hashtable->printStats();
	#ifdef SEARCH_STATS
	if (root_depth >= 8) print_stats();
	#endif
	//delete move_list;
}

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
	
	
	//update iteration record:
	if (best_score <= iteration->alpha || best_score >= iteration->beta)
		iteration->failed = true;
	iteration->move = best_move;
	iteration->score = best_score;
	iteration->nodes = node_count;
	iteration->pv = getPV();
	iteration->timed_out = TimedOut();
	iteration->seldepth = selective_depth;
	
	end_time = clock() / (CLOCKS_PER_SEC / 1000);
	iteration->time_taken = end_time - start_time;
	


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
	
	if(qPly > q_max_ply) {
		q_max_ply = qPly;
		if(qPly >= Q_CUTOFF) {
			return value;
		}
	}

	if (value >= beta) {
		return beta;	//hard
		//return value;	//soft
	}
	
	short delta = nPieceValue[nQueen];
	if (mask::pawns_spawn[!game->getActivePlayer()] & game->getPieceBB(game->getActivePlayer(), nPawn))
		delta += QUEEN_VALUE - PAWN_VALUE - PAWN_VALUE;
	
	if(value + delta <= alpha)
		return value;
		
	if(value > alpha )
		alpha = value;
	
	bool checked = game->isInCheck(game->getActivePlayer());
	unsigned char move_count = 0;

	//Get all of the capture moves:
	q_move_list[qPly].reset();

	//Exhaust all of the capture moves:
	while(q_move_list[qPly].NextQMove(checked)) {
		
		//Check for aborting conditions:
		if (abort()) return alpha;

		game->qMakeMove(*q_move_list[qPly].move());
		if(!game->isInCheck(!game->getActivePlayer())) {
			node_count++;
			
			// Delta/Futility pruning:
			/*
			if ( !checked
					&& value + q_move_list[qPly].move()->static_value + 100 <= alpha 
					&& !(game->isInCheck(game->getActivePlayer()))
			) {
				move_count++;
				game->qUnMakeMove(*q_move_list[qPly].move());
				continue;
			}
			*/
			value = -Hard_qSearch(game, -beta, -alpha, qPly+1);
			game->qUnMakeMove(*q_move_list[qPly].move());
			if( value >= beta ) {
				return beta; //hard
				//return value; //soft
			}
			if( value > alpha )
				alpha = value;

			move_count++;
		}
		else {
			game->qUnMakeMove(*q_move_list[qPly].move());
		}	
	}

	//Return scores:
	if (move_count == 0 && checked) {
		return (-MATE + (root_depth + qPly)); //checkmate
	}
	else {
		return alpha;
	}

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
	
	move_list[ply].reset();
	move_list[ply].addHash(hash_suggestion);
	move_list[ply].clearKillers();
	move_list[ply + 1].clearKillers();
	
	while( move_list[ply].NextRootMove() ) {
		//check for aborting conditions:
		if (abort()) return;

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
	
	bool checked = game->isInCheck();
	bool pv_node = (beta != alpha + 1); // zero_window = (beta == alpha + 1);

/**************************************************************************************
	Experimental null-window Beta Pruning
***************************************************************************************/
	short node_score = game->getRelativeMaterialValue();	
	#ifdef ENABLE_BETA_PRUNING
	if (!pv_node && depth <= 3 && !checked && (beta<NEARMATE)) {
		node_score = evaluate(game, this->side_to_move);
		if ( node_score - (depth * 50) >= beta)
			return node_score;
	}
	#endif
	
/**************************************************************************************
	Null move if it is not a King/pawn game and the previous move want a null move.
***************************************************************************************/
	move_list[ply + 1].clearKillers();
	
	#ifdef ENABLE_NULLMOVE
	if (//!pv_node
			!fNulled				//Do not null if we just nulled 
		&& !checked				//Do not null in check, thats stupid
		&& beta < NEARMATE
		&&	(score!=HASH_UNKNOWN || score < alpha)	//Do not null if the hash table says that we might get a fail low. (BUG: that is NOT what the condition written actually does!)
		//Do not null in a King/Pawn game:
		&& ((game->getPieceBB(game->getActivePlayer(), nPawn) | game->getPieceBB(game->getActivePlayer(), nKing)) != game->getOccupiedBB(game->getActivePlayer()))
		//&& (node_score = evaluate(game, this->side_to_move)) > beta
	){
		game->NullMove(); //make nullmove!
		if(depth - 1 - NULL_REDUCTION > 0) {
			score = -Hard_PVS(depth - 1 - NULL_REDUCTION, ply+1, game, -beta,-beta+1, true);
		}
		else {
			score = -Hard_qSearch(game, -beta, -beta + 1, 0);
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
	&& pv_node //!fNulled 
	&& ply > 1
	&& ((ply&1) ? root_alpha : -root_beta) == alpha
	&& ((ply&1) ? root_beta  : -root_alpha)== beta
	){
		score = Hard_PVS(depth-IID_REDUCTION, ply+1, game, alpha, beta, false);
		if(score > alpha) {
			hash_suggestion = hashtable->getHashMove(game->getKey());
		}
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
		//Check for aborting conditions:
		if (abort()) return alpha;
		
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

			bool extensions = 0;
			#ifdef ENABLE_CHECK_EXTENSIONS
			//Check extension:
			bool gives_check = game->isInCheck(game->getActivePlayer());
			/*
			if(game->isInCheck(game->getActivePlayer())) {
				depth_adjustment = 1; //INCREASE the search depth by 1. Implies that opponent is in check.
			}
			*/
			if ((depth <= 4 && gives_check)
				|| (pv_node && gives_check)
				//|| (pv_node && move_list[ply].phase() == CAPTURES)
			){
				extensions = true;
			}

			#endif

			#ifdef ENABLE_LMR
			//PVS with LMR
			if(move_count == 0)
				score = -Hard_PVS(depth - 1 + extensions, ply + 1, game, -beta, -alpha, false);
			else {
				score = alpha+1; //recommended hack.
				
				//LMR and Futility Conditions:
				if( move_list[ply].phase() == NONCAPTURES			// Do not reduce or prune winning captures or killers
				&&	!checked										// Do not reduce or prune if in check
				&&	!gives_check							// Do not reduce or prune if already extended aka oppenent in check
				&&	(move_list[ply].move()->promote_piece_to==0)	// No promotions
				) {
					
					#ifdef ENABLE_LMP
					// Experimental late move pruning
					if (!pv_node && depth <= 3 && move_count >= (LMR_THRESHOLD + 1) * depth
						&&	move_list[ply].phase() == NONCAPTURES
						&&	beta < NEARMATE && alpha > -NEARMATE
						&& (move_list[ply].move()->active_piece_id != PAWN_VALUE || //No open pawns
						(game->getPieceBB(!move_list[ply].move()->color, nPawn) & (move_list[ply].move()->color == WHITE ? mask::north[move_list[ply].move()->to] : mask::south[move_list[ply].move()->to])))
						) {
						move_count++;
						game->unMakeMove(*move_list[ply].move());
						continue;
					}
					#endif

					#ifdef ENABLE_FUTILITY_PRUNING
					//Futility Pruning:
					if (depth <= FUTILITY_DEPTH 
					&& alpha > -NEARMATE
					&& node_score + futility_margin[depth] + ((move_list[ply].phase() == CAPTURES || move_list[ply].phase() == BAD_CAPTURES) ? 100 : 0) <= alpha) {
						move_count++;
						game->unMakeMove(*move_list[ply].move());
						continue;
					}
					#endif					
					
					//LMR:
					if (move_list[ply].phase() == NONCAPTURES
					&&	move_count >= LMR_THRESHOLD				// Wait for LATE moves
					&&	depth >= LMR_HORIZON					// Do not reduce too close to the horizon
					&&	!extensions
					&& ( move_list[ply].move()->active_piece_id != PAWN_VALUE || //Do not reduce passed pawns
							(game->getPieceBB(!move_list[ply].move()->color,nPawn) & mask::passed_pawn[move_list[ply].move()->color][move_list[ply].move()->to]) )
					){
						unsigned char reduction = ((move_count >= (LMR_THRESHOLD * 2) && depth >= LMR_HORIZON) ? 2/*depth / LMR_HORIZON*/ : LMR_REDUCTION);
						//unsigned char reduction = LMR_REDUCTION;
						score = -Hard_PVS(depth-1-reduction, ply+1, game, -(alpha+1), -alpha, false);
					}
				}
					
				if(score > alpha) { //research
					score = -Hard_PVS(depth - 1 + extensions, ply + 1, game, -(alpha + 1), -alpha, false);
					if(score > alpha && score < beta) {
						score = -Hard_PVS(depth - 1 + extensions, ply + 1, game, -beta, -alpha, false);
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

bool SEARCH::abort() {

	if (node_count % NODES_FOR_TIME_CHECK == 0) {
		if (clock() / (CLOCKS_PER_SEC / 1000) - start_time >= allotted_time) {
			fTimeOut = true;
			return true;
		}
	}
	return false;
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