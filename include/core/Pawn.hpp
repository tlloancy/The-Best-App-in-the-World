#ifndef PAWN_HPP
#define PAWN_HPP

#include "Piece.hpp"
#include "../utils/Bitboard.hpp"

class Pawn : public Piece {
public:
    Pawn(const Pawn& other) = default;
    Pawn(Color c) : color_(c) {}
    virtual ~Pawn() = default;
    Pawn& operator=(const Pawn& other) = default;
    Color getColor() const override { return color_; }
    PieceType getType() const override { return PieceType::Pawn; }
    Piece* clone() const override { return new Pawn(*this); }
    virtual Bitboard generateMoves(Bitboard occupied, int square) const;

private:
    Color color_;
};

#endif