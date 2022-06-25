#pragma once

#include "Bitboard.h"
#include "MoveData.h"
#include "Board.h"

// the ai class
class MILK
{
private:
	int mDepth;
    int mKingValue, mQueenValue, mRookValue, mBishopValue, mKnightValue, mPawnValue;
    Colour mSide;
    MoveData mMoveToMake;
    bool mActive = false;
    
    float minimax(Board* board, int depth, Colour side, int alpha, int beta);
    float evaluatePosition(Board* board);

public:
    MILK();
    
	MoveData computeMove(Board* board);

	void setDepth(int newDepth) { mDepth = newDepth; }
    void setColour(Colour side) { mSide = side;      }
    void activate()             { mActive = true;    }
    void deactiveate()          { mActive = false;   }
    
    bool isActive ()            { return mActive;    }
    Colour getColour()          { return mSide;      }
};
