#include "../../include/core/Queen.hpp"
#include "../../include/core/Board.hpp"
#include <iostream>

extern "C" void _generate_queen_attacks(const Board* board, int square, Bitboard* attacks, const int* directions, const Piece** pieces);

Queen::Queen(Color c) : color_(c) {}

PieceType Queen::getType() const {
    return PieceType::Queen;
}

Color Queen::getColor() const {
    return color_;
}

Bitboard Queen::generateMoves(const Board& board, int square) const {
    return generateAttacks(board, square);
}

Bitboard Queen::generateAttacks(const Board& board, int square) const {
    Bitboard attacks(0);
    static const int queenDirs[8][2] = {{1, 0}, {1, 1}, {1, -1}, {0, 1}, {0, -1}, {-1, 0}, {-1, 1}, {-1, -1}};
    std::array<const Piece*, 64> piece_ptrs;
    for (size_t i = 0; i < 64; ++i) {
        piece_ptrs[i] = board.getPieces()[i].get();
    }
    _generate_queen_attacks(&board, square, &attacks, &queenDirs[0][0], piece_ptrs.data());
    return attacks;
}
/*
Bitboard Queen::generateAttacks(const Board& board, int square) const {
    Bitboard attacks(0);
    int rank = square / 8;
    int file = square % 8;
    Color ownColor = getColor();
    int queenDirs[8][2] = {{1, 0}, {1, 1}, {1, -1}, {0, 1}, {0, -1}, {-1, 0}, {-1, 1}, {-1, -1}};

    for (auto& dir : queenDirs) {
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
*/
