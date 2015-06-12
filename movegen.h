#ifndef movegen_h
#define movegen_h

#include "board.h"
#include "nextmove.h"

#include "parameters.h"
#include "macros.h"

//void CaptureGen(CHESSBOARD *b, MOVELIST * m);
//void CaptureGen(CHESSBOARD *b, QMOVELIST * capture_list);
void nonCaptureGen(CHESSBOARD *b, MOVELIST * m);

template <class LIST> void CaptureGen(CHESSBOARD * b, LIST * capture_list) {
//void CaptureGen(CHESSBOARD *b, MOVELIST * capture_list) {
/*************************************************************************
Generates captures and promotions. In order:
	-Knights
	-Bishops
	-Rook
	-Queen
	-King
	-Pawn

*************************************************************************/

	//vector<MOVE> capture_list;
	//capture_list.reserve(MOVE_LIST_SIZE);
	bool toMove = b->getActivePlayer();
	bool opponent = !toMove;

	
/*	-------------------------------------
	KNIGHT CAPTURES:
	------------------------------------- */

	bitboard pieceDB = b->getPieceBB(toMove,nKnight);
	 while(pieceDB) {
		 unsigned char from = getMostAdvanced(toMove, &pieceDB);
		 bitboard attackMoveDB = mask::knight_moves[from] & b->getOccupiedBB(opponent);
		 while(attackMoveDB) {
			 unsigned char target = bitscan_msb(attackMoveDB);
			 capture_list->addCapture(MOVE(from,target,toMove, nKnight, b->getBoard(target)));
			 attackMoveDB ^= (1i64 << target);
		 }
		 pieceDB ^= (1i64 << from); //remove bit from DB
	 }

/*	-------------------------------------
	BISHOP:
	------------------------------------- */
	pieceDB = b->getPieceBB(toMove, nBishop);// | b->getPieceBB(toMove, nQueen);
	while(pieceDB) {
		unsigned char from = getMostAdvanced(toMove, &pieceDB);
		bitboard toDB =	mask::MagicBishopMoves[from][((b->getOccupiedBB(BOTH)&mask::MagicBishopMask[from]) * mask::MagicBishopNumbers[from])>>mask::MagicBishopShift[from]] 
						& b->getOccupiedBB(!toMove);
		while(toDB) {
			unsigned char target = getMostAdvanced(toMove, &toDB);
			capture_list->addCapture(MOVE(from,target,toMove,nBishop, b->getBoard(target)));
			toDB^=(1i64<<target);
		}
		pieceDB^=(1i64<<from);
	}

	
		
/*	-------------------------------------
	ROOK CAPTURES:
	------------------------------------- */

	pieceDB = b->getPieceBB(toMove, nRook);// | b->getPieceBB(toMove, nQueen);
	while(pieceDB) {
		unsigned char from = getMostAdvanced(toMove, &pieceDB);
		bitboard toDB =	mask::MagicRookMoves[from][((b->getOccupiedBB(BOTH) & mask::MagicRookMask[from]) * mask::MagicRookNumbers[from])>>mask::MagicRookShift[from]] 
						& b->getOccupiedBB(!toMove);
		while(toDB) {
			unsigned char target = getMostAdvanced(toMove, &toDB);
			capture_list->addCapture(MOVE(from,target,toMove,nRook, b->getBoard(target)));
			toDB^=(1i64<<target);
		}
		pieceDB^=(1i64<<from);
	}

	
/*	-------------------------------------
	QUEEN CAPTURES:
	------------------------------------- */
	
	pieceDB = b->getPieceBB(toMove, nQueen);
	while(pieceDB) {
		unsigned char from = getMostAdvanced(toMove, &pieceDB);
		
		bitboard toDB =	(	mask::MagicBishopMoves[from][((b->getOccupiedBB(BOTH)&mask::MagicBishopMask[from]) * mask::MagicBishopNumbers[from])>>mask::MagicBishopShift[from]] 
						|	mask::MagicRookMoves[from][((b->getOccupiedBB(BOTH) & mask::MagicRookMask[from]) * mask::MagicRookNumbers[from])>>mask::MagicRookShift[from]] )
						&	b->getOccupiedBB(!toMove);
		while(toDB) {
			unsigned char target = getMostAdvanced(toMove, &toDB);
			capture_list->addCapture(MOVE(from,target,toMove,nQueen, b->getBoard(target)));
			toDB^=(1i64<<target);
		}
		/*
		toDB =	mask::MagicRookMoves[from][((b->getOccupiedBB(BOTH) & mask::MagicRookMask[from]) * mask::MagicRookNumbers[from])>>mask::MagicRookShift[]] 
						& b->getOccupiedBB(!toMove);
		while(toDB) {
			unsigned char target = getMostAdvanced(toMove, &toDB);
			capture_list->addCapture(MOVE(from,target,toMove,nQueen, b->getBoard(target)));
			toDB^=(1i64<<target);
		}
		*/
		pieceDB^=(1i64<<from);
	}
	
	
/*	-------------------------------------
	KING CAPTURES:
	------------------------------------- */

	unsigned char from = bitscan_msb(b->getPieceBB(toMove,nKing));
	bitboard validMoveDB = mask::king_moves[from] & b->getOccupiedBB(opponent);
	while(validMoveDB) {
		unsigned char target = bitscan_msb(validMoveDB);
		capture_list->addCapture(MOVE(from ,target, toMove, nKing, b->getBoard(target)));
		validMoveDB ^= (1i64 << target);
	}
	 
/*	-------------------------------------
	PAWN CAPTURES:
	------------------------------------- */

	//Pawn kills:
	pieceDB = b->getPieceBB(toMove,nPawn) & ~mask::pawns_spawn[!toMove];
	while(pieceDB) {
		//quit after exhausting all of the bits in the current database.
		unsigned char from = getMostAdvanced(toMove, &pieceDB);
		//Check for normal attacks (non promote kills):
		bitboard kill_list = (mask::pawn_captures[toMove][from] & b->getOccupiedBB(opponent)) & ~(RankMask(1)|RankMask(8));
		while(kill_list) {
			int index = bitscan_msb(kill_list);
			capture_list->addCapture(MOVE(from,index,toMove, nPawn,b->getBoard(index)) );
			kill_list ^= (1i64 << index);
		}
		//En passant kills:
		kill_list = mask::pawn_captures[toMove][from] & mask::square[b->getEnPassantIndex()];
		if(kill_list) {
			int index = bitscan_msb(kill_list);
			capture_list->addCapture(MOVE(from,index,toMove, nPawn,0,0,nPieceValue[nPawn]));
		}

		pieceDB ^= (1i64 << from); //remove bit from DB
	}

	//Pawn Promotions To Queen or Capture Promotions:
	pieceDB = (b->getPieceBB(toMove,nPawn) & mask::pawns_spawn[!toMove]);
	while(pieceDB) {
		unsigned char from = getMostAdvanced(toMove, &pieceDB);
		bitboard promote_kills	=	(mask::pawn_captures[toMove][from] &  b->getOccupiedBB(!toMove) );
		bitboard promote		=	(mask::pawn_advances[toMove][from] & ~b->getOccupiedBB(BOTH)    );
		
		//Promotion via attacks for all possibilities:
		while(promote_kills) {
			unsigned char to = bitscan_msb(promote_kills);
			capture_list->addCapture( MOVE(from,to,toMove,nPawn,b->getBoard(to), nQueen, Q_PROMOTE_CAPTURE) );
			//capture_list->addCapture( MOVE(from,to,toMove,nPawn,b->getBoard(to), nKnight, N_PROMOTE_CAPTURE) );
			//capture_list->addCapture( MOVE(from,to,toMove,nPawn,b->getBoard(to), nBishop, B_PROMOTE_CAPTURE) );
			//capture_list->addCapture( MOVE(from,to,toMove,nPawn,b->getBoard(to), nRook, R_PROMOTE_CAPTURE) );
			promote_kills ^= (1i64 << to);
		}
		pieceDB ^= (1i64 << from); //remove bit from DB
		
		//Promotion via advancement to Queen only:
		if(promote) {
			unsigned char to = bitscan_msb(promote);
			capture_list->addCapture( MOVE(from,to,toMove,nPawn,0, nQueen, Q_PROMOTE) );
			//capture_list->addCapture( MOVE(from,to,toMove,nPawn, 0, nKnight, N_PROMOTE) );
			//capture_list->addCapture( MOVE(from,to,toMove,nPawn, 0, nBishop, B_PROMOTE) );
			//capture_list->addCapture( MOVE(from,to,toMove,nPawn, 0, nRook, R_PROMOTE) );
		}
	}

}

#endif