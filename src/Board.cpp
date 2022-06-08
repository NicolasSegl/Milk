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

	// maybe make it make a pseudo move for now 

	// in the future we would have a pointer to move-piece as a bitboard ptr. refer to https://www.chessprogramming.org/General_Setwise_Operations#UpdateByMove
	// for testing
	/*std::cout << "what square (0-63): ";
	int shift1;
	std::cin >> shift1;
	// figure this out, but then realize we can just use boardSquares[shift1]
	Bitboard moves = mMoveGenerator.computePseudoPawnMoves(shift1, SIDE_WHITE, mBlackPiecesBB, mOccupiedBB);
        
	std::cout << "to what square (0-63): ";
	int shift2;
	std::cin >> shift2;
	
	calculateWhiteMoves();

	if ((BB::boardSquares[shift2] & moves) != 0)
	{
		std::cout << "it was a valid move\n";
		MoveData md;
		//md.colourBB = &mWhitePiecesBB;
		md.pieceBB = &mWhitePawnsBB;
		md.originSquare = shift1;
		md.targetSquare = shift2;
		makeMove(&md);
		std::cout << std::endl;
		print();
	}
    else
    {
        std::cout << "test move failed\n";
    }
	*/
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

// make it not a vector? an array with a set size?
void Board::calculateWhiteMoves()
{
	mWhiteMoves.clear();
	int captureIndex = 0;

	for (int square = 0; square < 64; square++)
	{
		Bitboard squareBB = BB::boardSquares[square];

		if ((squareBB & whitePiecesBB) == 0)
			continue;

		MoveData md;

		// these properties will always be true for all moves that are made by the white side
		md.side				= SIDE_WHITE;
		md.originSquare		= square;
        md.colourBB         = &whitePiecesBB;

		if (squareBB & whiteKnightsBB)
		{
			Bitboard knightMoves = mMoveGenerator.computePseudoKnightMoves(square, whitePiecesBB);
			md.pieceBB = &whiteKnightsBB;

			for (int knightSquare = 0; knightSquare < 64; knightSquare++)
			{
				Bitboard knightMoveSquareBB = BB::boardSquares[knightSquare];

				if (knightMoves & knightMoveSquareBB)
				{
					md.targetSquare    = knightSquare;
					md.capturedPieceBB = nullptr; // reset capture (if previous move captured a piece)
					
					// move was a capture
					if (BB::boardSquares[knightSquare] & blackPiecesBB)
						md.capturedPieceBB = getPieceBitboard(knightSquare, SIDE_BLACK);

					mWhiteMoves.push_back(md);
				}
			}
		}
		else if (squareBB & whiteKingBB)
		{
			Bitboard kingMoves = mMoveGenerator.computePseudoKingMoves(square, whitePiecesBB);
			md.pieceBB = &whiteKingBB;

			for (int kingSquare = 0; kingSquare < 64; kingSquare++)
			{
				Bitboard kingMoveSquareBB = BB::boardSquares[kingSquare];

				if (kingMoves & kingMoveSquareBB)
				{
					md.targetSquare	   = kingSquare;
					md.capturedPieceBB = nullptr; // reset capture

					if (BB::boardSquares[kingSquare] & blackPiecesBB)
						md.capturedPieceBB = getPieceBitboard(kingSquare, SIDE_BLACK);

					mWhiteMoves.push_back(md);
				}
			}
		}
		else if (squareBB & whitePawnsBB)
		{
			Bitboard pawnMoves = mMoveGenerator.computePseudoPawnMoves(square, SIDE_WHITE, blackPiecesBB, occupiedBB);
			md.pieceBB = &whitePawnsBB;

			for (int pawnSquare = 0; pawnSquare < 64; pawnSquare++)
			{
				Bitboard pawnMoveSquareBB = BB::boardSquares[pawnSquare];

				if (pawnMoves & pawnMoveSquareBB)
				{
					md.targetSquare = pawnSquare;
					md.capturedPieceBB = nullptr;

					if (BB::boardSquares[pawnSquare] & blackPiecesBB)
						md.capturedPieceBB = getPieceBitboard(pawnSquare, SIDE_BLACK);

					mWhiteMoves.push_back(md);
				}
			}
		}
	}

	// attack board by taking the moves of a piece on that square, and & it with opposite colour 
	// attack boards might be more for evaluation than move generation. checking pins and defend maps
	// selection sort as we move to put more important moves near the start of the vector 
}

bool Board::makeMove(MoveData* moveData)
{
	// our moves are pseudo legal, meaning we must also check to see if a check is actually preventing these moves. use attack tables?
	// maybe index an attack table and see if any piece was attacking the tile just moved from. if so, check if there is a check (reupdate attack table)
	// 

	Bitboard originTarget = BB::boardSquares[moveData->originSquare] ^ BB::boardSquares[moveData->targetSquare];
	*moveData->pieceBB	 ^= originTarget;
	*moveData->colourBB	 ^= originTarget;
    occupiedBB           ^= originTarget;
    emptyBB              ^= originTarget;

	// also updated all occupied pieces bitboard

	return false;
}

bool Board::unmakeMove(MoveData* moveData)
{
    
    
	return false;
}
