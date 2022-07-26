#pragma once
#include "Bitboard.h"
#include "MoveData.h"

// the ai may not have to check all forms of a draw
// we can just say that if no moves pass through at all and the king is not in check, then it's a draw, return 0

struct ChessPosition
{
	// maybe make a struct in Bitboard.h with all 14 bitboards? then board can have one. would make writing
	// this sturct a hell of a lot easier. lots of rewriting tho. idk if that's goofy or not ?
	// in board, could call it mChessPosition ? it would contain the en passant bb, pieces bb, castle privileges, and the side to move? 

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

	Bitboard occupiedBB  = 0;
	Bitboard emptyBB	 = 0;
	//Bitboard enPassantBB = 0;

	Byte castlePrivileges;
	Byte fiftyMoveCounter = 0;
	Byte enPassantSquare  = 0;

	Colour sideToMove;
};