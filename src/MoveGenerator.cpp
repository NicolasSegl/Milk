#include "MoveGenerator.h"

Bitboard MoveGenerator::computePseudoKingMoves(Byte pieceCoord, Bitboard friendlyPieces)
{
	// we already have predefined moves for the king so we need only index the correct element of the lookup table
	// then we simply ensure that we aren't allowing moves onto friendly pieces
	return BB::LookupTables::kingLookupTable[pieceCoord] & ~friendlyPieces;
}

Bitboard MoveGenerator::computePseudoKnightMoves(Byte pieceCoord, Bitboard friendlyPieces)
{
	// same process as above :D
	return BB::LookupTables::knightLookupTable[pieceCoord] & ~friendlyPieces;
}

Bitboard MoveGenerator::computePseudoPawnMoves(Byte pieceCoord, Colour side, Bitboard enemyPieces, Bitboard occupiedSquares)
{
	// if it's white we need to mask ranks to see if it has permissions to move two squares ahead
	Bitboard moves = (BB::LookupTables::pawnAttackLookupTable[side][pieceCoord] & enemyPieces);
	if (side == SIDE_WHITE)
	{
		Bitboard oneStep = (BB::LookupTables::boardSquares[pieceCoord] << 8) & ~occupiedSquares;
		// if it passed the above filter, it is now on rank 3 (if the first move was on rank 2: the home rank for white pieces)
		Bitboard twoStep = ((oneStep << 8) & BB::LookupTables::rankClear[BB::RANK_THIRD]) & ~occupiedSquares;
		moves |= oneStep | twoStep;
	}
	else if (side == SIDE_BLACK)
	{
		Bitboard oneStep = (BB::LookupTables::boardSquares[pieceCoord] >> 8) & ~occupiedSquares;
		// if it passed the above filter, it is now on rank 6 (if the first move was on rank 7: the home rank for black pieces)
		Bitboard twoStep = ((oneStep >> 8) & BB::LookupTables::rankClear[BB::RANK_SIXTH]) & ~occupiedSquares;
		moves |= oneStep | twoStep;
	}
	
	return moves;
}