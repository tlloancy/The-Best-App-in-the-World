#ifndef KING_HPP
#define KING_HPP

#include "Piece.hpp"
#include "../utils/Bitboard.hpp"

class King : public Piece {
public:
    King(const King& other) = default;
    King(Color c) : color_(c) {}
    virtual ~King() = default;
    King& operator=(const King& other) = default;
    Color getColor() const override { return color_; }
    PieceType getType() const override { return PieceType::King; }
    Piece* clone() const override { return new King(*this); }
    virtual Bitboard generateMoves(Bitboard occupied, int square) const;

private:
    Color color_;
};

#endif