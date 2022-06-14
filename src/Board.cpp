#include "Board.h"
#include "MoveGenerator.h"

#include <iostream>

// initialize all of the default values for the bitboard 
void Board::setBitboards()
{
	/* setting starting positions of all pieces on the board */

	// white pieces
	whitePawnsBB   = BB::Constants::cWPawnsStartBB;
	whiteRooksBB   = BB::Constants::cWRooksStartBB;
	whiteKnightsBB = BB::Constants::cWKnightsStartBB;
	whiteBishopsBB = BB::Constants::cWBishopsStartBB;
	whiteQueensBB  = BB::Constants::cWQueensStartBB;
	whiteKingBB    = BB::Constants::cWKingStartBB;
	whitePiecesBB  = whitePawnsBB | whiteRooksBB | whiteKnightsBB | whiteBishopsBB | whiteQueensBB | whiteKingBB;

	// black pieces
	blackPawnsBB   = BB::Constants::cBPawnsStartBB;
	blackRooksBB   = BB::Constants::cBRooksStartBB;
	blackKnightsBB = BB::Constants::cBKnightsStartBB;
	blackBishopsBB = BB::Constants::cBBishopsStartBB;
	blackQueensBB  = BB::Constants::cBQueensStartBB;
	blackKingBB    = BB::Constants::cBKingStartBB;
	blackPiecesBB  = blackPawnsBB | blackRooksBB | blackKnightsBB | blackBishopsBB | blackQueensBB | blackKingBB;
}

void Board::init()
{
	setBitboards();
	BB::initialize();
	mMoveGenerator.init();
}

// side is a default value with a value of -1. this value indicates no side was specified and to search all bitboards
Bitboard* Board::getPieceBitboard(Byte square, Colour side)
{
	Bitboard squareBB = BB::boardSquares[square];

	if (side == SIDE_WHITE || side == -1)
	{
		if	    (squareBB & whitePawnsBB)	 return &whitePawnsBB;
		else if (squareBB & whiteRooksBB)	 return &whiteRooksBB;
		else if (squareBB & whiteKnightsBB)  return &whiteKnightsBB;
		else if (squareBB & whiteBishopsBB)  return &whiteBishopsBB;
		else if (squareBB & whiteQueensBB)   return &whiteQueensBB;
		else if (squareBB & whiteKingBB)	 return &whiteKingBB;
	}
	if (side == SIDE_BLACK || side == -1)
	{
		if		(squareBB & blackPawnsBB)	 return &blackPawnsBB;
		else if (squareBB & blackRooksBB)	 return &blackRooksBB;
		else if (squareBB & blackKnightsBB)  return &blackKnightsBB;
		else if (squareBB & blackBishopsBB)  return &blackBishopsBB;
		else if (squareBB & blackQueensBB)   return &blackQueensBB;
		else if (squareBB & blackKingBB)	 return &blackKingBB;
	}

	return nullptr;
}

void Board::calculateBlackMoves()
{
	mBlackMoves.clear();
	mBlackMoves.reserve(100);

	for (int square = 0; square < 64; square++)
		calculatePieceMoves(SIDE_BLACK, square, mBlackMoves);
}

// make it not a vector? an array with a set size?
// this might speed it up considerably no?
// and then we could search through this array of moves until an invalid one is met, and then it would go on to the next one?

void Board::calculateWhiteMoves()
{
	mWhiteMoves.clear();
	mWhiteMoves.reserve(100); // to minimize the number of vector reallocations necessary

	for (int square = 0; square < 64; square++)
		calculatePieceMoves(SIDE_WHITE, square, mWhiteMoves);


	// attack board by taking the moves of a piece on that square, and & it with opposite colour 
	// attack boards might be more for evaluation than move generation. checking pins and defend maps
	// selection sort as we move to put more important moves near the start of the vector 
}

void Board::calculatePieceMoves(Colour side, Byte originSquare, std::vector<MoveData>& moveVector)
{
	MoveData md;
	if (side == SIDE_WHITE)
	{
		md.colourBB		    = &whitePiecesBB;
		md.capturedColourBB = &blackPiecesBB; // if a capture occurred, it would be a black piece
	}
	else
	{
		md.colourBB		    = &blackPiecesBB;
		md.capturedColourBB = &whitePiecesBB;
	}

	if (BB::boardSquares[originSquare] & *md.colourBB)
	{
		md.side = side;
		md.originSquare = originSquare;
		Bitboard moves = 0;
		Bitboard* pieceBBPtr = getPieceBitboard(originSquare, side);
		Bitboard pieceBB = *pieceBBPtr;

		md.pieceBB = pieceBBPtr;

		if ((pieceBB & whiteKnightsBB) || (pieceBB & blackKnightsBB))      moves = mMoveGenerator.computePseudoKnightMoves(originSquare, *md.colourBB);
		else if ((pieceBB & whiteKingBB) || (pieceBB & blackKingBB))       moves = mMoveGenerator.computePseudoKingMoves(originSquare, *md.colourBB);
		else if ((pieceBB & whitePawnsBB) || (pieceBB & blackPawnsBB))     moves = mMoveGenerator.computePseudoPawnMoves(originSquare, side, *md.capturedColourBB, occupiedBB);
		else if ((pieceBB & whiteRooksBB) || (pieceBB & blackRooksBB))	   moves = mMoveGenerator.computePseudoRookMoves(originSquare, *md.capturedColourBB, *md.colourBB);
		else if ((pieceBB & whiteBishopsBB) || (pieceBB & blackBishopsBB)) moves = mMoveGenerator.computePseudoBishopMoves(originSquare, *md.capturedColourBB, *md.colourBB);
		else if ((pieceBB & whiteQueensBB) || (pieceBB & blackQueensBB))   moves = mMoveGenerator.computePseudoQueenMoves(originSquare, *md.capturedColourBB, *md.colourBB);

		for (int square = 0; square < 64; square++)
		{
			if (moves & BB::boardSquares[square])
			{
				md.targetSquare    = square;
				md.capturedPieceBB = nullptr;

				if (BB::boardSquares[square] & *md.capturedColourBB)
					md.capturedPieceBB = getPieceBitboard(square, !side);

				moveVector.push_back(md);
			}
		}

	}
}

// black cannot take pieces and pawns sometimes are unable to move
bool Board::makeMove(MoveData* moveData)
{
	// our moves are pseudo legal, meaning we must also check to see if a check is actually preventing these moves. use attack tables?
	// maybe index an attack table and see if any piece was attacking the tile just moved from. if so, check if there is a check (reupdate attack table)
	// 

	Bitboard origin		  = BB::boardSquares[moveData->originSquare];
	Bitboard target		  = BB::boardSquares[moveData->targetSquare];
	Bitboard originTarget = origin ^ target; // 0s on from and to, 1s on everything else

	*moveData->pieceBB	 ^= originTarget;
	*moveData->colourBB	 ^= originTarget;

	if (moveData->capturedPieceBB) // if a piece was captured
	{
		*moveData->capturedPieceBB  ^= target; // only the target's square will have changed
		*moveData->capturedColourBB ^= target; // only the target's square will have changed
		occupiedBB					^= origin; // only the origin square is no longer occupied
		emptyBB						^= origin; // only the origin square is no longer occupied
	}

    occupiedBB           ^= originTarget;
    emptyBB              ^= originTarget;

	// also updated all occupied pieces bitboard

	return true;
	return false;
}

bool Board::unmakeMove(MoveData* moveData)
{
    
    
	return false;
}
