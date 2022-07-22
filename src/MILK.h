#pragma once

#include <vector>

#include "Bitboard.h"
#include "MoveData.h"
#include "Board.h"

namespace MilkConstants
{
    const int MAX_PLY = 15;

    // this is a value used to properly assign move scores
    // we use this value so that we can use smaller numbers to describe move scores given by certain captures
    // i.e. so a pawn capture would have a value of 1 + 100 = 101, whereas a killer move heuristic move would have 100 - 10 = 90
    // which would give the proper ordering to the capture move
    const Byte MVV_LVA_OFFSET = 100;

    const int MAX_KILLER_MOVES = 2;
    const Byte KILLER_MOVE_SCORE = 10;
}

// the ai class
class MILK
{
private:
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
    
    MoveData mKillerMoves[MilkConstants::MAX_PLY][2]{ {}, {} }; // maximum of 2 killer moves per ply
    void insertKillerMove(MoveData& move, Byte ply);
    
	int mDepth;
    int mKingValue, mQueenValue, mRookValue, mBishopValue, mKnightValue, mPawnValue;
    Colour mSide;
    MoveData mMoveToMake;
    bool mActive = false;

    int mNodes;
    
    int getScoreRelativeToSide(int score, Colour side) { return score * (1 - 2 * (Byte)side); }

    int negamax(Board* board, int depth, Colour side, int alpha, int beta, Byte ply);
    int quietMoveSearch(Board* board, Colour side, int alpha, int beta, Byte ply);

    void assignMoveScores(Board* board, std::vector<MoveData>& moves, Byte ply);
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
