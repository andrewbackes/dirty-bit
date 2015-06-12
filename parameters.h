#ifndef parameters_h
#define parameters_h

/*******************************************************************************

General Info:

*******************************************************************************/

#define ID_NAME		"DirtyBit"
#define ID_VERSION	"0.38"
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
#define NODES_FOR_TIME_CHECK		2000000

#define MAX_PLY						128

#define Q_CUTOFF					50

#define ENABLE_CHECK_EXTENSIONS

//#define ENABLE_KILLER_MOVES		//Currently has a bug which causes illegal moves to be played.

#define PVS_HORIZON					2

#define ENABLE_LMR
#define ENABLE_ROOT_LMR
#define LMR_THRESHOLD				4	// Minimum number of moves to search fully before reducing
#define LMR_HORIZON					3	// Depth at which reductions will not occur
#define LMR_REDUCTION				1	// Depth to reduce by

#define ENABLE_FUTILITY_PRUNING
#define FUTILITY_DEPTH				4
const short
futility_margin[] =					{ 0, 120, 120, 310, 310 };

#define ENABLE_NULLMOVE
#define NULL_REDUCTION				3

//#define ENABLE_IID
#define IID_REDUCTION				2
#define IID_HORIZON					6

//#define INSUFFICIENT_MATERIAL_CHECK

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
#define USE_9_12_MAGICS
//#define USE_11_14_MAGICS

/*******************************************************************************

Hashing:

*******************************************************************************/

#define HASH_TABLE_SIZE			1024		// Hash table size in megabytes

#define ENABLE_PAWN_HASH
#define PAWN_HASH_TABLE_SIZE	8			// in megabytes

//Hash Flags:
#define HASH_EXACT				2
#define HASH_ALPHA				0			// flag to indicate the value is at most this
#define HASH_BETA				1			// flag to indicate the value is at least this
#define HASH_UNKNOWN			1073741823	// floor(INFTY / 2)

/*******************************************************************************

Engine:

*******************************************************************************/

#define ASPIRATION_WINDOW		34
#define A_LOT					100					// TODO: come up with a better name.
#define A_LITTLE				16					// TODO: come up with a better name.
#define DEFAULT_NPS				2500000	// For when the current NPS has not yet been calculated.
#define DEFAULT_EBF				3.75	//3.75
#define TIME_WIGGLE_ROOM		1.25	//25% time buffer.

#define ENABLE_PONDERING		false

/*******************************************************************************

Evaluation:

*******************************************************************************/
//#define ENABLE_SIMPLE_EVAL

#define SIDE_TO_MOVE_BONUS				0//11

// Pawn evaluation:
#define PASSED_PAWN_EARLY_VALUE			10
#define PASSED_PAWN_LATE_VALUE			10
#define ISOLATED_PAWN_EARLY_PENALTY		1
#define ISOLATED_PAWN_LATE_PENALTY		1
#define DOUBLED_PAWN_EARLY_PENALTY		1	
#define DOUBLED_PAWN_LATE_PENALTY		2
#define BACKWARD_PAWN_EARLY_PENALTY		1
#define BACKWARD_PAWN_LATE_PENALTY		1
//#define CONNECTED_PAWN_EARLY_VALUE	5
//#define CONNECTED_PAWN_LATE_VALUE		10

// Pawn Shelter:
#define SHELTER_BONUS					3	//Temporary
#define PIECE_SHELTER_BONUS				0

#define SHELTER_SECOND_RANK_PENALTY		0
#define SHELTER_THIRD_RANK_PENALTY		1
#define SHELTER_FOURTH_RANK_PENALTY		2
#define SHELTER_FIFTH_RANK_PENALTY		3



// King Safety:
#define KNIGHT_KING_THREAT_EARLY_BONUS	3
#define KNIGHT_KING_THREAT_LATE_BONUS	3
#define BISHOP_KING_THREAT_EARLY_BONUS	3
#define BISHOP_KING_THREAT_LATE_BONUS	3
#define ROOK_KING_THREAT_EARLY_BONUS	5
#define ROOK_KING_THREAT_LATE_BONUS		5
#define QUEEN_KING_THREAT_EARLY_BONUS	9
#define QUEEN_KING_THREAT_LATE_BONUS	9

#define SEMIOPEN_THREAT_EARLY_BONUS		1
#define SEMIOPEN_THREAT_LATE_BONUS		0
#define OPEN_THREAT_EARLY_BONUS			2
#define OPEN_THREAT_LATE_BONUS			1

// Bishop and Knight:
#define BISHOP_PAIR_EARLY_BONUS			0	//0.31=0.
#define BISHOP_PAIR_LATE_BONUS			75

#define	KNIGHT_LATE_PENALTY				10

// Mobility:

#define QUEEN_MOBILITY_EARLY_BONUS		1	//Added to the bonus already given when calculating rook and bishop moves.
#define QUEEN_MOBILITY_LATE_BONUS		1	//Added to the bonus already given when calculating rook and bishop moves.

#define ROOK_SEMI_OPEN_FILE_EARLY_BONUS	1
#define ROOK_SEMI_OPEN_FILE_LATE_BONUS	1
#define ROOK_OPEN_FILE_EARLY_BONUS		3	//keep in mind this is added in addition to the semi bonus.
#define ROOK_OPEN_FILE_LATE_BONUS		1	//keep in mind this is added in addition to the semi bonus.
#define ROOK_MOBILITY_EARLY_BONUS		1
#define ROOK_MOBILITY_LATE_BONUS		1

#define BISHOP_MOBILITY_EARLY_BONUS		1
#define BISHOP_MOBILITY_LATE_BONUS		1

#define KNIGHT_MOBILITY_EARLY_BONUS		2
#define KNIGHT_MOBILITY_LATE_BONUS		2

// Piece Values:
#define PAWN_VALUE						100
#define KNIGHT_VALUE					325		//Remember: that making knight!=bishop will mess up a condition in SEE
#define BISHOP_VALUE					325
#define ROOK_VALUE						500
#define QUEEN_VALUE						975
#define KING_VALUE						9900

#endif