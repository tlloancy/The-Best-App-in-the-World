#ifndef PIECE_HPP
#define PIECE_HPP

#include "../utils/Bitboard.hpp"

enum class Color { White, Black };
enum class PieceType { Pawn, Knight, Bishop, Rook, Queen, King };

class Piece {
public:
    virtual ~Piece() = default;
    virtual Color getColor() const = 0;
    virtual PieceType getType() const = 0;
    virtual Piece* clone() const = 0;
    virtual Bitboard generateMoves(Bitboard occupied, int square) const = 0;
};

#endif