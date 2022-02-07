#include "Bitboard.h"

namespace BB
{
	namespace LookupTables
	{
		extern Bitboard knightLookupTable[64] { 0 };
		extern Bitboard kingLookupTable[64]   { 0 };
		extern Bitboard boardSquares[64]	  { 0 };
		extern Bitboard pawnAttackLookupTable[2][64]{ {0}, {0} };
		extern Bitboard fileClear[8]		  { 0 };
		extern Bitboard rankClear[8]		  { 0 };

		void initializeFileRankMasks()
		{
			for (int i = 0; i < 8; i++)
			{
				// intentional overflow such that fileClear[i]'s and rankClear[i]'s bits are all set to 1
				fileClear[i] = (Bitboard)(-1); 
				rankClear[i] = (Bitboard)(-1);

				for (int j = 0; j < 8; j++)
				{
					fileClear[i] ^= (Bitboard)1 << j * 8 + i;
					rankClear[i] ^= (Bitboard)1 << i * 8 + j;
				}
			}
		}

		Bitboard computeKingPseudoMoves(Bitboard kingLocBB)
		{
			// for moves north west, west, and south west, we need to clear file a to prevent overflow
			Bitboard kingAFileCleared = kingLocBB & fileClear[FILE_A]; // these are wrong now 
			// for moves north east, east, and south east, we need to clear file h to prevent overflow
			Bitboard kingHFileCleared = kingLocBB & fileClear[FILE_H]; // * must be only case 

			// consider all ordinal and cardinal directions
			return kingAFileCleared << 7 | kingAFileCleared >> 1 | kingAFileCleared >> 9 | kingLocBB << 8 |
				   kingHFileCleared << 9 | kingHFileCleared << 1 | kingHFileCleared >> 7 | kingLocBB >> 8;
		}

		Bitboard computeKnightPseudoMoves(Bitboard knightLocBB)
		{
			// prevent overflow
			Bitboard knightAFileCleared = knightLocBB & fileClear[FILE_A];
			Bitboard knightBFileCleared = knightLocBB & fileClear[FILE_B];
			Bitboard knightGFileCleared = knightLocBB & fileClear[FILE_G];
			Bitboard knightHFileCleared = knightLocBB & fileClear[FILE_H];

			Bitboard moves = 0;

			moves |= (knightAFileCleared & knightBFileCleared) << 6 | (knightAFileCleared & knightBFileCleared) >> 10; // check western horizontal moves
			moves |=  knightAFileCleared << 15 | knightAFileCleared >> 17;											   // check western vertical moves
			moves |= (knightGFileCleared & knightHFileCleared) << 10 | (knightGFileCleared & knightHFileCleared) >> 6; // check eastern horizontal moves
			moves |=  knightHFileCleared << 17 | knightHFileCleared >> 15;											   // check eastern vertiacal moves

			return moves;
		}

		Bitboard computePseudoPawnAttacks(Bitboard pawnLocBB, Colour side)
		{
			Bitboard pawnAFileCleared = pawnLocBB & fileClear[FILE_A];
			Bitboard pawnHFileCleared = pawnLocBB & fileClear[FILE_H];

			if (side == SIDE_WHITE)
				return pawnAFileCleared << 7 | pawnHFileCleared << 9;
			else if (side == SIDE_BLACK)
				return pawnAFileCleared >> 7 | pawnHFileCleared >> 9;
		}

		void initialize()
		{
			initializeFileRankMasks();

			// initialize elements for the general board positions, king move lookup table, and knight move lookup table
			for (int i = 0; i < 64; i++)
			{
				boardSquares[i]						   = (uint64_t)1 << i;
				kingLookupTable[i]					   = computeKingPseudoMoves(boardSquares[i]);
				knightLookupTable[i]				   = computeKnightPseudoMoves(boardSquares[i]);
				pawnAttackLookupTable[SIDE_WHITE][i]   = computePseudoPawnAttacks(boardSquares[i], SIDE_WHITE);
				pawnAttackLookupTable[SIDE_BLACK][i]   = computePseudoPawnAttacks(boardSquares[i], SIDE_BLACK);
			}
		}
	}
}