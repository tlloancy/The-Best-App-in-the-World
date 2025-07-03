#ifndef ROOK_HPP
#define ROOK_HPP

#include "Piece.hpp"
#include "../utils/Bitboard.hpp"

class Rook : public Piece {
public:
    Rook(const Rook& other) = default;
    Rook(Color c) : color_(c) {}
    virtual ~Rook() = default;
    Rook& operator=(const Rook& other) = default;
    Color getColor() const override { return color_; }
    PieceType getType() const override { return PieceType::Rook; }
    Piece* clone() const override { return new Rook(*this); }
    virtual Bitboard generateMoves(Bitboard occupied, int square) const;

private:
    Color color_;
};

#endif