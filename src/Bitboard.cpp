#include "Bitboard.h"

#include <iostream>

namespace BB
{
    extern Bitboard boardSquares[64]	          { 0 };
    extern Bitboard fileClear[8]		          { 0 };
    extern Bitboard rankClear[8]		          { 0 };

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

    int getLeastSignificantBitIndex(Bitboard board)
    {
        if (!board)
            return -1; // no set bits so return an invalid index

        for (int bitIndex = 0; bitIndex < 64; bitIndex++)
            if ((board & ((Bitboard)1 << bitIndex)) != 0)
                return bitIndex;
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
   /* Bitboard genAttackSets(int square, int index, bool isRook)
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
*/
    void initialize()
    {
        initializeFileRankMasks();
        
        for (int square = 0; square < 64; square++)
        {
            boardSquares[square]                        = (Bitboard)1 << square;
            //rookLookupTable[square]                     = //initRookBlockerTable(square);
        }
    }
}
