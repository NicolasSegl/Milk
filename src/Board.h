#pragma once

#include <vector>

#include "Bitboard.h"
#include "MoveGenerator.h"
#include "MoveData.h"

class Board
{
public:
	enum PieceType
	{
		WHITE_PAWN,
		WHITE_ROOK,
		WHITE_BISHOP,
		WHITE_QUEEN,
		WHITE_KNIGHT,
		WHITE_KING,

		BLACK_PAWN,
		BLACK_ROOK,
		BLACK_BISHOP,
		BLACK_QUEEN,
		BLACK_KNIGHT,
		BLACK_KING
	};

	// white piece bitboards
	Bitboard whitePiecesBB;
	Bitboard whitePawnsBB;
	Bitboard whiteRooksBB;
	Bitboard whiteKnightsBB;
	Bitboard whiteBishopsBB;
	Bitboard whiteQueensBB;
	Bitboard whiteKingBB;

	// black piece bitboards
	Bitboard blackPiecesBB;
	Bitboard blackPawnsBB;
	Bitboard blackRooksBB;
	Bitboard blackKnightsBB;
	Bitboard blackBishopsBB;
	Bitboard blackQueensBB;
	Bitboard blackKingBB;

	//Bitboard blackAttackBB = 0;
	//Bitboard whiteAttackBB = 0;
	Bitboard occupiedBB   = 0;
	Bitboard emptyBB      = 0;
	Bitboard enPassantBB  = 0;
    // defence tables
    
    Byte movePrivileges;
    
private:
	Bitboard mAttackTable[64]{ 0 };

	std::vector<MoveData> mWhiteMoves;
	std::vector<MoveData> mBlackMoves;

    MoveGenerator mMoveGenerator;

	// compare which moves are defending a piece of their own :D
	// by separating all valid moves with those moves that attack pieces, we can hasten the search algorithm. consider moves that take first
	// middle of the board bitboards (to encourage control of center) 4x4 grid in middle maybe?
	// array of 64 bitboards. each with exactly 1 square set to 1. could be very useful
	// make them all constats. for initializing the bitboards ?

	void setBitboards();
	Bitboard* getPieceBitboard(Byte square, Colour side = -1); // -1 indicates that no value was passed in

    void setCastlePrivileges(MoveData* castleMoveData, bool isKing);
	void setCastleMoveData(MoveData* castleMoveData, MoveData* kingMD, MoveData* rookMD);
	bool makeCastleMove(MoveData* moveData);

	void updateBitboardWithMove(MoveData* moveData);
	void undoPromotion(MoveData* moveData);
    Byte computedKingSquare(Bitboard kingBB);
    bool squareAttacked(Byte square, Colour attackingSide);

public:
	Board() {}

	void init();
	bool makeMove(MoveData* moveData);
	bool unmakeMove(MoveData* moveData);
	void promotePiece(MoveData* md, MoveData::EncodingBits promoteTo);
	void calculateSideMoves(Colour side);

	std::vector<MoveData>& getWhiteMoves() { return mWhiteMoves;     }
	std::vector<MoveData>& getBlackMoves() { return mBlackMoves;     }
    std::vector<MoveData>& getMoves(Colour side);
};
