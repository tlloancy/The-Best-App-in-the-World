#ifndef PIECE_HPP
#define PIECE_HPP

#include "../utils/Bitboard.hpp"

class Board;

enum class PieceType {
    Pawn,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};

enum class Color {
    White,
    Black
};

class Piece {
public:
    virtual ~Piece() = default;
    virtual PieceType getType() const = 0;
    virtual Color getColor() const = 0;
    virtual Bitboard generateMoves(const Board& board, int square) const = 0;
    virtual Bitboard generateAttacks(const Board& board, int square) const = 0;
    virtual Piece* clone() const = 0;
protected:
    Piece() = default;
};

#endif
