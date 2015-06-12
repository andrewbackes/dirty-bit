#ifndef movegen_h
#define movegen_h

#include <vector>

#include "board.h"

bool MoveCompare (MOVE i,MOVE j); //used in sorting.

std::vector<MOVE> CaptureGen(CHESSBOARD *b);
std::vector<MOVE> nonCaptureGen(CHESSBOARD *b);

std::vector<MOVE> MoveGen(CHESSBOARD *b);

void MoveSort(std::vector<MOVE> &move_list);
void MoveSort(std::vector<MOVE> &move_list, MOVE &priority1);
void MoveSort(std::vector<MOVE> &move_list, MOVE &priority1, MOVE &priority2);

void OrderMoves(MOVE ** linked_list, short size);

#endif