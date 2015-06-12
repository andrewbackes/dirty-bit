
#include "bitboards.h"

#include <iostream>

using namespace std;

#ifdef _MSC_VER
    #include <intrin.h>
    #pragma intrinsic(_BitScanReverse64)
    #pragma intrinsic(_BitScanForward64)
    #pragma intrinsic(__popcnt64)
#endif

bitboard mask::MagicRookMoves[64][MAGIC_ROOK_SIZE];	//16384
bitboard mask::MagicBishopMoves[64][MAGIC_BISHOP_SIZE];	//2048

bitboard quickhash(bitboard bb, bitboard key, int bits){
	//Taken with permission from Exacto 0.e source code:
	return (((bb*key) >> (64 - bits)));
}

#include <vector>
void generateMagicBishopVariations(bitboard ** &varMask, bitboard ** &varAttacksMask) {
	//bitboard varMask[64][64];
	//bitboard varAttacksMask[64][64];

	bitboard i;
	int bit;
	bitboard mask;
	bitboard varCount;
	
	vector<int> MaskBits;
	vector<int> IndexBits;

	int bitCount[64];
	

	for(bit = 0; bit < 64; bit++) {
		mask = mask::MagicBishopMask[bit];
		bitboard temp = mask;
		
		while(temp) {
			int b = bitscan_msb(temp);
			MaskBits.push_back(b);
			temp^=b; }
		bitCount[bit] = bitcount(mask);

		varCount = (((bitboard)1) << bitCount[bit]);
		for(i=0; i < varCount; i++) {
			varMask[bit][i] = 0;
			
			bitboard t=i;
			while(t) {
				int b = bitscan_msb(t);
				IndexBits.push_back(b);
				t^=b; }
			for(int j=0; j < IndexBits.size(); j++) {
				varMask[bit][(int)i] |= (((bitboard)1) << MaskBits[IndexBits[j]]);
			}
			int j;
			for (j=bit+9; j%8!=7 && j%8!=0 && j<=55 && (varMask[bit][i] & (((bitboard)1) << j)) == 0; j+=9) {
				if (j>=0 && j<=63) varAttacksMask[bit][i] |= (((bitboard)1) << j);
            }
			for (j=bit-9; j%8!=7 && j%8!=0 && j>=8 && (varMask[bit][i] & (((bitboard)1) << j)) == 0; j-=9) {
				if (j>=0 && j<=63) varAttacksMask[bit][i] |= (((bitboard)1) << j);
            }
			for (j=bit+7; j%8!=7 && j%8!=0 && j<=55 && (varMask[bit][i] & (((bitboard)1) << j)) == 0; j+=7) {
				if (j>=0 && j<=63) varAttacksMask[bit][i] |= (((bitboard)1) << j);
            }
			for (j=bit-7; j%8!=7 && j%8!=0 && j>=8 && (varMask[bit][i] & (((bitboard)1) << j)) == 0; j-=7) {
				if (j>=0 && j<=63) varAttacksMask[bit][i] |= (((bitboard)1) << j);
            }
			

		}

	}

}

/*
void initializeMagicBishops() {
	
	//First:
	bitboard varMask[64][64];
	bitboard varAttacksMask[64][64];

	bitboard i;
	int bit;
	bitboard mask;
	bitboard varCount;
	
	vector<int> MaskBits;
	vector<int> IndexBits;

	int bitCount[64];

	for(bit = 0; bit < 64; bit++) {
		mask = mask::MagicBishopMask[bit];
		
		bitboard temp = mask;
		while(temp) {
			int b = bitscan_msb(temp);
			MaskBits.push_back(b);
			temp^=(((bitboard)1) << b); }
		bitCount[bit] = bitcnt(mask);

		varCount = (((bitboard)1) << bitCount[bit]);
		for(i=0; i < varCount; i++) {
			varMask[bit][i] = 0;
			
			bitboard t=i;
			while(t) {
				int b = bitscan_msb(t);
				IndexBits.push_back(b);
				t^=(((bitboard)1) << b); }
			for(int j=0; j < IndexBits.size(); j++) {
				varMask[bit][(int)i] |= (((bitboard)1) << MaskBits[IndexBits[j]]);
			}
			int j;
			for (j=bit+9; j%8!=7 && j%8!=0 && j<=55 && (varMask[bit][i] & (((bitboard)1) << j)) == 0; j+=9);
				if (j>=0 && j<=63) varAttacksMask[bit][i] |= (((bitboard)1) << j);
			for (j=bit-9; j%8!=7 && j%8!=0 && j>=8 && (varMask[bit][i] & (((bitboard)1) << j)) == 0; j-=9);
				if (j>=0 && j<=63) varAttacksMask[bit][i] |= (((bitboard)1) << j);
			for (j=bit+7; j%8!=7 && j%8!=0 && j<=55 && (varMask[bit][i] & (((bitboard)1) << j)) == 0; j+=7);
				if (j>=0 && j<=63) varAttacksMask[bit][i] |= (((bitboard)1) << j);
			for (j=bit-7; j%8!=7 && j%8!=0 && j>=8 && (varMask[bit][i] & (((bitboard)1) << j)) == 0; j-=7);
				if (j>=0 && j<=63) varAttacksMask[bit][i] |= (((bitboard)1) << j);
		}

	}

	//Second:
	bitboard validmoves;
	bitboard var;
	int count;
	int j, magicIndex;

	for(bit=0; bit<=63; bit++) {
		count = bitcnt(mask::MagicBishopMask[bit]);
		var = ((bitboard)1) << count;

		for(i=0; i < var; i++) {
			validmoves = 0;
			magicIndex = (int)(varMask[bit][i] * mask::MagicBishopNumbers[bit]) >> mask::MagicBishopShift[bit];
			
			for(j = bit+9; j%8!=0 && j<=63;j+=9) {validmoves |= (((bitboard)1) << j); if ((varMask[bit][i] &(((bitboard)1) << j))!=0) break; }
			for(j = bit-9; j%8!=7 && j>=0; j-=9) { validmoves |= (((bitboard)1) << j); if ((varMask[bit][i] & (((bitboard)1) << j)) != 0) break; }
			for (j=bit+7; j%8!=7 && j<=63; j+=7) { validmoves |= (((bitboard)1) << j); if ((varMask[bit][i] & (((bitboard)1) << j)) != 0) break; }
			for (j=bit-7; j%8!=0 && j>=0; j-=7) { validmoves |= (((bitboard)1) << j); if ((varMask[bit][i] & (((bitboard)1) << j)) != 0) break; }
			mask::MagicBishopMoves[bit][magicIndex] = validmoves;

		}
	}
}
*/
bitboard findMagicBishop(int square, int bits) {
	//Due to laziness, this was taken with permission from Exacto 0.e source code:

	bitboard magic=0, x, y, nw_occ, sw_occ, se_occ, ne_occ;
	int o, nw = 0, sw = 0, se = 0, ne = 0, i, j, k, l, m, n;;
	bool incomplete = true;

	i = square % 8;
	j = (square - i) / 8;
	
	if((i > 1) && (j < 6)) nw = (i - 1 <= 6 - j) ? i - 1 : 6 - j; //min(i - 1, 6 - j);
	if((i > 1) && (j > 1)) sw = (i - 1 <= j - 1) ? i - 1 : j - 1; //min(i - 1, j - 1);
	if((i < 6) && (j > 1)) se = (6 - i <= j - 1) ? 6 - i : j - 1; //min(6 - i, j - 1);
	if((i < 6) && (j < 6)) ne = (6 - i <= 6 - j) ? 6 - i : 6 - j; //min(6 - i, 6 - j);
	
	bitboard all_set = 0xffffffffffffffff;

	while(incomplete) {
		incomplete = false;
		
		for(i = 0; i < MAGIC_BISHOP_SIZE ; i++) mask::MagicBishopMoves[square][i] = all_set;
		
		magic = mask::MagicBishopNumbers[square];
		for(k = 0; k <= ((((bitboard)1) << nw)-1); k++){ 
			for(l = 0; l <= ((((bitboard)1) << sw)-1); l++) {
				for(m = 0; m <= ((((bitboard)1) << se)-1); m++) {
					for(n = 0; n <= ((((bitboard)1) << ne)-1); n++ ) {
						nw_occ = 0;
						sw_occ = 0;
						se_occ = 0;
						ne_occ = 0;

						for(o = 0; o <= nw; o++) nw_occ += ((k & (((bitboard)1) << o)) << (square + ((6*o) + 7)));
						for(o = 0; o <= sw; o++) sw_occ += ((l & (((bitboard)1) << o)) << (square - ((10*o) + 9)));
						for(o = 0; o <= se; o++) se_occ += ((m & (((bitboard)1) << o)) << (square - ((8*o) + 7)));
						for(o = 0; o <= ne; o++) ne_occ += ((n & (((bitboard)1) << o)) << (square + ((8*o) + 9)));
						x = nw_occ + sw_occ + se_occ + ne_occ;
						
						y = 0;
						for(o = square, i = o%8, j = (o-i)/8; (i > 0) && (j < 7) && (((((bitboard)1) << o) & x) == 0); i = o % 8, j = (o-i)/8){o += 7; y += (((bitboard)1) << o);}
						for(o = square, i = o%8, j = (o-i)/8; (i > 0) && (j > 0) && (((((bitboard)1) << o) & x) == 0); i = o % 8, j = (o-i)/8){o -= 9; y += (((bitboard)1) << o);}
						for(o = square, i = o%8, j = (o-i)/8; (i < 7) && (j > 0) && (((((bitboard)1) << o) & x) == 0); i = o % 8, j = (o-i)/8){o -= 7; y += (((bitboard)1) << o);}
						for(o = square, i = o%8, j = (o-i)/8; (i < 7) && (j < 7) && (((((bitboard)1) << o) & x) == 0); i = o % 8, j = (o-i)/8){o += 9; y += (((bitboard)1) << o);}
						
						
						if(mask::MagicBishopMoves[square][quickhash(x, magic, bits)] == all_set){
						   mask::MagicBishopMoves[square][quickhash(x, magic, bits)] = y;
						} 
						else if(mask::MagicBishopMoves[square][quickhash(x, magic, bits)] != y) {
							cout << "Bummer, dude.  The rook_magic for square " << square << " is not magical after all.\n"; 
						}
						
					}
				}
			}
		}
	}
	
	
	
	return magic;
}

bitboard findMagicRook(int square, int bits) {
	//Due to laziness, this was taken with permission from Exacto 0.e source code:
	
	// This function populates rook moves table; to have it operate as a find magics file, swap commented portions
	
	bitboard magic=0, x, y, right_occ, left_occ, up_occ, down_occ;
	bitboard right = 0, left = 0, i, k, l, m, n;
	int o, up = 0, down = 0;
	bool incomplete = true;
	
	bitboard	const	two_to_the[64]				=	{	0x0000000000000001,	0x0000000000000002,	0x0000000000000004,	0x0000000000000008,	
				0x0000000000000010,	0x0000000000000020,	0x0000000000000040,	0x0000000000000080,	0x0000000000000100,	0x0000000000000200,	
				0x0000000000000400,	0x0000000000000800,	0x0000000000001000,	0x0000000000002000,	0x0000000000004000,	0x0000000000008000,	
				0x0000000000010000,	0x0000000000020000,	0x0000000000040000,	0x0000000000080000,	0x0000000000100000,	0x0000000000200000,	
				0x0000000000400000,	0x0000000000800000,	0x0000000001000000,	0x0000000002000000,	0x0000000004000000,	0x0000000008000000,	
				0x0000000010000000,	0x0000000020000000,	0x0000000040000000,	0x0000000080000000,	0x0000000100000000,	0x0000000200000000,
				0x0000000400000000,	0x0000000800000000,	0x0000001000000000,	0x0000002000000000,	0x0000004000000000,	0x0000008000000000,	
				0x0000010000000000,	0x0000020000000000,	0x0000040000000000,	0x0000080000000000,	0x0000100000000000,	0x0000200000000000,	
				0x0000400000000000,	0x0000800000000000,	0x0001000000000000,	0x0002000000000000,	0x0004000000000000,	0x0008000000000000,	
				0x0010000000000000,	0x0020000000000000,	0x0040000000000000,	0x0080000000000000,	0x0100000000000000,	0x0200000000000000,	
				0x0400000000000000,	0x0800000000000000,	0x1000000000000000,	0x2000000000000000,	0x4000000000000000,	0x8000000000000000};
	
    /*
    bitboard	const	not_two_to_the[64]			=	{	0xfffffffffffffffe,	0xfffffffffffffffd,	0xfffffffffffffffb,	0xfffffffffffffff7,
				0xffffffffffffffef,	0xffffffffffffffdf,	0xffffffffffffffbf,	0xffffffffffffff7f,	0xfffffffffffffeff,	0xfffffffffffffdff,	
				0xfffffffffffffbff,	0xfffffffffffff7ff,	0xffffffffffffefff,	0xffffffffffffdfff,	0xffffffffffffbfff,	0xffffffffffff7fff,	
				0xfffffffffffeffff,	0xfffffffffffdffff,	0xfffffffffffbffff,	0xfffffffffff7ffff,	0xffffffffffefffff,	0xffffffffffdfffff,	
				0xffffffffffbfffff,	0xffffffffff7fffff,	0xfffffffffeffffff,	0xfffffffffdffffff,	0xfffffffffbffffff,	0xfffffffff7ffffff,	
				0xffffffffefffffff,	0xffffffffdfffffff,	0xffffffffbfffffff,	0xffffffff7fffffff,	0xfffffffeffffffff,	0xfffffffdffffffff,	
				0xfffffffbffffffff,	0xfffffff7ffffffff,	0xffffffefffffffff,	0xffffffdfffffffff,	0xffffffbfffffffff,	0xffffff7fffffffff,	
				0xfffffeffffffffff,	0xfffffdffffffffff,	0xfffffbffffffffff,	0xfffff7ffffffffff,	0xffffefffffffffff,	0xffffdfffffffffff,	
				0xffffbfffffffffff,	0xffff7fffffffffff,	0xfffeffffffffffff,	0xfffdffffffffffff,	0xfffbffffffffffff,	0xfff7ffffffffffff,	
				0xffefffffffffffff,	0xffdfffffffffffff,	0xffbfffffffffffff,	0xff7fffffffffffff,	0xfeffffffffffffff,	0xfdffffffffffffff,	
				0xfbffffffffffffff,	0xf7ffffffffffffff,	0xefffffffffffffff,	0xdfffffffffffffff,	0xbfffffffffffffff,	0x7fffffffffffffff};
	*/
    
     bitboard	const	all_set						=		0xffffffffffffffff;

	
	if((square % 8) < 6) left = 6 - (square % 8);
	if((square % 8) > 1) right = (square % 8) - 1;
	if(((square - (square % 8)) / 8) < 6) up = 6 - ((square - (square % 8)) / 8);
	if(((square - (square % 8)) / 8) > 1) down = ((square - (square % 8)) / 8) - 1;
	
	while(incomplete) {
		incomplete = false;
		
		for(i = 0; i < MAGIC_ROOK_SIZE ; i++) mask::MagicRookMoves[square][i] = all_set; //(uncomment this line for magic generation)
		
		//magic = random64(); //(uncomment this line for magic generation)
		
		magic = mask::MagicRookNumbers[square];  //(comment this line out for magic generation)
		
		for(k = 0; k <= (two_to_the[right]-1); k++){
			for(l = 0; l <= (two_to_the[left]-1); l++) {
				for(m = 0; m <= (two_to_the[up]-1); m++) {
					for(n = 0; n <= (two_to_the[down]-1); n++ ) {
						
						// Generate all possible occupancy bitboards for the appropriate mask
						right_occ = k << ((square - (square % 8)) + 1);
						left_occ = l << ((square - (square % 8)) + ((square % 8)+1));
						up_occ = 0;
						down_occ = 0;
						for(o = 0; o <= up; o++) up_occ += ((m & two_to_the[o]) << (square + ((7*o) + 8)));
						for(o = 0; o <= down; o++) down_occ += ((n & two_to_the[o]) << (square - ((9*o) + 8)));
						x = up_occ + down_occ + right_occ + left_occ;
						
						// Generate the corresponding attacks bitboard to hash
						y = 0;
						for(o = square; ((o % 8) < 7) && ((two_to_the[o] & x) == 0);){o++; y += two_to_the[o];}
						for(o = square; (((o - (o%8))/8) < 7) && ((two_to_the[o] & x) == 0);){o += 8; y += two_to_the[o];}
						for(o = square; ((o % 8) > 0) && ((two_to_the[o] & x) == 0);){o--; y += two_to_the[o];}
						for(o = square; (((o - (o%8))/8) > 0) && ((two_to_the[o] & x) == 0);){o -= 8; y += two_to_the[o];}
						
						if(mask::MagicRookMoves[square][quickhash(x, magic, bits)] == all_set){
							// A new hash entry
							mask::MagicRookMoves[square][quickhash(x, magic, bits)] = y;
						} else if(mask::MagicRookMoves[square][quickhash(x, magic, bits)] != y) {
							// A bad collision
							//incomplete = true; //(uncomment this line for magic generation)
							cout << "Bummer, dude.  The rook_magic for square " << square << " is not magical after all.\n"; //(comment out for generation)
							//k = two_to_the[right];
							//l = two_to_the[left];
							//m = two_to_the[up];
							//n = two_to_the[down];
						}
						
					}}}}
	}
	return magic;
	return 0;
}


void populateMagicRookArray() {
	//Taken with permission from Exacto 0.e source code:
	for(int i = 0; i < 64; i++) {
		findMagicRook(i, 64-mask::MagicRookShift[i]);
	}
}

void populateMagicBishopArray() {
	//Taken with permission from Exacto 0.e source code:
	for(int i = 0; i < 64; i++) {
		findMagicBishop(i, 64-mask::MagicBishopShift[i]);
	}
	/*
	cout << "*** populateMagicBishopArray() ***\n";
	
	for(int i=0; i < 64; i++) {
		mask::MagicBishopMoves[i][0] = 0;
		bitprint(mask::MagicBishopMoves[i][0]);
	}*/
}


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
#ifdef _MSC_VER
	_BitScanForward64(&index, bb);
#else
    return __builtin_ffsll(bb) - 1;
#endif
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
#ifdef _MSC_VER
	return (unsigned char)(63 -__lzcnt64(bb));
#else
    return (unsigned char)(63 - __builtin_clzll(bb));
#endif

}

bool bitcheck(bitboard bb, unsigned char index) {
	if( bb & (((bitboard)1) << index))
		return true;
	else
		return false;
}


unsigned char bitcount(bitboard bb) {
//Try:
// return pc_table[MASK_POP_COUNT & bb] + pc_table[MASK_POP_COUNT & (bb >> 16)] + pc_table[MASK_POP_COUNT & (bb >> 32)] + pc_table[MASK_POP_COUNT & (bb >> 48)];
	/*
	unsigned char count = 0;
	while(bb) {
		count++;
		bb ^= (((bitboard)1) << bitscan_msb(bb) );
	}
	return count;
	*/
#ifdef _MSC_VER
	return (unsigned char)(__popcnt64(bb));
#else
    return (unsigned char)(__builtin_popcountll(bb));
#endif
    
}
