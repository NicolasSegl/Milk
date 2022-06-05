#ifndef MoveData_hpp
#define MoveData_hpp

typedef uint8_t  Byte;
typedef uint16_t DoubleByte;
typedef bool Colour;

struct MoveData
{
    // colour bitboards are unnecessary because we have the colour of the side that just moved?

    // these must be pointers for the sake of incremental updating
    Bitboard* pieceBB;
    //Bitboard* colourBB;

    // if these are not null pointers then a capture has taken place
    Bitboard* capturedPieceBB  = nullptr;
//    Bitboard* capturedColourBB = nullptr;

    Colour side;

    // only a byte long because they are only 0-63
    Byte  originSquare;
    Byte  targetSquare;

    // castling rights, en passant, half-move counter... etc

    //DoubleByte data;
};

#endif /* MoveData_hpp */
