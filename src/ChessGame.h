#pragma once

#include "Bitboard.h"
#include "Board.h"

class ChessGame
{
private:
	Board mBoard;
	Colour mSideToMove;

public:
	ChessGame() {}

	void runCLI();
};