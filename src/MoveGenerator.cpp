#include "MoveGenerator.h"

#include <iostream>

void MoveGenerator::init()
{
    for (int pieceLoc = 0; pieceLoc < 64; pieceLoc++)
    {
        initKingLT(pieceLoc);
        initKnightLT(pieceLoc);
        initPawnLT(SIDE_BLACK, pieceLoc);
        initPawnLT(SIDE_WHITE, pieceLoc);
    }
}

void MoveGenerator::initKnightLT(Byte knightLoc)
{
    Bitboard knightAFileCleared = BB::boardSquares[knightLoc] & BB::fileClear[BB::FILE_A];
    Bitboard knightBFileCleared = BB::boardSquares[knightLoc] & BB::fileClear[BB::FILE_B];
    Bitboard knightGFileCleared = BB::boardSquares[knightLoc] & BB::fileClear[BB::FILE_G];
    Bitboard knightHFileCleared = BB::boardSquares[knightLoc] & BB::fileClear[BB::FILE_H];

    Bitboard moves = 0;

    moves |= (knightAFileCleared & knightBFileCleared) << 6 | (knightAFileCleared & knightBFileCleared) >> 10; // check western horizontal moves
    moves |=  knightAFileCleared << 15 | knightAFileCleared >> 17;                                               // check western vertical moves
    moves |= (knightGFileCleared & knightHFileCleared) << 10 | (knightGFileCleared & knightHFileCleared) >> 6; // check eastern horizontal moves
    moves |=  knightHFileCleared << 17 | knightHFileCleared >> 15;                                               // check eastern vertiacal moves
    mKnightLookupTable[knightLoc] = moves;
}

void MoveGenerator::initKingLT(Byte kingLoc)
{
    // for moves north west, west, and south west, we need to clear file a to prevent overflow
    Bitboard kingAFileCleared = BB::boardSquares[kingLoc] & BB::fileClear[BB::FILE_A];
    // for moves north east, east, and south east, we need to clear file h to prevent overflow
    Bitboard kingHFileCleared = BB::boardSquares[kingLoc] & BB::fileClear[BB::FILE_H];

    // consider all ordinal and cardinal directions
    mKingLookupTable[kingLoc] = kingAFileCleared << 7 | kingAFileCleared >> 1 | kingAFileCleared >> 9 | BB::boardSquares[kingLoc] << 8 |
           kingHFileCleared << 9 | kingHFileCleared << 1 | kingHFileCleared >> 7 | BB::boardSquares[kingLoc] >> 8;
}

void MoveGenerator::initPawnLT(Colour side, Byte pawnLoc)
{
    Bitboard pawnAFileCleared = BB::boardSquares[pawnLoc] & BB::fileClear[BB::FILE_A];
    Bitboard pawnHFileCleared = BB::boardSquares[pawnLoc] & BB::fileClear[BB::FILE_H];

    if (side == SIDE_WHITE)
        mPawnAttackLookupTable[SIDE_WHITE][pawnLoc] = pawnAFileCleared << 7 | pawnHFileCleared << 9;
    else if (side == SIDE_BLACK)
        mPawnAttackLookupTable[SIDE_BLACK][pawnLoc] = pawnAFileCleared >> 7 | pawnHFileCleared >> 9;
}

Bitboard MoveGenerator::computePseudoKingMoves(Byte pieceCoord, Bitboard friendlyPieces)
{
	// we already have predefined moves for the king so we need only index the correct element of the lookup table
	// then we simply ensure that we aren't allowing moves onto friendly pieces
	return mKingLookupTable[pieceCoord] & ~friendlyPieces;
}

Bitboard MoveGenerator::computePseudoKnightMoves(Byte pieceCoord, Bitboard friendlyPieces)
{
	// same process as above :D
    return mKnightLookupTable[pieceCoord] & ~friendlyPieces;
}

Bitboard MoveGenerator::computePseudoPawnMoves(Byte pieceCoord, Colour side, Bitboard enemyPieces, Bitboard occupiedSquares)
{
	// if it's white we need to mask ranks to see if it has permissions to move two squares ahead
	Bitboard moves = mPawnAttackLookupTable[side][pieceCoord] & enemyPieces;

	if (side == SIDE_WHITE)
	{
		Bitboard oneStep = (BB::boardSquares[pieceCoord] << 8) & ~occupiedSquares;
		// if it passed the above filter, it is now on rank 3 (if the first move was on rank 2: the home rank for white pieces)
		Bitboard twoStep = ((oneStep << 8) & BB::rankClear[BB::RANK_THIRD]) & ~occupiedSquares;
		moves |= oneStep | twoStep;
	}
	else if (side == SIDE_BLACK)
	{
		Bitboard oneStep = (BB::boardSquares[pieceCoord] >> 8) & ~occupiedSquares;
		// if it passed the above filter, it is now on rank 6 (if the first move was on rank 7: the home rank for black pieces)
		Bitboard twoStep = ((oneStep >> 8) & BB::rankClear[BB::RANK_SIXTH]) & ~occupiedSquares;
		moves |= oneStep | twoStep;
	}
	
	return moves;
}

// value of true = stop adding pieces
inline bool slidingPieceMoveStep(int square, Bitboard* moves, Bitboard enemyPieces, Bitboard friendlyPieces)
{
    if (BB::boardSquares[square] & friendlyPieces)
        return true;

    *moves |= BB::boardSquares[square];

    if (BB::boardSquares[square] & enemyPieces)
        return true;

    return false;
}

Bitboard MoveGenerator::computePseudoRookMoves(Byte pieceCoord, Bitboard enemyPieces, Bitboard friendlyPieces)
{
    Bitboard moves = 0;

    // see if it works first, then clean it up. also move the entire side moves into movegenerator and out of board
    // cannot move to the edges of the board
    for (int square = pieceCoord + 8; square <= 63; square += 8) // north
        if (slidingPieceMoveStep(square, &moves, enemyPieces, friendlyPieces))
            break;
    for (int square = pieceCoord - 8; square >= 0; square -= 8) // south
        if (slidingPieceMoveStep(square, &moves, enemyPieces, friendlyPieces))
            break;
    // this remainder math is so that the moves stay in the same rank. compare the numbers to https://www.chessprogramming.org/Square_Mapping_Considerations
    for (int square = pieceCoord + 1; (square + 1) % 8 != 1; square++) // east
        if (slidingPieceMoveStep(square, &moves, enemyPieces, friendlyPieces))
            break;
    for (int square = pieceCoord - 1; (square - 1) % 8 != 6; square--) // west
        if (slidingPieceMoveStep(square, &moves, enemyPieces, friendlyPieces))
            break;

    return moves;
}

Bitboard MoveGenerator::computePseudoBishopMoves(Byte pieceCoord, Bitboard enemyPieces, Bitboard friendlyPieces)
{
    Bitboard moves = 0;

    for (int square = pieceCoord + 7; square <= 63 && (square - 1) % 8 != 6; square += 7)
        if (slidingPieceMoveStep(square, &moves, enemyPieces, friendlyPieces))
            break;
    for (int square = pieceCoord + 9; square <= 63 && (square + 1) % 8 != 1; square += 9)
        if (slidingPieceMoveStep(square, &moves, enemyPieces, friendlyPieces))
            break;
    for (int square = pieceCoord - 7; square >= 0 && (square + 1) % 8 != 1; square -= 7)
        if (slidingPieceMoveStep(square, &moves, enemyPieces, friendlyPieces))
            break;
    for (int square = pieceCoord - 9; square >= 0 && (square - 1) % 8 != 6; square -= 9)
        if (slidingPieceMoveStep(square, &moves, enemyPieces, friendlyPieces))
            break;

    return moves;
}

Bitboard MoveGenerator::computePseudoQueenMoves(Byte pieceCoord, Bitboard enemyPieces, Bitboard friendlyPieces)
{
    return computePseudoBishopMoves(pieceCoord, enemyPieces, friendlyPieces) | computePseudoRookMoves(pieceCoord, enemyPieces, friendlyPieces);
}