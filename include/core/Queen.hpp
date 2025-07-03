#ifndef QUEEN_HPP
#define QUEEN_HPP

#include "Piece.hpp"
#include "../utils/Bitboard.hpp"

class Queen : public Piece {
public:
    Queen(const Queen& other) = default;
    Queen(Color c) : color_(c) {}
    virtual ~Queen() = default;
    Queen& operator=(const Queen& other) = default;
    Color getColor() const override { return color_; }
    PieceType getType() const override { return PieceType::Queen; }
    Piece* clone() const override { return new Queen(*this); }
    virtual Bitboard generateMoves(Bitboard occupied, int square) const;

private:
    Color color_;
};

#endif