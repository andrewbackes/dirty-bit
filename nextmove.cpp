#include "nextmove.h"
#include "movegen.h"

using namespace std;

/*******************************************************************************

Returns the next move in the search. Lazy generation / sorts in phases. 
The next phase's moves are generated at the _end_ of the preceeding phase.

*******************************************************************************/

bool MOVELIST::NextMove(MOVE * next_move, SEARCH * search, short depth, CHESSBOARD * game, MOVE hash_suggestion) {

	if( move_phase == HASH_MOVE ) {
		if( list_index < list_size ) {
			list_index++;
			if(hash_suggestion.from != 64) {
				*next_move = hash_suggestion;
				return true;
			}
		}
		move_phase = CAPTURES;
		move_list = CaptureGen(game);
		MoveSort(move_list);
		list_index = 0;
		list_size = (unsigned char) move_list.size(); 
	}
//------	
	if( move_phase == CAPTURES ) {
		if( list_index < list_size ) {
			*next_move = move_list[list_index++];
			return true;
		}
		else {
			move_phase = KILLER_MOVE_1;
			list_index = 0;
			list_size = (search->countKiller(0,depth) > 0) ? 1 : 0;
		}
	}
//------
	if( move_phase == KILLER_MOVE_1 ) {
		if( list_index < list_size ) {
			list_index++;
			if(	game->getBoard(search->getKiller(0,depth).from) == search->getKiller(0,depth).active_piece_id
			&&	game->getBoard(search->getKiller(0,depth).to)	== search->getKiller(0,depth).captured_piece 
			&&	search->getKiller(0,depth).color == game->getActivePlayer() //confusing, but necessary because of when the toggleActivePlayer occurs in the search.
			) {
				*next_move = search->getKiller(0,depth);
				return true;
			}
		}
		move_phase = KILLER_MOVE_2;
		list_index = 0;
		list_size = (search->countKiller(1,depth) > 0) ? 1 : 0;
	}
//------
	if( move_phase == KILLER_MOVE_2 ) {
		if( list_index < list_size ) {
			list_index++;
			if(	game->getBoard(search->getKiller(1,depth).from) == search->getKiller(1,depth).active_piece_id
			&&	game->getBoard(search->getKiller(1,depth).to)	== search->getKiller(1,depth).captured_piece 
			&&	search->getKiller(1,depth).color == game->getActivePlayer() //confusing, but necessary because of when the toggleActivePlayer occurs in the search.
			) {
				*next_move = search->getKiller(1,depth);
				return true;
			}
		}
		move_phase = NONCAPTURES;
		move_list = nonCaptureGen(game);
		MoveSort(move_list);
		list_index = 0;
		list_size = (unsigned char) move_list.size();
	}
//------
	if( move_phase == NONCAPTURES ) {
		if( list_index < list_size ) {
			*next_move = move_list[list_index++];
			return true;
		}
		else {
			return false;
		}
	}
	//return false; //for safety. but if this executes, there is a problem.
}

bool MOVELIST::NextRootMove(MOVE * next_move, CHESSBOARD * game, MOVE hash_suggestion) {

	if( move_phase == HASH_MOVE ) {
		if( list_index < list_size ) {
			list_index++;
			if(hash_suggestion.from != 64) {
				*next_move = hash_suggestion;
				return true;
			}
		}
		move_phase = CAPTURES;
		move_list = CaptureGen(game);
		MoveSort(move_list);
		list_index = 0;
		list_size = (unsigned char) move_list.size(); 
	}
//------	
	if( move_phase == CAPTURES ) {
		if( list_index < list_size ) {
			*next_move = move_list[list_index++];
			return true;
		}
		else {
			move_phase = NONCAPTURES;
			move_list = nonCaptureGen(game);
			MoveSort(move_list);
			list_index = 0;
			list_size = (unsigned char) move_list.size();
		}
	}
//------
	if( move_phase == NONCAPTURES ) {
		if( list_index < list_size ) {
			*next_move = move_list[list_index++];
			return true;
		}
		else {
			return false;
		}
	}
	//return false; //for safety. but if this executes, there is a problem.
}