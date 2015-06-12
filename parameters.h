

/*******************************************************************************

General Info:

*******************************************************************************/

#define ID_NAME		"DirtyBit"
#define ID_VERSION	"0.3"
#define ID_AUTHOR	"Andrew Backes"


/*******************************************************************************

Debug:

*******************************************************************************/

//#define DEBUG_TIME
//#define DEBUG_SEARCH
//#define SEARCH_STATS


/*******************************************************************************

Search:

*******************************************************************************/

#define MAX_PLY						128

#define Q_CUTOFF					50

#define ENABLE_CHECK_EXTENSIONS

//#define ENABLE_KILLER_MOVES

#define PVS_HORIZON					2

#define ENABLE_LMR
#define LMR_THRESHOLD				4
#define LMR_HORIZON					3

#define ENABLE_NULLMOVE
#define NULL_REDUCTION				2

//#define ENABLE_IID
#define IID_REDUCTION				2
#define IID_HORIZON					5
/*******************************************************************************

NextMove()/MOVELIST/MOVE:

*******************************************************************************/

#define MOVE_LIST_SIZE 256

#define INITIATE		0

#define HASH_MOVE		0
#define CAPTURES		1
#define KILLER_MOVE_1	2
#define KILLER_MOVE_2	3
#define NONCAPTURES		4
#define BAD_CAPTURES	5


/*******************************************************************************

Move Generation:

*******************************************************************************/

//Hacked values to force these moves to be where they belong in the move list:

//Capture List:
#define Q_PROMOTE_CAPTURE	875
#define R_PROMOTE_CAPTURE	400
#define B_PROMOTE_CAPTURE	220
#define N_PROMOTE_CAPTURE	220
#define Q_PROMOTE			875

//Non-Capture List:
#define K_CASTLE			0
#define Q_CASTLE			0
#define N_PROMOTE			220
#define B_PROMOTE			220
#define R_PROMOTE			400

/*******************************************************************************

Board and Pieces:

*******************************************************************************/

#define nPawn				1
#define nKnight				2
#define nKing				3
#define nBishop				5
#define nRook				6
#define nQueen				7

#define WHITE				0
#define BLACK				1
#define BOTH				2

#define KING_SIDE			0
#define QUEEN_SIDE			1

/*******************************************************************************

Bitboards:

*******************************************************************************/

//#define mask::MagicRookShift[]	50
//#define mask::MagicBishopShift[]	53

/*******************************************************************************

Hashing:

*******************************************************************************/

#define HASH_TABLE_SIZE 1000			// Hash table size in megabytes

//Hash Flags:
#define HASH_EXACT		2
#define HASH_ALPHA		0			// flag to indicate the value is at most this
#define HASH_BETA		1			// flag to indicate the value is at least this
#define HASH_UNKNOWN	1073741823	// floor(INFINITY / 2)

/*******************************************************************************

Engine:

*******************************************************************************/

#define ASPIRATION_WINDOW		34
#define A_LOT					100					// TODO: come up with a better name.
#define A_LITTLE				16					// TODO: come up with a better name.
#define DEFAULT_NPS				2500000	// For when the current NPS has not yet been calculated.
#define DEFAULT_EBF				3.75
#define TIME_WIGGLE_ROOM		1.25	//25% time buffer.

#define ENABLE_PONDERING		false

/*******************************************************************************

Evaluation:

*******************************************************************************/

#define SIDE_TO_MOVE_BONUS			0//11

#define PASSED_PAWN_VALUE			10

#define BISHOP_PAIR_EARLY_BONUS		10
#define BISHOP_PAIR_LATE_BONUS		75

#define	KNIGHT_LATE_PENALTY			10

#define PAWN_VALUE					100
#define KNIGHT_VALUE				325
#define BISHOP_VALUE				325
#define ROOK_VALUE					500
#define QUEEN_VALUE					975
#define KING_VALUE					9900

