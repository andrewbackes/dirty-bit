#ifndef book_h
#define book_h

#include <vector>
#include <string>

#include "move.h"
#include "board.h"

class BOOK_LINE {
public:
	BOOK_LINE();
	BOOK_LINE(std::string name, std::string line) : name(name), line(line) {evaluation_score = 0;}
	
	std::string getName() {return name;}
	std::string getLine() {return line;}

	void eval();
	int evaluation_score;
private:
	std::string name;
	std::string line;

	
};

class BOOK {
public:
	BOOK() {fOutOfBook = false;}

	std::string recommendedMove(CHESSBOARD *game, std::string moves_played);
	
	void load(std::string filename);
	void load();

	bool OutOfBook()		{return fOutOfBook;}

	void demote(int index)	{ priorities.at(index)--; }
	void promote(int index) { priorities.at(index)++; }

	void addLine( int priority, std::string name, std::string line );

private:
	std::vector<int>		priorities;
	std::vector<BOOK_LINE>	lines;
	bool					fOutOfBook;
};

#endif