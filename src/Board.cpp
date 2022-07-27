#include "Board.h"
#include "MoveGenerator.h"
#include "Outcomes.h"

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
}

void Board::init()
{
	setBitboards();
	BB::initialize();
	mMoveGenerator.init();

	mZobristKeyGenerator.initHashKeys();
	mCurrentZobristKey = mZobristKeyGenerator.generateKey(&currentPosition);
	mPly = -1;
	insertMoveIntoHistory();
	currentPosition.castlePrivileges = (Byte)CastlingPrivilege::WHITE_SHORT_CASTLE | (Byte)CastlingPrivilege::WHITE_LONG_CASTLE |
		(Byte)CastlingPrivilege::BLACK_SHORT_CASTLE | (Byte)CastlingPrivilege::BLACK_LONG_CASTLE;
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
	rookMD->setMoveType(MoveData::EncodingBits::CASTLE_HALF_MOVE);
	kingMD->setMoveType(MoveData::EncodingBits::CASTLE_HALF_MOVE);
	// fix these !

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

	// checking if the move is an unmake move by seeing if there is already a rook castled (i.e. in the same position that this move will take this rook)
	// this works because unmaking castle moves uses this exact same routine and thanks to bit math it would undo the move 
	if (BB::boardSquares[rookMove.targetSquare] & *rookMove.pieceBB)
	{
		unmakeMove(&kingMove);
		unmakeMove(&rookMove);
		return true;
	}
	else
	{
		makeMove(&kingMove);
		makeMove(&rookMove);
	}

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
	Bitboard opPawnsBB = attackingSide == SIDE_WHITE ? currentPosition.whitePawnsBB : currentPosition.blackPawnsBB;

	// this gets all of the diagonal attacks of the pawns
	if (attackingSide == SIDE_WHITE)
	{
		if ((BB::boardSquares[square] & (opPawnsBB << 7)) || (BB::boardSquares[square] & (opPawnsBB << 9))) return true;
	}
	else
	{
		if ((BB::boardSquares[square] & (opPawnsBB >> 7)) || (BB::boardSquares[square] & (opPawnsBB >> 9))) return true;
	}
    
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

void Board::setEnPassantSquares(MoveData* moveData)
{
	currentPosition.enPassantSquare = 0;
	if (moveData->pieceBB == &currentPosition.whitePawnsBB && moveData->targetSquare - moveData->originSquare == 16) // if move is en passant
		currentPosition.enPassantSquare = moveData->targetSquare - 8;
	else if (moveData->pieceBB == &currentPosition.blackPawnsBB && moveData->originSquare - moveData->targetSquare == 16)
		currentPosition.enPassantSquare = moveData->targetSquare + 8;
}

// more moves are getting deleted than inserted. this should never happen.
// i would guess it's because in makeMove, sometimes this function is never
// called, however in unmakemove, it is called every time without fail?
// but we only ever want to call it if it was called while making the move in the first place

void Board::insertMoveIntoHistory()
{
	mPly++;
	mZobristKeyHistory[mPly] = mCurrentZobristKey;
}

void Board::deleteMoveFromHistory()
{
	mZobristKeyHistory[mPly] = 0;
	mPly--;
}

bool Board::makeMove(MoveData* moveData)
{
	if (moveData->moveType == MoveData::EncodingBits::SHORT_CASTLE || moveData->moveType == MoveData::EncodingBits::LONG_CASTLE)
	{
		if (!makeCastleMove(moveData))
			return false;
	}
	else
	{
		updateBitboardWithMove(moveData);

		Byte kingSquare = computedKingSquare(moveData->side == SIDE_WHITE ? currentPosition.whiteKingBB : currentPosition.blackKingBB);

		if (squareAttacked(kingSquare, !moveData->side))
		{
			currentPosition.castlePrivileges &= ~moveData->castlePrivilegesRevoked;
			// passing in false so that we do not update the zobrist key/history, as we never actually added it here
			// this means that otherwise it would be removing the previous zobrist key, eventually giving negative 
			// ply numbers and undefined behaviour
			unmakeMove(moveData, false);
			return false;
		}
	}

	setEnPassantSquares(moveData);

	currentPosition.castlePrivileges &= ~moveData->castlePrivilegesRevoked;
	if (moveData->moveType != MoveData::EncodingBits::CASTLE_HALF_MOVE) // ctrl+f all MoveData::EncodingBits calls
	{
		currentPosition.sideToMove = !currentPosition.sideToMove;
		mCurrentZobristKey = mZobristKeyGenerator.updateKey(mCurrentZobristKey, &currentPosition, moveData);
		insertMoveIntoHistory();
	}
	else
		mCurrentZobristKey = mZobristKeyGenerator.updateKey(mCurrentZobristKey, &currentPosition, moveData);

	return true;
}

bool Board::unmakeMove(MoveData* moveData, bool updateZobristHistory)
{
	// undoing moves is no longer returning the taken pieces back ? 
	if (moveData->moveType == MoveData::EncodingBits::SHORT_CASTLE || moveData->moveType == MoveData::EncodingBits::LONG_CASTLE)
		makeCastleMove(moveData);
	else
	{
		undoPromotion(moveData);
		updateBitboardWithMove(moveData);
	}

	if (updateZobristHistory)
		mCurrentZobristKey = mZobristKeyHistory[mPly - 1];

	currentPosition.enPassantSquare = moveData->enPassantSquare;
	currentPosition.castlePrivileges ^= moveData->castlePrivilegesRevoked;

	if (moveData->moveType != MoveData::EncodingBits::CASTLE_HALF_MOVE) // ctrl+f all MoveData::EncodingBits calls
	{
		currentPosition.sideToMove = !currentPosition.sideToMove;
		if (updateZobristHistory)
			deleteMoveFromHistory();
	}

	return true;
}

void Board::undoPromotion(MoveData* moveData)
{
	if (moveData->moveType == MoveData::EncodingBits::BISHOP_PROMO || moveData->moveType == MoveData::EncodingBits::ROOK_PROMO ||
		moveData->moveType == MoveData::EncodingBits::KNIGHT_PROMO || moveData->moveType == MoveData::EncodingBits::QUEEN_PROMO)
	{
		if (moveData->side == SIDE_WHITE) currentPosition.whitePawnsBB ^= BB::boardSquares[moveData->targetSquare];
		else							  currentPosition.blackPawnsBB ^= BB::boardSquares[moveData->targetSquare];
	}
	else
		return;

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
}

void Board::promotePiece(MoveData* md, MoveData::EncodingBits promoteTo)
{
	md->setMoveType(promoteTo);

	// make this a switch statement?
	if (md->side == SIDE_WHITE)
	{
		currentPosition.whitePawnsBB ^= BB::boardSquares[md->targetSquare];

		if		(promoteTo == MoveData::EncodingBits::QUEEN_PROMO)	currentPosition.whiteQueensBB |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::ROOK_PROMO)	currentPosition.whiteRooksBB |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::BISHOP_PROMO) currentPosition.whiteBishopsBB |= BB::boardSquares[md->targetSquare];
		else if (promoteTo == MoveData::EncodingBits::KNIGHT_PROMO) currentPosition.whiteKnightsBB |= BB::boardSquares[md->targetSquare];
	}
	else // side == SIDE_BLACK
	{
		currentPosition.blackPawnsBB ^= BB::boardSquares[md->targetSquare];

		if		(promoteTo == MoveData::EncodingBits::QUEEN_PROMO)	currentPosition.blackQueensBB |= BB::boardSquares[md->targetSquare];
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
