#include "ChessGame.h"

#include <iostream>

ChessGame::ChessGame()
{
	mBoard.init();

	mOriginSquare = -1;
	mTargetSquare = -1;
}

// gui so that maybe eventually you can feed in moves differnetly? 
void ChessGame::runGUI()
{
	mGUI.init(1000, 800);

	while (true)
	{
		mGUI.updateBoard(&mBoard);

		UserInput userInput = mGUI.getUserInput();

		switch (userInput.inputType)
		{
			case UserInput::InputType::SquareSelect:
			{
				if (mOriginSquare >= 0)
					mTargetSquare = userInput.squareLoc;
				else
					mOriginSquare = userInput.squareLoc;

				break;
			}
		}
	}
}

// also just a rook/bishop look up table for all attacks on any square (without considering blockers: i.e., they go to the ends of the board)
// blockers dont matter if they're on the end ? at the end of file or end of rank
// we have to mask these depending on the square
// learn about perfect hashing. 