#pragma once

#include <cinttypes>

// position data structure
typedef uint64_t Bitboard;
typedef uint8_t  Byte;
typedef uint16_t DoubleByte;
typedef bool Colour;

constexpr int SIDE_WHITE = 0;
constexpr int SIDE_BLACK = 1;

struct MoveData
{
	// colour bitboards are unnecessary because we have the colour of the side that just moved? 

	// these must be pointers for the sake of incremental updating
	Bitboard* pieceBB;
	//Bitboard* colourBB;

	// if these are not null pointers then a capture has taken place
	Bitboard* capturedPieceBB  = nullptr;
//	Bitboard* capturedColourBB = nullptr;

	Colour side;

	// only a byte long because they are only 0-63 
	Byte  originSquare;
	Byte  targetSquare;

	// castling rights, en passant, half-move counter... etc

	//DoubleByte data;
};

namespace BB
{
	enum File
	{
		FILE_A,
		FILE_B,
		FILE_C,
		FILE_D,
		FILE_E,
		FILE_F,
		FILE_G,
		FILE_H,
	};

	enum Rank
	{
		RANK_FIRST,
		RANK_SECOND,
		RANK_THIRD,
		RANK_FOURTH,
		RANK_FIFTH,
		RANK_SIXTH,
		RANK_SEVENTH,
		RANK_EIGHTH,
	};

	namespace Constants
	{
		/* predefined bitboards for piece starting positions (little endian file-rank mapping) */

		// white pieces
		const Bitboard cWPawnsStartBB = 65280;
		const Bitboard cWRooksStartBB = 129;
		const Bitboard cWKnightsStartBB = 66;
		const Bitboard cWBishopsStartBB = 36;
		const Bitboard cWQueensStartBB = 8;
		const Bitboard cWKingStartBB = 16;

		// black pieces
		const Bitboard cBPawnsStartBB = 71776119061217280;
		const Bitboard cBRooksStartBB = 9295429630892703744;
		const Bitboard cBKnightsStartBB = 4755801206503243776;
		const Bitboard cBBishopsStartBB = 2594073385365405696;
		const Bitboard cBQueensStartBB = 1152921504606846976;
		const Bitboard cBKingStartBB = 576460752303423488;
	}

	namespace LookupTables
	{
		// predefined king and knight moves (they are non-sliders. their legal moves are independent of other pieces)
		extern Bitboard knightLookupTable[64];
		extern Bitboard kingLookupTable[64];
		extern Bitboard pawnAttackLookupTable[2][64]; // 2 because pawns have different moves depending on side

		extern Bitboard boardSquares[64];

		extern Bitboard fileClear[8];
		extern Bitboard rankClear[8];

		void initialize();
	}
}