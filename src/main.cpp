#include "ChessGame.h"

#include <random>
#include <time.h>

// I LOVE YOU CHARLOTTE <3 I LOVE YOU MORE!!!!

int main()
{
	std::srand(time(NULL));

	ChessGame cg;
	cg.runGUI();
	return 0;
}

// TODO
// king can put itself into check (if its a pawn attacking the square)
//	this is because it is just checking where the pawns we have can attack, so &ing it doesnt actually do anything
// before transposition table, do drawing/checkmate. before/after that (but before transposition table) fix the kind putting itself in checm Board.cpp line 165ish
// TRANSPOSITION TABLE
// add delta pruning
// and ENDGAME PLAY
// transposition table
// pawn structure https://www.chessprogramming.org/Pawn_Pattern_and_Properties
// move ordering
//  killer move heuristic!
//  historic heuristic
// quiscience search --- currently working on this
//	consider also pawn promotions/advances
// null moves after this ! ! ! after quiet move search
// futility pruning
// figure out how this actually works so we aren't just adding it in brainlessly eh?

// make all structures as small as possible. make movedata use bits set to get data
// refactor everything so that all functions are short and do only one thing, and that methods are in appropriate classes
// for the above maybe look to other chess engines online. some have classes only for Search and one for making Moves, etc
// null moves
// extensions
// search windows
//  one simple extension is to simply increase depth by 1 when under check
// reductions
// menus
// choosing what to upgrade a pawn to
// undoing ai moves

// when checking draws, first check if all black pieces are even in the same spot and if all black pieces are in the same spot
// this can be done by comparing the values of blackpiecesbb and whitepiecesbb
// then check the half move counter