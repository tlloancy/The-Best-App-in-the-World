#ifndef KNIGHT_HPP
#define KNIGHT_HPP

#include "Piece.hpp"

class Knight : public Piece {
public:
    Knight(Color c);
    PieceType getType() const override;
    Color getColor() const override;
    Bitboard generateMoves(const Board& board, int square) const override;
    Bitboard generateAttacks(const Board& board, int square) const override;
    Piece* clone() const override { return new Knight(*this); }
private:
    Color color_;
};

#endif
