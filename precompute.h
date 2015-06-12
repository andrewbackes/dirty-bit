#ifndef precompute_h
#define precompute_h

#include "dirtybit.h"

/*******************************************************************************

precompute.h/.cpp is intended to hold all of the functions used to generate
precomputed data. Some of these are used at the time of initialization,
while others are ran once and the output is copied and pasted into the code.

*******************************************************************************/

unsigned char fit(short m, short M, short x, short x_max);

namespace precompute {
	void rays();
	
	//Used in Evaluation:
	void rule_of_the_squares();
	void castle_areas();
	void castle_pawns();
	void shelter_scores();
	void passed_pawns();
	void material_adjustments();

}


#endif

