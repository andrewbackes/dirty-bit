

#define RankOf(x) ((x)/8 + 1)
#define FileOf(x) ((x)%8)

//for input 1-8
#define RankMask(x) (bitboard)((bitboard)255 << (8*(x-1)))

//for input 0-7 or a-h
#define FileMask(x) (bitboard)((bitboard)72340172838076673 << (x))

//for input 0-63
#define RankMaskSq(x) (bitboard)( (bitboard)255 << (8 * (((x)/8)+1)) )
#define FileMaskSq(x) (bitboard)( (bitboard)72340172838076673 << ((x)%8))


