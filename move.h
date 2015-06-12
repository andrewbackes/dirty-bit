#ifndef move_h
#define move_h

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
	void setStaticValue();
	
	MOVE::MOVE(unsigned char from, unsigned char to, bool color, unsigned char active_piece_id, unsigned char captured_piece, unsigned char promote_piece_to, short static_value)
		: from(from), to(to), color(color), active_piece_id(active_piece_id), captured_piece(captured_piece), promote_piece_to(promote_piece_to), static_value(static_value)
	{
		setStaticValue();
	}

	MOVE::MOVE(unsigned char from, unsigned char to, bool color, unsigned char active_piece_id, unsigned char captured_piece, unsigned char promote_piece_to) 
		: from(from), to(to), color(color), active_piece_id(active_piece_id), captured_piece(captured_piece), promote_piece_to(promote_piece_to), static_value(0) 
	{ 
		setStaticValue(); 
	}
	MOVE::MOVE(unsigned char from, unsigned char to, bool color,unsigned char active_piece_id, unsigned char captured_piece) 
		: from(from), to(to), color(color), active_piece_id(active_piece_id), captured_piece(captured_piece), promote_piece_to(0), static_value(0) 
	{ 
		setStaticValue(); 
	}
	MOVE::MOVE(unsigned char from, unsigned char to, bool color, unsigned char active_piece_id) 
		: from(from), to(to), color(color), active_piece_id(active_piece_id), captured_piece(0), promote_piece_to(0), static_value(0) 
	{
		setStaticValue();
	}
	MOVE::MOVE(unsigned char from, unsigned char to, bool color) 
		: from(from), to(to), color(color), active_piece_id(0), captured_piece(0), promote_piece_to(0), static_value(0) 
	{
	
	}
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

const MOVE NO_MOVE = MOVE(64,64,0);

#endif