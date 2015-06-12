

#define RankOf(x) ((x)/8 + 1)
#define FileOf(x) ((x)%8)

//for input 1-8
#define RankMask(x) (bitboard)(255i64 << (8*(x-1)))

//for input 0-7 or a-h
#define FileMask(x) (bitboard)(72340172838076673i64 << (x))

//for input 0-63
#define RankMaskSq(x) (bitboard)( 255i64 << (8 * (((x)/8)+1)) )
#define FileMaskSq(x) (bitboard)( 72340172838076673i64 << ((x)%8))

