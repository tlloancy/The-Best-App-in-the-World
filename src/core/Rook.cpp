#include "../../include/core/Rook.hpp"
#include "../../include/core/Board.hpp"
#include <iostream>

extern "C" void _generate_rook_attacks(const Board* board, int square, Bitboard* attacks, const int* directions, const Piece** pieces);

Rook::Rook(Color c) : color_(c) {}

PieceType Rook::getType() const {
    return PieceType::Rook;
}

Color Rook::getColor() const {
    return color_;
}

Bitboard Rook::generateMoves(const Board& board, int square) const {
    return generateAttacks(board, square);
}

Bitboard Rook::generateAttacks(const Board& board, int square) const {
    Bitboard attacks(0);
    static const int rookDirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
    std::array<const Piece*, 64> piece_ptrs;
    for (size_t i = 0; i < 64; ++i) {
        piece_ptrs[i] = board.getPieces()[i].get();
    }
    _generate_rook_attacks(&board, square, &attacks, &rookDirs[0][0], piece_ptrs.data());
    return attacks;
}
/*
Bitboard Rook::generateAttacks(const Board& board, int square) const {
    Bitboard attacks(0);
    int rank = square / 8;
    int file = square % 8;
    Color ownColor = getColor();
    int rookDirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

    for (auto& dir : rookDirs) {
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
