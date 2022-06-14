#include "ChessGame.h"

#include <iostream>

ChessGame::ChessGame()
{
	mBoard.init();
	moveReset();
}

void ChessGame::moveReset()
{
	mOriginSquare = -1;
	mTargetSquare = -1;
}

std::vector<MoveData>& ChessGame::getMoves()
{
    if (mSideToMove == SIDE_WHITE)
        return mBoard.getWhiteMoves();
    else
        return mBoard.getBlackMoves();
}

void ChessGame::calculateCurrentTurnMoves()
{
	if (mSideToMove == SIDE_WHITE)
		mBoard.calculateWhiteMoves();
	else
		mBoard.calculateBlackMoves();
}

void ChessGame::getGUIInput()
{
	UserInput userInput = mGUI.getUserInput();

	switch (userInput.inputType)
	{
		// make it so that you reselect the origin square. if its the same side you just picked
		case UserInput::InputType::SquareSelect:
		{
			if (mOriginSquare >= 0)
			{
				mTargetSquare = userInput.squareLoc;
			}
			else
			{
				if ((mSideToMove == SIDE_WHITE && (mBoard.whitePiecesBB & BB::boardSquares[userInput.squareLoc])) ||
					(mSideToMove == SIDE_BLACK && (mBoard.blackPiecesBB & BB::boardSquares[userInput.squareLoc])))
				{
					mOriginSquare = userInput.squareLoc;
					mGUI.setSelectedSquare(userInput.squareLoc);
				}
			}

			if (mOriginSquare == mTargetSquare)
				mTargetSquare = -1;

			break;
		}
		case UserInput::InputType::GameClose:
		{
			exit(0);
			break;
		}
	}
}

void ChessGame::runGUI()
{
	mGUI.init(1000, 800);

	while (true)
	{
		mGUI.updateBoard(&mBoard);
		getGUIInput();

		if (mTargetSquare >= 0 && mOriginSquare >= 0)
		{
			calculateCurrentTurnMoves();
			std::vector<MoveData>& moves = getMoves();

			bool moveMade = false;
			for (int moveIndex = 0; moveIndex < moves.size(); moveIndex++)
			{
				if (moves[moveIndex].originSquare == mOriginSquare && moves[moveIndex].targetSquare == mTargetSquare)
					if (mBoard.makeMove(&moves[moveIndex]))
                    {
                        moveMade = true;
						mGUI.setMoveColours(&moves[moveIndex]);
						mSideToMove = !mSideToMove;
						break;
                    }
			}

			moveReset();
			if (!moveMade)// && mTargetSquare != )
                mGUI.unselectSelectedSquare();
		}
	}
}

// also just a rook/bishop look up table for all attacks on any square (without considering blockers: i.e., they go to the ends of the board)
// blockers dont matter if they're on the end ? at the end of file or end of rank
// we have to mask these depending on the square
// learn about perfect hashing.
