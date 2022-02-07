#include "ChessGame.h"

#include <iostream>

void ChessGame::runCLI()
{
	std::cout << "type q at any time to exit the program\n";

	while (true)
	{
		// initialize board
	}
}

// make a lookuptables with all attacks of the rooks. should be [64][4096] (2^12 = 4096). because the corner's will have 12 bits, 6 to the right and 6 to the left
// do the same but for bishops. should be much smaller 
// make other tables. nRookBits and nBishopBits. these will be 64 integers, numbering the number of bits needed for maxiumum moves for either a rook or a bishop
// blokers dont matter if they're on the end ? at the end of file or end of rank
// we have to mask these depending on the square
// learn about perfect hashing. 
// https://www.chessprogramming.org/Magic_Bitboards