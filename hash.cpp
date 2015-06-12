#include "hash.h"

void HASHTABLE::initialize(long long megabytes) {
		bytes = (megabytes * 1048576);
		size = bytes / sizeof(HASH);
		table = new HASH[size]; 
}

void HASHTABLE::reinitialize(long long megabytes) {
		delete table;
		initialize(megabytes);
}

int HASHTABLE::SearchHash(bitboard key, unsigned char depth, int alpha, int beta) {
	HASH * pEntry = &table[key % size];
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
	HASH * pEntry = &table[key % size];
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

MOVE HASHTABLE::getHashMove(bitboard key, int depth) {
	HASH * pEntry = &table[key % size];
	if( pEntry->key == key) {
		if(pEntry->depth >= depth) {
			return pEntry->best_move;
		}
	}
	return NO_MOVE;
}
void HASHTABLE::RecordHash(bitboard key, unsigned char depth, int val, unsigned char hashflag, MOVE * hashed_move) {
	//Simple always replace scheme.
	//TODO:		-change to scheme to replace if same depth or deeper.
	HASH * pEntry = &table[key % size];
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
	HASH * pEntry = &table[key % size];
	if( hashflag >= pEntry->flags) {
		pEntry->key = key;
		pEntry->value = val;
		pEntry->flags = hashflag;
		pEntry->depth = depth;
	}
}
