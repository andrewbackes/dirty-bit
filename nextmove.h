#ifndef nextmove_h
#define nextmove_h

#include "move.h"
#include "board.h"
#include "SEE.h"

#include "parameters.h"

bool valid_killer(MOVE killer, CHESSBOARD * linked_game);

class MOVELIST {
public:
	MOVELIST() {
					pMove = NULL;			

					next_move_phase = HASH_MOVE; 
					move_phase = HASH_MOVE;
					
					moves_searched_counter = 0;

					move_index = 0; 
					capture_index = 0;
					
					move_list_size = 0;
					capture_list_size = 0;
					
					killer_counter[0] = 0;
					killer_counter[1] = 0;
					killer_move[0].from = 64;
					killer_move[1].from = 64;
					hash_suggestion.from = 64;
	}
	void linkGame(CHESSBOARD * game) {linked_game = game;}

	short			PopulateCompleteList();
	bool			NextMove();
	bool			NextRootMove(bool already_generated = false);
	unsigned char	phase() {return move_phase;}
	
	void			reset() {	next_move_phase = HASH_MOVE; hash_suggestion.from = 64; 
								move_index = 0; move_list_size =0; capture_index=0; capture_list_size = 0; }
	
	void			addMove(MOVE m) {		move_list[move_list_size] = m; 
											//move_list[move_list_size].static_value += STE(linked_game, move_list[move_list_size]);
											move_list_size++; }

	void			addCapture(MOVE m) {	capture_list[capture_list_size] = m;
											//Debug:
											/*
											if(SEE3(linked_game, capture_list[capture_list_size]) != SEE1(linked_game, capture_list[capture_list_size])) {
												std::cout << std::endl << "MOVE: ";
												capture_list[capture_list_size].print();
												std::cout << std::endl;
												linked_game->print();
												std::cout << "SEE1: " << SEE1(linked_game, capture_list[capture_list_size]) << std::endl;
												std::cout << "SEE2: " << SEE2(linked_game, capture_list[capture_list_size]) << std::endl;
												std::cout << "SEE3: " << SEE3(linked_game, capture_list[capture_list_size]) << std::endl;
												system("PAUSE");
											}
											*/
											capture_list[capture_list_size].static_value += SEE3(linked_game, capture_list[capture_list_size]);
											//capture_list[capture_list_size].static_value += STE(linked_game, capture_list[capture_list_size]);
											capture_list_size++; }
	MOVE*			move() { return pMove; }
	unsigned char	capture_size() { return capture_list_size; }
	unsigned char	move_size() { return move_list_size; }
	
	unsigned char	moves_searched() { return moves_searched_counter; }
	void			move_searched() { moves_searched_counter++; }

	void			OrderMoves();
	void			OrderCaptures();

	void			addHash(MOVE passed_hash) { hash_suggestion = passed_hash; move_list_size = 1; }
	void			clearHash() { hash_suggestion.from = 64; }

	void			clearKillers();
	short			countKiller(unsigned char index) {return killer_counter[index];}
	MOVE			getKiller(unsigned char index) {return killer_move[index];}
	void			addKiller(MOVE killer);
	
	MOVE			hash_suggestion;

	MOVE			move_at(unsigned char i) { return move_list[i]; }
	MOVE			capture_at(unsigned char i) { return capture_list[i]; }
	/*
	MOVE& operator[](unsigned int i){ return move_list[i];}
	const MOVE& operator[](unsigned int i)const{ return move_list[i];}
	*/

	MOVE	killer_move[2];
	short	killer_counter[2];

	#ifdef SEARCH_STATS
		int current_phase;
	#endif

private:

	MOVE capture_list[64];
	MOVE move_list[256];

	MOVE* pMove; //Points the the currently selected move in either list.
	
	unsigned char capture_list_size;
	unsigned char move_list_size;
	unsigned char moves_searched_counter;

	unsigned char move_index;
	unsigned char capture_index;

	unsigned char next_move_phase;
	unsigned char move_phase;
	CHESSBOARD * linked_game;
};

class QMOVELIST {
public:
	QMOVELIST() {	
					move_phase = INITIATE; 
					list_index = 0;
					list_size = 0;
	}
	void linkGame(CHESSBOARD * game) {linked_game = game;}

	bool			NextQMove(bool generate_all = false);
	unsigned char	phase() {return move_phase;}
	MOVE*			move() {return pMove;}
	void			reset() { move_phase = INITIATE; list_index = 0; list_size = 0; }
	void			addCapture(MOVE m) {	move_list[list_size] = m;
											move_list[list_size].static_value += SEE3(linked_game, move_list[list_size]);
											//move_list[list_size].static_value += STE(linked_game, move_list[list_size]);
											list_size++; }
	void			addMove(MOVE m) {		move_list[list_size] = m; 
											//move_list[list_size].static_value += STE(linked_game, move_list[list_size]);
											list_size++; }
	unsigned char	size() { return list_size; }
	void			OrderCaptures();

	MOVE			at(unsigned char i) { return move_list[i]; }

private:
	CHESSBOARD * linked_game;

	MOVE* pMove;
	
	MOVE move_list[64];
	
	unsigned char list_size;
	unsigned char list_index;

	unsigned char move_phase;

};


#endif