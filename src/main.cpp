#include "ChessGame.h"

// I LOVE YOU CHARLOTTE <3 I LOVE YOU MORE!!!!

int main()
{
	ChessGame cg;
	cg.runGUI();
	return 0;
}

// TODO
// move ordering
//  killer move heuristic!
//  historic heuristic
// quiscience search --- currently working on this
// figure out how this actually works so we aren't just adding it in brainlessly eh?

// make all structures as small as possible. make movedata use bits set to get data
// refactor everything so that all functions are short and do only one thing, and that methods are in appropriate classes
// for the above maybe look to other chess engines online. some have classes only for Search and one for making Moves, etc
// null moves
// extensions
//  one simple extension is to simply increase depth by 1 when under check
// reductions
// menus
// choosing what to upgrade a pawn to
// undoing ai moves

// it would seem that the quiscience search is like minimax, only it considers only moves that capture, promote, or put a king in check
