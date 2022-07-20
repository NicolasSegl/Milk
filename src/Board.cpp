#include "Board.h"
#include "MoveGenerator.h"

#include <iostream>
#include <cmath>

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

    occupiedBB = whitePiecesBB | blackPiecesBB;
    emptyBB = ~occupiedBB;

    castlePrivileges = ~(0); // full priveleges for both sides at the start of the game
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
        kingMD->pieceBB  = &whiteKingBB;
        kingMD->colourBB = &whitePiecesBB;

        rookMD->pieceBB  = &whiteRooksBB;
        rookMD->colourBB = &whitePiecesBB;
    }
    else // the move is from the black side
    {
        kingMD->pieceBB  = &blackKingBB;
        kingMD->colourBB = &blackPiecesBB;

        rookMD->pieceBB  = &blackRooksBB;
        rookMD->colourBB = &blackPiecesBB;
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

			occupiedBB ^= origin; // only the origin square is no longer occupied
			emptyBB	   ^= origin; // only the origin square is no longer occupied
		}
		else
		{
			// as the target square is not actually the location of the piece for an en passant capture, we need to find the actual piece we're getting rid 
			Bitboard capturedPieceBB = moveData->side == SIDE_WHITE ? (target >> 8) : (target << 8);
			*moveData->capturedPieceBB  ^= capturedPieceBB;
			*moveData->capturedColourBB ^= capturedPieceBB;
			occupiedBB ^= origin | capturedPieceBB | target; // get rid of the origin, the captured piece, and fill in where the pawn once was
			emptyBB	   ^= origin | capturedPieceBB | target;
		}
	}
	else
	{
		occupiedBB ^= originTarget;
		emptyBB	   ^= originTarget;
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
    Bitboard opPawnsBB = attackingSide   == SIDE_WHITE ? whitePawnsBB : blackPawnsBB;
	// this is looking at the tiles that can be attacked FROM THE POSITION OF THE KING, IF IT WERE A PAWN
	// but we need to look at where the king could be attacked from (that pawn would be +- 7 or +- 9 from the king's pos)
    if (mMoveGenerator.pawnAttackLookupTable[attackingSide][square] & opPawnsBB) return true; // it's here to fix the king putting itself into check 
    
    Bitboard opKnightsBB = attackingSide == SIDE_WHITE ? whiteKnightsBB : blackKnightsBB;
    if (mMoveGenerator.knightLookupTable[square] & opKnightsBB)                  return true;
    
    Bitboard opKingsBB = attackingSide    == SIDE_WHITE ? whiteKingBB : blackKingBB;
    if (mMoveGenerator.kingLookupTable[square] & opKingsBB)                      return true;
    
    Bitboard opBishopsQueensBB = attackingSide == SIDE_WHITE ? whiteBishopsBB | whiteQueensBB : blackBishopsBB | blackQueensBB;
    // we need the queen lookup table
    // but instead a bishop lookuptable and a rook lookup table!
    Bitboard enemyPieces    = attackingSide == SIDE_WHITE ? whitePiecesBB : blackPiecesBB;
    Bitboard friendlyPieces = attackingSide == SIDE_WHITE ? blackPiecesBB : whitePiecesBB;
    if (mMoveGenerator.computePseudoBishopMoves(square, enemyPieces, friendlyPieces) & opBishopsQueensBB) return true;
    
    Bitboard opRooksQueens = attackingSide == SIDE_WHITE ? whiteRooksBB | whiteQueensBB : blackRooksBB | blackQueensBB;
    if (mMoveGenerator.computePseudoRookMoves(square, enemyPieces, friendlyPieces) & opRooksQueens) return true;
    
    return false;
}

bool Board::makeMove(MoveData* moveData)
{
	if (moveData->moveType == MoveData::EncodingBits::SHORT_CASTLE || moveData->moveType == MoveData::EncodingBits::LONG_CASTLE)
	{
		if (makeCastleMove(moveData))
		{
			castlePrivileges &= ~moveData->castlePrivilegesRevoked;
			return true;
		}
		else
			return false;
	}

	castlePrivileges &= ~moveData->castlePrivilegesRevoked;

	enPassantBB = 0;
	if (moveData->pieceBB == &whitePawnsBB && moveData->targetSquare - moveData->originSquare == 16) // if move is en passant
		enPassantBB |= BB::boardSquares[moveData->targetSquare - 8];
	else if (moveData->pieceBB == &blackPawnsBB && moveData->originSquare - moveData->targetSquare == 16)
		enPassantBB |= BB::boardSquares[moveData->targetSquare + 8];

	updateBitboardWithMove(moveData);

	Byte kingSquare = computedKingSquare(moveData->side == SIDE_WHITE ? whiteKingBB : blackKingBB);

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
		// this needs to somehow give perms back?
		castlePrivileges ^= moveData->castlePrivilegesRevoked;
		return true;
	}
    
	castlePrivileges ^= moveData->castlePrivilegesRevoked;
	enPassantBB      = moveData->enPassantBB;
    
	undoPromotion(moveData);
	
	updateBitboardWithMove(moveData);

	return false;
}

void Board::undoPromotion(MoveData* moveData)
{
	switch (moveData->moveType)
	{
		case MoveData::EncodingBits::QUEEN_PROMO:
		{
			if (moveData->side == SIDE_WHITE) whiteQueensBB ^= BB::boardSquares[moveData->targetSquare];
			else							  blackQueensBB ^= BB::boardSquares[moveData->targetSquare];
			break;
		}
		case MoveData::EncodingBits::BISHOP_PROMO:
		{
			if (moveData->side == SIDE_WHITE) whiteBishopsBB ^= BB::boardSquares[moveData->targetSquare];
			else							  blackBishopsBB ^= BB::boardSquares[moveData->targetSquare];
			break;
		}
		case MoveData::EncodingBits::ROOK_PROMO:
		{
			if (moveData->side == SIDE_WHITE) whiteRooksBB ^= BB::boardSquares[moveData->targetSquare];
			else							  blackRooksBB ^= BB::boardSquares[moveData->targetSquare];
			break;
		}
		case MoveData::EncodingBits::KNIGHT_PROMO:
		{
			if (moveData->side == SIDE_WHITE) whiteKnightsBB ^= BB::boardSquares[moveData->targetSquare];
			else							  blackKnightsBB ^= BB::boardSquares[moveData->targetSquare];
			break;
		}
		default:
			break;
	}
	if (moveData->moveType == MoveData::EncodingBits::BISHOP_PROMO || moveData->moveType == MoveData::EncodingBits::ROOK_PROMO ||
		moveData->moveType == MoveData::EncodingBits::KNIGHT_PROMO || moveData->moveType == MoveData::EncodingBits::QUEEN_PROMO)
	{
		if (moveData->side == SIDE_WHITE) whitePawnsBB ^= BB::boardSquares[moveData->targetSquare];
		else							  blackPawnsBB ^= BB::boardSquares[moveData->targetSquare];
	}
}

void Board::promotePiece(MoveData* md, MoveData::EncodingBits promoteTo)
{
	md->setMoveType(promoteTo);

	if (md->side == SIDE_WHITE)
	{
		whitePawnsBB ^= BB::boardSquares[md->targetSquare];

		if (promoteTo == MoveData::EncodingBits::QUEEN_PROMO)		whiteQueensBB |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::ROOK_PROMO)	whiteRooksBB |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::BISHOP_PROMO) whiteBishopsBB |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::KNIGHT_PROMO) whiteKnightsBB |= BB::boardSquares[md->targetSquare];
	}
	else // side == SIDE_BLACK
	{
		blackPawnsBB ^= BB::boardSquares[md->targetSquare];

		if (promoteTo == MoveData::EncodingBits::QUEEN_PROMO)		blackQueensBB |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::ROOK_PROMO)	blackRooksBB |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::BISHOP_PROMO) blackBishopsBB |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::KNIGHT_PROMO) blackKnightsBB |= BB::boardSquares[md->targetSquare];
	}
}

std::vector<MoveData>& Board::getMoves(Colour side)
{
    if (side == SIDE_WHITE) return mWhiteMoves;
    else                    return mBlackMoves;
}
