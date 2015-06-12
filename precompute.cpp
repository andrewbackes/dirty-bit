

/*******************************************************************************

precompute.h/.cpp is intended to hold all of the functions used to generate
precomputed data. Some of these are used at the time of initialization,
while others are ran once and the output is copied and pasted into the code.

*******************************************************************************/

#include "precompute.h"

short curve(short x) {
	// returns y=x^2
	return x*x;
}

unsigned char fit(short m, short M, short x, short mesh) {
	short resolution = 128;
	return (unsigned char)(m + ((((M - m)*resolution*curve(x)) / curve(mesh - 1)) / resolution));
}

unsigned char	passed_pawn_value[8]; //= { 0, 7, 7, 10, 14, 19, 27, 27 }; // was 0, 7, 7, ...
unsigned char	passed_pawn_late_value[8];// = { 0, 10, 10, 13, 18, 25, 36, 36 };
unsigned char	canidate_pp_value[8];// = { 0, 5, 5, 7, 10, 14, 20, 20 };	//75% of pp
unsigned char	canidate_pp_late_value[8];// = { 0, 9, 9, 11, 16, 23, 33, 33 };
unsigned char	pp_self_blocking_penalty[8];// = { 41, 0, 0, 6, 11, 26, 41, 41 }; // > x5
unsigned char	pp_unthreatened_path_bonus[8];// = { 0, 0, 0, 1, 3, 7, 12, 12 }; // x3 /2
unsigned char	pp_clear_path_bonus[8];// = { 12, 0, 0, 0, 2, 4, 8, 12 }; //index 0 is a clear path.
void precompute::passed_pawns() {
	int rows_to_count = 7;

	passed_pawn_value[0] = 0;
	passed_pawn_late_value[0] = 0;
	canidate_pp_value[0] = 0;
	canidate_pp_late_value[0] = 0;
	pp_unthreatened_path_bonus[0] = 0;
	pp_clear_path_bonus[0] = 0;
	pp_self_blocking_penalty[0] = 0;

	for (int row = 1; row < rows_to_count; row++){
		passed_pawn_value[row] = PASSED_PAWN_EARLY_MIN_VALUE + fit(0, PASSED_PAWN_EARLY_MAX_VALUE - PASSED_PAWN_EARLY_MIN_VALUE, row-1, rows_to_count-1);
		passed_pawn_late_value[row] = PASSED_PAWN_LATE_MIN_VALUE + fit(0, PASSED_PAWN_LATE_MAX_VALUE - PASSED_PAWN_LATE_MIN_VALUE, row-1, rows_to_count-1);
		canidate_pp_value[row] = CANIDATE_EARLY_MIN_VALUE + fit(0, CANIDATE_EARLY_MAX_VALUE - CANIDATE_EARLY_MIN_VALUE, row-1, rows_to_count-1);
		canidate_pp_late_value[row] = CANIDATE_LATE_MIN_VALUE + fit(0, CANIDATE_LATE_MAX_VALUE - CANIDATE_LATE_MIN_VALUE, row-1, rows_to_count-1);

	}

	passed_pawn_value[7] = 0;
	passed_pawn_late_value[7] = 0;
	canidate_pp_value[7] = 0;
	canidate_pp_late_value[7] = 0;
	
	rows_to_count = 8;
	for (int row = 1; row < rows_to_count; row++) {
		pp_unthreatened_path_bonus[row] = UNTHREATENED_PATH_MIN_BONUS+fit(0, UNTHREATENED_PATH_MAX_BONUS - UNTHREATENED_PATH_MIN_BONUS, row-1, rows_to_count-1);
		pp_clear_path_bonus[row] = CLEAR_PATH_MIN_BONUS+fit(0, CLEAR_PATH_MAX_BONUS - CLEAR_PATH_MIN_BONUS, row-1, rows_to_count-1);

		pp_self_blocking_penalty[row] = (SELF_BLOCKING_MAX_PENALTY)
			-fit(0, SELF_BLOCKING_MAX_PENALTY - SELF_BLOCKING_MIN_PENALTY, row-1, rows_to_count-1);
	}
}
unsigned char BISHOP_PAIR_BONUS[17];
void precompute::material_adjustments() {
	for (int i = 0; i <= 16; i++) {
		BISHOP_PAIR_BONUS[16 - i] = fit(BISHOP_PAIR_EARLY_BONUS, BISHOP_PAIR_LATE_BONUS, i, 17);
	}

}
bitboard mask::ray[64][64];
void precompute::rays() {
	// fills the array with bitboards representing a ray that a 
	// sliding piece would make from one square to another square
	// meaning: ray[from_square][to_square]


	//for each source square, make its diagonal and straight rays.
	for (int source = 0; source < 64; source++) {
		
		//check to see if the destination square can be reached by a ray
		for (int dest = 0; dest < 64; dest++) {
			
			mask::ray[source][dest] = 0;
			bitboard mask = 0;
			if (dest == source) continue;
			
			//verticle
			if ((source - dest) % 8 == 0) {
				if (source - dest > 0) {
					//down
					for (int sq = source - 8; sq >= dest; sq = sq - 8) mask |= (((bitboard)1) << sq); }
				else {
					//up
					for (int sq = source + 8; sq <= dest; sq = sq + 8) mask |= (((bitboard)1) << sq); }
			}

			// Diagonal left
			else if ((source - dest) % 9 == 0) {
				// NW
				if ( /*(source%7 != 0) && */(dest > source) ) {
					for (int sq = source + 9; sq <= dest; sq = sq + 9) mask |= (((bitboard)1) << sq); }
				// SE
				if ( /*(source%8 != 0) && */(source > dest) ) {
					for (int sq = source - 9; sq >= dest; sq = sq - 9) mask |= (((bitboard)1) << sq); }
			}
			
			// Diagonal right
			else if ((source - dest) % 7 == 0) {
				// NE
				if ( /*(source%8 != 0) && */(dest > source) ) {
					for (int sq = source + 7; sq <= dest; sq = sq + 7) mask |= (((bitboard)1) << sq); }
				// SW
				if ( /*(source%7 != 0) && */(source > dest) ) {
					for (int sq = source - 7; sq >= dest; sq = sq - 7) mask |= (((bitboard)1) << sq); }
			}
			// Possible Horizontal
			else {
				if (source > dest) {
					//possibly right
					//check the distance to the right side of the board.
					int dist = source % 8;
					if (source - dest <= dist) {
						for (int sq = source - 1; sq >= dest; sq--) mask |= (((bitboard)1) << sq);
					}
				}
				else {
					//possibly left
					//check the distance to the left
					int dist = 7 - (source % 8);
					if (dest - source <= dist) {
						for (int sq = source + 1; sq <= dest; sq++) mask |= (((bitboard)1) << sq);
					}
				}
			}
			mask::ray[source][dest] = mask;
		}
	}
}

bitboard mask::rule_of_the_square[2][64];
void precompute::rule_of_the_squares() {
	
	//WHITE
	for (int sq = 0; sq < 64; sq++) {
		int dist = 8 - RankOf(sq)+1;
		bitboard beam = (((bitboard)1) << sq) | mask::north[sq];
		
		bitboard mask = beam;
		//go left:
		for (int j = 0; j < dist; j++) {
			mask |= ((((bitboard)1) << (sq+j)) | mask::north[sq+j]);
			if (FileOf(sq) + j == 7) break;
		}
		//go right:
		for (int j = 0; j < dist; j++) {
			mask |= ((((bitboard)1) << (sq - j)) | mask::north[sq - j]);
			if (FileOf(sq) - j == 0) break;
		}
		mask::rule_of_the_square[WHITE][sq] = mask;
	}
	//BLACK
	for (int sq = 0; sq < 64; sq++) {
		int dist = RankOf(sq);
		bitboard beam = (((bitboard)1) << sq) | mask::south[sq];

		bitboard mask = beam;
		//go left:
		for (int j = 0; j < dist; j++) {
			mask |= ((((bitboard)1) << (sq + j)) | mask::south[sq + j]);
			if (FileOf(sq) + j == 7) break;
		}
		//go right:
		for (int j = 0; j < dist; j++) {
			mask |= ((((bitboard)1) << (sq - j)) | mask::south[sq - j]);
			if (FileOf(sq) - j == 0) break;
		}
		mask::rule_of_the_square[BLACK][sq] = mask;
	}

}

void precompute::castle_areas() {
	bitboard t = (((bitboard)1) << h1) | (((bitboard)1) << g1) | (((bitboard)1) << f1);
	std::cout << "mask::short_castle_area[WHITE] = " << t << std::endl;
	t = (((bitboard)1) << h8) | (((bitboard)1) << g8) | (((bitboard)1) << f8);
	std::cout << "mask::short_castle_area[BLACK] = " << t << std::endl;
	t = (((bitboard)1) << a1) | (((bitboard)1) << b1) | (((bitboard)1) << c1);
	std::cout << "mask::long_castle_area[WHITE] = " << t << std::endl;
	t = (((bitboard)1) << a8) | (((bitboard)1) << b8) | (((bitboard)1) << c8);
	std::cout << "mask::long_castle_area[BLACK] = " << t << std::endl;

	bitboard w = (((bitboard)1) << e1) | (((bitboard)1) << e2) | (((bitboard)1) << d1) | (((bitboard)1) << d2);
	bitboard b = (((bitboard)1) << e8) | (((bitboard)1) << e7) | (((bitboard)1) << d8) | (((bitboard)1) << d7);
	std::cout << "mask:between_castle_area[] = {" << w << " , " << b << " };\n";
	
}
void precompute::castle_pawns() {
	bitboard t = 0;
	std::cout << "mask::short_castle_pawns[]={ ";
	t = (((bitboard)1) << h2) | (((bitboard)1) << g2) | (((bitboard)1) << f2);
	std::cout << t << " , ";
	t = (((bitboard)1) << h7) | (((bitboard)1) << g7) | (((bitboard)1) << f7);
	std::cout << t << " };" << std::endl;

	std::cout << "mask::long_castle_pawns[]={ ";
	t = (((bitboard)1) << a2) | (((bitboard)1) << b2) | (((bitboard)1) << c2);
	std::cout << t << " , ";
	t = (((bitboard)1) << a7) | (((bitboard)1) << b7) | (((bitboard)1) << c7);
	std::cout << t << " };" << std::endl;

	std::cout << "mask::mid_castle_pawns[]={ ";
	t = (((bitboard)1) << d2) | (((bitboard)1) << e2) | (((bitboard)1) << f2);
	std::cout << t << " , ";
	t = (((bitboard)1) << d7) | (((bitboard)1) << e7) | (((bitboard)1) << f7);
	std::cout << t << " };" << std::endl;

}

unsigned char shelter_score[4096];
void precompute::shelter_scores() {
	
	unsigned char penalty[5] = { 32, 0, 11, 20, 27 };
	
	int r[2][5];
	r[WHITE][0] = 0;
	r[WHITE][1] = (1 << 0) | (1 << 1) | (1 << 2);
	r[WHITE][2] = (1 << 3) | (1 << 4) | (1 << 5);
	r[WHITE][3] = (1 << 6) | (1 << 7) | (1 << 8);
	r[WHITE][4] = (1 << 9) | (1 << 10) | (1 << 11);

	int file[3];
	file[0] = (1 << 2) | (1 << 5) | (1 << 8) | (1 << 11);
	file[1] = (1 << 1) | (1 << 4) | (1 << 7) | (1 << 10);
	file[2] = (1 << 0) | (1 << 3) | (1 << 6) | (1 << 9);

	for (int key = 0; key < 4096; key++) {
		int score = 0;

		//WHITE:
		for (int i = 0; i < 3; i++) {
			// get the least advanced bit.
			int col = file[i] & key;
			int row = 0;
			for (int j = 1; j < 5; j++) {
				if (col & r[WHITE][j]) {
					row = j;
					break;
				}
			}
			score += penalty[row];
		}
		shelter_score[key] = (score == 0 ? penalty[2] : score);
	}



	/*
	for (int i = 0; i <= 4; i++) {
		for (int j = 0; j <= 4; j++) {
			for (int k = 0; k <= 4; k++) {
				int key = 0;
				// i, j, k represent files.
				// the number in each means that row's bit is turned on.
				// example:
				// 0 0 0	11 10 9
				// 0 1 0	8  7  6
				// 0 0 1	5  4  3
				// 1 0 0	2  1  0
				// - - -
				int value = penalty[i] + penalty[j] + penalty[k];
				int i_bit = (i ? 2 + ((i-1) * 3) : 0);
				int j_bit = (j ? 1 + ((j-1) * 3) : 0);
				int k_bit = (k ? 0 + ((k-1) * 3) : 0);
				key = key | (1 << i_bit) | (1 << j_bit) | (1 << k_bit);
				shelter_score[key] = (value == 0 ? penalty[2] : value);
			}
		}
	}
	*/

}

// file:	20,18,16,10,10, 16, 18,20
// row:		0,14,19,26,35,46,59,126
//int row[] = { 0, 14, 19, 26, 35, 46, 59, 106 };
//int col[] = { 20, 18, 16, 10, 10, 16, 18, 20 };
/*
int col[] = { 10, 9, 8, 7, 7, 8, 9, 10 };

cout << "0, 0, 0, 0, 0, 0, 0, 0" << endl;
cout << "5, 5, 5, 5, 5, 5, 5, 5, 5, 5" << endl;
for (int r = 1; r < 6; r++) {
for (int c = 0; c < 8; c++) {
int val = col[c] * pow(1.4, r-1);

cout << val << ",";
}
cout << endl;
}
cout << "0, 0, 0, 0, 0, 0, 0, 0" << endl;

cout << endl;


cout << "0, 0, 0, 0, 0, 0, 0, 0" << endl;

for (int r = 5; r > 0; r--) {
for (int c = 7; c >=0 ; c--) {
int val = col[c] * pow(1.4, r - 1);

cout << val << ",";
}
cout << endl;
}
cout << "5, 5, 5, 5, 5, 5, 5, 5, 5, 5" << endl;
cout << "0, 0, 0, 0, 0, 0, 0, 0" << endl;

*/

/*
for (int r = 7; r >= 0; r--) {
for (int c = 7; c >= 0; c--) {
cout << row[r] + col[c] << ",";
}
cout << endl;
}
*/
/*
cout << (FileMask(g));
cout << endl;
cout << (FileMask(f)|FileMask(h));
cout << endl;
cout << (FileMask(e)|FileMask(g));
cout << endl;
cout << (FileMask(d)|FileMask(f));
cout << endl;
cout << (FileMask(c)|FileMask(e));
cout << endl;
cout << (FileMask(b)|FileMask(d));
cout << endl;
cout << (FileMask(a)|FileMask(c));
cout << endl;
cout << (FileMask(b));
cout << endl;
*/
/*
bitboard rook_masks[65];
bitboard bishop_masks[65];
int l;

for(int r = 0; r <= 7; r++){
for(int j = 0; j <= 7; j++){
rook_masks[(8*j) + r] = 0;
for(int k = 1; k <= 6; k++){
if(k != r) rook_masks[(8*j) + r] += (((bitboard)1) << ((8*j) + k));  // Horizontal
if(k != j) rook_masks[(8*j) + r] += (((bitboard)1) << ((8*k) + r));  // Vertical
}

bishop_masks[(8*j) + r] = 0;
for(int k = r+1, l=j+1; max(k,l) <= 6; k++, l++) bishop_masks[(8*j)+r]+=(((bitboard)1)<<((8*l)+k));        // Up, left
for(int k = r+1, l=j-1; (k <= 6) & (l >= 1); k++, l--) bishop_masks[(8*j)+r]+=(((bitboard)1)<<((8*l)+k));  // Down, left
for(int k = r-1, l=j+1; (k >= 1) & (l <= 6); k--, l++) bishop_masks[(8*j)+r]+=(((bitboard)1)<<((8*l)+k));  // Up, right
for(int k = r-1, l=j-1; min(k, l) >= 1; k--, l--) bishop_masks[(8*j)+r]+=(((bitboard)1)<<((8*l)+k));       // Down, right


}
}

ofstream myfile;
myfile.open ("output.txt");
bitboard border = mask::file[a] | mask::file[h] | mask::rank[1] | mask::rank[8];

myfile << "const bitboard MagicBishopMask[65] = {";
for(int r =0; r < 64 ; r++) {
myfile << bishop_masks[r] << ", ";
}
myfile << "0000000000000000000000000000000000000000000000000000000000000000";
myfile << "};\n\n";

myfile << "const bitboard MagicRookMask[65] = {";
for(int r =0; r < 64 ; r++) {
myfile << rook_masks[r] << ", ";
}
myfile << "0000000000000000000000000000000000000000000000000000000000000000";
myfile << "};\n\n";

myfile.close();
//-----------------------------------------------------------
*/

/*
cout << "const bitboard passed_pawn[2][64] = {";
cout << "{\n";
for(int r=0; r < 64; r++) {
bitboard mask = mask::north[r];
if( r % 8 < 7)
mask |= mask::north[r+1];
if( r % 8 > 0)
mask |= mask::north[r-1];
//bitprint(mask);
cout << mask;
if(r<63) cout << ",";
if(r%3 == 0) cout << endl;
}
cout << "\n},\n{\n";
for(int r=0; r < 64; r++) {
bitboard mask = mask::south[r];
if( r % 8 < 7)
mask |= mask::south[r+1];
if( r % 8 > 0)
mask |= mask::south[r-1];
//bitprint(mask);
cout << mask;
if(r<63) cout << ",";
if(r%3 == 0) cout << endl;
}
cout << "};\n";
*/