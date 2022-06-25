#include "ChessGame.h"

#include <iostream>

ChessGame::ChessGame()
{
	mBoard.init();
	moveReset();
    mMilk.activate();
    mSideToMove = SIDE_WHITE;
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
		case UserInput::InputType::UndoMove:
		{
			if (moveHistory.size() > 0)
				if (moveHistory.back())
				{
					mSideToMove = !mSideToMove;
					mBoard.unmakeMove(moveHistory.back());
					if (moveHistory.size() > 1)
						mGUI.setMoveColours(moveHistory[moveHistory.size() - 2]);
					else // if the first move is taken back, set all tiles to nothing!
						mGUI.resetAllColours();

					moveHistory.pop_back();
				}
			break;
		}
	}
}

// no castle moves are being generated ... 
void ChessGame::makeMove(MoveData* moveData)
{
    if (mBoard.makeMove(moveData))
    {
        mGUI.setMoveColours(moveData);
        mSideToMove = !mSideToMove;

        if (moveData->pieceBB == &mBoard.whitePawnsBB || moveData->pieceBB == &mBoard.blackPawnsBB)
            if (moveData->targetSquare >= 56 || moveData->targetSquare <= 7)
                mBoard.promotePiece(moveData, MoveData::EncodingBits::QUEEN_PROMO);

        moveHistory.push_back(new MoveData);
        *moveHistory.back() = *moveData;
    }
}

void ChessGame::runGUI()
{
	mGUI.init(1000, 800);
    
	while (true)
	{
		mGUI.updateBoard(&mBoard);
        
        if (mMilk.isActive() && mMilk.getColour() == mSideToMove)
        {
            MoveData MILKMove = mMilk.computeMove(&mBoard); // this also makes the move. change that ?
            makeMove(&MILKMove);
        }
        else
            getGUIInput();

		if (mTargetSquare >= 0 && mOriginSquare >= 0)
		{
			mBoard.calculateSideMoves(mSideToMove);
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

						if (moves[moveIndex].pieceBB == &mBoard.whitePawnsBB || moves[moveIndex].pieceBB == &mBoard.blackPawnsBB)
							if (moves[moveIndex].targetSquare >= 56 || moves[moveIndex].targetSquare <= 7)
								mBoard.promotePiece(&moves[moveIndex], MoveData::EncodingBits::QUEEN_PROMO);

						moveHistory.push_back(new MoveData);
						*moveHistory.back() = moves[moveIndex];

                        break;
                    }
			}

			moveReset();
			if (!moveMade)
                mGUI.unselectSelectedSquare();
		}
	}
}
