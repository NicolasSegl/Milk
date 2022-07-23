#pragma once

#include "Bitboard.h"

// subject to a name change
class TranspositionTable
{
private:
	uint64_t getRandom64();

	uint64_t mPieceHashKeys[12][64];
	uint64_t mSideToPlayHashKey;
	uint64_t mEnpassantHashKeys[64];
	uint64_t mCastleHashKeys[16]; // 2^4. 1 bit set for each castle privilege
	
public:
	TranspositionTable();

	void initHashKeys();
	uint64_t generateZobristKey();
};