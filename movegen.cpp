
#include <vector>
#include <algorithm>
#include <iostream>

#include "move.h"
#include "board.h"
#include "utilities.h"
#include "evaluate.h"
#include "nextmove.h"
#include "movegen.h"

using namespace std;

void nonCaptureGen(CHESSBOARD *b, MOVELIST * move_list ) {
/*************************************************************************
Todo: generate the furthest advanced piece's move first.

In order:
	-Castles
	-Knights
	-Bishops
	-Rook
	-Queen
	-King
	-Pawn

*************************************************************************/	
	
	//vector<MOVE> move_list;
	//move_list.reserve(MOVE_LIST_SIZE);
	bool toMove = b->getActivePlayer();
	bitboard pieceDB;

/*	-------------------------------------
	Castles:
	------------------------------------- */
	if(b->getCastlingRights(toMove, KING_SIDE) == true ) {
		if(((b->getOccupiedBB(BOTH) & mask::kCastle_path[toMove])==0) 
			&& (mask::kRook_spawn[toMove] & b->getPieceBB(toMove,nRook)) 
			&& (mask::king_spawn[toMove] & b->getPieceBB(toMove,nKing))	
		) {	
			if( !b->isInCheck()
				&& !b->isAttacked(bitscan_msb(mask::kBishop_spawn[toMove]))  
			) {
				unsigned char from = bitscan_msb(mask::king_spawn[toMove]);
				unsigned char target = bitscan_msb(mask::kCastle[toMove]);
				move_list->addMove(MOVE(from,target,toMove,nKing,0,0,K_CASTLE));
			}
		}
	}
	if(b->getCastlingRights(toMove, QUEEN_SIDE) == true ) {
		if(((b->getOccupiedBB(BOTH) & mask::qCastle_path[toMove]) == 0) 
			&& (mask::qRook_spawn[toMove] & b->getPieceBB(toMove,nRook)) 
			&& (mask::king_spawn[toMove] & b->getPieceBB(toMove,nKing))	
		) {
					
			if( !b->isInCheck() 
				&& !b->isAttacked(bitscan_msb(mask::queen_spawn[toMove]))   
			) {
				unsigned char from = bitscan_msb(mask::king_spawn[toMove]);
				unsigned char target = bitscan_msb(mask::qCastle[toMove]);
				move_list->addMove(MOVE(from,target,toMove,nKing,0,0,Q_CASTLE));
			}
		}
	}
	
/*	-------------------------------------
	REMAINING PROMOTIONS (non-Queen):
	------------------------------------- */

	//Pawn Promotions To Queen or Capture Promotions:
	pieceDB = (b->getPieceBB(toMove,nPawn) & mask::pawns_spawn[!toMove]);
	while(pieceDB) {
		unsigned char from = getMostAdvanced(toMove, &pieceDB);
		bitboard promote_kills	=	(mask::pawn_captures[toMove][from] &  b->getOccupiedBB(!toMove) );
		bitboard promote		=	(mask::pawn_advances[toMove][from] & ~b->getOccupiedBB(BOTH)    );
		
		//Promotion via attacks for all possibilities:
		while(promote_kills) {
			unsigned char to = bitscan_msb(promote_kills);
			//move_list->addMove( MOVE(from,to,toMove,nPawn,b->getBoard(to), nQueen, Q_PROMOTE_CAPTURE) );
			move_list->addMove( MOVE(from,to,toMove,nPawn,b->getBoard(to), nKnight, N_PROMOTE_CAPTURE) );
			move_list->addMove( MOVE(from,to,toMove,nPawn,b->getBoard(to), nBishop, B_PROMOTE_CAPTURE) );
			move_list->addMove( MOVE(from,to,toMove,nPawn,b->getBoard(to), nRook, R_PROMOTE_CAPTURE) );
			promote_kills ^= (1i64 << to);
		}
		pieceDB ^= (1i64 << from); //remove bit from DB
		
		//Promotion via advancement to Queen only:
		if(promote) {
			unsigned char to = bitscan_msb(promote);
			//move_list->addCapture( MOVE(from,to,toMove,nPawn,0, nQueen, Q_PROMOTE) );
			move_list->addMove( MOVE(from,to,toMove,nPawn, 0, nKnight, N_PROMOTE) );
			move_list->addMove( MOVE(from,to,toMove,nPawn, 0, nBishop, B_PROMOTE) );
			move_list->addMove( MOVE(from,to,toMove,nPawn, 0, nRook, R_PROMOTE) );
		}
	}


	/*
	pieceDB = (b->getPieceBB(toMove,nPawn) & mask::pawns_spawn[!toMove]);
	while(pieceDB) {
		unsigned char from = getMostAdvanced(toMove, &pieceDB);
		bitboard promote		=	(mask::pawn_advances[toMove][from] & ~b->getOccupiedBB(BOTH)    );
		
		//Promotion via advance for every piece except queen:
		while(promote) {
			unsigned char to = bitscan_msb(promote);
			move_list->addMove( MOVE(from,to,toMove,nPawn, 0, nKnight, N_PROMOTE) );
			move_list->addMove( MOVE(from,to,toMove,nPawn, 0, nBishop, B_PROMOTE) );
			move_list->addMove( MOVE(from,to,toMove,nPawn, 0, nRook, R_PROMOTE) );
			promote ^= (1i64 << to);
		}
		pieceDB ^= (1i64 << from); //remove bit from DB
	}
	*/
/*	-------------------------------------
	KNIGHT MOVES:
	------------------------------------- */
	
	pieceDB = b->getPieceBB(toMove,nKnight);
	 while(pieceDB) {
		 unsigned char from = getMostAdvanced(toMove, &pieceDB);
		 bitboard validMoveDB = mask::knight_moves[from] & ~b->getOccupiedBB(BOTH);
		 while(validMoveDB) {
			 unsigned char target = bitscan_msb(validMoveDB);
			 move_list->addMove(MOVE(from,target,toMove, nKnight, 0));
			 validMoveDB ^= (1i64 << target);
		 }
		 pieceDB ^= (1i64 << from); //remove bit from DB
	 }
	 
/*	-------------------------------------
	BISHOP MOVES:
	------------------------------------- */

	pieceDB = b->getPieceBB(toMove, nBishop);// | b->getPieceBB(toMove, nQueen);
	while(pieceDB) {
		unsigned char from = getMostAdvanced(toMove, &pieceDB);
		bitboard toDB =	mask::MagicBishopMoves[from][((b->getOccupiedBB(BOTH)&mask::MagicBishopMask[from]) * mask::MagicBishopNumbers[from])>>mask::MagicBishopShift[from]] 
						& ~b->getOccupiedBB(BOTH);
		while(toDB) {
			unsigned char target = getMostAdvanced(toMove, &toDB);
			move_list->addMove(MOVE(from,target,toMove,b->getBoard(from), 0));
			toDB^=(1i64<<target);
		}
		pieceDB^=(1i64<<from);
	}
	
	

/*	-------------------------------------
	ROOK MOVES:
	------------------------------------- */

	pieceDB = b->getPieceBB(toMove, nRook);// | b->getPieceBB(toMove, nQueen);
	while(pieceDB) {
		unsigned char from = getMostAdvanced(toMove, &pieceDB);
		bitboard toDB =	mask::MagicRookMoves[from][((b->getOccupiedBB(BOTH) & mask::MagicRookMask[from]) * mask::MagicRookNumbers[from])>>mask::MagicRookShift[from]] 
						& ~b->getOccupiedBB(BOTH);
		while(toDB) {
			unsigned char target = getMostAdvanced(toMove, &toDB);
			move_list->addMove(MOVE(from,target,toMove,b->getBoard(from), 0));
			toDB^=(1i64<<target);
		}
		pieceDB^=(1i64<<from);
	}

	
/*	-------------------------------------
	QUEEN MOVES:
	------------------------------------- */
	
	pieceDB = b->getPieceBB(toMove, nQueen);
	while(pieceDB) {
		unsigned char from = getMostAdvanced(toMove, &pieceDB);
		bitboard toDB =	( mask::MagicRookMoves[from][((b->getOccupiedBB(BOTH) & mask::MagicRookMask[from]) * mask::MagicRookNumbers[from])>>mask::MagicRookShift[from]] 
						| mask::MagicBishopMoves[from][((b->getOccupiedBB(BOTH)&mask::MagicBishopMask[from]) * mask::MagicBishopNumbers[from])>>mask::MagicBishopShift[from]] )
						& ~b->getOccupiedBB(BOTH);
		while(toDB) {
			unsigned char target = getMostAdvanced(toMove, &toDB);
			move_list->addMove(MOVE(from,target,toMove,nQueen, 0));
			toDB^=(1i64<<target);
		}
		/*
		toDB =	mask::MagicBishopMoves[from][((b->getOccupiedBB(BOTH)&mask::MagicBishopMask[from]) * mask::MagicBishopNumbers[from])>>mask::MagicBishopShift[]] 
						& ~b->getOccupiedBB(BOTH);
		while(toDB) {
			unsigned char target = getMostAdvanced(toMove, &toDB);
			move_list->addMove(MOVE(from,target,toMove,nQueen, 0));
			toDB^=(1i64<<target);
		}
		*/
		pieceDB^=(1i64<<from);
	}
	
	
/*	-------------------------------------
	KING MOVES:
	------------------------------------- */

	pieceDB = b->getPieceBB(toMove,nKing);
	unsigned char from = bitscan_msb(pieceDB);
	bitboard validMoveDB = mask::king_moves[from] & ~b->getOccupiedBB(BOTH);
	while(validMoveDB) {
		int target = bitscan_msb(validMoveDB);
		move_list->addMove(MOVE(from,target,toMove,nKing, 0));
		validMoveDB ^= (1i64 << target);
	}
	
/*	-------------------------------------
	PAWN MOVES:
	------------------------------------- */

	pieceDB = b->getPieceBB(toMove,nPawn) & ~mask::pawns_spawn[!toMove];

	while(pieceDB) {
		//quit after exhausting all of the bits in the current database.
		unsigned char from = getMostAdvanced(toMove, &pieceDB);
		
		//Single Advance:
		bitboard advance = mask::pawn_advances[toMove][from] & ~b->getOccupiedBB(BOTH) ;
		if(advance) {
			unsigned char to = bitscan_msb(advance);
			move_list->addMove(MOVE(from,to,toMove, nPawn));
			
			//Double Advance:
			advance = mask::pawn_double_advances[toMove][from] & ~b->getOccupiedBB(BOTH);
			if(advance) {
				to = bitscan_msb(advance);
				move_list->addMove(MOVE(from,to,toMove, nPawn));
			}
		}
		pieceDB ^= (1i64 << from); //remove bit from DB
	}

}



