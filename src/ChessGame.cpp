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
			// make it so that you reselect the origin square. if its the same side you just picked
			case UserInput::InputType::SquareSelect:
			{
				if (mOriginSquare >= 0)
				{
					if (BB::boardSquares[userInput.squareLoc] )
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

		if (mTargetSquare >= 0 && mOriginSquare >= 0)
		{
			mBoard.calculateWhiteMoves();
			std::vector<MoveData>& moves = mBoard.getWhiteMoves();

			for (int moveIndex = 0; moveIndex < moves.size(); moveIndex++)
			{
				if (moves[moveIndex].originSquare == mOriginSquare && moves[moveIndex].targetSquare == mTargetSquare)
					if (mBoard.makeMove(&moves[moveIndex]))
						mGUI.setMoveColours(&moves[moveIndex]);
			}
            
			moveReset();
		}
	}
}

// also just a rook/bishop look up table for all attacks on any square (without considering blockers: i.e., they go to the ends of the board)
// blockers dont matter if they're on the end ? at the end of file or end of rank
// we have to mask these depending on the square
// learn about perfect hashing. 
