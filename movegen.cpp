
#include <vector>
#include <algorithm>
#include <iostream>

#include "move.h"
#include "board.h"
#include "utilities.h"
#include "evaluate.h"

using namespace std;

bool MoveCompare (MOVE i,MOVE j) { 	
	return (i.static_value>j.static_value); 
}

unsigned char getMostAdvanced(bool toMove, bitboard * pieceDB) {
//Returns the bit that is furthest advanced for the side to move.
	if(toMove == WHITE) 
		return bitscan_msb(*pieceDB);
	else
		return bitscan_lsb(*pieceDB);
}

vector<MOVE> nonCaptureGen(CHESSBOARD *b) {
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
	
	vector<MOVE> move_list;
	bool toMove = b->getActivePlayer();
	bitboard pieceDB;

/*	-------------------------------------
	Castles:
	------------------------------------- */
 
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
				move_list.push_back(MOVE(from,target,toMove,nKing, b->getBoard(target)));
			}
		}
	}
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
				move_list.push_back(MOVE(from,target,toMove,nKing,b->getBoard(target)));
			}
		}
	}

/*	-------------------------------------
	KNIGHT MOVES:
	------------------------------------- */
	
	pieceDB = b->getPieceBB(toMove,nKnight);
	 while(pieceDB) {
		 unsigned char from = getMostAdvanced(toMove, &pieceDB);
		 bitboard validMoveDB = mask::knight_moves[from] & ~b->getOccupiedBB(BOTH);
		 while(validMoveDB) {
			 unsigned char target = bitscan_msb(validMoveDB);
			 move_list.push_back(MOVE(from,target,toMove, nKnight, 0));
			 validMoveDB ^= (1i64 << target);
		 }
		 pieceDB ^= (1i64 << from); //remove bit from DB
	 }
	 
/*	-------------------------------------
	BISHOP MOVES:
	------------------------------------- */

	pieceDB = b->getPieceBB(toMove, nBishop);
	while(pieceDB) {
		unsigned char from = getMostAdvanced(toMove, &pieceDB);
		//NW:
		bitboard nwMoves = mask::nw[from];
		unsigned char blockedSquare = bitscan_lsb(nwMoves & b->getOccupiedBB(BOTH));
		nwMoves ^= mask::nw[blockedSquare];
		nwMoves &= ~b->getOccupiedBB(BOTH);
		while(nwMoves) {
			unsigned char target = bitscan_msb(nwMoves);
			move_list.push_back(MOVE(from,target,toMove,nBishop, 0));
			nwMoves^=(1i64 << target);
		}
		//NE:
		bitboard neMoves = mask::ne[from];
		blockedSquare = bitscan_lsb(neMoves & b->getOccupiedBB(BOTH));
		neMoves ^= mask::ne[blockedSquare];
		neMoves &= ~b->getOccupiedBB(BOTH);
		while(neMoves) {
			unsigned char target = bitscan_msb(neMoves);
			move_list.push_back(MOVE(from,target,toMove,nBishop,0));
			neMoves^=(1i64 << target);
		}
		//SE:
		bitboard seMoves = mask::se[from];
		blockedSquare = bitscan_msb(seMoves & b->getOccupiedBB(BOTH));
		seMoves ^= mask::se[blockedSquare];
		seMoves &= ~b->getOccupiedBB(BOTH);
		while(seMoves) {
			unsigned char target = bitscan_msb(seMoves);
			move_list.push_back(MOVE(from,target,toMove,nBishop,0));
			seMoves^=(1i64 << target);
		}
		//SW:
		bitboard swMoves = mask::sw[from];
		blockedSquare = bitscan_msb(swMoves & b->getOccupiedBB(BOTH));
		swMoves ^= mask::sw[blockedSquare];
		swMoves &= ~b->getOccupiedBB(BOTH);
		while(swMoves) {
			unsigned char target = bitscan_msb(swMoves);
			move_list.push_back(MOVE(from,target,toMove,nBishop,0));
			swMoves^=(1i64 << target);
		} 
		pieceDB ^= (1i64 << from); //remove bit from DB
	}

/*	-------------------------------------
	ROOK MOVES:
	------------------------------------- */

	pieceDB = b->getPieceBB(toMove, nRook);
	while(pieceDB) {
		unsigned char from = getMostAdvanced(toMove, &pieceDB);
		//North:
		bitboard northMoves = mask::north[from];
		unsigned char blockedSquare = bitscan_lsb(northMoves & b->getOccupiedBB(BOTH));
		northMoves ^= mask::north[blockedSquare];
		northMoves &= ~b->getOccupiedBB(BOTH);
		while(northMoves) {
			unsigned char target = bitscan_msb(northMoves);
			move_list.push_back(MOVE(from,target,toMove,nRook,0));
			northMoves^=(1i64 << target);
		}
		//East:
		bitboard eastMoves = mask::east[from];
		blockedSquare = bitscan_msb(eastMoves & b->getOccupiedBB(BOTH));
		eastMoves ^= mask::east[blockedSquare];
		eastMoves &= ~b->getOccupiedBB(BOTH);
		while(eastMoves) {
			unsigned char target = bitscan_msb(eastMoves);
			move_list.push_back(MOVE(from,target,toMove,nRook,0));
			eastMoves^=(1i64 << target);
		}
		//South:
		bitboard southMoves = mask::south[from];
		blockedSquare = bitscan_msb(southMoves & b->getOccupiedBB(BOTH));
		southMoves ^= mask::south[blockedSquare];
		southMoves &= ~b->getOccupiedBB(BOTH);
		while(southMoves) {
			unsigned char target = bitscan_msb(southMoves);
			move_list.push_back(MOVE(from,target,toMove,nRook,0));
			southMoves^=(1i64 << target);
		}
		//West:
		bitboard westMoves = mask::west[from];
		blockedSquare = bitscan_lsb(westMoves & b->getOccupiedBB(BOTH));
		westMoves ^= mask::west[blockedSquare];
		westMoves &= ~b->getOccupiedBB(BOTH);
		while(westMoves) {
			unsigned char target = bitscan_msb(westMoves);
			move_list.push_back(MOVE(from,target,toMove,nRook,0));
			westMoves^=(1i64 << target);
		} 
		pieceDB ^= (1i64 << from); //remove bit from DB
	}

/*	-------------------------------------
	QUEEN MOVES:
	------------------------------------- */
	
	pieceDB = b->getPieceBB(toMove, nQueen);
	while(pieceDB) {
		int from = getMostAdvanced(toMove, &pieceDB);
		//North:
		bitboard northMoves = mask::north[from];
		unsigned char blockedSquare = bitscan_lsb(northMoves & b->getOccupiedBB(BOTH));
		northMoves ^= mask::north[blockedSquare];
		northMoves &= ~b->getOccupiedBB(BOTH);
		while(northMoves) {
			unsigned char target = bitscan_msb(northMoves);
			move_list.push_back(MOVE(from,target,toMove,nQueen,0));
			northMoves^=(1i64 << target);
		}
		//East:
		bitboard eastMoves = mask::east[from];
		blockedSquare = bitscan_msb(eastMoves & b->getOccupiedBB(BOTH));
		eastMoves ^= mask::east[blockedSquare];
		eastMoves &= ~b->getOccupiedBB(BOTH);
		while(eastMoves) {
			unsigned char target = bitscan_msb(eastMoves);
			move_list.push_back(MOVE(from,target,toMove,nQueen, 0));
			eastMoves^=(1i64 << target);
		}
		//South:
		bitboard southMoves = mask::south[from];
		blockedSquare = bitscan_msb(southMoves & b->getOccupiedBB(BOTH));
		southMoves ^= mask::south[blockedSquare];
		southMoves &= ~b->getOccupiedBB(BOTH);
		while(southMoves) {
			unsigned char target = bitscan_msb(southMoves);
			move_list.push_back(MOVE(from,target,toMove,nQueen,0));
			southMoves^=(1i64 << target);
		}
		//West:
		bitboard westMoves = mask::west[from];
		blockedSquare = bitscan_lsb(westMoves & b->getOccupiedBB(BOTH));
		westMoves ^= mask::west[blockedSquare];
		westMoves &= ~b->getOccupiedBB(BOTH);
		while(westMoves) {
			unsigned char target = bitscan_msb(westMoves);
			move_list.push_back(MOVE(from,target,toMove,nQueen, 0));
			westMoves^=(1i64 << target);
		} 
		//NW:
		bitboard nwMoves = mask::nw[from];
		blockedSquare = bitscan_lsb(nwMoves & b->getOccupiedBB(BOTH));
		nwMoves ^= mask::nw[blockedSquare];
		nwMoves &= ~b->getOccupiedBB(BOTH);
		while(nwMoves) {
			unsigned char target = bitscan_msb(nwMoves);
			move_list.push_back(MOVE(from,target,toMove,nQueen, 0));
			nwMoves^=(1i64 << target);
		}
		//NE:
		bitboard neMoves = mask::ne[from];
		blockedSquare = bitscan_lsb(neMoves & b->getOccupiedBB(BOTH));
		neMoves ^= mask::ne[blockedSquare];
		neMoves &= ~b->getOccupiedBB(BOTH);
		while(neMoves) {
			unsigned char target = bitscan_msb(neMoves);
			move_list.push_back(MOVE(from,target,toMove,nQueen, 0));
			neMoves^=(1i64 << target);
		}
		//SE:
		bitboard seMoves = mask::se[from];
		blockedSquare = bitscan_msb(seMoves & b->getOccupiedBB(BOTH));
		seMoves ^= mask::se[blockedSquare];
		seMoves &= ~b->getOccupiedBB(BOTH);
		while(seMoves) {
			unsigned char target = bitscan_msb(seMoves);
			move_list.push_back(MOVE(from,target,toMove,nQueen, 0));
			seMoves^=(1i64 << target);
		}
	
		//SW:
		bitboard swMoves = mask::sw[from];
		blockedSquare = bitscan_msb(swMoves & b->getOccupiedBB(BOTH));
		swMoves ^= mask::sw[blockedSquare];
		swMoves &= ~b->getOccupiedBB(BOTH);
		while(swMoves) {
			unsigned char target = bitscan_msb(swMoves);
			move_list.push_back(MOVE(from,target,toMove,nQueen,0));
			swMoves^=(1i64 << target);
		}
		pieceDB ^= (1i64 << from); //remove bit from DB
	}

/*	-------------------------------------
	KING MOVES:
	------------------------------------- */

	pieceDB = b->getPieceBB(toMove,nKing);
	unsigned char from = bitscan_msb(pieceDB);
	bitboard validMoveDB = mask::king_moves[from] & ~b->getOccupiedBB(BOTH);
	while(validMoveDB) {
		int target = bitscan_msb(validMoveDB);
		move_list.push_back(MOVE(from,target,toMove,nKing, 0));
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
			move_list.push_back(MOVE(from,to,toMove, nPawn));
			
			//Double Advance:
			advance = mask::pawn_double_advances[toMove][from] & ~b->getOccupiedBB(BOTH);
			if(advance) {
				to = bitscan_msb(advance);
				move_list.push_back(MOVE(from,to,toMove, nPawn));
			}
		}
		pieceDB ^= (1i64 << from); //remove bit from DB
	}


/*	------------------------------------- */
	 
	 return move_list;

}

vector<MOVE> CaptureGen(CHESSBOARD *b) {
/*************************************************************************
Generates captures and promotions. In order:
	-Knights
	-Bishops
	-Rook
	-Queen
	-King
	-Pawn

*************************************************************************/

	vector<MOVE> capture_list;
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
			 capture_list.push_back(MOVE(from,target,toMove, nKnight, b->getBoard(target)));
			 attackMoveDB ^= (1i64 << target);
		 }
		 pieceDB ^= (1i64 << from); //remove bit from DB
	 }

/*	-------------------------------------
	BISHOP/QUEEN CAPTURES:
	------------------------------------- */

	pieceDB = b->getPieceBB(toMove, nBishop);
	while(pieceDB) {
		unsigned char from = getMostAdvanced(toMove, &pieceDB);
		//NW:
		unsigned char attackedSquare = bitscan_lsb(mask::nw[from] & b->getOccupiedBB(BOTH));
		if( (attackedSquare < 64) && ( (1i64 << attackedSquare)&b->getOccupiedBB(opponent)) )
			capture_list.push_back(MOVE(from,attackedSquare,toMove,nBishop, b->getBoard(attackedSquare)));

		//NE:
		attackedSquare = bitscan_lsb(mask::ne[from] & b->getOccupiedBB(BOTH));
		if( (attackedSquare < 64) && ( (1i64 << attackedSquare)&b->getOccupiedBB(opponent)) )
			capture_list.push_back(MOVE(from,attackedSquare,toMove,nBishop, b->getBoard(attackedSquare)));

		//SE:
		attackedSquare = bitscan_msb(mask::se[from] & b->getOccupiedBB(BOTH));
		if( (attackedSquare < 64) && ( (1i64 << attackedSquare)&b->getOccupiedBB(opponent)) )
			capture_list.push_back(MOVE(from,attackedSquare,toMove,nBishop, b->getBoard(attackedSquare)));

		//SW:
		attackedSquare = bitscan_msb( mask::sw[from] & b->getOccupiedBB(BOTH));
		if( (attackedSquare < 64) && ( (1i64 << attackedSquare)&b->getOccupiedBB(opponent)) )
			capture_list.push_back(MOVE(from,attackedSquare,toMove,nBishop, b->getBoard(attackedSquare)));
		
		pieceDB ^= (1i64 << from); //remove bit from DB
	}
		
/*	-------------------------------------
	ROOK CAPTURES:
	------------------------------------- */

	pieceDB = b->getPieceBB(toMove, nRook);
	while(pieceDB) {
		unsigned char from = getMostAdvanced(toMove, &pieceDB);
		//North:
		unsigned char attackedSquare = bitscan_lsb(mask::north[from] & b->getOccupiedBB(BOTH));
		if( (attackedSquare < 64) && ( (1i64 << attackedSquare)&b->getOccupiedBB(opponent)) )
			capture_list.push_back(MOVE(from,attackedSquare,toMove,nRook,b->getBoard(attackedSquare)));

		//East:
		attackedSquare = bitscan_msb(mask::east[from] & b->getOccupiedBB(BOTH));
		if(mask::square[attackedSquare] & b->getOccupiedBB(opponent) )
			capture_list.push_back(MOVE(from,attackedSquare,toMove,nRook,b->getBoard(attackedSquare)));

		//South:
		attackedSquare = bitscan_msb(mask::south[from] & b->getOccupiedBB(BOTH));
		if( (attackedSquare < 64) && ( (1i64 << attackedSquare)&b->getOccupiedBB(opponent)) )
			capture_list.push_back(MOVE(from,attackedSquare,toMove,nRook,b->getBoard(attackedSquare)));

		//West:
		attackedSquare = bitscan_lsb(mask::west[from] & b->getOccupiedBB(BOTH));
		if( (attackedSquare < 64) && ( (1i64 << attackedSquare)&b->getOccupiedBB(opponent)) )
			capture_list.push_back(MOVE(from,attackedSquare,toMove,nRook,b->getBoard(attackedSquare)));
 
		pieceDB ^= (1i64 << from); //remove bit from DB
	}

/*	-------------------------------------
	QUEEN CAPTURES:
	------------------------------------- */

	pieceDB = b->getPieceBB(toMove, nQueen);
	while(pieceDB) {
		unsigned char from = getMostAdvanced(toMove, &pieceDB);
		//NW:
		unsigned char attackedSquare = bitscan_lsb( mask::nw[from] & b->getOccupiedBB(BOTH) );
		if( (attackedSquare < 64) && ( (1i64 << attackedSquare)&b->getOccupiedBB(opponent)) )
			capture_list.push_back(MOVE(from,attackedSquare,toMove,nQueen, b->getBoard(attackedSquare)));

		//NE:
		attackedSquare = bitscan_lsb( mask::ne[from] & b->getOccupiedBB(BOTH));
		if( (attackedSquare < 64) && ( (1i64 << attackedSquare)&b->getOccupiedBB(opponent)) )
			capture_list.push_back(MOVE(from,attackedSquare,toMove,nQueen, b->getBoard(attackedSquare)));

		//SE:
		attackedSquare = bitscan_msb(mask::se[from] & b->getOccupiedBB(BOTH));
		if( (attackedSquare < 64) && ( (1i64 << attackedSquare)&b->getOccupiedBB(opponent)) )
			capture_list.push_back(MOVE(from,attackedSquare,toMove,nQueen, b->getBoard(attackedSquare)));

		//SW:
		attackedSquare = bitscan_msb(mask::sw[from] & b->getOccupiedBB(BOTH));
		if( (attackedSquare < 64) && ( (1i64 << attackedSquare)&b->getOccupiedBB(opponent)) )
			capture_list.push_back(MOVE(from,attackedSquare,toMove,nQueen, b->getBoard(attackedSquare)));
		
		//North:
		attackedSquare = bitscan_lsb(mask::north[from] & b->getOccupiedBB(BOTH));
		if( (attackedSquare < 64) && ( (1i64 << attackedSquare)&b->getOccupiedBB(opponent)) )
			capture_list.push_back(MOVE(from,attackedSquare,toMove,nQueen,b->getBoard(attackedSquare)));

		//East:
		attackedSquare = bitscan_msb(mask::east[from] & b->getOccupiedBB(BOTH));
		if( (attackedSquare < 64) && ( (1i64 << attackedSquare)&b->getOccupiedBB(opponent)) )
			capture_list.push_back(MOVE(from,attackedSquare,toMove,nQueen,b->getBoard(attackedSquare)));

		//South:
		attackedSquare = bitscan_msb( mask::south[from] & b->getOccupiedBB(BOTH));
		if( (attackedSquare < 64) && ( (1i64 << attackedSquare)&b->getOccupiedBB(opponent)) )
			capture_list.push_back(MOVE(from,attackedSquare,toMove,nQueen,b->getBoard(attackedSquare)));

		//West:
		attackedSquare = bitscan_lsb(mask::west[from] & b->getOccupiedBB(BOTH));
		if( (attackedSquare < 64) && ( (1i64 << attackedSquare)&b->getOccupiedBB(opponent)) )
			capture_list.push_back(MOVE(from,attackedSquare,toMove,nQueen,b->getBoard(attackedSquare)));

		pieceDB ^= (1i64 << from); //remove bit from DB
	}

/*	-------------------------------------
	KING CAPTURES:
	------------------------------------- */

	unsigned char from = bitscan_msb(b->getPieceBB(toMove,nKing));
	bitboard validMoveDB = mask::king_moves[from] & b->getOccupiedBB(opponent);
	while(validMoveDB) {
		unsigned char target = bitscan_msb(validMoveDB);
		capture_list.push_back(MOVE(from ,target, toMove, nKing, b->getBoard(target)));
		validMoveDB ^= (1i64 << target);
	}
	 
/*	-------------------------------------
	PAWN CAPTURES:
	------------------------------------- */

	//Pawn Promotions:
	pieceDB = (b->getPieceBB(toMove,nPawn) & mask::pawns_spawn[!toMove]);
	while(pieceDB) {
		unsigned char from = getMostAdvanced(toMove, &pieceDB);
		bitboard promote =	(mask::pawn_captures[toMove][from] &  b->getOccupiedBB(!toMove) )
						 |	(mask::pawn_advances[toMove][from] & ~b->getOccupiedBB(BOTH)    );
		while(promote) {
			unsigned char to = bitscan_msb(promote);
			//Promote to both Queen and Knight:
			capture_list.push_back( MOVE(from,to,toMove,nPawn,b->getBoard(to), nQueen, nPieceValue[nQueen]-nPieceValue[nPawn]) );
			capture_list.push_back( MOVE(from,to,toMove,nPawn,b->getBoard(to), nKnight, nPieceValue[nKnight]-nPieceValue[nPawn]) );
			capture_list.push_back( MOVE(from,to,toMove,nPawn,b->getBoard(to), nRook, nPieceValue[nRook]-nPieceValue[nPawn]) );
			capture_list.push_back( MOVE(from,to,toMove,nPawn,b->getBoard(to), nBishop, nPieceValue[nBishop]-nPieceValue[nPawn]) );
			promote ^= (1i64 << to);
		}
		pieceDB ^= (1i64 << from); //remove bit from DB
	}
	
	//Pawn kills:
	pieceDB = b->getPieceBB(toMove,nPawn) & ~mask::pawns_spawn[!toMove];
	while(pieceDB) {
		//quit after exhausting all of the bits in the current database.
		unsigned char from = getMostAdvanced(toMove, &pieceDB);
		//Check for normal attacks:
		bitboard kill_list = mask::pawn_captures[toMove][from] & b->getOccupiedBB(opponent);
		while(kill_list) {
			int index = bitscan_msb(kill_list);
			capture_list.push_back(MOVE(from,index,toMove, nPawn,b->getBoard(index)) );
			kill_list ^= (1i64 << index);
		}
		//En passant kills:
		kill_list = mask::pawn_captures[toMove][from] & mask::square[b->getEnPassantIndex()];
		if(kill_list) {
			int index = bitscan_msb(kill_list);
			capture_list.push_back(MOVE(from,index,toMove, nPawn,0,0,nPieceValue[nPawn]));
		}

		pieceDB ^= (1i64 << from); //remove bit from DB
	}

/*	------------------------------------- */

	return capture_list;
}

vector<MOVE> MoveGen(CHESSBOARD *b) {
	vector<MOVE> move_list;
	
	bool toMove = b->getActivePlayer();

/*	-------------------------------------
	PAWN MOVES:
	------------------------------------- */
	
	bitboard pieceDB = b->getPieceBB(toMove,nPawn);

	while(pieceDB) {
		//quit after exhausting all of the bits in the current database.
		int from = bitscan_msb(pieceDB);
		
		//Single Advance:
		bitboard advance = mask::pawn_advances[toMove][from] & ~b->getOccupiedBB(BOTH) ;
		if(advance) {
			int to = bitscan_lsb(advance);
			move_list.push_back(MOVE(from,to,toMove, nPawn));
			
			//Double Advance:
			advance = mask::pawn_double_advances[toMove][from] & ~b->getOccupiedBB(BOTH);
			if(advance) {
				to = bitscan_lsb(advance);
				move_list.push_back(MOVE(from,to,toMove, nPawn));
			}
		}
		
		//Pawn attacks:
		bitboard kill_list = mask::pawn_captures[toMove][from] & ( b->getOccupiedBB(!toMove) ); // | mask::square[b->getEnPassantIndex()]) ;					
		while(kill_list) {
			int index = bitscan_lsb(kill_list);
			move_list.push_back(MOVE(from,index,toMove, nPawn, b->getBoard(index)) );
			kill_list ^= (1i64 << index);
		}

		pieceDB ^= (1i64 << from); //remove bit from DB
	}
	
	
	//Pawn Promotions:
	pieceDB = (b->getPieceBB(toMove,nPawn) & mask::pawns_spawn[!toMove]);
	while(pieceDB) {
		int from = bitscan_lsb(pieceDB);
		bitboard promote =	(  mask::pawn_advances[toMove][from] & ~b->getOccupiedBB(BOTH) )	//check that no piece blocks an advance.
							| (mask::pawn_captures[toMove][from] & b->getOccupiedBB(!toMove) ); //check for possible kills.
		while(promote) {
			int to = bitscan_lsb(promote);
			//Promote to both Queen and Knight:
			move_list.push_back( MOVE(from,to,toMove,nPawn,b->getBoard(to), nQueen) );
			move_list.push_back( MOVE(from,to,toMove,nPawn,b->getBoard(to), nKnight) );
			promote ^= (1i64 << to);
		}
		pieceDB ^= (1i64 << from); //remove bit from DB
	}

/*	-------------------------------------
	KNIGHT MOVES:
	------------------------------------- */
	
	pieceDB = b->getPieceBB(toMove,nKnight);
	 while(pieceDB) {
		 int from = bitscan_lsb(pieceDB);
		 bitboard validMoveDB = mask::knight_moves[from] & ~b->getOccupiedBB(toMove);
		 while(validMoveDB) {
			 int target = bitscan_lsb(validMoveDB);
			 move_list.push_back(MOVE(from,target,toMove, nKnight, b->getBoard(target))); //add something for captures here!
			 validMoveDB ^= (1i64 << target);
		 }
		 pieceDB ^= (1i64 << from); //remove bit from DB
	 }
	 
/*	-------------------------------------
	BISHOP MOVES:
	------------------------------------- */

	pieceDB = b->getPieceBB(toMove, nBishop);
	while(pieceDB) {
		int from = bitscan_lsb(pieceDB);
		//NW:
		bitboard nwMoves = mask::nw[from];
		int blockedSquare = bitscan_lsb(nwMoves & b->getOccupiedBB(BOTH));
		nwMoves ^= mask::nw[blockedSquare];
		nwMoves &= ~b->getOccupiedBB(toMove);
		while(nwMoves) {
			int target = bitscan_lsb(nwMoves);
			move_list.push_back(MOVE(from,target,toMove,nBishop, b->getBoard(target)));
			nwMoves^=(1i64 << target);
		}
		//NE:
		bitboard neMoves = mask::ne[from];
		blockedSquare = bitscan_lsb(neMoves & b->getOccupiedBB(BOTH));
		neMoves ^= mask::ne[blockedSquare];
		neMoves &= ~b->getOccupiedBB(toMove);
		while(neMoves) {
			int target = bitscan_lsb(neMoves);
			move_list.push_back(MOVE(from,target,toMove,nBishop,b->getBoard(target)));
			neMoves^=(1i64 << target);
		}
		//SE:
		bitboard seMoves = mask::se[from];
		blockedSquare = bitscan_msb(seMoves & b->getOccupiedBB(BOTH));
		seMoves ^= mask::se[blockedSquare];
		seMoves &= ~b->getOccupiedBB(toMove);
		while(seMoves) {
			int target = bitscan_lsb(seMoves);
			move_list.push_back(MOVE(from,target,toMove,nBishop,b->getBoard(target)));
			seMoves^=(1i64 << target);
		}
		//SW:
		bitboard swMoves = mask::sw[from];
		blockedSquare = bitscan_msb(swMoves & b->getOccupiedBB(BOTH));
		swMoves ^= mask::sw[blockedSquare];
		swMoves &= ~b->getOccupiedBB(toMove);
		while(swMoves) {
			int target = bitscan_lsb(swMoves);
			move_list.push_back(MOVE(from,target,toMove,nBishop,b->getBoard(target)));
			swMoves^=(1i64 << target);
		} 
		pieceDB ^= (1i64 << from); //remove bit from DB
	}

/*	-------------------------------------
	ROOK MOVES:
	------------------------------------- */

	pieceDB = b->getPieceBB(toMove, nRook);
	while(pieceDB) {
		int from = bitscan_lsb(pieceDB);
		//North:
		bitboard northMoves = mask::north[from];
		int blockedSquare = bitscan_lsb(northMoves & b->getOccupiedBB(BOTH));
		northMoves ^= mask::north[blockedSquare];
		northMoves &= ~b->getOccupiedBB(toMove);
		while(northMoves) {
			int target = bitscan_lsb(northMoves);
			move_list.push_back(MOVE(from,target,toMove,nRook,b->getBoard(target)));
			northMoves^=(1i64 << target);
		}
		//East:
		bitboard eastMoves = mask::east[from];
		blockedSquare = bitscan_msb(eastMoves & b->getOccupiedBB(BOTH));
		eastMoves ^= mask::east[blockedSquare];
		eastMoves &= ~b->getOccupiedBB(toMove);
		while(eastMoves) {
			int target = bitscan_lsb(eastMoves);
			move_list.push_back(MOVE(from,target,toMove,nRook,b->getBoard(target)));
			eastMoves^=(1i64 << target);
		}
		//South:
		bitboard southMoves = mask::south[from];
		blockedSquare = bitscan_msb(southMoves & b->getOccupiedBB(BOTH));
		southMoves ^= mask::south[blockedSquare];
		southMoves &= ~b->getOccupiedBB(toMove);
		while(southMoves) {
			int target = bitscan_lsb(southMoves);
			move_list.push_back(MOVE(from,target,toMove,nRook,b->getBoard(target)));
			southMoves^=(1i64 << target);
		}
		//West:
		bitboard westMoves = mask::west[from];
		blockedSquare = bitscan_lsb(westMoves & b->getOccupiedBB(BOTH));
		westMoves ^= mask::west[blockedSquare];
		westMoves &= ~b->getOccupiedBB(toMove);
		while(westMoves) {
			int target = bitscan_lsb(westMoves);
			move_list.push_back(MOVE(from,target,toMove,nRook,b->getBoard(target)));
			westMoves^=(1i64 << target);
		} 
		pieceDB ^= (1i64 << from); //remove bit from DB
	}

/*	-------------------------------------
	QUEEN MOVES:
	------------------------------------- */
	
	pieceDB = b->getPieceBB(toMove, nQueen);
	while(pieceDB) {
		int from = bitscan_lsb(pieceDB);
		//North:
		bitboard northMoves = mask::north[from];
		int blockedSquare = bitscan_lsb(northMoves & b->getOccupiedBB(BOTH));
		northMoves ^= mask::north[blockedSquare];
		northMoves &= ~b->getOccupiedBB(toMove);
		while(northMoves) {
			int target = bitscan_lsb(northMoves);
			move_list.push_back(MOVE(from,target,toMove,nQueen, b->getBoard(target)));
			northMoves^=(1i64 << target);
		}
		//East:
		bitboard eastMoves = mask::east[from];
		blockedSquare = bitscan_msb(eastMoves & b->getOccupiedBB(BOTH));
		eastMoves ^= mask::east[blockedSquare];
		eastMoves &= ~b->getOccupiedBB(toMove);
		while(eastMoves) {
			int target = bitscan_lsb(eastMoves);
			move_list.push_back(MOVE(from,target,toMove,nQueen, b->getBoard(target)));
			eastMoves^=(1i64 << target);
		}
		//South:
		bitboard southMoves = mask::south[from];
		blockedSquare = bitscan_msb(southMoves & b->getOccupiedBB(BOTH));
		southMoves ^= mask::south[blockedSquare];
		southMoves &= ~b->getOccupiedBB(toMove);
		while(southMoves) {
			int target = bitscan_lsb(southMoves);
			move_list.push_back(MOVE(from,target,toMove,nQueen, b->getBoard(target)));
			southMoves^=(1i64 << target);
		}
		//West:
		bitboard westMoves = mask::west[from];
		blockedSquare = bitscan_lsb(westMoves & b->getOccupiedBB(BOTH));
		westMoves ^= mask::west[blockedSquare];
		westMoves &= ~b->getOccupiedBB(toMove);
		while(westMoves) {
			int target = bitscan_lsb(westMoves);
			move_list.push_back(MOVE(from,target,toMove,nQueen, b->getBoard(target)));
			westMoves^=(1i64 << target);
		} 
		//NW:
		bitboard nwMoves = mask::nw[from];
		blockedSquare = bitscan_lsb(nwMoves & b->getOccupiedBB(BOTH));
		nwMoves ^= mask::nw[blockedSquare];
		nwMoves &= ~b->getOccupiedBB(toMove);
		while(nwMoves) {
			int target = bitscan_lsb(nwMoves);
			move_list.push_back(MOVE(from,target,toMove,nQueen, b->getBoard(target)));
			nwMoves^=(1i64 << target);
		}
		//NE:
		bitboard neMoves = mask::ne[from];
		blockedSquare = bitscan_lsb(neMoves & b->getOccupiedBB(BOTH));
		neMoves ^= mask::ne[blockedSquare];
		neMoves &= ~b->getOccupiedBB(toMove);
		while(neMoves) {
			int target = bitscan_lsb(neMoves);
			move_list.push_back(MOVE(from,target,toMove,nQueen, b->getBoard(target)));
			neMoves^=(1i64 << target);
		}
		//SE:
		bitboard seMoves = mask::se[from];
		blockedSquare = bitscan_msb(seMoves & b->getOccupiedBB(BOTH));
		seMoves ^= mask::se[blockedSquare];
		seMoves &= ~b->getOccupiedBB(toMove);
		while(seMoves) {
			int target = bitscan_lsb(seMoves);
			move_list.push_back(MOVE(from,target,toMove,nQueen, b->getBoard(target)));
			seMoves^=(1i64 << target);
		}
	
		//SW:
		bitboard swMoves = mask::sw[from];
		blockedSquare = bitscan_msb(swMoves & b->getOccupiedBB(BOTH));
		swMoves ^= mask::sw[blockedSquare];
		swMoves &= ~b->getOccupiedBB(toMove);
		while(swMoves) {
			int target = bitscan_lsb(swMoves);
			move_list.push_back(MOVE(from,target,toMove,nQueen, b->getBoard(target)));
			swMoves^=(1i64 << target);
		}
		pieceDB ^= (1i64 << from); //remove bit from DB
	}
	
/*	-------------------------------------
	KING MOVES:
	------------------------------------- */

	pieceDB = b->getPieceBB(toMove,nKing);
	int from = bitscan_lsb(pieceDB);
	bitboard validMoveDB = mask::king_moves[from] & ~b->getOccupiedBB(toMove);
	while(validMoveDB) {
		int target = bitscan_lsb(validMoveDB);
		move_list.push_back(MOVE(from,target,toMove,nKing, b->getBoard(target)));
		validMoveDB ^= (1i64 << target);
	}
	  
	if(b->getCastlingRights(toMove, QUEEN_SIDE) == true ) {
		if(((b->getOccupiedBB(BOTH) & mask::qCastle_path[toMove]) == 0) 
			&& (mask::qRook_spawn[toMove] & b->getPieceBB(toMove,nRook)) 
			&& (mask::king_spawn[toMove] & b->getPieceBB(toMove,nKing))	
		) {
					
			if( !b->isInCheck() 
				&& !b->isAttacked(bitscan_lsb(mask::queen_spawn[toMove]))   
			) {

				int from = bitscan_lsb(mask::king_spawn[toMove]);
				int target = bitscan_lsb(mask::qCastle[toMove]);
				move_list.push_back(MOVE(from,target,toMove,nKing, b->getBoard(target)));
			}
		}
	}
	if(b->getCastlingRights(toMove, KING_SIDE) == true ) {
		if(((b->getOccupiedBB(BOTH) & mask::kCastle_path[toMove])==0) 
			&& (mask::kRook_spawn[toMove] & b->getPieceBB(toMove,nRook)) 
			&& (mask::king_spawn[toMove] & b->getPieceBB(toMove,nKing))	
		) {
				
			if( !b->isInCheck()
				&& !b->isAttacked(bitscan_lsb(mask::kBishop_spawn[toMove]))  
			) {
				int from = bitscan_lsb(mask::king_spawn[toMove]);
				int target = bitscan_lsb(mask::kCastle[toMove]);
				move_list.push_back(MOVE(from,target,toMove,nKing,b->getBoard(target)));
			}
		}
	}

/*	------------------------------------- */
	 
	 return move_list;

}



//////////////////////////////////////////////////////////////////////////
// SELECTION SORT:
//////////////////////////////////////////////////////////////////////////
int find_max(vector<MOVE>& search_vector, int start_index, int end_index) {
	//returns the POSITION in the vector with the minimal value
	int max_position = start_index;
	for (int i = start_index+1; i <= end_index; i ++) {
		if( search_vector.at(i).static_value > search_vector.at(max_position).static_value )
			max_position = i;
	}
	return max_position;
}
void selection_sort(vector<MOVE>& sort_vector) {
//Last test: depth 1-12, 8.5sec, 2.05m NPS, 10453755 nodes.

	int vector_length = (int) sort_vector.size();
	for (int i = 0; i < vector_length - 1; i++) {
		int max_index = find_max(sort_vector, i, vector_length-1);
		if (max_index != i)
			swap(sort_vector.at(i), sort_vector.at(max_index));
	}
}


void insertion_sort(vector<MOVE>& v) {
	int j = 0;
	MOVE value;
	int v_length = (int) v.size();
	for (int i=1; i < v_length; i++ ) {
		value = v[i];
		j= i -1;
		while (j >=0 && v[j].static_value < value.static_value) {
			v[j+1] = v[j];
			j=j-1;
		}
		v[j+1] = value;
	}
}

void MoveSort(vector<MOVE> & move_list) {
	int j = 0;
	MOVE value;
	int v_length = (int) move_list.size();
	for (int i=1; i < v_length; i++ ) {
		value = move_list[i];
		j= i -1;
		while (j >=0 && move_list[j].static_value < value.static_value) {
			move_list[j+1] = move_list[j];
			j=j-1;
		}
		move_list[j+1] = value;
	}
}

void MoveSort(vector<MOVE> & move_list, MOVE & priority1) {
	int j = 0;
	MOVE value;
	int v_length = (int) move_list.size();
	for (int i=1; i < v_length; i++ ) {
		if(move_list[i] == priority1)
			move_list[i].static_value += 1950;
		value = move_list[i];
		j= i -1;
		while (j >=0 && move_list[j].static_value < value.static_value) {
			move_list[j+1] = move_list[j];
			j=j-1;
		}
		move_list[j+1] = value;
	}
}

void MoveSort(vector<MOVE> & move_list, MOVE & priority1, MOVE & priority2) {
	int j = 0;
	MOVE value;
	int v_length = (int) move_list.size();
	for (int i=1; i < v_length; i++ ) {
		if(move_list[i] == priority1)
			move_list[i].static_value += 10000;
		else if(move_list[i] == priority2)
			move_list[i].static_value += 9000;
	
		value = move_list[i];
		j= i -1;
		while (j >=0 && move_list[j].static_value < value.static_value) {
			move_list[j+1] = move_list[j];
			j=j-1;
		}
		move_list[j+1] = value;
	}
}