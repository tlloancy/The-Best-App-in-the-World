#include "../../include/core/Knight.hpp"
#include "../../include/core/Board.hpp"
#include <iostream>

extern "C" void _generate_knight_attacks(const Board* board, int square, Bitboard* attacks, const int* directions, const Piece** pieces);

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
    static const int knightDirs[8][2] = {{2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};
    std::array<const Piece*, 64> piece_ptrs;
    for (size_t i = 0; i < 64; ++i) {
        piece_ptrs[i] = board.getPieces()[i].get();
    }
    _generate_knight_attacks(&board, square, &attacks, &knightDirs[0][0], piece_ptrs.data());
    return attacks;
}
/*
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
*/
