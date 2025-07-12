#ifndef PAWN_HPP
#define PAWN_HPP

#include "Piece.hpp"

class Pawn : public Piece {
public:
    Pawn(Color c);
    PieceType getType() const override;
    Color getColor() const override;
    Bitboard generateMoves(const Board& board, int square) const override;
    Bitboard generateAttacks(const Board& board, int square) const override;
    Piece* clone() const override;
private:
    Color color_;
};

#endif
