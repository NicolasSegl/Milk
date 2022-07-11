#pragma once

#include <vector>

#include "Bitboard.h"
#include "MoveData.h"

class Board;

class MoveGenerator
{
private:
    void initKnightLT(Byte knightLoc);
    void initKingLT(Byte kingLoc);
    void initPawnLT(Colour side, Byte pawnLoc);
    
    // the following functions are used when generating moves
    void calculatePieceMoves(Board* board, Colour side, Byte originSquare, std::vector<MoveData>& moveVec);
    void calculateCastleMoves(Board* board, Colour side, std::vector<MoveData>& moveVec);
    void findMoveCaptures(Board* board, Bitboard moves, MoveData& md, std::vector<MoveData>& moveVec);
    Bitboard calculatePsuedoMove(Board* board, MoveData* md, Bitboard& pieceBB);
    void setCastlePrivileges(Board* board, MoveData* castleMoveData, bool isKing);
    Bitboard* getPieceBitboard(Board* board, Byte square, Colour side = -1); // -1 indicates that no value was passed in


public:
    MoveGenerator() {}
    Bitboard pawnAttackLookupTable[2][64]; // 2 because pawns have different moves depending on side
    Bitboard knightLookupTable[64];
    Bitboard kingLookupTable[64];
    
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
    
    void calculateSideMoves(Board* board, Colour side, std::vector<MoveData>& moveVec);
    void generateCaptureMoves(std::vector<MoveData>& moveVec, Colour side, Board* board);
};
