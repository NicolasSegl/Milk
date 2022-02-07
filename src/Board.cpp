#include "Board.h"
#include "MoveGenerator.h"

#include <iostream>

// initialize all of the default values for the bitboard 
void Board::setBitboards()
{
	/* setting starting positions of all pieces on the board */

	// white pieces
	mWhitePawnsBB   = BB::Constants::cWPawnsStartBB;
	mWhiteRooksBB   = BB::Constants::cWRooksStartBB;
	mWhiteKnightsBB = BB::Constants::cWKnightsStartBB;
	mWhiteBishopsBB = BB::Constants::cWBishopsStartBB;
	mWhiteQueensBB  = BB::Constants::cWQueensStartBB;
	mWhiteKingBB    = BB::Constants::cWKingStartBB;
	mWhitePiecesBB  = mWhitePawnsBB | mWhiteRooksBB | mWhiteKnightsBB | mWhiteBishopsBB | mWhiteQueensBB | mWhiteKingBB;

	// black pieces
	mBlackPawnsBB   = BB::Constants::cBPawnsStartBB;
	mBlackRooksBB   = BB::Constants::cBRooksStartBB;
	mBlackKnightsBB = BB::Constants::cBKnightsStartBB;
	mBlackBishopsBB = BB::Constants::cBBishopsStartBB;
	mBlackQueensBB  = BB::Constants::cBQueensStartBB;
	mBlackKingBB    = BB::Constants::cBKingStartBB;
	mBlackPiecesBB  = mBlackPawnsBB | mBlackRooksBB | mBlackKnightsBB | mBlackBishopsBB | mBlackQueensBB | mBlackKingBB;
}

void Board::init()
{
	setBitboards();
	BB::LookupTables::initialize();

	// maybe make it make a pseudo move for now 

	// in the future we would have a pointer to move-piece as a bitboard ptr. refer to https://www.chessprogramming.org/General_Setwise_Operations#UpdateByMove
	
	print();

	// for testing
	std::cout << "what square (0-63): ";
	int shift1;
	std::cin >> shift1;
	// figure this out, but then realize we can just use boardSquares[shift1]
	Bitboard moves = MoveGenerator::computePseudoKnightMoves(shift1, mWhitePiecesBB); // assume it's white. 
	// assume it was a king
	std::cout << "to what square (0-63): ";
	int shift2;
	std::cin >> shift2;

	calculateWhiteMoves();

	if ((BB::LookupTables::boardSquares[shift2] & moves) != 0)
	{
		std::cout << "it was a valid move\n";
		MoveData md;
		//md.colourBB = &mWhitePiecesBB;
		md.pieceBB = &mWhiteKnightsBB;
		md.originSquare = shift1;
		md.targetSquare = shift2;
		makeMove(&md);
		std::cout << std::endl;
		print();
	}
}

// side is a default value with a value of -1. this value indicates no side was specified and to search all bitboards
Bitboard* Board::getPieceBitboard(Byte square, Colour side)
{
	Bitboard squareBB = BB::LookupTables::boardSquares[square];

	if (side == SIDE_WHITE || side == -1)
	{
		if	    (squareBB & mWhitePawnsBB)	 return &mWhitePawnsBB;
		else if (squareBB & mWhiteRooksBB)	 return &mWhiteRooksBB;
		else if (squareBB & mWhiteKnightsBB) return &mWhiteKnightsBB;
		else if (squareBB & mWhiteBishopsBB) return &mWhiteBishopsBB;
		else if (squareBB & mWhiteQueensBB)  return &mWhiteQueensBB;
		else if (squareBB & mWhiteKingBB)	 return &mWhiteKingBB;
	}
	if (side == SIDE_BLACK || side == -1)
	{
		if		(squareBB & mBlackPawnsBB)	 return &mBlackPawnsBB;
		else if (squareBB & mBlackRooksBB)	 return &mBlackRooksBB;
		else if (squareBB & mBlackKnightsBB) return &mBlackKnightsBB;
		else if (squareBB & mBlackBishopsBB) return &mBlackBishopsBB;
		else if (squareBB & mBlackQueensBB)  return &mBlackQueensBB;
		else if (squareBB & mBlackKingBB)	 return &mBlackKingBB;
	}

	return nullptr;
}

// might be able to do this in one function later ? refactor 
// make it not a vector? an array with a set size?
void Board::calculateWhiteMoves()
{
	mWhiteMoves.clear();
	int captureIndex = 0;

	for (int square = 0; square < 64; square++)
	{
		Bitboard squareBB = BB::LookupTables::boardSquares[square];

		if ((squareBB & mWhitePiecesBB) == 0)
			continue;

		MoveData md;

		// these properties will always be true for all moves that are made by the white side
		md.side				= SIDE_WHITE;
		md.originSquare		= square;

		if (squareBB & mWhiteKnightsBB)
		{
			Bitboard knightMoves = MoveGenerator::computePseudoKnightMoves(square, mWhitePiecesBB);
			md.pieceBB = &mWhiteKnightsBB;

			for (int knightSquare = 0; knightSquare < 64; knightSquare++)
			{
				Bitboard knightMoveSquareBB = BB::LookupTables::boardSquares[knightSquare];

				if (knightMoves & knightMoveSquareBB)
				{
					md.targetSquare    = knightSquare;
					md.capturedPieceBB = nullptr; // reset capture (if previous move captured a piece)
					
					// move was a capture
					if (BB::LookupTables::boardSquares[knightSquare] & mBlackPiecesBB)
						md.capturedPieceBB = getPieceBitboard(knightSquare, SIDE_BLACK);

					mWhiteMoves.push_back(md);
				}
			}
		}
		else if (squareBB & mWhiteKingBB)
		{
			Bitboard kingMoves = MoveGenerator::computePseudoKingMoves(square, mWhitePiecesBB);
			md.pieceBB = &mWhiteKingBB;

			for (int kingSquare = 0; kingSquare < 64; kingSquare++)
			{
				Bitboard kingMoveSquareBB = BB::LookupTables::boardSquares[kingSquare];

				if (kingMoves & kingMoveSquareBB)
				{
					md.targetSquare	   = kingSquare;
					md.capturedPieceBB = nullptr; // reset capture

					if (BB::LookupTables::boardSquares[kingSquare] & mBlackPiecesBB)
						md.capturedPieceBB = getPieceBitboard(kingSquare, SIDE_BLACK);

					mWhiteMoves.push_back(md);
				}
			}
		}
		else if (squareBB & mWhitePawnsBB)
		{
			Bitboard pawnMoves = MoveGenerator::computePseudoPawnMoves(square, SIDE_WHITE, mBlackPiecesBB, mOccupiedBB);
			md.pieceBB = &mWhitePawnsBB;

			for (int pawnSquare = 0; pawnSquare < 64; pawnSquare++)
			{
				Bitboard pawnMoveSquareBB = BB::LookupTables::boardSquares[pawnSquare];

				if (pawnMoves & pawnMoveSquareBB)
				{
					md.targetSquare = pawnSquare;
					md.capturedPieceBB = nullptr;

					if (BB::LookupTables::boardSquares[pawnSquare] & mBlackPiecesBB)
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

	Bitboard originTarget = BB::LookupTables::boardSquares[moveData->originSquare] ^ BB::LookupTables::boardSquares[moveData->targetSquare];
	*moveData->pieceBB	 ^= originTarget;
//	*moveData->colourBB	 ^= originTarget;

	// also updated all occupied pieces bitboard

	return false;
}

bool Board::unmakeMove(MoveData* moveData)
{
	return false;
}

void Board::print()
{
	// printing right to left 
	int file = 0;
	int startOfRank = 56; // eighth rank
	while (startOfRank >= 0)
	{
		// calculates if a bit is set in that digit of the bitboard
		if ((mWhitePawnsBB & BB::LookupTables::boardSquares[file + startOfRank]) != 0)
			std::cout << "P";
		else if ((mBlackPawnsBB & BB::LookupTables::boardSquares[file + startOfRank]) != 0)
			std::cout << "p";
		else if ((mWhiteRooksBB & BB::LookupTables::boardSquares[file + startOfRank]) != 0)
			std::cout << "R";
		else if ((mBlackRooksBB & BB::LookupTables::boardSquares[file + startOfRank]) != 0)
			std::cout << "r";
		else if ((mWhiteKnightsBB & BB::LookupTables::boardSquares[file + startOfRank]) != 0)
			std::cout << "N";
		else if ((mBlackKnightsBB & BB::LookupTables::boardSquares[file + startOfRank]) != 0)
			std::cout << "n";
		else if ((mWhiteBishopsBB & BB::LookupTables::boardSquares[file + startOfRank]) != 0)
			std::cout << "B";
		else if ((mBlackBishopsBB & BB::LookupTables::boardSquares[file + startOfRank]) != 0)
			std::cout << "b";
		else if ((mWhiteQueensBB & BB::LookupTables::boardSquares[file + startOfRank]) != 0)
			std::cout << "Q";
		else if ((mBlackQueensBB & BB::LookupTables::boardSquares[file + startOfRank]) != 0)
			std::cout << "q";
		else if ((mWhiteKingBB & BB::LookupTables::boardSquares[file + startOfRank]) != 0)
			std::cout << "K";
		else if ((mBlackKingBB & BB::LookupTables::boardSquares[file + startOfRank]) != 0)
			std::cout << "k";
		else
			std::cout << "0";

		if ((file + 1 + startOfRank) % 8 == 0)
		{
			std::cout << std::endl;
			file = 0;
			startOfRank -= 8;
		}
		else
			file++;
	}
}