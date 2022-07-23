#pragma once

#include <vector>

#include "Bitboard.h"
#include "MoveGenerator.h"
#include "MoveData.h"
#include "ChessPosition.h"
#include "TranspositionTable.h"

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

	ChessPosition currentPosition;
    
private:
	Bitboard mAttackTable[64]{ 0 };

	//::vector<ChessPosition> mPositionHistory; // pass this into outcomehandler
	std::vector<MoveData> mWhiteMoves;
	std::vector<MoveData> mBlackMoves;

    MoveGenerator mMoveGenerator;
	TranspositionTable mTranspositionTable;

	void setBitboards();
	void setCastleMoveData(MoveData* castleMoveData, MoveData* kingMD, MoveData* rookMD);
	bool makeCastleMove(MoveData* moveData);

	void updateBitboardWithMove(MoveData* moveData);
	void undoPromotion(MoveData* moveData);

public:
	Board() {}

	void init();
	bool makeMove(MoveData* moveData);
	bool unmakeMove(MoveData* moveData);
	void promotePiece(MoveData* md, MoveData::EncodingBits promoteTo);

	void calculateSideMoves(Colour side);
	void calculateSideMovesCapturesOnly(Colour side);

	Byte computedKingSquare(Bitboard kingBB);
	bool squareAttacked(Byte square, Colour attackingSide);

	std::vector<MoveData>& getWhiteMoves() { return mWhiteMoves;     }
	std::vector<MoveData>& getBlackMoves() { return mBlackMoves;     }
    std::vector<MoveData>& getMoves(Colour side);
};
