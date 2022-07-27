#pragma once

#include <vector>

#include "Bitboard.h"
#include "MoveGenerator.h"
#include "MoveData.h"
#include "ChessPosition.h"
#include "ZobristKeyGenerator.h"

class Board
{    
private:
	Bitboard mAttackTable[64]{ 0 };

	//::vector<ChessPosition> mPositionHistory; // pass this into outcomehandler
	std::vector<MoveData> mWhiteMoves;
	std::vector<MoveData> mBlackMoves;

    MoveGenerator mMoveGenerator;

	uint64_t mCurrentZobristKey;
	ZobristKeyGenerator mZobristKeyGenerator;
	ZobristKey mZobristKeyHistory[1000];

	int mPly;
	int mFiftyMoveCounter;

	void setBitboards();

	// functions for making/unmaking moves
	void setCastleMoveData(MoveData* castleMoveData, MoveData* kingMD, MoveData* rookMD);
	bool makeCastleMove(MoveData* moveData);
	void setEnPassantSquares(MoveData* moveData);
	void updateBitboardWithMove(MoveData* moveData);
	void undoPromotion(MoveData* moveData);

	void insertMoveIntoHistory();
	void deleteMoveFromHistory();

public:
	Board() {}

	ChessPosition currentPosition;

	void init();
	bool makeMove(MoveData* moveData);
	bool unmakeMove(MoveData* moveData, bool updateZobristHistory = true);
	void promotePiece(MoveData* md, MoveData::EncodingBits promoteTo);

	void calculateSideMoves(Colour side);
	void calculateSideMovesCapturesOnly(Colour side);

	Byte computedKingSquare(Bitboard kingBB);
	bool squareAttacked(Byte square, Colour attackingSide);

	std::vector<MoveData>& getWhiteMoves() { return mWhiteMoves;        }
	std::vector<MoveData>& getBlackMoves() { return mBlackMoves;        }
	ZobristKey* getZobristKeyHistory()	   { return mZobristKeyHistory; }
	short getCurrentPly()				   { return mPly;				}
	std::vector<MoveData>& getMoves(Colour side);
};
