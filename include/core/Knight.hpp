#ifndef KNIGHT_HPP
#define KNIGHT_HPP

#include "Piece.hpp"
#include "../utils/Bitboard.hpp"

class Knight : public Piece {
public:
    Knight(const Knight& other) = default;
    Knight(Color c) : color_(c) {}
    virtual ~Knight() = default;
    Knight& operator=(const Knight& other) = default;
    Color getColor() const override { return color_; }
    PieceType getType() const override { return PieceType::Knight; }
    Piece* clone() const override { return new Knight(*this); }
    virtual Bitboard generateMoves(Bitboard occupied, int square) const;

private:
    Color color_;
};

#endif