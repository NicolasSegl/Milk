#include "Board.h"
#include "MoveGenerator.h"

#include <iostream>
#include <cmath>
#include <random>

// initialize all of the default values for the bitboard
void Board::setBitboards()
{
	/* setting starting positions of all pieces on the board */

	// white pieces
	currentPosition.whitePawnsBB   = BB::Constants::cWPawnsStartBB;
	currentPosition.whiteRooksBB   = BB::Constants::cWRooksStartBB;
	currentPosition.whiteKnightsBB = BB::Constants::cWKnightsStartBB;
	currentPosition.whiteBishopsBB = BB::Constants::cWBishopsStartBB;
	currentPosition.whiteQueensBB  = BB::Constants::cWQueensStartBB;
	currentPosition.whiteKingBB    = BB::Constants::cWKingStartBB;
	currentPosition.whitePiecesBB  = currentPosition.whitePawnsBB | currentPosition.whiteRooksBB | currentPosition.whiteKnightsBB | 
									 currentPosition.whiteBishopsBB | currentPosition.whiteQueensBB | currentPosition.whiteKingBB;

	// black pieces
	currentPosition.blackPawnsBB   = BB::Constants::cBPawnsStartBB;
	currentPosition.blackRooksBB   = BB::Constants::cBRooksStartBB;
	currentPosition.blackKnightsBB = BB::Constants::cBKnightsStartBB;
	currentPosition.blackBishopsBB = BB::Constants::cBBishopsStartBB;
	currentPosition.blackQueensBB  = BB::Constants::cBQueensStartBB;
	currentPosition.blackKingBB    = BB::Constants::cBKingStartBB;
	currentPosition.blackPiecesBB  = currentPosition.blackPawnsBB | currentPosition.blackRooksBB | currentPosition.blackKnightsBB | 
									 currentPosition.blackBishopsBB | currentPosition.blackQueensBB | currentPosition.blackKingBB;

	currentPosition.occupiedBB = currentPosition.whitePiecesBB | currentPosition.blackPiecesBB;
	currentPosition.emptyBB = ~currentPosition.occupiedBB;

	currentPosition.castlePrivileges = ~(0); // full priveleges for both sides at the start of the game
}

void Board::init()
{
	setBitboards();
	BB::initialize();
	mMoveGenerator.init();
}

void Board::calculateSideMoves(Colour side)
{
	std::vector<MoveData>& sideMovesRef = side == SIDE_WHITE ? mWhiteMoves : mBlackMoves;
    // should probably just calculate sideMovsRef in MoveGenerator::calculateSideMoves()
    mMoveGenerator.calculateSideMoves(this, side, sideMovesRef);
}

void Board::calculateSideMovesCapturesOnly(Colour side)
{
	std::vector<MoveData>& sideMovesRef = side == SIDE_WHITE ? mWhiteMoves : mBlackMoves;
	mMoveGenerator.calculateSideMoves(this, side, sideMovesRef, true);
}

void Board::setCastleMoveData(MoveData* castleMoveData, MoveData* kingMD, MoveData* rookMD)
{
	kingMD->originSquare = castleMoveData->originSquare;
	kingMD->targetSquare = castleMoveData->targetSquare;

    if (castleMoveData->side == SIDE_WHITE)
    {
        kingMD->pieceBB  = &currentPosition.whiteKingBB;
        kingMD->colourBB = &currentPosition.whitePiecesBB;

        rookMD->pieceBB  = &currentPosition.whiteRooksBB;
        rookMD->colourBB = &currentPosition.whitePiecesBB;
    }
    else // the move is from the black side
    {
        kingMD->pieceBB  = &currentPosition.blackKingBB;
        kingMD->colourBB = &currentPosition.blackPiecesBB;

        rookMD->pieceBB  = &currentPosition.blackRooksBB;
        rookMD->colourBB = &currentPosition.blackPiecesBB;
    }
}

bool Board::makeCastleMove(MoveData* md)
{
    static MoveData kingMove;
    static MoveData rookMove;
    setCastleMoveData(md, &kingMove, &rookMove);

    if (md->moveType == MoveData::EncodingBits::SHORT_CASTLE)
    {
        // prevent castling if squares are under attack
        for (int square = 0; square <= 2; square++)
            if (squareAttacked(md->originSquare + square, !md->side))
                return false;
        
        rookMove.originSquare = md->originSquare + 3;
        rookMove.targetSquare = md->targetSquare - 1;
    }
    else if (md->moveType == MoveData::EncodingBits::LONG_CASTLE)
    {
        // prevent castling if squares are under attack
        for (int square = 0; square <= 2; square++)
            if (squareAttacked(md->originSquare - square, !md->side))
                return false;
        
        rookMove.originSquare = md->originSquare - 4;
        rookMove.targetSquare = md->targetSquare + 1;
    }
    else
        return false;

    makeMove(&kingMove);
    makeMove(&rookMove);

    return true;
}

void Board::updateBitboardWithMove(MoveData* moveData)
{
	// our moves are pseudo legal, meaning we must also check to see if a check is actually preventing these moves. use attack tables?
	// maybe index an attack table and see if any piece was attacking the tile just moved from. if so, check if there is a check (reupdate attack table)
	Bitboard origin = BB::boardSquares[moveData->originSquare];
	Bitboard target = BB::boardSquares[moveData->targetSquare];
	Bitboard originTarget = origin ^ target; // 0s on from and to, 1s on everything else

	*moveData->pieceBB ^= originTarget;
	*moveData->colourBB ^= originTarget;

	if (moveData->capturedPieceBB) // if a piece was captured
	{
		if (moveData->moveType != MoveData::EncodingBits::EN_PASSANT_CAPTURE)
		{
			*moveData->capturedPieceBB ^= target; // only the target's square will have changed
			*moveData->capturedColourBB ^= target; // only the target's square will have changed

			currentPosition.occupiedBB ^= origin; // only the origin square is no longer occupied
			currentPosition.emptyBB	   ^= origin; // only the origin square is no longer occupied
		}
		else
		{
			// as the target square is not actually the location of the piece for an en passant capture, we need to find the actual piece we're getting rid 
			Bitboard capturedPieceBB = moveData->side == SIDE_WHITE ? (target >> 8) : (target << 8);
			*moveData->capturedPieceBB  ^= capturedPieceBB;
			*moveData->capturedColourBB ^= capturedPieceBB;
			currentPosition.occupiedBB ^= origin | capturedPieceBB | target; // get rid of the origin, the captured piece, and fill in where the pawn once was
			currentPosition.emptyBB	   ^= origin | capturedPieceBB | target;
		}
	}
	else
	{
		currentPosition.occupiedBB ^= originTarget;
		currentPosition.emptyBB	   ^= originTarget;
	}
}

Byte Board::computedKingSquare(Bitboard kingBB)
{
    for (int i = 0; i < 64; i++)
        if (BB::boardSquares[i] & kingBB)
            return i;
    
    return -1;
}

bool Board::squareAttacked(Byte square, Colour attackingSide)
{
    Bitboard opPawnsBB = attackingSide   == SIDE_WHITE ? currentPosition.whitePawnsBB : currentPosition.blackPawnsBB;
	// this is looking at the tiles that can be attacked FROM THE POSITION OF THE KING, IF IT WERE A PAWN
	// but we need to look at where the king could be attacked from (that pawn would be +- 7 or +- 9 from the king's pos)
    if (mMoveGenerator.pawnAttackLookupTable[attackingSide][square] & opPawnsBB) return true; // it's here to fix the king putting itself into check 
    
    Bitboard opKnightsBB = attackingSide == SIDE_WHITE ? currentPosition.whiteKnightsBB : currentPosition.blackKnightsBB;
    if (mMoveGenerator.knightLookupTable[square] & opKnightsBB)                  return true;
    
    Bitboard opKingsBB = attackingSide    == SIDE_WHITE ? currentPosition.whiteKingBB : currentPosition.blackKingBB;
    if (mMoveGenerator.kingLookupTable[square] & opKingsBB)                      return true;
    
    Bitboard opBishopsQueensBB = attackingSide == SIDE_WHITE ? currentPosition.whiteBishopsBB | currentPosition.whiteQueensBB  
															 : currentPosition.blackBishopsBB | currentPosition.blackQueensBB;
    // we need the queen lookup table
    // but instead a bishop lookuptable and a rook lookup table!
    Bitboard enemyPieces    = attackingSide == SIDE_WHITE ? currentPosition.whitePiecesBB : currentPosition.blackPiecesBB;
    Bitboard friendlyPieces = attackingSide == SIDE_WHITE ? currentPosition.blackPiecesBB : currentPosition.whitePiecesBB;
    if (mMoveGenerator.computePseudoBishopMoves(square, enemyPieces, friendlyPieces) & opBishopsQueensBB) return true;
    
    Bitboard opRooksQueens = attackingSide == SIDE_WHITE ? currentPosition.whiteRooksBB | currentPosition.whiteQueensBB  
														 : currentPosition.blackRooksBB | currentPosition.blackQueensBB;
    if (mMoveGenerator.computePseudoRookMoves(square, enemyPieces, friendlyPieces) & opRooksQueens) return true;
    
    return false;
}

// the ai 
bool Board::makeMove(MoveData* moveData)
{
	if (moveData->moveType == MoveData::EncodingBits::SHORT_CASTLE || moveData->moveType == MoveData::EncodingBits::LONG_CASTLE)
	{
		if (makeCastleMove(moveData))
		{
			currentPosition.castlePrivileges &= ~moveData->castlePrivilegesRevoked;
			return true;
		}
		else
			return false;
	}

	currentPosition.castlePrivileges &= ~moveData->castlePrivilegesRevoked;

	currentPosition.enPassantBB = 0;
	if (moveData->pieceBB == &currentPosition.whitePawnsBB && moveData->targetSquare - moveData->originSquare == 16) // if move is en passant
		currentPosition.enPassantBB |= BB::boardSquares[moveData->targetSquare - 8];
	else if (moveData->pieceBB == &currentPosition.blackPawnsBB && moveData->originSquare - moveData->targetSquare == 16)
	{
		currentPosition.enPassantBB |= BB::boardSquares[moveData->targetSquare + 8];
	}

	updateBitboardWithMove(moveData);

	Byte kingSquare = computedKingSquare(moveData->side == SIDE_WHITE ? currentPosition.whiteKingBB : currentPosition.blackKingBB);

	if (squareAttacked(kingSquare, !moveData->side))
	{
		unmakeMove(moveData);
		return false;
	}

	return true;
}

bool Board::unmakeMove(MoveData* moveData)
{
	if (moveData->moveType == MoveData::EncodingBits::SHORT_CASTLE || moveData->moveType == MoveData::EncodingBits::LONG_CASTLE)
	{
		makeCastleMove(moveData);
		currentPosition.castlePrivileges ^= moveData->castlePrivilegesRevoked;
		return true;
	}
    
	currentPosition.castlePrivileges ^= moveData->castlePrivilegesRevoked;
	currentPosition.enPassantBB       = moveData->enPassantBB;
    
	undoPromotion(moveData);
	
	updateBitboardWithMove(moveData);

	return true;
}

void Board::undoPromotion(MoveData* moveData)
{
	switch (moveData->moveType)
	{
		case MoveData::EncodingBits::QUEEN_PROMO:
		{
			if (moveData->side == SIDE_WHITE) currentPosition.whiteQueensBB ^= BB::boardSquares[moveData->targetSquare];
			else							  currentPosition.blackQueensBB ^= BB::boardSquares[moveData->targetSquare];
			break;
		}
		case MoveData::EncodingBits::BISHOP_PROMO:
		{
			if (moveData->side == SIDE_WHITE) currentPosition.whiteBishopsBB ^= BB::boardSquares[moveData->targetSquare];
			else							  currentPosition.blackBishopsBB ^= BB::boardSquares[moveData->targetSquare];
			break;
		}
		case MoveData::EncodingBits::ROOK_PROMO:
		{
			if (moveData->side == SIDE_WHITE) currentPosition.whiteRooksBB ^= BB::boardSquares[moveData->targetSquare];
			else							  currentPosition.blackRooksBB ^= BB::boardSquares[moveData->targetSquare];
			break;
		}
		case MoveData::EncodingBits::KNIGHT_PROMO:
		{
			if (moveData->side == SIDE_WHITE) currentPosition.whiteKnightsBB ^= BB::boardSquares[moveData->targetSquare];
			else							  currentPosition.blackKnightsBB ^= BB::boardSquares[moveData->targetSquare];
			break;
		}
		default:
			break;
	}
	if (moveData->moveType == MoveData::EncodingBits::BISHOP_PROMO || moveData->moveType == MoveData::EncodingBits::ROOK_PROMO ||
		moveData->moveType == MoveData::EncodingBits::KNIGHT_PROMO || moveData->moveType == MoveData::EncodingBits::QUEEN_PROMO)
	{
		if (moveData->side == SIDE_WHITE) currentPosition.whitePawnsBB ^= BB::boardSquares[moveData->targetSquare];
		else							  currentPosition.blackPawnsBB ^= BB::boardSquares[moveData->targetSquare];
	}
}

void Board::promotePiece(MoveData* md, MoveData::EncodingBits promoteTo)
{
	md->setMoveType(promoteTo);

	if (md->side == SIDE_WHITE)
	{
		currentPosition.whitePawnsBB ^= BB::boardSquares[md->targetSquare];

		if (promoteTo == MoveData::EncodingBits::QUEEN_PROMO)		currentPosition.whiteQueensBB |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::ROOK_PROMO)	currentPosition.whiteRooksBB |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::BISHOP_PROMO) currentPosition.whiteBishopsBB |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::KNIGHT_PROMO) currentPosition.whiteKnightsBB |= BB::boardSquares[md->targetSquare];
	}
	else // side == SIDE_BLACK
	{
		currentPosition.blackPawnsBB ^= BB::boardSquares[md->targetSquare];

		if (promoteTo == MoveData::EncodingBits::QUEEN_PROMO)		currentPosition.blackQueensBB |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::ROOK_PROMO)	currentPosition.blackRooksBB |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::BISHOP_PROMO) currentPosition.blackBishopsBB |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::KNIGHT_PROMO) currentPosition.blackKnightsBB |= BB::boardSquares[md->targetSquare];
	}
}

std::vector<MoveData>& Board::getMoves(Colour side)
{
    if (side == SIDE_WHITE) return mWhiteMoves;
    else                    return mBlackMoves;
}
