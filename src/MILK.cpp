#include "MILK.h"

#include <iostream>
#include <limits>

MILK::MILK()
{
    mKingValue   = 20000;
    mQueenValue  = 900;
    mRookValue   = 500;
    mBishopValue = 300;
    mKnightValue = 300;
    mPawnValue   = 100;
    
    // by default
    mSide = SIDE_WHITE;
    mDepth = 3;
}

MoveData MILK::computeMove(Board* board)
{
    minimax(board, mDepth, mSide, -std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
    return mMoveToMake;
}

float MILK::evaluatePosition(Board* board)
{
    float whiteEval = 0;
    float blackEval = 0;
    
    for (int square = 0; square < 64; square++)
    {
        if      (BB::boardSquares[square] & board->whitePawnsBB)   whiteEval += mPawnValue;
        else if (BB::boardSquares[square] & board->whiteKnightsBB) whiteEval += mKnightValue;
        else if (BB::boardSquares[square] & board->whiteBishopsBB) whiteEval += mBishopValue;
        else if (BB::boardSquares[square] & board->whiteRooksBB)   whiteEval += mRookValue;
        else if (BB::boardSquares[square] & board->whiteQueensBB)  whiteEval += mQueenValue;
        else if (BB::boardSquares[square] & board->whiteKingBB)    whiteEval += mKingValue;
        
        else if (BB::boardSquares[square] & board->blackPawnsBB)   blackEval += mPawnValue;
        else if (BB::boardSquares[square] & board->blackKnightsBB) blackEval += mKnightValue;
        else if (BB::boardSquares[square] & board->blackBishopsBB) blackEval += mBishopValue;
        else if (BB::boardSquares[square] & board->blackRooksBB)   blackEval += mRookValue;
        else if (BB::boardSquares[square] & board->blackQueensBB)  blackEval += mQueenValue;
        else if (BB::boardSquares[square] & board->blackKingBB)    blackEval += mKingValue;
    }
    
    return whiteEval - blackEval;
}

float maxf(float a, float b)
{
    if (a > b)
        return a;
    return b;
}

float minf(float a, float b)
{
    if (a < b)
        return a;
    return b;
}

// alpha beta pruning DOES NOT WORK! MILK is making different moves so we need to fix this
// the ai is making the first possible move in its list each time ?

float MILK::minimax(Board* board, int depth, Colour side, int alpha, int beta)
{
    if (depth == 0) // OR the game is over at this position
        return evaluatePosition(board);
    
    board->calculateSideMoves(side);
    std::vector<MoveData> moves = board->getMoves(side);
    
    if (side == mSide) // if maximizing
    {
        float maxEval = -std::numeric_limits<int>::max(); // arbitrarily large number that any other position will be better
                        
        // it is making just one move for black each time it computes a move. when it does this the move privileges get fucked?
        
        for (MoveData& move : moves)
        {
            // if makemove is legal (i.e. wouldn't result in a check)
            board->makeMove(&move);
            float eval = minimax(board, depth - 1, !side, alpha, beta);
            board->unmakeMove(&move); // privileges are not being returned upon unmaking the move !
            maxEval = maxf(maxEval, eval);
            
            if (maxEval == eval && depth == mDepth)
                mMoveToMake = move;
            
            alpha = maxf(alpha, eval);
            if (alpha >= beta)
                break;
            
        }
        
        return maxEval;
    }
    else // if minimizing
    {
        float minEval = std::numeric_limits<int>::max();
        
        for (MoveData& move : moves)
        {
            board->makeMove(&move);
            float eval = minimax(board, depth - 1, !side, alpha, beta);
            board->unmakeMove(&move);            
            minEval = minf(minEval, eval);
            
            beta = minf(beta, eval);
            if (alpha >= beta)
                break;
        }
        
        return minEval;
    }
}

// I CANT FIGURE THIS SHIT OUT ANYWAAY LOLOLOL WHY IT NO WORK SOOO FUNNNY FUCK U C++
