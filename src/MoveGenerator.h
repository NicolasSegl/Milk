#pragma once

#include "Bitboard.h"
#include "MoveData.h"

class MoveGenerator
{
private:

    void initKnightLT(Byte knightLoc);
    void initKingLT(Byte kingLoc);
    void initPawnLT(Colour side, Byte pawnLoc);

public:
    MoveGenerator() {}

    Bitboard knightLookupTable[64];
    Bitboard kingLookupTable[64];
    Bitboard rookLookupTable[64];
    Bitboard bishopLookupTable[64];
    Bitboard pawnAttackLookupTable[2][64]; // 2 because pawns have different moves depending on side

    void init();

    // pseudo meaning that they do not account for if they result in a check!
    Bitboard computePseudoKingMoves(Byte pieceCoord, Bitboard friendlyPieces);
    Bitboard computePseudoKnightMoves(Byte pieceCoord, Bitboard friendlyPieces);
    Bitboard computePseudoPawnMoves(Byte pieceCoord, Colour side, Bitboard enemyPieces, Bitboard occupiedSquares, Bitboard enPassantBB);
    Bitboard computePseudoRookMoves(Byte pieceCoord, Bitboard enemyPieces, Bitboard friendlyPieces);
    Bitboard computePseudoBishopMoves(Byte pieceCoord, Bitboard enemyPieces, Bitboard friendlyPieces);
    Bitboard computePseudoQueenMoves(Byte pieceCoord, Bitboard enemyPieces, Bitboard friendlyPieces);

    Bitboard computeSpecialMoves();
    MoveData computeCastleMoveData(Colour side, Byte privileges, Bitboard occupied, Privilege castleType);
};

/*


is that what we need the attack bitboards for ? attack bitboards[64]
so that we can refer to a specific square and see where it can move?

we would calculate all the moves that a piece can make. then set the bitboard at that piece's index on the board
to all of those locations. when a move is specified, check that index's bitboard and see if the suggested move is
legal (or pseudolegal). if it is, simply make the move :)

*/