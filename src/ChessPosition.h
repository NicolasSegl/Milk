#pragma once
#include "Bitboard.h"

// the ai may not have to check all forms of a draw
// we can just say that if no moves pass through at all and the king is not in check, then it's a draw, return 0

struct ChessPosition
{
	// maybe make a struct in Bitboard.h with all 14 bitboards? then board can have one. would make writing
	// this sturct a hell of a lot easier. lots of rewriting tho. idk if that's goofy or not ?
	// in board, could call it mChessPosition ? it would contain the en passant bb, pieces bb, castle privileges, and the side to move? 
};