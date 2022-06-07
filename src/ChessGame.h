#pragma once

#include <SFML/Graphics.hpp>

#include "Bitboard.h"
#include "Board.h"
#include "ChessGUI.h"

// maybe abstract this into more classes after it starts working
class ChessGame
{
private:
	Board mBoard;
	Colour mSideToMove;
	ChessGUI mGUI;

	int mOriginSquare, mTargetSquare;

public:
	ChessGame();

	void runGUI();
};