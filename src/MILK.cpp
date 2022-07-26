#include "MILK.h"
#include "SquarePieceTables.h"
#include "Outcomes.h"

#include <iostream>
#include <limits>
#include <algorithm>
#include <SFML/Graphics.hpp>

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
    mSide = SIDE_BLACK;
    mDepth = 6;
    mTranspositionTable = new TranspositionHashEntry[mTranspositionTableSize];
    clearTranspositionTable();
}

void MILK::clearTranspositionTable()
{
    for (int i = 0; i < mTranspositionTableSize; i++)
        mTranspositionTable[i].hashFlag = TranspositionHashEntry::NONEXISTENT;
}

MoveData MILK::computeMove(Board* board)
{
    mNodes = 0;
    mMoveToMake.setMoveType(MoveData::EncodingBits::INVALID);
    sf::Clock clock;
    //std::cout << "to be eval: " << negamax(board, mDepth, mSide, -std::numeric_limits<int>::max(), std::numeric_limits<int>::max(), 0) << std::endl;
    std::cout << "privileges: " << (int)board->currentPosition.castlePrivileges << std::endl;
    // okay so it is removing all ability to castle from just a single move. awesome bro lit a ffffff
    negamax(board, mDepth, mSide, -std::numeric_limits<int>::max(), std::numeric_limits<int>::max(), 0);
    //std::cout << "time to calculate move: " << clock.getElapsedTime().asSeconds() << std::endl;
    return mMoveToMake;
}

int MILK::evaluatePosition(Board* board)
{
    int whiteEval = 0;
    int blackEval = 0;
    
    for (int square = 0; square < 64; square++)
    {
        if (BB::boardSquares[square] & board->currentPosition.emptyBB) // optimization
            continue;

        // for white pawns
        {
            // this is checking blocked pawns
            // if (northOne (i.e. << 8) & whitePawns), then SUBTRACT a value as a penalty

        }

        // consider piece value and piece square table
        if      (BB::boardSquares[square] & board->currentPosition.whitePawnsBB)   whiteEval += mPawnValue   + pst::pawnTable[63 - square];
        else if (BB::boardSquares[square] & board->currentPosition.whiteKnightsBB) whiteEval += mKnightValue + pst::knightTable[63 - square];
        else if (BB::boardSquares[square] & board->currentPosition.whiteBishopsBB) whiteEval += mBishopValue + pst::bishopTable[63 - square];
        else if (BB::boardSquares[square] & board->currentPosition.whiteRooksBB)   whiteEval += mRookValue   + pst::rookTable[63 - square];
        else if (BB::boardSquares[square] & board->currentPosition.whiteQueensBB)  whiteEval += mQueenValue  + pst::queenTable[63 - square];
        else if (BB::boardSquares[square] & board->currentPosition.whiteKingBB)    whiteEval += mKingValue   + pst::kingTable[63 - square]; // here check if pawns are near the king (shield)
         
        else if (BB::boardSquares[square] & board->currentPosition.blackPawnsBB)   blackEval += mPawnValue   + pst::pawnTable[square];
        else if (BB::boardSquares[square] & board->currentPosition.blackKnightsBB) blackEval += mKnightValue + pst::knightTable[square];
        else if (BB::boardSquares[square] & board->currentPosition.blackBishopsBB) blackEval += mBishopValue + pst::bishopTable[square];
        else if (BB::boardSquares[square] & board->currentPosition.blackRooksBB)   blackEval += mRookValue   + pst::rookTable[square];
        else if (BB::boardSquares[square] & board->currentPosition.blackQueensBB)  blackEval += mQueenValue  + pst::queenTable[square];
        else if (BB::boardSquares[square] & board->currentPosition.blackKingBB)    blackEval += mKingValue   + pst::kingTable[square];
    }
    
    return whiteEval - blackEval;
}

// this function is used to sort the moves. we need it to be called for every iteration of the move loop
// because we only want to swap the moves that are important to us (and not sort the entire move vector,
// as that would result in us sorting lots of moves we would never even look at, wasiting lots of time)
void MILK::selectMove(std::vector<MoveData>& moves, Byte startIndex)
{
    for (int i = startIndex + 1; i < moves.size(); i++)
        if (moves[i].moveScore > moves[startIndex].moveScore)
            std::swap(moves[i], moves[startIndex]);
}

void MILK::assignMoveScores(Board* board, std::vector<MoveData>& moves, Byte ply)
{
    for (int i = 0; i < moves.size(); i++)
    {
        if (moves[i].capturedPieceBB) // move is violent
            moves[i].moveScore += MilkConstants::MVV_LVA_OFFSET + MVV_LVATable[getMVV_LVAPieceType(board, moves[i].capturedPieceBB)]
                                                                              [getMVV_LVAPieceType(board, moves[i].pieceBB)];
        else // move is quiet
            for (int j = 0; j < MilkConstants::MAX_KILLER_MOVES; j++)
                if (moves[i] == mKillerMoves[ply][j])
                {
                    moves[i].moveScore += MilkConstants::MVV_LVA_OFFSET - MilkConstants::KILLER_MOVE_SCORE;
                    break;
                }
    }
}

void MILK::insertKillerMove(MoveData& move, Byte ply)
{
    if (move == mKillerMoves[ply][0])
        return;
    else
    {
        for (int i = 1; i < MilkConstants::MAX_KILLER_MOVES; i++)
            mKillerMoves[ply][i] = mKillerMoves[ply][i - 1];

        mKillerMoves[ply][0] = move;
    }
}

MILK::MVV_LVAPieceTypes MILK::getMVV_LVAPieceType(Board* board, Bitboard* bb)
{
    if      (bb == &board->currentPosition.whitePawnsBB   || bb == &board->currentPosition.blackPawnsBB)   return MVV_LVAPieceTypes::PAWN;
    else if (bb == &board->currentPosition.whiteKnightsBB || bb == &board->currentPosition.blackKnightsBB) return MVV_LVAPieceTypes::KNIGHT;
    else if (bb == &board->currentPosition.whiteBishopsBB || bb == &board->currentPosition.blackBishopsBB) return MVV_LVAPieceTypes::BISHOP;
    else if (bb == &board->currentPosition.whiteRooksBB   || bb == &board->currentPosition.blackRooksBB)   return MVV_LVAPieceTypes::ROOK;
    else if (bb == &board->currentPosition.whiteQueensBB  || bb == &board->currentPosition.blackQueensBB)  return MVV_LVAPieceTypes::QUEEN;
    else return MVV_LVAPieceTypes::KING;
}

int MILK::calculateExtension(Board* board, Colour side)
{
    // single response


    // check extensions
    Bitboard kingBB = side == SIDE_WHITE ? board->currentPosition.whiteKingBB : board->currentPosition.blackKingBB;
    Byte kingSquare = board->computedKingSquare(kingBB);
    if (board->squareAttacked(kingSquare, !side))
        return 1;

    return 0;
}

// understand this before
// what is a beta cuttoff? why are we comparing standpat and alpha?
// then put in delta pruning

int MILK::quietMoveSearch(Board* board, Colour side, int alpha, int beta, Byte ply)
{
    mNodes++;
    // the lower bound for the best possible move for the moving side. if no capture move would result in a better position for the playing side,
    // then we just would simply not make the capture move (and return the calculated best move evaluation, aka alpha)
    int standPat = getScoreRelativeToSide(evaluatePosition(board), side);

    // understand this
    if (standPat >= beta)
        return beta;

    // delta pruning


    if (standPat > alpha)
        alpha = standPat;

    if (alpha >= beta)
        return beta;

    if (ply >= MilkConstants::MAX_PLY)
        return alpha;

    board->calculateSideMovesCapturesOnly(side);
    std::vector<MoveData> moves = board->getMoves(side);
    assignMoveScores(board, moves, ply);

    for (int i = 0; i < moves.size(); i++)
    {
        selectMove(moves, i);
       // if (moves[i].moveScore < 10)
         //   break;

        if (board->makeMove(&moves[i]))
        {
            int eval = -quietMoveSearch(board, !side, -beta, -alpha, ply + 1);
            board->unmakeMove(&moves[i]);

            if (eval >= beta)
                return beta;
            if (eval > alpha)
                alpha = eval;
        }
    }

    return alpha;
}

void MILK::insertTranspositionEntry(TranspositionHashEntry* hashEntry, int maxEval, int ogAlpha, int beta)
{
    // replacement scheme. let's just replace whenever the depth is greater how about?
    // if we ever get to this point in the search, however, we will replace
    // also, if we were going to search deeper than to whatever depth the transposition went to,
    // we need to disregard it (and just use the best move first)

    // at first, assume that the evaluation recorded in the entry is exact (i.e. derived directly from MILK::evaluatePosition)
    hashEntry->hashFlag = TranspositionHashEntry::EXACT;
     
    // if the maximum evaluation for this transposition was NOT better than move found in a sibling node
    if (maxEval < ogAlpha) hashEntry->hashFlag = TranspositionHashEntry::UPPER_BOUND;
    // if the maximum evaluation for this transposition was too 
    if (maxEval >= beta)   hashEntry->hashFlag = TranspositionHashEntry::LOWER_BOUND;

    mTranspositionTable[hashEntry->zobristKey % mTranspositionTableSize] = *hashEntry;
}

int MILK::negamax(Board* board, int depth, Colour side, int alpha, int beta, Byte ply)
{
    if (depth == 0)
        return quietMoveSearch(board, side, alpha, beta, ply);

    if (Outcomes::isThreefoldRepetition(board->getZobristKeyHistory(), board->getCurrentPly()))
        return 0;

    // used for determining the transposition table entry's flag for this call to negamax
    int ogAlpha = alpha; 

    board->calculateSideMoves(side);
    std::vector<MoveData> moves = board->getMoves(side);

    int maxEval = -std::numeric_limits<int>::max();
    assignMoveScores(board, moves, ply);

    for (int i = 0; i < moves.size(); i++)
    {
        selectMove(moves, i); // swaps current move with the most likely good move in the move list

        // if makemove is legal (i.e. wouldn't result in a check)
        if (board->makeMove(&moves[i]))
        {
            if (moves[i].castlePrivilegesRevoked > 0)
            {
                int here = 5;
            }

            if (moves[i].moveType == MoveData::EncodingBits::PAWN_PROMOTION)
                board->promotePiece(&moves[i], MoveData::EncodingBits::QUEEN_PROMO);

            int eval = -negamax(board, depth - 1 + calculateExtension(board, side), !side, -beta, -alpha , ply + 1);
            board->unmakeMove(&moves[i]);

            // checking to see if it's invalid just to ensure that some move is made, even if it is terrible
            if ((eval > maxEval || mMoveToMake.moveType == MoveData::EncodingBits::INVALID) && ply == 0)
                mMoveToMake = moves[i];

            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha)
            {
                insertKillerMove(moves[i], ply);
                break;
            }
        }
    }

    

    return alpha;
}