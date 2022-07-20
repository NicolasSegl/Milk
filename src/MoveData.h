#ifndef MoveData_hpp
#define MoveData_hpp

typedef uint8_t  Byte;
typedef uint16_t DoubleByte;
typedef bool Colour;

enum class Privilege
{
    WHITE_LONG_CASTLE  = 1 << 0,
    WHITE_SHORT_CASTLE = 1 << 1,
    BLACK_LONG_CASTLE  = 1 << 2,
    BLACK_SHORT_CASTLE = 1 << 3,
};

struct MoveData
{
    // insofar, capture and en_passant_square are never used. unsure about invalid/none's purposes
    enum class EncodingBits
    {       
        CAPTURE              = 1 << 0,
        LONG_CASTLE          = 1 << 1,
        SHORT_CASTLE         = 1 << 2,
        INVALID              = 1 << 3,
        NONE                 = 1 << 4,
        REGULAR              = 1 << 5,
        EN_PASSANT_CAPTURE   = 1 << 6,
        QUEEN_PROMO          = 1 << 7,
        ROOK_PROMO           = 1 << 8,
        BISHOP_PROMO         = 1 << 9,
        KNIGHT_PROMO         = 1 << 10,
        EN_PASSANT_SQUARE    = 1 << 11,
        PAWN_PROMOTION       = 1 << 12,
    };

    // these must be pointers for the sake of incremental updating
    Bitboard* pieceBB;
    Bitboard* colourBB;

    // if these are not null pointers then a capture has taken place
    Bitboard* capturedPieceBB  = nullptr;
    Bitboard* capturedColourBB = nullptr;

    Bitboard enPassantBB = 0; // enPassantBB can only be certain values (squares on files 3 and 5), so a value of 0 indicates no en passant squares

    Colour side;

    // only a byte long because they are only 0-63
    Byte originSquare;
    Byte targetSquare;

    // castling rights, en passant, half-move counter... etc https://www.chessprogramming.org/Encoding_Moves
    EncodingBits moveType  = EncodingBits::REGULAR;
    Byte castlePrivilegesRevoked = 0;
    Byte moveScore = 0;

    void setMoveType(EncodingBits mt) { moveType = mt; }
};

#endif /* MoveData_hpp */
