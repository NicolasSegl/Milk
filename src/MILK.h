#pragma once

#include "Bitboard.h"
#include "MoveData.h"
#include "Board.h"

// the ai class
class MILK
{
private:
	int mDepth;

public:
	MoveData computeMove(Board* board);

	void setDepth(int newDepth) { mDepth = newDepth; }
};