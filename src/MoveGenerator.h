#pragma once

#include "Bitboard.h"

class MoveGenerator
{
private:
	// can also have predefined pawn attacks 

public:
	MoveGenerator() {}

	static Bitboard computePseudoKingMoves(Byte pieceCoord, Bitboard friendlyPieces);
	static Bitboard computePseudoKnightMoves(Byte pieceCoord, Bitboard friendlyPieces);
	static Bitboard computePseudoPawnMoves(Byte pieceCoord, Colour side, Bitboard enemyPieces, Bitboard occupiedSquares);

	// static MoveData** getAllMoves (for one colour)
};

/*


is that what we need the attack bitboards for ? attack bitboards[64]
so that we can refer to a specific square and see where it can move?

we would calculate all the moves that a piece can make. then set the bitboard at that piece's index on the board
to all of those locations. when a move is specified, check that index's bitboard and see if the suggested move is
legal (or pseudolegal). if it is, simply make the move :)

*/