#ifndef move_h
#define move_h

#include <iostream>
#include "utilities.h"

#include "parameters.h"

class MOVE {
public:
	
	short			static_value; //may be empty
	unsigned char	to;
	unsigned char	from;
	unsigned char	active_piece_id;
	unsigned char	captured_piece;
	unsigned char	promote_piece_to;
	bool			color;
	

	void print(bool verbose = false);
	
	MOVE::MOVE(unsigned char from, unsigned char to, bool color, unsigned char active_piece_id, unsigned char captured_piece, unsigned char promote_piece_to, short static_value)
		: from(from), to(to), color(color), active_piece_id(active_piece_id), captured_piece(captured_piece), promote_piece_to(promote_piece_to), static_value(static_value)
	{}

	MOVE::MOVE(unsigned char from, unsigned char to, bool color, unsigned char active_piece_id, unsigned char captured_piece, unsigned char promote_piece_to) 
		: from(from), to(to), color(color), active_piece_id(active_piece_id), captured_piece(captured_piece), promote_piece_to(promote_piece_to), static_value(0) 
	{}
	MOVE::MOVE(unsigned char from, unsigned char to, bool color,unsigned char active_piece_id, unsigned char captured_piece) 
		: from(from), to(to), color(color), active_piece_id(active_piece_id), captured_piece(captured_piece), promote_piece_to(0), static_value(0) 
	{}
	MOVE::MOVE(unsigned char from, unsigned char to, bool color, unsigned char active_piece_id) 
		: from(from), to(to), color(color), active_piece_id(active_piece_id), captured_piece(0), promote_piece_to(0), static_value(0) 
	{}
	MOVE::MOVE(unsigned char from, unsigned char to, bool color) 
		: from(from), to(to), color(color), active_piece_id(0), captured_piece(0), promote_piece_to(0), static_value(0) 
	{}
	MOVE::MOVE() 
		: from(64), to(64), active_piece_id(64), captured_piece(64), promote_piece_to(64), static_value(0) 
	{}

	bool operator==(MOVE b) {
		if( from != b.from)
			return false;
		if( to != b.to)
			return false;
		if( active_piece_id != b.active_piece_id )
			return false;
		if( color != b.color)
			return false;
		if( captured_piece != b.captured_piece)
			return false;
		if( promote_piece_to != b.promote_piece_to)
			return false;
		return true;
	}

};

class CAPTURER {
public:
	CAPTURER() {from = 64;}
	//CAPTURER(unsigned char piece_id, unsigned char index) : piece_id(piece_id), index(index) {}
	CAPTURER(MOVE m) {
		to = m.to;
		from = m.from;
		piece_id = m.active_piece_id;
		captured_id = m.captured_piece;
		color = m.color;
	}
	CAPTURER(unsigned char piece_id, unsigned char captured_id, unsigned char from, unsigned char to, bool color) :
		piece_id(piece_id), captured_id(captured_id), from(from), to(to), color(color) {}

	void print() {std::cout << index_to_alg((int)from) << index_to_alg((int) to) << " "; }

	unsigned char piece_id;
	unsigned char captured_id;
	unsigned char from;
	unsigned char to;
	bool color;

private:
	
};

bool equal(MOVE a, MOVE b);

const MOVE NO_MOVE = MOVE(64,64,0);

#endif