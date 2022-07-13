#include "MILK.h"
#include "SquarePieceTables.h"

#include <iostream>
#include <limits>
#include <algorithm>
#include <SFML/Graphics.hpp>

const Byte capture = 1;


// still issues with castling making more kings
MILK::MILK()
{
    mKingValue   = 20000;
    mQueenValue  = 900;
    mRookValue   = 500;
    mBishopValue = 330;
    mKnightValue = 320;
    mPawnValue   = 100;
    
    // by default
    mSide = SIDE_WHITE;
    mDepth = 5;
}

MoveData MILK::computeMove(Board* board)
{
    mQuietNodes = 0;
    mNodes = 0;

    mMoveToMake.setMoveType(MoveData::EncodingBits::INVALID);
    sf::Clock clock;
    //minimax(board, mDepth, mSide, -std::numeric_limits<int>::max(), std::numeric_limits<int>::max(), 0);
    negamax(board, mDepth, mSide, -std::numeric_limits<int>::max(), std::numeric_limits<int>::max(), 0);
    std::cout << "time to calculate move: " << clock.getElapsedTime().asSeconds() << std::endl;
    std::cout << "number of regular nodes: " << mNodes << std::endl;
    std::cout << "number of quiet nodes: " << mQuietNodes << std::endl;
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

void MILK::selectMove(std::vector<MoveData>& moves, Byte startIndex)
{
    for (int i = startIndex + 1; i < moves.size(); i++)
        if (moves[i].moveScore > moves[startIndex].moveScore)
            std::swap(moves[i], moves[startIndex]);
}

void MILK::assignMoveScores(Board* board, std::vector<MoveData>& moves)
{
    for (int i = 0; i < moves.size(); i++)
    {
        if (moves[i].capturedPieceBB)
            moves[i].moveScore += MVV_LVATable[getMVV_LVAPieceType(board, moves[i].capturedPieceBB)]
                                              [getMVV_LVAPieceType(board, moves[i].pieceBB)];
    }
}

MILK::MVV_LVAPieceTypes MILK::getMVV_LVAPieceType(Board* board, Bitboard* bb)
{
    if      (bb == &board->whitePawnsBB   || bb == &board->blackPawnsBB)   return MVV_LVAPieceTypes::PAWN;
    else if (bb == &board->whiteKnightsBB || bb == &board->blackKnightsBB) return MVV_LVAPieceTypes::KNIGHT;
    else if (bb == &board->whiteBishopsBB || bb == &board->blackBishopsBB) return MVV_LVAPieceTypes::BISHOP;
    else if (bb == &board->whiteRooksBB   || bb == &board->blackRooksBB)   return MVV_LVAPieceTypes::ROOK;
    else if (bb == &board->whiteQueensBB  || bb == &board->blackQueensBB)  return MVV_LVAPieceTypes::QUEEN;
    else return MVV_LVAPieceTypes::KING;
}

int MILK::calculateExtension(Board* board, Colour side)
{
    // single response


    // check extensions
    Bitboard kingBB = side == SIDE_WHITE ? board->whiteKingBB : board->blackKingBB;
    Byte kingSquare = board->computedKingSquare(kingBB);
    if (board->squareAttacked(kingSquare, !side))
        return 1;

    return 0;
}

// understand this before
// still makes complete dog moves sometimes. maybe it forgets what side its on?
// something is just barely off, where it sacrifices the wrong piece or some shit. like it thinks its on the wrong side

int MILK::quietMoveSearch(Board* board, Colour side, int alpha, int beta)
{
    int standPat = evaluatePosition(board); // * -1 * mSide ? so it can work if ai is playing either black or white

    if (standPat >= beta)
        return beta;
    if (standPat > alpha)
        alpha = standPat;

    board->calculateSideMovesCapturesOnly(side);
    std::vector<MoveData> moves = board->getMoves(side);

    for (int i = 0; i < moves.size(); i++)
    {
        if (board->makeMove(&moves[i]))
        {
            int eval = -quietMoveSearch(board, !side, -beta, -alpha);
            board->unmakeMove(&moves[i]);

            if (eval >= beta)
                return beta;
            if (eval > alpha)
                alpha = eval;
        }
    }

    return alpha;
}

// the quiet search must be returning god awful values, as it is making crazy stupid sacrifices
// but why then does it only work at mDetph = 1?

// this is because when you search only captures to get to your evaluations, you end up with extreme results
// you never get a result that is just okay IF you don't make the capture or IF the enemy doesn't make the capture
// we aren't considering if they DON'T make the capture

// min (or black) is currently finding incredibly small values because it is only picking trades it will definitely win
// thus it is returning really small values back to max, so that it only has bad moves to play
// min is always finding these t

// convert this to negamax? would back the quiescence search muchhhhh easier
int MILK::minimax(Board* board, int depth, Colour side, int alpha, int beta, Byte ply)
{
    if (depth == 0) // at the end of regular search
    {
        return quietMoveSearch(board, side, alpha, beta);
    }

    // quiet nodes is adding like an infinite number of moves. 250000 to 14000

    mNodes++;
    board->calculateSideMoves(side);
    std::vector<MoveData> moves = board->getMoves(side);

    if (side == mSide) // if maximizing
    {
        int maxEval = -std::numeric_limits<int>::max();
        assignMoveScores(board, moves);
        
        for (int i = 0; i < moves.size(); i++)
        {
            selectMove(moves, i); // swaps current move with the most likely good move in the move list
            
            // if makemove is legal (i.e. wouldn't result in a check)
            if (board->makeMove(&moves[i]))
            {
                int eval = minimax(board, depth - 1 + calculateExtension(board, side), !side, alpha, beta, ply + 1);
                board->unmakeMove(&moves[i]);

                // checking to see if it's invalid just to ensure that some move is made, even if it is terrible
                if ((eval > maxEval || mMoveToMake.moveType == MoveData::EncodingBits::INVALID) && depth == mDepth)
                {
                   // std::cout << "max eval: " << eval << std::endl;
                    mMoveToMake = moves[i];
                }

                maxEval = std::max(maxEval, eval);
                alpha = std::max(alpha, eval);
                if (beta <= alpha)
                    break;
            }
        }
        
        return maxEval;
    }
    else // if minimizing
    {
        int minEval = std::numeric_limits<int>::max();
        assignMoveScores(board, moves);
        
        for (int i = 0; i < moves.size(); i++)
        {
            selectMove(moves, i); // swaps current move with the most likely good move in the move list
            
            if (board->makeMove(&moves[i]))
            {
                int eval = minimax(board, depth - 1 + calculateExtension(board, side), !side, alpha, beta, ply + 1);
                board->unmakeMove(&moves[i]);
                minEval = std::min(minEval, eval);

                beta = std::min(beta, eval);
                if (beta <= alpha)
                    break;
            }
        }
        
        return minEval;
    }
}

int MILK::negamax(Board* board, int depth, Colour side, int alpha, int beta, Byte ply)
{
    if (depth == 0)
        return evaluatePosition(board);

    mNodes++;
    board->calculateSideMoves(side);
    std::vector<MoveData> moves = board->getMoves(side);

    // multiply score by side ?

    int maxEval = -std::numeric_limits<int>::max();
    assignMoveScores(board, moves);

    for (int i = 0; i < moves.size(); i++)
    {
        selectMove(moves, i); // swaps current move with the most likely good move in the move list

        // if makemove is legal (i.e. wouldn't result in a check)
        if (board->makeMove(&moves[i]))
        {
            int eval = -negamax(board, depth - 1 + calculateExtension(board, side), !side, -beta, -alpha , ply + 1);
            board->unmakeMove(&moves[i]);

            // checking to see if it's invalid just to ensure that some move is made, even if it is terrible
            if ((eval > maxEval || mMoveToMake.moveType == MoveData::EncodingBits::INVALID) && depth == mDepth)
            {
                // std::cout << "max eval: " << eval << std::endl;
                mMoveToMake = moves[i];
            }

            maxEval = std::max(maxEval, eval);
            if (beta <= alpha)
                break;

            //if (eval >= beta)
              //  return beta;
            alpha = std::max(alpha, eval);

        }
    }

    return alpha;
}