// This is for compiler efficiency
//
// headers have been arranged in the order of dependency.
//
//

typedef unsigned char BYTE;

#include "parameters.h"
#include "macros.h"
#include "bitboards.h"
#include "precompute.h"
#include "move.h"
#include "hash.h"
#include "board.h"
#include "engine.h"
#include "movegen.h"
#include "perft.h"
#include "SEE.h"
#include "nextmove.h"
#include "evaluate.h"
#include "search.h"

#include "utilities.h"
#include "uci.h"
#include "book.h"
