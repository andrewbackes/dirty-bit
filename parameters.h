#ifndef parameters_h
#define parameters_h

/*******************************************************************************

General Info:

*******************************************************************************/

#define ID_NAME		"DirtyBit"
#define ID_VERSION	"0.39.7 (issue 8b dev)"
#define ID_AUTHOR	"Andrew Backes"


/*******************************************************************************

Debug:

*******************************************************************************/

#define DEBUG_TIME
//#define DEBUG_SEARCH
//#define SEARCH_STATS
//#define DEBUG_ZOBRIST
//#define DEBUG_KILLERS

/*******************************************************************************

Search:

*******************************************************************************/
#define NODES_FOR_TIME_CHECK		2000000

#define MAX_PLY						128

#define Q_CUTOFF					50

#define ENABLE_CHECK_EXTENSIONS

#define ENABLE_KILLER_MOVES		

#define PVS_HORIZON					2

#define ENABLE_LMR
#define ENABLE_ROOT_LMR
#define LMR_THRESHOLD				4	// Minimum number of moves to search fully before reducing
#define LMR_HORIZON					3	// Depth at which reductions will not occur
#define LMR_REDUCTION				1	// Depth to reduce by

#define ENABLE_FUTILITY_PRUNING
#define FUTILITY_DEPTH				4
const short
//futility_margin[] =					{ 0, 190, 190, 375, 375 };
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

#define WHITE				false
#define BLACK				true
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
#define DEPTH_CUTOFF			100

#define ENABLE_PONDERING		false

/*******************************************************************************

Evaluation:

*******************************************************************************/

#define PAWN_PHASE_VALUE				0
#define KNIGHT_PHASE_VALUE				1
#define BISHOP_PHASE_VALUE				1
#define ROOK_PHASE_VALUE				2
#define QUEEN_PHASE_VALUE				4
#define TOTAL_PHASE						(PAWN_PHASE_VALUE*16 + KNIGHT_PHASE_VALUE*4 + BISHOP_PHASE_VALUE*4 + ROOK_PHASE_VALUE*4 + QUEEN_PHASE_VALUE*2)

// Piece Values:
#define PAWN_VALUE						100
#define KNIGHT_VALUE					375//325		//Remember: that making knight!=bishop will mess up a condition in SEE
#define BISHOP_VALUE					375//325
#define ROOK_VALUE						500
#define QUEEN_VALUE						1050//975
#define KING_VALUE						9900
#define EDGE_PAWN_PENALTY				8//0//15

// Adjust piece values as a function of pawns:
//#define ENABLE_PIECE_VALUE_ADJUSTMENTS
const char KNIGHT_ADJUSTMENT[] =		{ -31, -25, -19, -13, -6, 0, 6, 13, 19};
const char BISHOP_ADJUSTMENT[] =		{ 38, 31, 25, 19, 13, 0, -13, -19, -25 };
const char ROOK_ADJUSTMENT[] =			{ 62, 50, 38, 25, 13, 0, -13, -25, -38 };

//#define ENABLE_SIMPLE_EVAL
#define SIDE_TO_MOVE_EARLY_BONUS			2//5//11
#define SIDE_TO_MOVE_LATE_BONUS				4//8//11

// PST
#define ENABLE_QUEEN_PST
//#define NEW_PSTS
//#define OLD_PSTS
#define TEST_PSTS

// Pawn evaluation:
#define PASSED_PAWN_EARLY_MIN_VALUE		4//9
#define PASSED_PAWN_EARLY_MAX_VALUE		60//66
#define PASSED_PAWN_LATE_MIN_VALUE		10//13
#define PASSED_PAWN_LATE_MAX_VALUE		120//99
#define CANIDATE_EARLY_MIN_VALUE		4//4
#define CANIDATE_EARLY_MAX_VALUE		36//36
#define CANIDATE_LATE_MIN_VALUE			6//9
#define CANIDATE_LATE_MAX_VALUE			48//48
#define GUARANTEED_PROMOTION_VALUE		(QUEEN_VALUE - PAWN_VALUE - PASSED_PAWN_LATE_MAX_VALUE - 66)
//Late only. From self:
#define SELF_BLOCKING_MIN_PENALTY		2
#define SELF_BLOCKING_MAX_PENALTY		8//15
//Late only. From Enemies:
#define UNTHREATENED_PATH_MIN_BONUS		4
#define UNTHREATENED_PATH_MAX_BONUS		24//20
#define CLEAR_PATH_MIN_BONUS			4
#define CLEAR_PATH_MAX_BONUS			24//20

#define ISOLATED_PAWN_EARLY_PENALTY		5//1//10
#define ISOLATED_PAWN_LATE_PENALTY		10//1//18
#define ISOLATED_OPEN_FILE				5//0//12

#define DOUBLED_PAWN_EARLY_PENALTY		5//6//1//9//6	
#define DOUBLED_PAWN_LATE_PENALTY		10//12//2//15//25
#define DOUBLED_PAWN_OPEN_FILE			5//0//15
#define DOUBLED_PAWN_REAR_ADVANCED		2//0//6

#define BACKWARD_PAWN_EARLY_PENALTY		2//1//1//0//8
#define BACKWARD_PAWN_LATE_PENALTY		4//2//10//0//12 
#define BACKWARD_PAWN_OPEN_FILE			1//0//10//0//8 //additionally

//two types of connectedness:
#define CONNECTED_PAWN_EARLY_VALUE		2
#define CONNECTED_PAWN_LATE_VALUE		3
#define PAWN_DUO_EARLY_VALUE			3
#define PAWN_DUO_LATE_VALUE				7

// Pawn Shelter:
const unsigned char
shelter_penalty[] =						{ 0, 0, 1, 4, 8, 9, 10, 10 };//{ 0, 0, 5, 10, 13, 16, 17, 18 };//{ 0, 0, 11, 20, 27, 32, 34, 36 };
const unsigned char
storm_penalty[] =						{ 0, 0, 0, 0, 4, 8, 12, 0 };//{ 0, 0, 0, 0, 4, 13, 27, 0 };// { 0, 0, 0, 9, 27, 54, 0, 0 };

// King Safety:
const unsigned char
threat_score[] =						//{ 0, 50, 75, 88, 93, 96, 98, 100, 100, 100, 100, 100, 100, 100, 100, 100 };
{ 0, 50, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 };
#define THREAT_RATIO					100//100
#define KNIGHT_THREAT_WEIGHT			3//1
#define BISHOP_THREAT_WEIGHT			3//1
#define ROOK_THREAT_WEIGHT				5//2
#define QUEEN_THREAT_WEIGHT				9//4

#define SEMIOPEN_THREAT_EARLY_BONUS		1
#define SEMIOPEN_THREAT_LATE_BONUS		0
#define OPEN_THREAT_EARLY_BONUS			2
#define OPEN_THREAT_LATE_BONUS			1

// Bishop and Knight:
#define BISHOP_PAIR_EARLY_BONUS			25	//0.31=0.
#define BISHOP_PAIR_LATE_BONUS			50//75
							 			//{ 75, 75, 72, 69, 66, 63, 59, 56, 53, 47, 44, 41, 38, 34, 31, 28, 25, 0 };
										//{ 75, 75, 70, 65, 60, 55, 50, 45, 40, 35, 30, 25, 20, 15, 10, 5,0 };
										  
#define	KNIGHT_LATE_PENALTY				10//0//10
#define ROOK_REDUNDANCY_LATE_PENALTY	0

// Mobility:
const char knight_mobility[] =			{ -19, -13, -6, 0, 6, 13, 16, 19, 19 };
const char bishop_mobility[] =			{ -13, -6, 2, 9, 16, 23, 29, 33, 36, 37, 38, 39, 40, 40, 41, 41 };
const char rook_mobility[] =			{ -10, -7, -4, -1, 2, 5, 7, 10, 12, 13, 14, 14, 15, 15, 16, 16};
const char queen_mobility[] =			{ -5, -4, -3, -2, -1, 1, 2, 3, 4, 5, 6, 8, 8, 9, 9, 10};

#define QUEEN_MOBILITY_EARLY_BONUS		1//1
#define QUEEN_MOBILITY_LATE_BONUS		1//1

#define ROOK_SEMI_OPEN_FILE_EARLY_BONUS	9//1//10
#define ROOK_SEMI_OPEN_FILE_LATE_BONUS	9//1//10
#define ROOK_OPEN_FILE_EARLY_BONUS		9//3//24	
#define ROOK_OPEN_FILE_LATE_BONUS		18//1//14	
#define ROOK_MOBILITY_EARLY_BONUS		1//1
#define ROOK_MOBILITY_LATE_BONUS		1//1
#define ROOK_SUPPORTING_PP_EARLY_BONUS	10//10
#define ROOK_SUPPORTING_PP_LATE_BONUS	20//25
#define ROOK_7TH_FILE_EARLY_BONUS		15//15
#define ROOK_7TH_FILE_LATE_BONUS		25//25

#define BISHOP_MOBILITY_EARLY_BONUS		1//1
#define BISHOP_MOBILITY_LATE_BONUS		1//1

#define KNIGHT_MOBILITY_EARLY_BONUS		2//2
#define KNIGHT_MOBILITY_LATE_BONUS		2//2

//Patterns:
//#define ENABLE_PATTERNS
#define TRAPPED_BISHOP					100//170
#define TRAPPED_ROOK					50//60


#endif