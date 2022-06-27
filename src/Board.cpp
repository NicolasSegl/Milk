#include "Board.h"
#include "MoveGenerator.h"

#include <iostream>

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

    mMovePrivileges = ~(0); // full priveleges for both sides at the start of the game

	//mMovePrivileges = 0;
}

void Board::init()
{
	setBitboards();
	BB::initialize();
	mMoveGenerator.init();
}

// side is a default value with a value of -1. this value indicates no side was specified and to search all bitboards
Bitboard* Board::getPieceBitboard(Byte square, Colour side)
{
	Bitboard squareBB = BB::boardSquares[square];

	if (side == SIDE_WHITE || side == -1)
	{
		if	    (squareBB & whitePawnsBB)	 return &whitePawnsBB;
		else if (squareBB & whiteRooksBB)	 return &whiteRooksBB;
		else if (squareBB & whiteKnightsBB)  return &whiteKnightsBB;
		else if (squareBB & whiteBishopsBB)  return &whiteBishopsBB;
		else if (squareBB & whiteQueensBB)   return &whiteQueensBB;
		else if (squareBB & whiteKingBB)	 return &whiteKingBB;
	}
	if (side == SIDE_BLACK || side == -1)
	{
		if		(squareBB & blackPawnsBB)	 return &blackPawnsBB;
		else if (squareBB & blackRooksBB)	 return &blackRooksBB;
		else if (squareBB & blackKnightsBB)  return &blackKnightsBB;
		else if (squareBB & blackBishopsBB)  return &blackBishopsBB;
		else if (squareBB & blackQueensBB)   return &blackQueensBB;
		else if (squareBB & blackKingBB)	 return &blackKingBB;
	}

	return nullptr;
}

// attack board by taking the moves of a piece on that square, and & it with opposite colour
// attack boards might be more for evaluation than move generation. checking pins and defend maps
// selection sort as we move to put more important moves near the start of the vector
void Board::calculateSideMoves(Colour side)
{
	std::vector<MoveData>& sideMovesRef = side == SIDE_WHITE ? mWhiteMoves : mBlackMoves;

	sideMovesRef.clear();
	sideMovesRef.reserve(100);
	
	for (int square = 0; square < 64; square++)
		calculatePieceMoves(side, square, sideMovesRef);

	// check if castle moves are possible before computing them (otherwise resulting in invalid moves)
    MoveData shortCastleMD, longCastleMD;
    
	// castle moves are asshole raping the whole thing somehow

	if (side == SIDE_WHITE) // white castle moves
	{
		if (whiteKingBB == 0)
			return;

        shortCastleMD = mMoveGenerator.computeCastleMoveData(side, mMovePrivileges, occupiedBB, Privilege::WHITE_SHORT_CASTLE);
        longCastleMD  = mMoveGenerator.computeCastleMoveData(side, mMovePrivileges, occupiedBB, Privilege::WHITE_LONG_CASTLE);
	}
	else // black castle moves
	{
		if (blackKingBB == 0)
			return;

		shortCastleMD = mMoveGenerator.computeCastleMoveData(side, mMovePrivileges, occupiedBB, Privilege::BLACK_SHORT_CASTLE);
		longCastleMD  = mMoveGenerator.computeCastleMoveData(side, mMovePrivileges, occupiedBB, Privilege::BLACK_LONG_CASTLE);
	}
    
    if (shortCastleMD.moveType != MoveData::EncodingBits::INVALID) // the or is for debug
        sideMovesRef.push_back(shortCastleMD);
    if (longCastleMD.moveType != MoveData::EncodingBits::INVALID)
        sideMovesRef.push_back(longCastleMD);
}

void Board::calculatePieceMoves(Colour side, Byte originSquare, std::vector<MoveData>& moveVector)
{
	MoveData md;
	if (side == SIDE_WHITE)
	{
		md.colourBB		    = &whitePiecesBB;
		md.capturedColourBB = &blackPiecesBB; // if a capture occurred, it would be a black piece
	}
	else
	{
		md.colourBB		    = &blackPiecesBB;
		md.capturedColourBB = &whitePiecesBB;
	}
	
	if (BB::boardSquares[originSquare] & *md.colourBB)
	{
		md.side = side;
		md.originSquare = originSquare;

		Bitboard moves = 0;
		Bitboard* pieceBBPtr = getPieceBitboard(originSquare, side);

		md.pieceBB = pieceBBPtr;

		moves = calculatePsuedoMove(&md, *pieceBBPtr);

		if (moves > 0)
			findMoveCaptures(moves, md, moveVector);
	}
}

Bitboard Board::calculatePsuedoMove(MoveData* md, Bitboard& pieceBB)
{
	if ((pieceBB & whiteKnightsBB) || (pieceBB & blackKnightsBB))
		return mMoveGenerator.computePseudoKnightMoves(md->originSquare, *md->colourBB);

	else if ((pieceBB & whitePawnsBB) || (pieceBB & blackPawnsBB))     
		return mMoveGenerator.computePseudoPawnMoves(md->originSquare, md->side, *md->capturedColourBB, emptyBB, enPassantBB);

	else if ((pieceBB & whiteBishopsBB) || (pieceBB & blackBishopsBB)) 
		return mMoveGenerator.computePseudoBishopMoves(md->originSquare, *md->capturedColourBB, *md->colourBB);

	else if ((pieceBB & whiteQueensBB) || (pieceBB & blackQueensBB))  
		return mMoveGenerator.computePseudoQueenMoves(md->originSquare, *md->capturedColourBB, *md->colourBB);

	else if ((pieceBB & whiteRooksBB) || (pieceBB & blackRooksBB)) // these moves may change castling privileges
	{
		if (md->side == SIDE_WHITE && md->originSquare == 0 && (mMovePrivileges & (Byte)Privilege::WHITE_LONG_CASTLE))
			md->privilegesRevoked |= (Byte)Privilege::WHITE_LONG_CASTLE;
		else if (md->side == SIDE_WHITE && md->originSquare == 7 && (mMovePrivileges & (Byte)Privilege::WHITE_SHORT_CASTLE))
			md->privilegesRevoked |= (Byte)Privilege::WHITE_SHORT_CASTLE;
		else if (md->side == SIDE_BLACK && md->originSquare == 56 && (mMovePrivileges & (Byte)Privilege::BLACK_LONG_CASTLE))
			md->privilegesRevoked |= (Byte)Privilege::BLACK_LONG_CASTLE;
		else if (md->side == SIDE_BLACK && md->originSquare == 63 && (mMovePrivileges & (Byte)Privilege::BLACK_SHORT_CASTLE))
			md->privilegesRevoked |= (Byte)Privilege::BLACK_SHORT_CASTLE;

		return mMoveGenerator.computePseudoRookMoves(md->originSquare, *md->capturedColourBB, *md->colourBB);
	}

	else if ((pieceBB & whiteKingBB) || (pieceBB & blackKingBB)) // these moves may change castling privileges
	{
		if (md->side == SIDE_WHITE && md->originSquare == 4 && (mMovePrivileges & (Byte)Privilege::WHITE_LONG_CASTLE))
			md->privilegesRevoked |= (Byte)Privilege::WHITE_LONG_CASTLE;
		if (md->side == SIDE_WHITE && md->originSquare == 4 && (mMovePrivileges & (Byte)Privilege::WHITE_SHORT_CASTLE))
			md->privilegesRevoked |= (Byte)Privilege::WHITE_SHORT_CASTLE;
		if (md->side == SIDE_BLACK && md->originSquare == 60 && (mMovePrivileges & (Byte)Privilege::BLACK_LONG_CASTLE))
			md->privilegesRevoked |= (Byte)Privilege::BLACK_LONG_CASTLE;
		if (md->side == SIDE_BLACK && md->originSquare == 60 && (mMovePrivileges & (Byte)Privilege::BLACK_SHORT_CASTLE))
			md->privilegesRevoked |= (Byte)Privilege::BLACK_SHORT_CASTLE;

		return mMoveGenerator.computePseudoKingMoves(md->originSquare, *md->colourBB);
	}
}

void Board::findMoveCaptures(Bitboard moves, MoveData& md, std::vector<MoveData>& moveVector)
{
	for (int square = 0; square < 64; square++)
	{
		if (moves & BB::boardSquares[square])
		{
			md.targetSquare = square;
			md.capturedPieceBB = nullptr;

			if (BB::boardSquares[square] & *md.capturedColourBB)
				md.capturedPieceBB = getPieceBitboard(square, !md.side);
			if (BB::boardSquares[square] & enPassantBB)
			{
				if (moves & enPassantBB)
				{
					if (md.pieceBB == &whitePawnsBB)
					{
						md.capturedPieceBB = getPieceBitboard(square - 8, SIDE_BLACK);
						md.setMoveType(MoveData::EncodingBits::EN_PASSANT_CAPTURE);
						md.enPassantBB = enPassantBB;
					}
					else if (md.pieceBB == &blackPawnsBB)
					{
						md.capturedPieceBB = getPieceBitboard(square + 8, SIDE_WHITE);
						md.setMoveType(MoveData::EncodingBits::EN_PASSANT_CAPTURE);
						md.enPassantBB = enPassantBB;
					}
				}
			}
			else if (enPassantBB)
				md.enPassantBB = enPassantBB;

			// remove castling privileges if a rook was captured
			if (md.capturedPieceBB == &blackRooksBB)
			{
				if (md.targetSquare == 63)		md.privilegesRevoked |= (Byte)Privilege::BLACK_SHORT_CASTLE;
				else if (md.targetSquare == 56) md.privilegesRevoked |= (Byte)Privilege::BLACK_LONG_CASTLE;
			}
			else if (md.capturedPieceBB == &whiteRooksBB)
			{
				if (md.targetSquare == 7)		md.privilegesRevoked |= (Byte)Privilege::WHITE_SHORT_CASTLE;
				else if (md.targetSquare == 0)  md.privilegesRevoked |= (Byte)Privilege::WHITE_LONG_CASTLE;
			}

			moveVector.push_back(md);
		}
	}
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
    // have to essentially make two moves. later we'll have to check their legality
    static MoveData kingMove;
    static MoveData rookMove;
    setCastleMoveData(md, &kingMove, &rookMove);

    if (md->moveType == MoveData::EncodingBits::SHORT_CASTLE)
    {
        rookMove.originSquare = md->originSquare + 3;
        rookMove.targetSquare = md->targetSquare - 1;
    }
    else if (md->moveType == MoveData::EncodingBits::LONG_CASTLE)
    {
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

bool Board::makeMove(MoveData* moveData)
{
	// maybe up here check if it would result in a check? right now all moves are just pseudo

    if (moveData->moveType == MoveData::EncodingBits::SHORT_CASTLE || moveData->moveType == MoveData::EncodingBits::LONG_CASTLE)
    {
        makeCastleMove(moveData);
		mMovePrivileges ^= moveData->privilegesRevoked;
        return true;
    }
	
	mMovePrivileges ^= moveData->privilegesRevoked;

	enPassantBB = 0;
	if (moveData->pieceBB == &whitePawnsBB && moveData->targetSquare - moveData->originSquare == 16) // if en passant ? 
		enPassantBB |= BB::boardSquares[moveData->targetSquare - 8];
	else if (moveData->pieceBB == &blackPawnsBB && moveData->originSquare - moveData->targetSquare == 16) 
		enPassantBB |= BB::boardSquares[moveData->targetSquare + 8];

	updateBitboardWithMove(moveData);

	return true;
}

bool Board::unmakeMove(MoveData* moveData)
{
	if (moveData->moveType == MoveData::EncodingBits::SHORT_CASTLE || moveData->moveType == MoveData::EncodingBits::LONG_CASTLE)
	{
		makeCastleMove(moveData);
		mMovePrivileges ^= moveData->privilegesRevoked;
		return true;
	}
    
	mMovePrivileges ^= moveData->privilegesRevoked;
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
