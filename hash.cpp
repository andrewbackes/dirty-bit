#include "hash.h"

void HASHTABLE::initialize(long long megabytes) {
		bytes = (megabytes * 1048576);
		// 1028 * 1048576 = 1073741824
		// 1073741824 / 24 = 44739242.666666666666666666666667
		// 2^25 = 33554432

		size = 33554432;
		//size = bytes / sizeof(HASH);
		table = new HASH[size]; 
}

void HASHTABLE::reinitialize(long long megabytes) {
		delete table;
		initialize(megabytes);
}

int HASHTABLE::SearchHash(bitboard key, unsigned char depth, int alpha, int beta) {
	//HASH * pEntry = &table[key % size];
	HASH * pEntry = &table[key & 33554431 ];
	if( pEntry->key == key) {
		if( pEntry->depth >= depth) {
			if(pEntry->flags == HASH_EXACT)
				return pEntry->value;
			if( (pEntry->flags == HASH_ALPHA) && (pEntry->value <= alpha) )
				return alpha;
			if( (pEntry->flags == HASH_BETA) && (pEntry->value >= beta) )
				return beta;
		}
	}
	return HASH_UNKNOWN;
}

int HASHTABLE::SearchHash(bitboard key, unsigned char depth, int alpha, int beta, MOVE * hashed_move) {
	//HASH * pEntry = &table[key % size];
	HASH * pEntry = &table[key & 33554431 ];
	if( pEntry->key == key) {
		if( pEntry->depth >= depth) {
			if(pEntry->flags == HASH_EXACT)
				return pEntry->value;
			if( (pEntry->flags == HASH_ALPHA) && (pEntry->value <= alpha) )
				return alpha;
			if( (pEntry->flags == HASH_BETA) && (pEntry->value >= beta) )
				return beta;
		}
		*hashed_move = pEntry->best_move;
	}
	return HASH_UNKNOWN;
}

int HASHTABLE::getHashScore(bitboard key) {
	//HASH * pEntry = &table[key % size];
	HASH * pEntry = &table[key & 33554431 ];
	if( pEntry->key == key) {
		return pEntry->value;
	}
	return HASH_UNKNOWN;
}

MOVE HASHTABLE::getHashMove(bitboard key) {
	//HASH * pEntry = &table[key % size];
	HASH * pEntry = &table[key & 33554431 ];
	if( pEntry->key == key) {
		//if(pEntry->depth >= depth) {
			return pEntry->best_move;
		//}
	}
	return NO_MOVE;
}

void HASHTABLE::RecordHash(bitboard key, unsigned char depth, int val, unsigned char hashflag, MOVE * hashed_move) {
	//TODO:		-change to scheme to replace if same depth or deeper.
	/*
	if(hashed_move->from == 64)
		empty_saves++;
	else
		non_empty_saves++;
	*/
	//HASH * pEntry = &table[key % size];
	HASH * pEntry = &table[key & 33554431 ];
	if( hashflag >= pEntry->flags) {
		pEntry->key = key;
		pEntry->best_move = *hashed_move;
		pEntry->value = val;
		pEntry->flags = hashflag;
		pEntry->depth = depth;
	}
}

void HASHTABLE::RecordHash(bitboard key, unsigned char depth, int val, unsigned char hashflag) {
	//RecordHash(key,depth,val,hashflag, &MOVE());
	//HASH * pEntry = &table[key % size];
	HASH * pEntry = &table[key & 33554431 ];
	if( hashflag >= pEntry->flags) {
		pEntry->key = key;
		pEntry->value = val;
		pEntry->flags = hashflag;
		pEntry->depth = depth;
		pEntry->best_move.from = 64;
	}
}
