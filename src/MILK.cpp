#include "MILK.h"
#include "SquarePieceTables.h"

#include <iostream>
#include <limits>
#include <algorithm>
#include <SFML/Graphics.hpp>

MILK::MILK()
{
    mKingValue   = 20000;
    mQueenValue  = 900;
    mRookValue   = 500;
    mBishopValue = 330;
    mKnightValue = 320;
    mPawnValue = 100;
    
    // by default
    mSide = SIDE_WHITE;
    mDepth = 5;
}

MoveData MILK::computeMove(Board* board)
{
    evaluatePosition(board);
    mMoveToMake.setMoveType(MoveData::EncodingBits::INVALID);
    sf::Clock clock;
    minimax(board, mDepth, mSide, -std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
    std::cout << "time to calculate move: " << clock.getElapsedTime().asSeconds() << std::endl;
    return mMoveToMake;
}

// do something to find the string of moves that gets eval = 100 at the start?
int MILK::evaluatePosition(Board* board)
{
    int whiteEval = 0;
    int blackEval = 0;
    
    for (int square = 0; square < 64; square++)
    {
        if (BB::boardSquares[square] & board->emptyBB) // optimization
            continue;

        // consider piece value and piece square table
        if (BB::boardSquares[square] & board->whitePawnsBB)
        {
            whiteEval += mPawnValue;
            whiteEval += pst::pawnTable[63 - square];
        }
        else if (BB::boardSquares[square] & board->whiteKnightsBB)
        {
            whiteEval += mKnightValue;
            whiteEval += pst::knightTable[63 - square];
        }
        else if (BB::boardSquares[square] & board->whiteBishopsBB)
        {
            whiteEval += mBishopValue;
            whiteEval += pst::bishopTable[63 - square];
        }
        else if (BB::boardSquares[square] & board->whiteRooksBB)
        {
            whiteEval += mRookValue;
            whiteEval += pst::rookTable[63 - square];
        }
        else if (BB::boardSquares[square] & board->whiteQueensBB)
        {
            whiteEval += mQueenValue;
            whiteEval += pst::queenTable[63 - square];
        }
        else if (BB::boardSquares[square] & board->whiteKingBB)
        {
            whiteEval += mKingValue;
            whiteEval += pst::kingTable[63 - square];
        }
        
        else if (BB::boardSquares[square] & board->blackPawnsBB)
        {
            blackEval += mPawnValue;
            blackEval += pst::pawnTable[square];
        }
        else if (BB::boardSquares[square] & board->blackKnightsBB)
        {
            blackEval += mKnightValue;
            blackEval += pst::knightTable[square];
        }
        else if (BB::boardSquares[square] & board->blackBishopsBB)
        {
            blackEval += mBishopValue;
            blackEval += pst::bishopTable[square];
        }
        else if (BB::boardSquares[square] & board->blackRooksBB)
        {
            blackEval += mRookValue;
            blackEval += pst::rookTable[square];
        }
        else if (BB::boardSquares[square] & board->blackQueensBB)
        {
            blackEval += mQueenValue;
            blackEval += pst::queenTable[square];
        }
        else if (BB::boardSquares[square] & board->blackKingBB)
        {
            blackEval += mKingValue;
            blackEval += pst::kingTable[square];
        }
    }
    
    return whiteEval - blackEval;
}

// should eval ever be plus or negative infinity? how would this even happen? 
int MILK::minimax(Board* board, int depth, Colour side, int alpha, int beta)
{
    if (depth == 0) // OR the game is over at this position
        return evaluatePosition(board);
    
    board->calculateSideMoves(side);
    std::vector<MoveData> moves = board->getMoves(side);
    
    if (side == mSide) // if maximizing
    {
        int maxEval = -std::numeric_limits<int>::max(); // arbitrarily large number that any other position will be better
                                
        int moveCount = 0;

        for (MoveData& move : moves)
        {
            moveCount++;
            // if makemove is legal (i.e. wouldn't result in a check)
            board->makeMove(&move);
            int eval = minimax(board, depth - 1, !side, alpha, beta);
            board->unmakeMove(&move);

            // checking to see if it's invalid just to ensure that some move is made, even if it is terrible
            if ((eval > maxEval || mMoveToMake.moveType == MoveData::EncodingBits::INVALID && depth == mDepth) && depth == mDepth)
            {
                mMoveToMake = move;
            }

            maxEval = std::max(maxEval, eval);
            
            alpha = std::max(alpha, eval);
            if (beta <= alpha)
                break;
        }
        
        return maxEval;
    }
    else // if minimizing
    {
        int minEval = std::numeric_limits<int>::max();
        
        for (MoveData& move : moves)
        {
            board->makeMove(&move);
            int eval = minimax(board, depth - 1, !side, alpha, beta);
            board->unmakeMove(&move);            
            minEval = std::min(minEval, eval);

            beta = std::min(beta, eval);
            if (beta <= alpha)
               break;
        }
        
        return minEval;
    }
}