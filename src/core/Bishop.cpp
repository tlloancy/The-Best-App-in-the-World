#include "../../include/core/Bishop.hpp"
#include "../../include/core/Board.hpp"
#include <iostream>

Bishop::Bishop(Color c) : color_(c) {}

PieceType Bishop::getType() const {
    return PieceType::Bishop;
}

Color Bishop::getColor() const {
    return color_;
}

Bitboard Bishop::generateMoves(const Board& board, int square) const {
    return generateAttacks(board, square);
}

Bitboard Bishop::generateAttacks(const Board& board, int square) const {
    Bitboard attacks(0);
    int rank = square / 8;
    int file = square % 8;
    Color ownColor = getColor();
    int bishopDirs[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

    for (auto& dir : bishopDirs) {
        int newRank = rank + dir[0];
        int newFile = file + dir[1];
        while (newRank >= 0 && newRank < 8 && newFile >= 0 && newFile < 8) {
            int newSquare = newRank * 8 + newFile;
            if (board.getPieces()[newSquare]) {
                if (board.getPieces()[newSquare]->getColor() != ownColor) {
                    attacks.setBit(newSquare);
                }
                break;
            }
            attacks.setBit(newSquare);
            newRank += dir[0];
            newFile += dir[1];
        }
    }
    return attacks;
}
