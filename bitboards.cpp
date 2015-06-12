
#include "bitboards.h"

#include <intrin.h>
#include <iostream>

using namespace std;

#pragma intrinsic(_BitScanReverse64)
#pragma intrinsic(_BitScanForward64)

void bitprint(bitboard bb)
{
	bitboard x=1;
	for(int i=63; i>=0; i--){
		if( bb & (x<<i) )
			std::cout << "1";
		else
			std::cout << "0";
		if(i%8 == 0)
			std::cout << std::endl;
	}
	std::cout << std::endl;
}

//Processor Instruction list:
//Search the mask data from least significant bit (LSB) to the most significant bit (MSB) for a set bit (1).
//		unsigned char_BitScanForward64(unsigned long * Index,  unsigned __int64 Mask);
//
//Search the mask data from most significant bit (MSB) to least significant bit (LSB) for a set bit (1).
//		unsigned char _BitScanReverse64(unsigned long * Index,  unsigned __int64 Mask);
//
//AMD - Counts the number of leading zeros in a 64-byte integer.
//		unsigned __int64 __lzcnt64(unsigned __int64 value);



unsigned char bitscan_lsb(bitboard bb)
{
	if(bb == 0)
		return 64;
	
	unsigned long index =0;
	_BitScanForward64(&index, bb);
	//if (index != __lzcnt64(bb)) 
	//	cout << "_Bit: " << index << ", lz: " << __lzcnt64(bb) << endl;
	return((unsigned char)index);
	
}

unsigned char bitscan_msb(bitboard bb)
{
	if(bb == 0)
		return 64;
	
	/*
	unsigned long index =0;
	_BitScanReverse64(&index, bb);
	return((unsigned char)index);
	*/

	return (unsigned char)(63 -__lzcnt64(bb));
}

bool bitcheck(bitboard bb, unsigned char index) {
	if( bb & (1i64 << index))
		return true;
	else
		return false;
}


unsigned char bitcnt(bitboard bb) {
//Try:
// return pc_table[MASK_POP_COUNT & bb] + pc_table[MASK_POP_COUNT & (bb >> 16)] + pc_table[MASK_POP_COUNT & (bb >> 32)] + pc_table[MASK_POP_COUNT & (bb >> 48)];
	
	unsigned char count = 0;
	while(bb) {
		count++;
		bb ^= (1i64 << bitscan_msb(bb) );
	}
	return count;
	
}
