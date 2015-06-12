
#include <iostream>

#include "nextmove.h"
#include "movegen.h"
#include "SEE.h"

#include "parameters.h"

using namespace std;

/*******************************************************************************

Returns the next move in the search. Lazy generation / sorts in phases. 
The next phase's moves are generated at the _end_ of the preceeding phase.

*******************************************************************************/

bool MOVELIST::NextMove() {

	if( next_move_phase == HASH_MOVE ) {
		if( move_index < move_list_size ) {
			move_index++;
			if(hash_suggestion.from != 64) {
				//*next_move = hash_suggestion;
				pMove = &hash_suggestion;
				move_phase = HASH_MOVE;
				#ifdef SEARCH_STATS
					current_phase = HASH_MOVE;
				#endif
				return true;
			}
		}
		next_move_phase = CAPTURES;
		capture_index = 0;
		capture_list_size = 0;
		CaptureGen(linked_game, this);
		OrderCaptures();
	}
//------	
	if( next_move_phase == CAPTURES ) {
		if( capture_index < capture_list_size 
			&& capture_list[capture_index].static_value >= 0) //stop at the loosing captures.
		{
			/*
			//DEBUG ONLY:
			if(capture_list_size > 5) {
				std::cout << "C: ";
				capture_list[capture_index].print() ;
				std::cout << "(" << capture_list[capture_index].static_value << ")\n";
			}
			*/
			
			/*
			while( equal(move_list[capture_index], hash_suggestion) ) {
				capture_index++;
			}
			*/
			pMove = &capture_list[capture_index];
			move_phase = CAPTURES;
			#ifdef SEARCH_STATS
				current_phase = CAPTURES;
			#endif
			capture_index++;
			return true;
		}
		else {
			#ifdef ENABLE_KILLER_MOVES
				next_move_phase = KILLER_MOVE_1;
				move_index = 0;
				move_list_size = (countKiller(0) > 0) ? 1 : 0;
			#endif
			#ifndef ENABLE_KILLER_MOVES
				next_move_phase = NONCAPTURES;
				move_index = 0;
				move_list_size = 0;
				nonCaptureGen(linked_game, this);
				OrderMoves();
			#endif
		}
	}
//------
	if( next_move_phase == KILLER_MOVE_1 ) {
		if( move_index < move_list_size ) {
			move_index++;
			if(	linked_game->getBoard(getKiller(0).from) == getKiller(0).active_piece_id
			//&&	linked_game->getBoard(getKiller(0).to)	== getKiller(0).captured_piece 
			&&	getKiller(0).color == linked_game->getActivePlayer() 
			) {
				/*
				//DEBUG ONLY:
				if(capture_list_size > 5) {
					cout << "K: ";
					killer_move[0].print();
					cout << "(" << killer_move[0].static_value << ")\n";
				}
				*/
				
				pMove = &killer_move[0];
				move_phase = KILLER_MOVE_1;
				#ifdef SEARCH_STATS
					current_phase = KILLER_MOVE_1;
				#endif
				return true;
			}
		}
		next_move_phase = KILLER_MOVE_2;
		move_index = 0;
		move_list_size = (countKiller(1) > 0) ? 1 : 0;
	}
//------
	if( next_move_phase == KILLER_MOVE_2 ) {
		if( move_index < move_list_size ) {
			move_index++;
			if(	linked_game->getBoard(getKiller(1).from) == getKiller(1).active_piece_id
			//&&	linked_game->getBoard(getKiller(1).to)	== getKiller(1).captured_piece 
			&&	getKiller(1).color == linked_game->getActivePlayer() 
			) {
				/*
				//DEBUG ONLY:
				if(capture_list_size > 5) {
					cout << "K: ";
					killer_move[1].print();
					cout << "(" << killer_move[1].static_value << ")\n";
				}
				*/
				//*next_move = getKiller(1);
				pMove = &killer_move[1];
				move_phase = KILLER_MOVE_2;
				#ifdef SEARCH_STATS
					current_phase = KILLER_MOVE_2;
				#endif
				return true;
			}
		}
		next_move_phase = NONCAPTURES;
		move_index = 0;
		move_list_size = 0;
		nonCaptureGen(linked_game, this);
		OrderMoves();
	}
//------
	if( next_move_phase == NONCAPTURES ) {
		if( move_index < move_list_size ) {
			/*
			//DEBUG ONLY:
			if(capture_list_size > 5) {
				std::cout << "M: ";
				move_list[move_index].print() ;
				std::cout << "(" << move_list[move_index].static_value << ")\n";
			}
			*/
			//*next_move = move_list[move_index];
			
			//Advance the counter until it is not a duplicate with a killer:
			
			/*while( equal(move_list[move_index], killer_move[0])
			|| equal(move_list[move_index], killer_move[1])
			|| equal(move_list[move_index], hash_suggestion) ) {
				move_index++;
			}*/

			pMove = &move_list[move_index];
			move_phase = NONCAPTURES;
			#ifdef SEARCH_STATS
				current_phase = NONCAPTURES;
			#endif
			move_index++;
			return true;
		}
		else {
			next_move_phase = BAD_CAPTURES;
		}
	}
	if(next_move_phase == BAD_CAPTURES ) {
		if( capture_index < capture_list_size ) {
			/*
			//DEBUG ONLY:
			if(capture_list_size > 5) {
				std::cout << "L: ";
				capture_list[capture_index].print() ;
				std::cout << "(" << capture_list[capture_index].static_value << ")\n";
			}
			*/
			//*next_move = capture_list[capture_index];
			pMove = &capture_list[capture_index];
			move_phase = BAD_CAPTURES;
			#ifdef SEARCH_STATS
				current_phase = BAD_CAPTURES;
			#endif
			capture_index++;
			return true;
		}
	}
/*
//DEBUG:
if(capture_list_size > 5) {
	linked_game->print();
	if(hash_suggestion.from != 64) {
		cout << "H: ";
		hash_suggestion.print();
		std::cout << "(" << hash_suggestion.static_value << ")\n";
	}
	int i =0;
	while( i < capture_list_size) {
		if(capture_list[i].static_value < 0)
			break;
		std::cout << "C: ";
		capture_list[i].print() ;
		std::cout << "(" << capture_list[i].static_value << ")\n";
		i++;
	}
	if(countKiller(0) > 0) {
		cout << "K: ";
		killer_move[0].print();
		cout << "(" << killer_move[0].static_value << ")\n";
	}
	if(countKiller(1) > 0) {
		cout << "K: ";
		killer_move[1].print();
		cout << "(" << killer_move[1].static_value << ")\n";
	}
	for( int j=0; j< move_list_size; j++ ) {
		std::cout << "M: ";
		move_list[j].print() ;
		std::cout << "(" << move_list[j].static_value << ")\n";
	}
	while( i < capture_list_size) {
		std::cout << "L: ";
		capture_list[i].print() ;
		std::cout << "(" << capture_list[i].static_value << ")\n";
		i++;
	}
	system("PAUSE");
}
*/
	
	return false;
}

void QMOVELIST::OrderCaptures() {
	int j = 0;
	MOVE value;
	for (int i=0; i <size(); i++ ) {
		value = move_list[i];
		j= i -1;
		while (j >=0 && move_list[j].static_value < value.static_value) {
			move_list[j+1] = move_list[j];
			j=j-1;
		}
		move_list[j+1] = value;
	}
}

void MOVELIST::OrderCaptures() {
	int j = 0;
	MOVE value;
	for (int i=0; i < capture_size(); i++ ) {		
		value = capture_list[i];
		j= i -1;
		while (j >=0 && capture_list[j].static_value < value.static_value) {
			capture_list[j+1] = capture_list[j];
			j=j-1;
		}
		capture_list[j+1] = value;
	}
}

void MOVELIST::OrderMoves() {
	int j = 0;
	MOVE value;
	for (int i=0; i < move_size(); i++ ) {
		value = move_list[i];
		j= i -1;
		while (j >=0 && move_list[j].static_value < value.static_value) {
			move_list[j+1] = move_list[j];
			j=j-1;
		}
		move_list[j+1] = value;
	}
}

short MOVELIST::PopulateCompleteList() {
	//returns the total number of moves generated.
	short total_list_size = 0;
	if (!empty_move(hash_suggestion)) {
		total_list_size++;
	}
	
	capture_index = 0;
	capture_list_size = 0;
	CaptureGen(linked_game, this);
	OrderCaptures();
	total_list_size += capture_list_size;
	
	move_index = 0;
	move_list_size = 0;
	nonCaptureGen(linked_game, this);
	OrderMoves();
	total_list_size += move_list_size;

	return total_list_size;
}

bool MOVELIST::NextRootMove(bool already_generated) {
	if (next_move_phase == HASH_MOVE) {
		if (move_index < move_list_size) {
			move_index++;
			if (hash_suggestion.from != 64) {
				pMove = &hash_suggestion;
				move_phase = HASH_MOVE;
				#ifdef SEARCH_STATS
				current_phase = HASH_MOVE;
				#endif
				return true;
			}
		}
		next_move_phase = CAPTURES;
		capture_index = 0;
		if (!already_generated) {
			capture_list_size = 0;
			CaptureGen(linked_game, this);
			OrderCaptures();
		}
	}
//------	
	if( next_move_phase == CAPTURES ) {
		if( capture_index < capture_list_size
			&& capture_list[capture_index].static_value >= 0 ) //Stop at loosing captures.
		{
			//*next_move = capture_list[capture_index++];
			pMove = &capture_list[capture_index++];
			move_phase = CAPTURES;
			#ifdef SEARCH_STATS
				current_phase = CAPTURES;
			#endif
			return true;
		}
		else {
			next_move_phase = NONCAPTURES;
			move_index = 0;
			if (!already_generated) {
				move_list_size = 0;
				nonCaptureGen(linked_game, this);
				OrderMoves();
			}
		}
	}
//------
	if( next_move_phase == NONCAPTURES ) {
		if( move_index < move_list_size ) {
			//*next_move = move_list[move_index++];
			pMove = &move_list[move_index++];
			move_phase = NONCAPTURES;
			#ifdef SEARCH_STATS
				current_phase = NONCAPTURES;
			#endif
			return true;
		}
		else {
			next_move_phase = BAD_CAPTURES;
		}
	}
	if( next_move_phase == BAD_CAPTURES ) {
		if( capture_index < capture_list_size ) {
			//*next_move = capture_list[capture_index++];
			pMove = &capture_list[capture_index++];
			move_phase = BAD_CAPTURES;
			#ifdef SEARCH_STATS
				current_phase = BAD_CAPTURES;
			#endif
			return true;
		}
	}
	return false;
}

void MOVELIST::clearKillers() {
	//killer_move[0].from = 64;
	//killer_move[1].from = 64;
	killer_counter[0] = 0;
	killer_counter[1] = 0;	
}

void MOVELIST::addKiller(MOVE killer) {
	//cout << "Added Killer: ";
	//killer.print(true);
	//cout << endl;
	if(killer_counter[0] == 0) {
		killer_move[0]= killer;
		killer_counter[0]=1;
		return;
	}
	
	if(killer_counter[1] == 0) {
		killer_move[1] = killer;
		killer_counter[1]=1;
		return;
	}
	
	if(killer_move[0] == killer) {
		killer_counter[0]++;
		return;
	}
	if(killer_move[1] == killer) {
		killer_counter[1]++;
		return;
	}

	if(killer_counter[0] >= killer_counter[1]) {
		killer_move[1] = killer;
		killer_counter[1]=1;
	}
	else {
		killer_move[0]= killer;
		killer_counter[0]=1;
	}
}


bool QMOVELIST::NextQMove() {
	if( move_phase == INITIATE ) {
		move_phase = CAPTURES;
		list_index = 0;
		list_size = 0;
		CaptureGen(linked_game, this);
		OrderCaptures();
	}

//------	
	if( move_phase == CAPTURES ) {
		if( list_index < list_size 
			&& move_list[list_index].static_value >=0 ) {
			//*next_move = move_list[list_index++];
			pMove = &move_list[list_index++];
			return true;
		}
		else {
			list_index = 0;
			list_size = 0;
			move_phase = BAD_CAPTURES;
		}
	}
//------
	/*
	if( move_phase == BAD_CAPTURES ) {
		if( list_index < list_size ) {
			*next_move = move_list[list_index++];
			return true;
		}
		else {
			return false;
		}
	}
	*/
	return false; //for safety. but if this executes, there is a problem.
}