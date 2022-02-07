#pragma once

#include <vector>

#include "Bitboard.h"

class Board
{
private:
	// white piece bitboards
	Bitboard mWhitePiecesBB;
	Bitboard mWhitePawnsBB;
	Bitboard mWhiteRooksBB;
	Bitboard mWhiteKnightsBB;
	Bitboard mWhiteBishopsBB;
	Bitboard mWhiteQueensBB;
	Bitboard mWhiteKingBB;

	// black piece bitboards
	Bitboard mBlackPiecesBB;
	Bitboard mBlackPawnsBB;
	Bitboard mBlackRooksBB;
	Bitboard mBlackKnightsBB;
	Bitboard mBlackBishopsBB;
	Bitboard mBlackQueensBB;
	Bitboard mBlackKingBB;

	Bitboard mOccupiedBB     = 0;
	Bitboard mEmptyBB        = 0;
	// attack/possible moves bb
	// defence tables

	Bitboard mAttackTable[64]{ 0 };

	std::vector<MoveData> mWhiteMoves;
	std::vector<MoveData> mBlackMoves;
	
	// compare which moves are defending a piece of their own :D
	// by separating all valid moves with those moves that attack pieces, we can hasten the search algorithm. consider moves that take first 
	// middle of the board bitboards (to encourage control of center) 4x4 grid in middle maybe?
	// array of 64 bitboards. each with exactly 1 square set to 1. could be very useful
	// make them all constats. for initializing the bitboards ?

	// if it turns out we need to get blokers and use clear files or clear ranks or a mask of them, put them into an array if it makes it easier .
	// for now can leave it as is tho

	void setBitboards();
	Bitboard* getPieceBitboard(Byte square, Colour side = -1); // -1 indicates that no value was passed in 

	// an array of 64 bitboards, each representing one square of the board! can & them and check if != 0 to see if that piece is in that square
	// loop i = i < 64; i++ and see if pos & whitepawns != 0. if true then pass in move generation for a pawn at that position in the board?
	// having an occupied bitboard might make checking move validity. easier. first check if that square is occupied, then the colour, then the specific piece

	// a move list. each move has all the data tho, to unmake it. 

	void print();

public:
	Board() {}

	void init();
	bool makeMove(MoveData* moveData);
	bool unmakeMove(MoveData* moveData);

	void calculateWhiteMoves();

	std::vector<MoveData>* getWhiteMoves() { return &mWhiteMoves; }
};
