#pragma once

#include <vector>
#include <SFML/Graphics.hpp>

#include "Bitboard.h"
#include "Board.h"
#include "ChessGUI.h"
#include "MoveData.h"
#include "MILK.h"

// maybe abstract this into more classes after it starts working
class ChessGame
{
private:
	Board mBoard;
	Colour mSideToMove;
	ChessGUI mGUI;
    MILK mMilk;

	int mOriginSquare, mTargetSquare;
	std::vector<MoveData*> moveHistory;

	void moveReset();

	std::vector<MoveData>& getMoves();

	void getGUIInput();
    bool makeMove(MoveData* moveData);

public:
	ChessGame();

	void runGUI();
};
