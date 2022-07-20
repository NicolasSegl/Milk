#include "MoveGenerator.h"
#include "Board.h"

void MoveGenerator::init()
{
    for (int pieceLoc = 0; pieceLoc < 64; pieceLoc++)
    {
        initKingLT(pieceLoc);
        initKnightLT(pieceLoc);
        initPawnLT(SIDE_BLACK, pieceLoc);
        initPawnLT(SIDE_WHITE, pieceLoc);
        //mQueenLookupTable[pieceLoc] = computePseudoRookMoves(pieceLoc, 0, 0) | computePseudoBishopMoves(pieceLoc, 0, 0);
    }
}

void MoveGenerator::initKnightLT(Byte knightLoc)
{
    Bitboard knightAFileClearedBB = BB::boardSquares[knightLoc] & BB::fileClear[BB::FILE_A];
    Bitboard knightBFileClearedBB = BB::boardSquares[knightLoc] & BB::fileClear[BB::FILE_B];
    Bitboard knightGFileClearedBB = BB::boardSquares[knightLoc] & BB::fileClear[BB::FILE_G];
    Bitboard knightHFileClearedBB = BB::boardSquares[knightLoc] & BB::fileClear[BB::FILE_H];

    Bitboard movesBB = 0;

    movesBB |= (knightAFileClearedBB & knightBFileClearedBB) << 6 |  (knightAFileClearedBB & knightBFileClearedBB) >> 10; // check western horizontal moves
    movesBB |=  knightAFileClearedBB << 15 | knightAFileClearedBB >> 17;                                                  // check western vertical moves
    movesBB |= (knightGFileClearedBB & knightHFileClearedBB) << 10 | (knightGFileClearedBB & knightHFileClearedBB) >> 6;  // check eastern horizontal moves
    movesBB |=  knightHFileClearedBB << 17 | knightHFileClearedBB >> 15;                                                  // check eastern vertiacal moves
    knightLookupTable[knightLoc] = movesBB;
}

void MoveGenerator::initKingLT(Byte kingLoc)
{
    // for moves north west, west, and south west, we need to clear file a to prevent overflow
    Bitboard kingAFileClearedBB = BB::boardSquares[kingLoc] & BB::fileClear[BB::FILE_A];
    // for moves north east, east, and south east, we need to clear file h to prevent overflow
    Bitboard kingHFileClearedBB = BB::boardSquares[kingLoc] & BB::fileClear[BB::FILE_H];

    // consider all ordinal and cardinal directions
    kingLookupTable[kingLoc] = kingAFileClearedBB << 7 | kingAFileClearedBB >> 1 | kingAFileClearedBB >> 9 | BB::boardSquares[kingLoc] << 8 |
           kingHFileClearedBB << 9 | kingHFileClearedBB << 1 | kingHFileClearedBB >> 7 | BB::boardSquares[kingLoc] >> 8;
}

void MoveGenerator::initPawnLT(Colour side, Byte pawnLoc)
{
    Bitboard pawnAFileClearedBB = BB::boardSquares[pawnLoc] & BB::fileClear[BB::FILE_A];
    Bitboard pawnHFileClearedBB = BB::boardSquares[pawnLoc] & BB::fileClear[BB::FILE_H];

    if (side == SIDE_WHITE)
        pawnAttackLookupTable[SIDE_WHITE][pawnLoc] = pawnAFileClearedBB << 7 | pawnHFileClearedBB << 9;
    else if (side == SIDE_BLACK)
        pawnAttackLookupTable[SIDE_BLACK][pawnLoc] = pawnAFileClearedBB >> 9 | pawnHFileClearedBB >> 7;
}

Bitboard MoveGenerator::computePseudoKingMoves(Byte pieceCoord, Bitboard friendlyPiecesBB)
{
	// we already have predefined moves for the king so we need only index the correct element of the lookup table
	// then we simply ensure that we aren't allowing moves onto friendly pieces
	return kingLookupTable[pieceCoord] & ~friendlyPiecesBB;
}

Bitboard MoveGenerator::computePseudoKnightMoves(Byte pieceCoord, Bitboard friendlyPiecesBB)
{
	// same process as above :D
    return knightLookupTable[pieceCoord] & ~friendlyPiecesBB;
}

Bitboard MoveGenerator::computePseudoPawnMoves(Byte pieceCoord, Colour side, Bitboard enemyPiecesBB, Bitboard emptyBB, Bitboard enPassantBB)
{
	// if it's white we need to mask ranks to see if it has permissions to move two squares ahead
	Bitboard movesBB = (pawnAttackLookupTable[side][pieceCoord] & enemyPiecesBB) | (pawnAttackLookupTable[side][pieceCoord] & enPassantBB);

	if (side == SIDE_WHITE)
	{
		Bitboard oneStepBB = (BB::boardSquares[pieceCoord] << 8) & emptyBB;
		// if the twostep is on the fourth rank, it would mean the pawn was on its home row
		Bitboard twoStepBB = ((oneStepBB << 8) & ~BB::rankClear[BB::RANK_FOURTH]) & emptyBB;
		movesBB |= oneStepBB | twoStepBB;
	}
	else if (side == SIDE_BLACK)
	{
		Bitboard oneStepBB = (BB::boardSquares[pieceCoord] >> 8) & emptyBB;
		// if the twostep is on the fifth rank, it would mean the pawn was on its home row
		Bitboard twoStepBB = ((oneStepBB >> 8) & ~BB::rankClear[BB::RANK_FIFTH]) & emptyBB;
		movesBB |= oneStepBB | twoStepBB;
	}

	return movesBB;
}

// value of true = stop adding pieces
inline bool slidingPieceMoveStep(int square, Bitboard* movesBB, Bitboard enemyPiecesBB, Bitboard friendlyPiecesBB)
{
    if (BB::boardSquares[square] & friendlyPiecesBB)
        return true;

    *movesBB |= BB::boardSquares[square];

    if (BB::boardSquares[square] & enemyPiecesBB)
        return true;

    return false;
}

Bitboard MoveGenerator::computePseudoRookMoves(Byte pieceCoord, Bitboard enemyPiecesBB, Bitboard friendlyPiecesBB)
{
    Bitboard movesBB = 0;

    // see if it works first, then clean it up. also move the entire side moves into movegenerator and out of board
    // cannot move to the edges of the board
    for (int square = pieceCoord + 8; square <= 63; square += 8) // north
        if (slidingPieceMoveStep(square, &movesBB, enemyPiecesBB, friendlyPiecesBB))
            break;
    for (int square = pieceCoord - 8; square >= 0; square -= 8) // south
        if (slidingPieceMoveStep(square, &movesBB, enemyPiecesBB, friendlyPiecesBB))
            break;
    // this remainder math is so that the moves stay in the same rank. compare the numbers to https://www.chessprogramming.org/Square_Mapping_Considerations
    for (int square = pieceCoord + 1; (square + 1) % 8 != 1 && square <= 63; square++) // east
        if (slidingPieceMoveStep(square, &movesBB, enemyPiecesBB, friendlyPiecesBB))
            break;

    for (int square = pieceCoord - 1; (square - 1) % 8 != 6 && square >= 0; square--) // west right cause this works UNLESS its on the bottom rank right ?
        if (slidingPieceMoveStep(square, &movesBB, enemyPiecesBB, friendlyPiecesBB))
            break;

    return movesBB;
}

Bitboard MoveGenerator::computePseudoBishopMoves(Byte pieceCoord, Bitboard enemyPiecesBB, Bitboard friendlyPiecesBB)
{
    Bitboard moves = 0;
    int test = pieceCoord + 7;

    for (int square = pieceCoord + 7; square <= 63 && (square - 1) % 8 != 6; square += 7)
        if (slidingPieceMoveStep(square, &moves, enemyPiecesBB, friendlyPiecesBB))
            break;
    for (int square = pieceCoord + 9; square <= 63 && (square + 1) % 8 != 1; square += 9)
        if (slidingPieceMoveStep(square, &moves, enemyPiecesBB, friendlyPiecesBB))
            break;
    for (int square = pieceCoord - 7; square >= 0 && (square + 1) % 8 != 1; square -= 7)
        if (slidingPieceMoveStep(square, &moves, enemyPiecesBB, friendlyPiecesBB))
            break;
    for (int square = pieceCoord - 9; square >= 0 && (square - 1) % 8 != 6; square -= 9)
        if (slidingPieceMoveStep(square, &moves, enemyPiecesBB, friendlyPiecesBB))
            break;

    return moves;
}

Bitboard MoveGenerator::computePseudoQueenMoves(Byte pieceCoord, Bitboard enemyPiecesBB, Bitboard friendlyPiecesBB)
{
    return computePseudoBishopMoves(pieceCoord, enemyPiecesBB, friendlyPiecesBB) | computePseudoRookMoves(pieceCoord, enemyPiecesBB, friendlyPiecesBB);
}

void MoveGenerator::setCastleMovePrivilegesRevoked(Colour side, Byte privileges, Byte* privilegesToBeRevoked)
{
    if (side == SIDE_WHITE)
    {
        if (privileges & (Byte)Privilege::WHITE_LONG_CASTLE)  *privilegesToBeRevoked |= (Byte)Privilege::WHITE_LONG_CASTLE;
        if (privileges & (Byte)Privilege::WHITE_SHORT_CASTLE) *privilegesToBeRevoked |= (Byte)Privilege::WHITE_SHORT_CASTLE;
    }
    else
    {
        if (privileges & (Byte)Privilege::BLACK_LONG_CASTLE)  *privilegesToBeRevoked |= (Byte)Privilege::BLACK_LONG_CASTLE;
        if (privileges & (Byte)Privilege::BLACK_SHORT_CASTLE) *privilegesToBeRevoked |= (Byte)Privilege::BLACK_SHORT_CASTLE;
    }
}

MoveData MoveGenerator::computeCastleMoveData(Colour side, Byte privileges, Bitboard occupiedBB, Privilege castleType)
{
    MoveData md;
    md.side = side;
    md.setMoveType(MoveData::EncodingBits::NONE);
    int lower, higher;

    switch (castleType)
    {
        case Privilege::WHITE_SHORT_CASTLE:
        case Privilege::BLACK_SHORT_CASTLE:
        
            if (((privileges & (Byte)Privilege::WHITE_SHORT_CASTLE) && side == SIDE_WHITE) ||
                 (privileges & (Byte)Privilege::BLACK_SHORT_CASTLE) && side == SIDE_BLACK)
            {

                if (side == SIDE_WHITE) { lower = 5;  higher = 7; }
                if (side == SIDE_BLACK) { lower = 61; higher = 63; }

                // check if the castle would be legal
                for (int tile = lower; tile < higher; tile++)
                    if (BB::boardSquares[tile] & occupiedBB)
                    {
                        md.setMoveType(MoveData::EncodingBits::INVALID);
                        break;
                    }

                if (md.moveType != MoveData::EncodingBits::INVALID)
                {
                    md.setMoveType(MoveData::EncodingBits::SHORT_CASTLE);
                    setCastleMovePrivilegesRevoked(md.side, privileges, &md.castlePrivilegesRevoked);

                    md.originSquare = lower  - 1;
                    md.targetSquare = higher - 1;
                }

                return md;
            }

            break;
        
        case Privilege::WHITE_LONG_CASTLE:
        case Privilege::BLACK_LONG_CASTLE:

            if (((privileges & (Byte)Privilege::WHITE_LONG_CASTLE) && side == SIDE_WHITE) ||
                 (privileges & (Byte)Privilege::BLACK_LONG_CASTLE) && side == SIDE_BLACK)
            {

                if (side == SIDE_WHITE) { lower = 0;  higher = 3; }
                if (side == SIDE_BLACK) { lower = 56; higher = 59; }

                // check if the castle would be legal
                for (int tile = higher; tile > lower; tile--)
                    if (BB::boardSquares[tile] & occupiedBB)
                    {
                        md.setMoveType(MoveData::EncodingBits::INVALID);
                        break;
                    }
                        //md.setMoveType(MoveData::EncodingBits::INVALID);

                if (md.moveType != MoveData::EncodingBits::INVALID)
                {
                    md.setMoveType(MoveData::EncodingBits::LONG_CASTLE);
                    setCastleMovePrivilegesRevoked(md.side, privileges, &md.castlePrivilegesRevoked);

                    md.originSquare = higher + 1;
                    md.targetSquare = lower + 2;
                }

                return md;
            }

            break;
    }

    md.setMoveType(MoveData::EncodingBits::INVALID);
    return md;
}

void MoveGenerator::calculateCaptureMoves(Board* board, std::vector<MoveData>& moveVec, Colour side)
{
    calculateSideMoves(board, side, moveVec, true);
}

// side is a default value with a value of -1. this value indicates no side was specified and to search all bitboards
Bitboard* MoveGenerator::getPieceBitboard(Board* board, Byte square, Colour side)
{
    Bitboard squareBB = BB::boardSquares[square];

    if (side == SIDE_WHITE || side == -1)
    {
        if        (squareBB & board->whitePawnsBB)  return &board->whitePawnsBB;
        else if (squareBB & board->whiteRooksBB)    return &board->whiteRooksBB;
        else if (squareBB & board->whiteKnightsBB)  return &board->whiteKnightsBB;
        else if (squareBB & board->whiteBishopsBB)  return &board->whiteBishopsBB;
        else if (squareBB & board->whiteQueensBB)   return &board->whiteQueensBB;
        else if (squareBB & board->whiteKingBB)     return &board->whiteKingBB;
    }
    if (side == SIDE_BLACK || side == -1)
    {
        if        (squareBB & board->blackPawnsBB)  return &board->blackPawnsBB;
        else if (squareBB & board->blackRooksBB)    return &board->blackRooksBB;
        else if (squareBB & board->blackKnightsBB)  return &board->blackKnightsBB;
        else if (squareBB & board->blackBishopsBB)  return &board->blackBishopsBB;
        else if (squareBB & board->blackQueensBB)   return &board->blackQueensBB;
        else if (squareBB & board->blackKingBB)     return &board->blackKingBB;
    }

    return nullptr;
}

void MoveGenerator::calculateSideMoves(Board* board, Colour side, std::vector<MoveData>& moveVec, bool captureOnly)
{
    moveVec.clear();
    moveVec.reserve(100);
    
    for (int square = 0; square < 64; square++)
        calculatePieceMoves(board, side, square, moveVec, captureOnly);

    if (!captureOnly)
        calculateCastleMoves(board, side, moveVec);
}

void MoveGenerator::calculateCastleMoves(Board* board, Colour side, std::vector<MoveData>& movesVec)
{
    // check if castle moves are possible before computing them (otherwise resulting in invalid moves)
    MoveData shortCastleMD, longCastleMD;
    
    if (side == SIDE_WHITE) // white castle moves
    {
        if (board->whiteKingBB == 0 || board->whiteRooksBB == 0)
            return;

        shortCastleMD = computeCastleMoveData(side, board->castlePrivileges, board->occupiedBB, Privilege::WHITE_SHORT_CASTLE);
        longCastleMD  = computeCastleMoveData(side, board->castlePrivileges, board->occupiedBB, Privilege::WHITE_LONG_CASTLE);
    }
    else // black castle moves
    {
        if (board->blackKingBB == 0 || board->blackRooksBB == 0)
            return;

        shortCastleMD = computeCastleMoveData(side, board->castlePrivileges, board->occupiedBB, Privilege::BLACK_SHORT_CASTLE);
        longCastleMD  = computeCastleMoveData(side, board->castlePrivileges, board->occupiedBB, Privilege::BLACK_LONG_CASTLE);
    }
    
    if (shortCastleMD.moveType != MoveData::EncodingBits::INVALID)
        movesVec.push_back(shortCastleMD);
    if (longCastleMD.moveType != MoveData::EncodingBits::INVALID)
        movesVec.push_back(longCastleMD);
}

void MoveGenerator::calculatePieceMoves(Board* board, Colour side, Byte originSquare, std::vector<MoveData>& moveVec, bool captureOnly)
{
    MoveData md;
    
    // captured piece would be the opposite colour of the piece moving
    md.colourBB         = side == SIDE_WHITE ? &board->whitePiecesBB : &board->blackPiecesBB;
    md.capturedColourBB = side == SIDE_WHITE ? &board->blackPiecesBB : &board->whitePiecesBB;
    
    if (BB::boardSquares[originSquare] & *md.colourBB)
    {
        md.side = side;
        md.originSquare = originSquare;

        Bitboard movesBB = 0;
        Bitboard* pieceBBPtr = getPieceBitboard(board, originSquare, side);

        md.pieceBB = pieceBBPtr;

        movesBB = calculatePsuedoMove(board, &md, *pieceBBPtr);

        if (movesBB > 0)
            addMoves(board, movesBB, md, moveVec, captureOnly);
    }
}

void MoveGenerator::setCastlePrivileges(Board* board, MoveData* md, bool isKing)
{
    // its giving the permissions BACK
    if (isKing)
    {
        if (md->side == SIDE_WHITE && md->originSquare == 4 && (board->castlePrivileges & (Byte)Privilege::WHITE_LONG_CASTLE))
            md->castlePrivilegesRevoked |= (Byte)Privilege::WHITE_LONG_CASTLE;
        if (md->side == SIDE_WHITE && md->originSquare == 4 && (board->castlePrivileges & (Byte)Privilege::WHITE_SHORT_CASTLE))
            md->castlePrivilegesRevoked |= (Byte)Privilege::WHITE_SHORT_CASTLE;
        if (md->side == SIDE_BLACK && md->originSquare == 60 && (board->castlePrivileges & (Byte)Privilege::BLACK_LONG_CASTLE))
            md->castlePrivilegesRevoked |= (Byte)Privilege::BLACK_LONG_CASTLE;
        if (md->side == SIDE_BLACK && md->originSquare == 60 && (board->castlePrivileges & (Byte)Privilege::BLACK_SHORT_CASTLE))
            md->castlePrivilegesRevoked |= (Byte)Privilege::BLACK_SHORT_CASTLE;
    }
    else
    {
        if (md->side == SIDE_WHITE && md->originSquare == 0 && (board->castlePrivileges & (Byte)Privilege::WHITE_LONG_CASTLE))
            md->castlePrivilegesRevoked |= (Byte)Privilege::WHITE_LONG_CASTLE;
        else if (md->side == SIDE_WHITE && md->originSquare == 7 && (board->castlePrivileges & (Byte)Privilege::WHITE_SHORT_CASTLE))
            md->castlePrivilegesRevoked |= (Byte)Privilege::WHITE_SHORT_CASTLE;
        else if (md->side == SIDE_BLACK && md->originSquare == 56 && (board->castlePrivileges & (Byte)Privilege::BLACK_LONG_CASTLE))
            md->castlePrivilegesRevoked |= (Byte)Privilege::BLACK_LONG_CASTLE;
        else if (md->side == SIDE_BLACK && md->originSquare == 63 && (board->castlePrivileges & (Byte)Privilege::BLACK_SHORT_CASTLE))
            md->castlePrivilegesRevoked |= (Byte)Privilege::BLACK_SHORT_CASTLE;
    }
}

Bitboard MoveGenerator::calculatePsuedoMove(Board* board, MoveData* md, Bitboard& pieceBB)
{
    if ((pieceBB & board->whiteKnightsBB) || (pieceBB & board->blackKnightsBB))
        return computePseudoKnightMoves(md->originSquare, *md->colourBB);

    else if ((pieceBB & board->whitePawnsBB) || (pieceBB & board->blackPawnsBB))
        return computePseudoPawnMoves(md->originSquare, md->side, *md->capturedColourBB, board->emptyBB, board->enPassantBB);

    else if ((pieceBB & board->whiteBishopsBB) || (pieceBB & board->blackBishopsBB))
        return computePseudoBishopMoves(md->originSquare, *md->capturedColourBB, *md->colourBB);

    else if ((pieceBB & board->whiteQueensBB) || (pieceBB & board->blackQueensBB))
        return computePseudoQueenMoves(md->originSquare, *md->capturedColourBB, *md->colourBB);

    else if ((pieceBB & board->whiteRooksBB) || (pieceBB & board->blackRooksBB)) // these moves may change castling privileges
    {
        setCastlePrivileges(board, md, false);
        return computePseudoRookMoves(md->originSquare, *md->capturedColourBB, *md->colourBB);
    }

    else if ((pieceBB & board->whiteKingBB) || (pieceBB & board->blackKingBB)) // these moves may change castling privileges
    {
        setCastlePrivileges(board, md, true);
        return computePseudoKingMoves(md->originSquare, *md->colourBB);
    }
}

bool MoveGenerator::doesCaptureAffectCastle(Board* board, MoveData* md)
{
    bool doesAffectCastlePrivileges = false;
    if (md->capturedPieceBB == &board->blackRooksBB)
    {
        doesAffectCastlePrivileges = true;
        if (md->targetSquare == 63 && (board->castlePrivileges & (Byte)Privilege::BLACK_SHORT_CASTLE))
            md->castlePrivilegesRevoked |= (Byte)Privilege::BLACK_SHORT_CASTLE;
        else if (md->targetSquare == 56 && (board->castlePrivileges & (Byte)Privilege::BLACK_LONG_CASTLE)) 
            md->castlePrivilegesRevoked |= (Byte)Privilege::BLACK_LONG_CASTLE;
        else
            doesAffectCastlePrivileges = false;
    }
    else if (md->capturedPieceBB == &board->whiteRooksBB)
    {
        doesAffectCastlePrivileges = true;
        if (md->targetSquare == 7 && (board->castlePrivileges & (Byte)Privilege::WHITE_SHORT_CASTLE))
            md->castlePrivilegesRevoked |= (Byte)Privilege::WHITE_SHORT_CASTLE;
        else if (md->targetSquare == 0 && (board->castlePrivileges & (Byte)Privilege::WHITE_LONG_CASTLE))
            md->castlePrivilegesRevoked |= (Byte)Privilege::WHITE_LONG_CASTLE;
        else
            doesAffectCastlePrivileges = false;
    }

    return doesAffectCastlePrivileges;
}

void MoveGenerator::setEnPassantMoveData(Board* board, int square, Bitboard pieceMovesBB, MoveData* md)
{
    if (BB::boardSquares[square] & board->enPassantBB)
    {
        if (pieceMovesBB & board->enPassantBB)
        {
            if (md->pieceBB == &board->whitePawnsBB)
            {
                md->capturedPieceBB = getPieceBitboard(board, square - 8, SIDE_BLACK);
                md->setMoveType(MoveData::EncodingBits::EN_PASSANT_CAPTURE);
                md->enPassantBB = board->enPassantBB;
            }
            else if (md->pieceBB == &board->blackPawnsBB)
            {
                md->capturedPieceBB = getPieceBitboard(board, square + 8, SIDE_WHITE);
                md->setMoveType(MoveData::EncodingBits::EN_PASSANT_CAPTURE);
                md->enPassantBB = board->enPassantBB;
            }
        }
    }
    else if (board->enPassantBB)
        md->enPassantBB = board->enPassantBB;
}

void MoveGenerator::addMoves(Board* board, Bitboard movesBB, MoveData& md, std::vector<MoveData>& moveVec, bool captureOnly)
{
    for (int square = 0; square < 64; square++)
    {
        if (movesBB & BB::boardSquares[square])
        {
            md.targetSquare = square;
            md.capturedPieceBB = nullptr;
            md.setMoveType(MoveData::EncodingBits::REGULAR);

            if (BB::boardSquares[square] & *md.capturedColourBB)
                md.capturedPieceBB = getPieceBitboard(board, square, !md.side);

            if (captureOnly && !md.capturedPieceBB)
                continue;

            if (md.pieceBB == &board->whitePawnsBB && md.targetSquare >= 56) md.setMoveType(MoveData::EncodingBits::PAWN_PROMOTION);
            if (md.pieceBB == &board->blackPawnsBB && md.targetSquare <= 7)  md.setMoveType(MoveData::EncodingBits::PAWN_PROMOTION);
            setEnPassantMoveData(board, square, movesBB, &md);

            // we need to reset the privileges revoked after a move that captures a rook has changed them
            // otherwise, no matter what move the piece makes (i.e. no capturing the rook), will change the privileges
            bool resetCastlePrivileges = doesCaptureAffectCastle(board, &md);

            moveVec.push_back(md);

            if (resetCastlePrivileges)
                md.castlePrivilegesRevoked = 0;
        }
    }
}
