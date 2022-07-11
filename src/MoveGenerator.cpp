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
    Bitboard knightAFileCleared = BB::boardSquares[knightLoc] & BB::fileClear[BB::FILE_A];
    Bitboard knightBFileCleared = BB::boardSquares[knightLoc] & BB::fileClear[BB::FILE_B];
    Bitboard knightGFileCleared = BB::boardSquares[knightLoc] & BB::fileClear[BB::FILE_G];
    Bitboard knightHFileCleared = BB::boardSquares[knightLoc] & BB::fileClear[BB::FILE_H];

    Bitboard moves = 0;

    moves |= (knightAFileCleared & knightBFileCleared) << 6 | (knightAFileCleared & knightBFileCleared) >> 10; // check western horizontal moves
    moves |=  knightAFileCleared << 15 | knightAFileCleared >> 17;                                               // check western vertical moves
    moves |= (knightGFileCleared & knightHFileCleared) << 10 | (knightGFileCleared & knightHFileCleared) >> 6; // check eastern horizontal moves
    moves |=  knightHFileCleared << 17 | knightHFileCleared >> 15;                                               // check eastern vertiacal moves
    knightLookupTable[knightLoc] = moves;
}

void MoveGenerator::initKingLT(Byte kingLoc)
{
    // for moves north west, west, and south west, we need to clear file a to prevent overflow
    Bitboard kingAFileCleared = BB::boardSquares[kingLoc] & BB::fileClear[BB::FILE_A];
    // for moves north east, east, and south east, we need to clear file h to prevent overflow
    Bitboard kingHFileCleared = BB::boardSquares[kingLoc] & BB::fileClear[BB::FILE_H];

    // consider all ordinal and cardinal directions
    kingLookupTable[kingLoc] = kingAFileCleared << 7 | kingAFileCleared >> 1 | kingAFileCleared >> 9 | BB::boardSquares[kingLoc] << 8 |
           kingHFileCleared << 9 | kingHFileCleared << 1 | kingHFileCleared >> 7 | BB::boardSquares[kingLoc] >> 8;
}

void MoveGenerator::initPawnLT(Colour side, Byte pawnLoc)
{
    Bitboard pawnAFileCleared = BB::boardSquares[pawnLoc] & BB::fileClear[BB::FILE_A];
    Bitboard pawnHFileCleared = BB::boardSquares[pawnLoc] & BB::fileClear[BB::FILE_H];

    if (side == SIDE_WHITE)
        pawnAttackLookupTable[SIDE_WHITE][pawnLoc] = pawnAFileCleared << 7 | pawnHFileCleared << 9;
    else if (side == SIDE_BLACK)
        pawnAttackLookupTable[SIDE_BLACK][pawnLoc] = pawnAFileCleared >> 9 | pawnHFileCleared >> 7;
}

Bitboard MoveGenerator::computePseudoKingMoves(Byte pieceCoord, Bitboard friendlyPieces)
{
	// we already have predefined moves for the king so we need only index the correct element of the lookup table
	// then we simply ensure that we aren't allowing moves onto friendly pieces
	return kingLookupTable[pieceCoord] & ~friendlyPieces;
}

Bitboard MoveGenerator::computePseudoKnightMoves(Byte pieceCoord, Bitboard friendlyPieces)
{
	// same process as above :D
    return knightLookupTable[pieceCoord] & ~friendlyPieces;
}

Bitboard MoveGenerator::computePseudoPawnMoves(Byte pieceCoord, Colour side, Bitboard enemyPieces, Bitboard emptyBB, Bitboard enPassantBB)
{
	// if it's white we need to mask ranks to see if it has permissions to move two squares ahead
	Bitboard moves = (pawnAttackLookupTable[side][pieceCoord] & enemyPieces) | (pawnAttackLookupTable[side][pieceCoord] & enPassantBB);

	if (side == SIDE_WHITE)
	{
		Bitboard oneStep = (BB::boardSquares[pieceCoord] << 8) & emptyBB;
		// if the twostep is on the fourth rank, it would mean the pawn was on its home row
		Bitboard twoStep = ((oneStep << 8) & ~BB::rankClear[BB::RANK_FOURTH]) & emptyBB;
		moves |= oneStep | twoStep;
	}
	else if (side == SIDE_BLACK)
	{
		Bitboard oneStep = (BB::boardSquares[pieceCoord] >> 8) & emptyBB;
		// if the twostep is on the fifth rank, it would mean the pawn was on its home row
		Bitboard twoStep = ((oneStep >> 8) & ~BB::rankClear[BB::RANK_FIFTH]) & emptyBB;
		moves |= oneStep | twoStep;
	}

	return moves;
}

// value of true = stop adding pieces
inline bool slidingPieceMoveStep(int square, Bitboard* moves, Bitboard enemyPieces, Bitboard friendlyPieces)
{
    if (BB::boardSquares[square] & friendlyPieces)
        return true;

    *moves |= BB::boardSquares[square];

    if (BB::boardSquares[square] & enemyPieces)
        return true;

    return false;
}

Bitboard MoveGenerator::computePseudoRookMoves(Byte pieceCoord, Bitboard enemyPieces, Bitboard friendlyPieces)
{
    Bitboard moves = 0;

    // see if it works first, then clean it up. also move the entire side moves into movegenerator and out of board
    // cannot move to the edges of the board
    for (int square = pieceCoord + 8; square <= 63; square += 8) // north
        if (slidingPieceMoveStep(square, &moves, enemyPieces, friendlyPieces))
            break;
    for (int square = pieceCoord - 8; square >= 0; square -= 8) // south
        if (slidingPieceMoveStep(square, &moves, enemyPieces, friendlyPieces))
            break;
    // this remainder math is so that the moves stay in the same rank. compare the numbers to https://www.chessprogramming.org/Square_Mapping_Considerations
    for (int square = pieceCoord + 1; (square + 1) % 8 != 1 && square <= 63; square++) // east
        if (slidingPieceMoveStep(square, &moves, enemyPieces, friendlyPieces))
            break;

    for (int square = pieceCoord - 1; (square - 1) % 8 != 6 && square > 0; square--) // west right cause this works UNLESS its on the bottom rank right ?
     //   if (square >= 0)
        if (slidingPieceMoveStep(square, &moves, enemyPieces, friendlyPieces))
            break;

    return moves;
}

Bitboard MoveGenerator::computePseudoBishopMoves(Byte pieceCoord, Bitboard enemyPieces, Bitboard friendlyPieces)
{
    Bitboard moves = 0;
    int test = pieceCoord + 7;

    for (int square = pieceCoord + 7; square <= 63 && (square - 1) % 8 != 6; square += 7)
        if (slidingPieceMoveStep(square, &moves, enemyPieces, friendlyPieces))
            break;
    for (int square = pieceCoord + 9; square <= 63 && (square + 1) % 8 != 1; square += 9)
        if (slidingPieceMoveStep(square, &moves, enemyPieces, friendlyPieces))
            break;
    for (int square = pieceCoord - 7; square >= 0 && (square + 1) % 8 != 1; square -= 7)
        if (slidingPieceMoveStep(square, &moves, enemyPieces, friendlyPieces))
            break;
    for (int square = pieceCoord - 9; square >= 0 && (square - 1) % 8 != 6; square -= 9)
        if (slidingPieceMoveStep(square, &moves, enemyPieces, friendlyPieces))
            break;

    return moves;
}

Bitboard MoveGenerator::computePseudoQueenMoves(Byte pieceCoord, Bitboard enemyPieces, Bitboard friendlyPieces)
{
    return computePseudoBishopMoves(pieceCoord, enemyPieces, friendlyPieces) | computePseudoRookMoves(pieceCoord, enemyPieces, friendlyPieces);
}

MoveData MoveGenerator::computeCastleMoveData(Colour side, Byte privileges, Bitboard occupied, Privilege castleType)
{
    MoveData md;
    md.side = side;
    md.setMoveType(MoveData::EncodingBits::NONE);
    int lower, higher;

    if (castleType == Privilege::WHITE_SHORT_CASTLE || castleType == Privilege::BLACK_SHORT_CASTLE)
    {
        if (side == SIDE_WHITE) { lower = 5;  higher = 7;  }
        if (side == SIDE_BLACK) { lower = 61; higher = 63; }

        if (((privileges & (int)Privilege::WHITE_SHORT_CASTLE) && side == SIDE_WHITE) ||
             (privileges & (int)Privilege::BLACK_SHORT_CASTLE) && side == SIDE_BLACK)
        {
            // check if the castle would be legal
            for (int tile = lower; tile < higher; tile++) // king is on tile 4,
                if (BB::boardSquares[tile] & occupied)
                {
                  //  std::cout << "occupied: \n";
                    //BB::printBitboard(occupied);
                    md.setMoveType(MoveData::EncodingBits::INVALID);
                    break;
                }
                    //md.setMoveType(MoveData::EncodingBits::INVALID);

            if (md.moveType != MoveData::EncodingBits::INVALID)
            {
                md.setMoveType(MoveData::EncodingBits::SHORT_CASTLE);
                if (md.side == SIDE_WHITE) md.privilegesRevoked |= (Byte)Privilege::WHITE_LONG_CASTLE | (Byte)Privilege::WHITE_SHORT_CASTLE;
                else                       md.privilegesRevoked |= (Byte)Privilege::BLACK_LONG_CASTLE | (Byte)Privilege::BLACK_SHORT_CASTLE;

                md.originSquare = lower  - 1;
                md.targetSquare = higher - 1;
            }

            return md;
        }
    }
    else if (castleType == Privilege::WHITE_LONG_CASTLE || castleType == Privilege::BLACK_LONG_CASTLE)
    {
        if (side == SIDE_WHITE) { lower = 0;  higher = 3;  }
        if (side == SIDE_BLACK) { lower = 56; higher = 59; }

        if (((privileges & (int)Privilege::WHITE_LONG_CASTLE) && side == SIDE_WHITE) ||
             (privileges & (int)Privilege::BLACK_LONG_CASTLE) && side == SIDE_BLACK)
        {
            // check if the castle would be legal
            for (int tile = higher; tile > lower; tile--) // king is on tile 4,
                if (BB::boardSquares[tile] & occupied)
                {
                    md.setMoveType(MoveData::EncodingBits::INVALID);
                    break;
                }
                    //md.setMoveType(MoveData::EncodingBits::INVALID);

            if (md.moveType != MoveData::EncodingBits::INVALID)
            {
                md.setMoveType(MoveData::EncodingBits::LONG_CASTLE);
                if (md.side == SIDE_WHITE) md.privilegesRevoked |= (Byte)Privilege::WHITE_LONG_CASTLE | (Byte)Privilege::WHITE_SHORT_CASTLE;
                else                       md.privilegesRevoked |= (Byte)Privilege::BLACK_LONG_CASTLE | (Byte)Privilege::BLACK_SHORT_CASTLE;

                md.originSquare = higher + 1;
                md.targetSquare = lower + 2;
            }

            return md;
        }
    }

    md.setMoveType(MoveData::EncodingBits::INVALID);
    return md;
}

void MoveGenerator::generateCaptureMoves(std::vector<MoveData>& moveVec, Colour side, Board* board)
{
    moveVec.clear();
    for (int square = 0; square < 64; square++)
    {
        // the above functions return bitboards right? just compare them with the board->blackPieces
        // these are the only moves that will be capture moves
     //   if (BB::boardSquares[square] & )
        // just use the function currently in Board, but only push_back() moves that are capture moves
    }
}

// side is a default value with a value of -1. this value indicates no side was specified and to search all bitboards
Bitboard* MoveGenerator::getPieceBitboard(Board* board, Byte square, Colour side)
{
    Bitboard squareBB = BB::boardSquares[square];

    if (side == SIDE_WHITE || side == -1)
    {
        if        (squareBB & board->whitePawnsBB)     return &board->whitePawnsBB;
        else if (squareBB & board->whiteRooksBB)     return &board->whiteRooksBB;
        else if (squareBB & board->whiteKnightsBB)  return &board->whiteKnightsBB;
        else if (squareBB & board->whiteBishopsBB)  return &board->whiteBishopsBB;
        else if (squareBB & board->whiteQueensBB)   return &board->whiteQueensBB;
        else if (squareBB & board->whiteKingBB)     return &board->whiteKingBB;
    }
    if (side == SIDE_BLACK || side == -1)
    {
        if        (squareBB & board->blackPawnsBB)     return &board->blackPawnsBB;
        else if (squareBB & board->blackRooksBB)     return &board->blackRooksBB;
        else if (squareBB & board->blackKnightsBB)  return &board->blackKnightsBB;
        else if (squareBB & board->blackBishopsBB)  return &board->blackBishopsBB;
        else if (squareBB & board->blackQueensBB)   return &board->blackQueensBB;
        else if (squareBB & board->blackKingBB)     return &board->blackKingBB;
    }

    return nullptr;
}

void MoveGenerator::calculateSideMoves(Board* board, Colour side, std::vector<MoveData>& moveVec)
{
    //std::vector<MoveData>& sideMovesRef = side == SIDE_WHITE ? mWhiteMoves : mBlackMoves;

    moveVec.clear();
    moveVec.reserve(100);
    
    for (int square = 0; square < 64; square++)
        calculatePieceMoves(board, side, square, moveVec);

    calculateCastleMoves(board, side, moveVec);
}

void MoveGenerator::calculateCastleMoves(Board* board, Colour side, std::vector<MoveData>& movesVec)
{
    // check if castle moves are possible before computing them (otherwise resulting in invalid moves)
    MoveData shortCastleMD, longCastleMD;
    
    if (side == SIDE_WHITE) // white castle moves
    {
        if (board->whiteKingBB == 0)
            return;

        shortCastleMD = computeCastleMoveData(side, board->movePrivileges, board->occupiedBB, Privilege::WHITE_SHORT_CASTLE);
        longCastleMD  = computeCastleMoveData(side, board->movePrivileges, board->occupiedBB, Privilege::WHITE_LONG_CASTLE);
    }
    else // black castle moves
    {
        if (board->blackKingBB == 0)
            return;

        shortCastleMD = computeCastleMoveData(side, board->movePrivileges, board->occupiedBB, Privilege::BLACK_SHORT_CASTLE);
        longCastleMD  = computeCastleMoveData(side, board->movePrivileges, board->occupiedBB, Privilege::BLACK_LONG_CASTLE);
    }
    
    if (shortCastleMD.moveType != MoveData::EncodingBits::INVALID)
        movesVec.push_back(shortCastleMD);
    if (longCastleMD.moveType != MoveData::EncodingBits::INVALID)
        movesVec.push_back(longCastleMD);
}

void MoveGenerator::calculatePieceMoves(Board* board, Colour side, Byte originSquare, std::vector<MoveData>& moveVec)
{
    MoveData md;
    
    // captured piece would be the opposite colour of the piece moving
    md.colourBB         = side == SIDE_WHITE ? &board->whitePiecesBB : &board->blackPiecesBB;
    md.capturedColourBB = side == SIDE_WHITE ? &board->blackPiecesBB : &board->whitePiecesBB;
    
    if (BB::boardSquares[originSquare] & *md.colourBB)
    {
        md.side = side;
        md.originSquare = originSquare;

        Bitboard moves = 0;
        Bitboard* pieceBBPtr = getPieceBitboard(board, originSquare, side);

        md.pieceBB = pieceBBPtr;

        moves = calculatePsuedoMove(board, &md, *pieceBBPtr);

        if (moves > 0)
            findMoveCaptures(board, moves, md, moveVec);
    }
}

void MoveGenerator::setCastlePrivileges(Board* board, MoveData* md, bool isKing)
{
    if (isKing)
    {
        if (md->side == SIDE_WHITE && md->originSquare == 4 && (board->movePrivileges & (Byte)Privilege::WHITE_LONG_CASTLE))
            md->privilegesRevoked |= (Byte)Privilege::WHITE_LONG_CASTLE;
        if (md->side == SIDE_WHITE && md->originSquare == 4 && (board->movePrivileges & (Byte)Privilege::WHITE_SHORT_CASTLE))
            md->privilegesRevoked |= (Byte)Privilege::WHITE_SHORT_CASTLE;
        if (md->side == SIDE_BLACK && md->originSquare == 60 && (board->movePrivileges & (Byte)Privilege::BLACK_LONG_CASTLE))
            md->privilegesRevoked |= (Byte)Privilege::BLACK_LONG_CASTLE;
        if (md->side == SIDE_BLACK && md->originSquare == 60 && (board->movePrivileges & (Byte)Privilege::BLACK_SHORT_CASTLE))
            md->privilegesRevoked |= (Byte)Privilege::BLACK_SHORT_CASTLE;
    }
    else
    {
        if (md->side == SIDE_WHITE && md->originSquare == 0 && (board->movePrivileges & (Byte)Privilege::WHITE_LONG_CASTLE))
            md->privilegesRevoked |= (Byte)Privilege::WHITE_LONG_CASTLE;
        else if (md->side == SIDE_WHITE && md->originSquare == 7 && (board->movePrivileges & (Byte)Privilege::WHITE_SHORT_CASTLE))
            md->privilegesRevoked |= (Byte)Privilege::WHITE_SHORT_CASTLE;
        else if (md->side == SIDE_BLACK && md->originSquare == 56 && (board->movePrivileges & (Byte)Privilege::BLACK_LONG_CASTLE))
            md->privilegesRevoked |= (Byte)Privilege::BLACK_LONG_CASTLE;
        else if (md->side == SIDE_BLACK && md->originSquare == 63 && (board->movePrivileges & (Byte)Privilege::BLACK_SHORT_CASTLE))
            md->privilegesRevoked |= (Byte)Privilege::BLACK_SHORT_CASTLE;
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

// note that this function (as of now) actually adds the moves to the moveVector as well
void MoveGenerator::findMoveCaptures(Board* board, Bitboard moves, MoveData& md, std::vector<MoveData>& moveVec)
{
    for (int square = 0; square < 64; square++)
    {
        if (moves & BB::boardSquares[square])
        {
            md.targetSquare = square;
            md.capturedPieceBB = nullptr;

            if (BB::boardSquares[square] & *md.capturedColourBB)
                md.capturedPieceBB = getPieceBitboard(board, square, !md.side);
            if (BB::boardSquares[square] & board->enPassantBB)
            {
                if (moves & board->enPassantBB)
                {
                    if (md.pieceBB == &board->whitePawnsBB)
                    {
                        md.capturedPieceBB = getPieceBitboard(board, square - 8, SIDE_BLACK);
                        md.setMoveType(MoveData::EncodingBits::EN_PASSANT_CAPTURE);
                        md.enPassantBB = board->enPassantBB;
                    }
                    else if (md.pieceBB == &board->blackPawnsBB)
                    {
                        md.capturedPieceBB = getPieceBitboard(board, square + 8, SIDE_WHITE);
                        md.setMoveType(MoveData::EncodingBits::EN_PASSANT_CAPTURE);
                        md.enPassantBB = board->enPassantBB;
                    }
                }
            }
            else if (board->enPassantBB)
                md.enPassantBB = board->enPassantBB;

            // remove castling privileges if a rook was captured
            if (md.capturedPieceBB == &board->blackRooksBB)
            {
                if (md.targetSquare == 63)        md.privilegesRevoked |= (Byte)Privilege::BLACK_SHORT_CASTLE;
                else if (md.targetSquare == 56) md.privilegesRevoked |= (Byte)Privilege::BLACK_LONG_CASTLE;
            }
            else if (md.capturedPieceBB == &board->whiteRooksBB)
            {
                if (md.targetSquare == 7)        md.privilegesRevoked |= (Byte)Privilege::WHITE_SHORT_CASTLE;
                else if (md.targetSquare == 0)  md.privilegesRevoked |= (Byte)Privilege::WHITE_LONG_CASTLE;
            }

            moveVec.push_back(md);
        }
    }
}
