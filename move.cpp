
#include "move.h"
#include "utilities.h"

#include <iostream>

std::string MOVE::to_str() {
	return (index_to_alg(from) + index_to_alg(to));
}

void MOVE::print(bool verbose) {
	std::cout << index_to_alg(from) << index_to_alg(to) << " ";
	if(verbose == true) {
		std::cout << "(color: " << this->color 
			<< ". piece: " << (int)this->active_piece_id 
			<< ". captures: " << (int)this->captured_piece 
			<< ". promotes: " << (int)this->promote_piece_to << ") ";
	}
}

bool equal(MOVE a, MOVE b) {
	if(a.from != b.from || a.to != b.to)
		return false;
	return true;
}
