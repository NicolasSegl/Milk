#pragma once

#include <vector>

#include "Bitboard.h"
#include "MoveData.h"
#include "Board.h"

const int MILK_MAX_DEPTH = 15;

// the ai class
class MILK
{
private:
    int mNodes;
    int mQuietNodes;

    enum MVV_LVAPieceTypes
    {
        KING,
        QUEEN,
        ROOK,
        BISHOP,
        KNIGHT,
        PAWN,
        NONE,
    };
    
    Byte MVV_LVATable[7][6] =
    {
        { 99, 99, 99, 99, 99, 99}, // Victim: K, Attacker: P, N, B, R, Q, K
        { 20, 19, 19, 18, 17, 16}, // Victim: Q, Attacker: P, N, B, R, Q, K
        { 15, 14, 14, 13, 12, 11}, // Victim: R, Attacker: P, N, B, R, Q, K
        { 10, 9, 9, 8, 7, 6},      // Victim: B, Attacker: P, N, B, R, Q, K
        { 10, 9, 9, 8, 7, 6},      // Victim: N, Attacker: P, N, B, R, Q, K
        { 5, 4, 4, 3, 2, 1},       // Victim: P, Attacker: P, N, B, R, Q, K
        { 0, 0, 0, 0, 0, 0},       // Victim: None, Attacker: P, N, B, R, Q, K
    };
    
    MVV_LVAPieceTypes getMVV_LVAPieceType(Board* board, Bitboard* bb);
    
    MoveData* mKillerMoves[MILK_MAX_DEPTH][2]; // maximum of 2 killer moves per ply
    
	int mDepth;
    int mKingValue, mQueenValue, mRookValue, mBishopValue, mKnightValue, mPawnValue;
    Colour mSide;
    MoveData mMoveToMake;
    bool mActive = false;
    
    int getScoreRelativeToSide(int score, Colour side) { return score * (1 - 2 * (Byte)side); }

    int minimax(Board* board, int depth, Colour side, int alpha, int beta, Byte ply);
    int negamax(Board* board, int depth, Colour side, int alpha, int beta, Byte ply);
    int quietMoveSearch(Board* board, Colour side, int alpha, int beta, Byte ply);

    void assignMoveScores(Board* board, std::vector<MoveData>& moves);
    void selectMove(std::vector<MoveData>& moves, Byte startIndex);

    int evaluatePosition(Board* board);
    int calculateExtension(Board* board, Colour side);

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
