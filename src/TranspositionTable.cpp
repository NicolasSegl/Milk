#include <random>

#include "TranspositionTable.h"

TranspositionTable::TranspositionTable()
{
	initHashKeys();
}

void TranspositionTable::initHashKeys()
{
	for (int pieceType = 0; pieceType < 12; pieceType++)
		for (int square = 0; square < 64; square++)
			mPieceHashKeys[pieceType][64] = getRandom64();

	// for the sake of the tranposition table, should we store the square of the enPassant instead of a bitboard? i think yes? 
	for (int castle = 0; castle < 16; castle++)
		mCastleHashKeys[castle] = getRandom64();

	for (int square = 0; square < 64; square++)
		mEnpassantHashKeys[square] = getRandom64();

	mSideToPlayHashKey = getRandom64();
}

uint64_t TranspositionTable::getRandom64()
{
	return (rand()) | (rand() << 16) | ((uint64_t)rand() << 32) | ((uint64_t)rand() << 48);
}