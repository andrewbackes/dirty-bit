#ifndef nextmove_h
#define nextmove_h

#include <vector>

#include "move.h"
#include "board.h"
#include "search.h"

#define HASH_MOVE		0
#define CAPTURES		1
#define KILLER_MOVE_1	2
#define KILLER_MOVE_2	3
#define NONCAPTURES		4

class MOVELIST {
public:
	MOVELIST() {	move_phase = HASH_MOVE; 
					list_index = 0; 
					list_size = 1; }
	
	bool NextMove(MOVE * next_move, SEARCH * search, short depth, CHESSBOARD * game, MOVE hash_suggestion);
	bool NextRootMove(MOVE * next_move, CHESSBOARD * game, MOVE hash_suggestion);
	unsigned char phase() {return move_phase;}
	
private:
	std::vector<MOVE> move_list;
	unsigned char list_size;
	unsigned char list_index; // 9 queens on an open board is 252 moves. this *may* not be enough moves.
	unsigned char move_phase;
};


#endif