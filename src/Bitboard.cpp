#include "Bitboard.h"

#include <iostream>
#include <vector>
#include <random>

const bool IS_ROOK   = true;
const bool IS_BISHOP = false;

namespace BB
{
	namespace LookupTables
	{
		extern Bitboard knightLookupTable[64] { 0 };
		extern Bitboard kingLookupTable[64]   { 0 };
		extern Bitboard rookLookupTable[64]   { 0 };

		extern Bitboard boardSquares[64]	          { 0 };
		extern Bitboard pawnAttackLookupTable[2][64]  { { 0 }, { 0 } };
		extern Bitboard fileClear[8]		          { 0 };
		extern Bitboard rankClear[8]		          { 0 };

		extern Bitboard rookMoveVariations[64][4096]  { { 0 }, { 0 } };
		extern Bitboard rookAttackSets[64][4096]      { { 0 }, { 0 } };
		extern Bitboard rookMagicNumbers[64]          { 0 };

		extern Bitboard bishopMoveVariations[64][1024]{  { 0 }, { 0 } };
		extern Bitboard bishopAttackSets[64][1024]    {  { 0 }, { 0 } };
		extern Bitboard bishopMagicNumbers[64]        { 0 };

		int nRookBits[64] =
		{
		    12, 11, 11, 11, 11, 11, 11, 12,
		    11, 10, 10, 10, 10, 10, 10, 11,
		    11, 10, 10, 10, 10, 10, 10, 11,
		    11, 10, 10, 10, 10, 10, 10, 11,
		    11, 10, 10, 10, 10, 10, 10, 11,
		    11, 10, 10, 10, 10, 10, 10, 11,
		    11, 10, 10, 10, 10, 10, 10, 11,
		    12, 11, 11, 11, 11, 11, 11, 12,
		};

		int nBishopBits[64] =
		{
            0//6, 6,
		};

		int nRookBitShifts[64] = // 64 - nRookBits[index]
		{
		    52, 53, 53, 53, 53, 53, 53, 52,
		    53, 54, 54, 54, 54, 54, 54, 53,
		    53, 54, 54, 54, 54, 54, 54, 53,
		    53, 54, 54, 54, 54, 54, 54, 53,
		    53, 54, 54, 54, 54, 54, 54, 53,
		    53, 54, 54, 54, 54, 54, 54, 53,
		    53, 54, 54, 54, 54, 54, 54, 53,
		    52, 53, 53, 53, 53, 53, 53, 52,
		};

		void initializeFileRankMasks()
		{
			for (int i = 0; i < 8; i++)
			{
				// intentional overflow making fileClear[i]'s and rankClear[i]'s bits all set to 1
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
			Bitboard kingAFileCleared = kingLocBB & fileClear[FILE_A];
			// for moves north east, east, and south east, we need to clear file h to prevent overflow
			Bitboard kingHFileCleared = kingLocBB & fileClear[FILE_H];

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

        int getLeastSignificantBitIndex(Bitboard board)
        {
            if (!board)
                return -1; // no set bits so return an invalid index

            for (int bitIndex = 0; bitIndex < 64; bitIndex++)
                if ((board & ((Bitboard)1 << bitIndex)) != 0)
                    return bitIndex;
        }

        // attack mask contains all squares the piece attacks
        Bitboard genOccupancies(int index, int numOfSetBits, Bitboard attackMask, bool isRook, int piecePosition)
        {
            Bitboard occ = 0;

            for (int i = 0; i < numOfSetBits; i++)
            {
                // gets each square that the piece is attacking (not the border squares of the rows/columns)
                int square = getLeastSignificantBitIndex(attackMask);

                // we are just popping one bit per iteration of this for loop from the attackMask (removing one attack square)
                attackMask ^= (Bitboard)1 << square;

                // right because for any one square, number of bits in the attack mask = numOfSetBits

                // this is what determines which squares actually get set on the occupancy
                if (index & (1 << i))
                {
                    occ |= (Bitboard)1 << square; // adds the actual tile to the attack board


                    // add border attacks ! if there is no blocker, you can move to the border tiles

                    // this is different for rooks and bishops
                    if (isRook)
                    {
                      /*  if ((boardSquares[square] & ~rankClear[RANK_SECOND])  && (boardSquares[square] & ~fileClear[piecePosition % 8]) && piecePosition != square - 8)
                            occ |= (Bitboard)1 << (square - 8);

                        if ((boardSquares[square] & ~fileClear[FILE_B]) && (boardSquares[square] & ~rankClear[piecePosition / 8]) && piecePosition != square - 1)
                            occ |= Bitboard(1) << (square - 1);

                        if ((boardSquares[square] & ~fileClear[FILE_G]) && (boardSquares[square] & ~rankClear[piecePosition / 8]) && piecePosition != square + 1)
                            occ |= (Bitboard)1 << (square + 1);

                        if ((boardSquares[square] & ~rankClear[RANK_SEVENTH]) && (boardSquares[square] & ~fileClear[piecePosition % 8]) && piecePosition != square + 8)
                            occ |= (Bitboard)1 << (square + 8);
                            */
                    }
                }
            }

            return occ;
        }

        int getRandomNum()
        {
            return rand() | (rand() << 16) | (rand() << 32) | (rand() << 48);
        }

        void genMagicNums(bool isRook)
        {
            for (int square = 0; square < 64; square++)
            {
                bool fail = false;
                int bitCount = isRook ? nRookBits[square] : nBishopBits[square];
                int moveVariations = 1 << bitCount;
                Bitboard* usedBy = new Bitboard[moveVariations];
                Bitboard magicNumber;
                int attempts = 0;

                do
                {
                    attempts++;
                    magicNumber = getRandomNum() & getRandomNum() & getRandomNum();
                    for (int i = 0; i < moveVariations; i++)
                        usedBy[i] = 0;

                    for (int move = 0; move < moveVariations && !fail; move++)
                    {
                        int index;
                        if (isRook)
                            index = (rookMoveVariations[square][move] * magicNumber) >> (64 - bitCount);
                        else
                            index = (bishopMoveVariations[square][move] * magicNumber) >> (64 - bitCount);

                        // fail = false (so a successful magic number) is the randomly calculated index
                        Bitboard attackSet = isRook ? rookAttackSets[square][move] : bishopAttackSets[square][move];
                        fail = usedBy[index] != 0 && usedBy[index] != attackSet;

                        usedBy[index] = attackSet;
                    }

                } while (fail && attempts < 10000);

                if (isRook)
                    rookMagicNumbers[square]   =  magicNumber;
                else
                    bishopMagicNumbers[square] = magicNumber;
            }
        }

        // may need to make two tables, one with the corner tiles included
        Bitboard initRookBlockerTable(int square)
        {
            Bitboard moves = 0;

            /*
            note that these masks will not include border tiles
            this is because it is unnecessary. border tiles cannot block anything,
            meaning that we do not need to consider them for blocking
            otherwise, border tiles would have 14 set bits, and our array size would have to be 2^14, instead of 2^12 (for rooks)
            */

            // rows
            for (int i = 1; i <= square % 8 - 1; i++)
                moves |= boardSquares[square] >> i;
            for (int i = 1; i <= 8 - square % 8 - 2; i++)
                moves |= boardSquares[square] << i;

            // columns
            for (int i = 1; i <= square / 8 - 1; i++)
                moves |= boardSquares[square] >> i * 8;
            for (int i = 1; i <= 8 - square / 8 - 2; i++)
                moves |= boardSquares[square] << i * 8;

            return moves;
        }

        void initializeBishopTable(int square)
        {

        }

        void printBitboard(Bitboard bitboard)
        {
            // have to print by last rank first,
            // but print files left to right

            int cRank = 8;
            int cFile = 0;

            while (cRank > 0)
            {
                if (bitboard & (boardSquares[(cRank - 1) * 8 + cFile]))
                    std::cout << 1;
                else
                    std::cout << 0;

                cFile++;

                if (cFile >= 8)
                {
                    std::cout << std::endl;
                    cFile = 0;
                    cRank--;
                }
            }
            std::cout << std::endl;
        }

        // generates a bitboard with only the squares that are being attacked
        Bitboard genAttackSets(int square, int index, bool isRook)
        {
            Bitboard attackSet = 0;

            // this code is adapted from https://web.archive.org/web/20170120211959/http://www.afewmorelines.com/understanding-magic-bitboards-in-chess-programming/
            if (isRook)
            {
                // for loops that increase k until the first square that is attacked shows up
                // there are 4. one for each direction (up, down, left, right)

                int firstAttackSquare;
                for (firstAttackSquare = square + 8; firstAttackSquare <=55 && (rookMoveVariations[square][index] & ((Bitboard)1 << firstAttackSquare)) == 0; firstAttackSquare += 8) ; // this is done to increase k !
                if (firstAttackSquare >= 0 && firstAttackSquare < 64)
                    attackSet |= (Bitboard)1 << firstAttackSquare;

                for (firstAttackSquare = square - 8; firstAttackSquare >= 8 && (rookMoveVariations[square][index] & ((Bitboard)1 << firstAttackSquare)) == 0; firstAttackSquare -= 8) ;
                if (firstAttackSquare >= 0 && firstAttackSquare < 64)
                    attackSet |= (Bitboard)1 << firstAttackSquare;

                // uses modulo so that the square cannot loop back to the next or previous row
                for (firstAttackSquare = square + 1; firstAttackSquare % 8 != 0 && firstAttackSquare % 8 != 7 && (rookMoveVariations[square][index] & ((Bitboard)1 << firstAttackSquare)) == 0; firstAttackSquare++) ;
                if (firstAttackSquare >= 0 && firstAttackSquare < 64)
                    attackSet |= (Bitboard)1 << firstAttackSquare;

                for (firstAttackSquare = square - 1; firstAttackSquare % 8 != 0 && firstAttackSquare % 8 != 7 && (rookMoveVariations[square][index] & ((Bitboard)1 << firstAttackSquare)) == 0; firstAttackSquare--) ;
                if (firstAttackSquare >= 0 && firstAttackSquare < 64)
                    attackSet |= (Bitboard)1 << firstAttackSquare;
            }

            return attackSet;

        }

		void initialize()
		{
			initializeFileRankMasks();

			// have to set the board squares before everything else
			for (int i = 0; i < 64; i++)
                boardSquares[i] = (Bitboard)1 << i;

            printBitboard(~fileClear[FILE_G]);

			// initialize elements for the general board positions, king move lookup table, and knight move lookup table
			for (int square = 0; square < 64; square++)
			{
				kingLookupTable[square]					    = computeKingPseudoMoves(boardSquares[square]);
				knightLookupTable[square]				    = computeKnightPseudoMoves(boardSquares[square]);
				pawnAttackLookupTable[SIDE_WHITE][square]   = computePseudoPawnAttacks(boardSquares[square], SIDE_WHITE);
				pawnAttackLookupTable[SIDE_BLACK][square]   = computePseudoPawnAttacks(boardSquares[square], SIDE_BLACK);
                rookLookupTable[square]                     = initRookBlockerTable(square);

                // sliding pieces magics

                // sets all possible move variations. not that some of these variations will be effectively the same as many others, so attack sets must be made
                // rooks
                for (int j = 0; j < (1 << nRookBits[square]); j++)
                {
                    // might have to make these not include adjacent moves/border tiles as the below will handle that i believeeee
                    rookMoveVariations[square][j] = genOccupancies(j, nRookBits[square], rookLookupTable[square], IS_ROOK, square);
                    rookAttackSets[square][j] = genAttackSets(square, j, IS_ROOK);

                    // attack set only SHOWS WHAT IT ATTACKS NOT ALL POSSIBLE MOVES
                }
            }

            //genMagicNums(IS_ROOK);
			for (int i = 0; i < 64; i++)
			{
			    std::cout << "==================\n";
			    std::cout << "magic number: " << rookMagicNumbers[i] << std::endl;
                printBitboard(rookMoveVariations[i][1023]);
			}
 		}
	}
}
