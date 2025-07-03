#ifndef BISHOP_HPP
#define BISHOP_HPP

#include "Piece.hpp"
#include "../utils/Bitboard.hpp"

class Bishop : public Piece {
public:
    Bishop(const Bishop& other) = default;
    Bishop(Color c) : color_(c) {}
    virtual ~Bishop() = default;
    Bishop& operator=(const Bishop& other) = default;
    Color getColor() const override { return color_; }
    PieceType getType() const override { return PieceType::Bishop; }
    Piece* clone() const override { return new Bishop(*this); }
    virtual Bitboard generateMoves(Bitboard occupied, int square) const;

private:
    Color color_;
};

#endif