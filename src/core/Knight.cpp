#include "../../include/core/Knight.hpp"
#include "../../include/core/Board.hpp"
#include <iostream>

Knight::Knight(Color c) : color_(c) {}

PieceType Knight::getType() const {
    return PieceType::Knight;
}

Color Knight::getColor() const {
    return color_;
}

Bitboard Knight::generateMoves(const Board& board, int square) const {
    return generateAttacks(board, square);
}

Bitboard Knight::generateAttacks(const Board& board, int square) const {
    Bitboard attacks(0);
    int rank = square / 8;
    int file = square % 8;
    Color ownColor = getColor();
    int knightMoves[8][2] = {{2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};

    for (auto& move : knightMoves) {
        int newRank = rank + move[0];
        int newFile = file + move[1];
        if (newRank >= 0 && newRank < 8 && newFile >= 0 && newFile < 8) {
            int newSquare = newRank * 8 + newFile;
            if (!board.getPieces()[newSquare] || board.getPieces()[newSquare]->getColor() != ownColor) {
                attacks.setBit(newSquare);
            }
        }
    }
    return attacks;
}

Piece* Knight::clone() const {
    return new Knight(*this);
}
