#include "../../include/core/Pawn.hpp"
#include "../../include/core/Board.hpp"
#include <iostream>
#include <array>

extern "C" void _generate_pawn_attacks(const Board* board, int square, Bitboard* attacks, const int* directions, const Piece** pieces);

Pawn::Pawn(Color c) : color_(c) {}

PieceType Pawn::getType() const {
    return PieceType::Pawn;
}

Color Pawn::getColor() const {
    return color_;
}

Bitboard Pawn::generateMoves(const Board& board, int square) const {
    Bitboard moves = generateAttacks(board, square);
    int dir = (getColor() == Color::White) ? 8 : -8;
    int startRank = (getColor() == Color::White) ? 1 : 6;
    int rank = square / 8;
    int file = square % 8;
    int forward = square + dir;
    if (forward >= 0 && forward < 64 && !board.getPieces()[forward]) {
        moves.setBit(forward);
        if (rank == startRank) {
            int doubleForward = square + 2 * dir;
            if (doubleForward >= 0 && doubleForward < 64 && !board.getPieces()[doubleForward]) {
                moves.setBit(doubleForward);
            }
        }
    }
    return moves;
}
/*
Bitboard Pawn::generateAttacks(const Board& board, int square) const {
    Bitboard attacks(0);
    static const int pawnDirs[2][2] = {{(getColor() == Color::White) ? 1 : -1, -1}, {(getColor() == Color::White) ? 1 : -1, 1}};
    std::array<const Piece*, 64> piece_ptrs;
    for (size_t i = 0; i < 64; ++i) {
        piece_ptrs[i] = board.getPieces()[i].get();
    }
    _generate_pawn_attacks(&board, square, &attacks, &pawnDirs[0][0], piece_ptrs.data());
    return attacks;
}
*/
Bitboard Pawn::generateAttacks(const Board& board, int square) const {
    Bitboard attacks(0);
    int dir = (getColor() == Color::White) ? 8 : -8;
    int rank = square / 8;
    int file = square % 8;
    int attackDirs[2] = {dir - 1, dir + 1};
    Color opponent = (getColor() == Color::White) ? Color::Black : Color::White;
    for (int attackDir : attackDirs) {
        int attackSquare = square + attackDir;
        if (attackSquare >= 0 && attackSquare < 64 && std::abs((attackSquare % 8) - file) == 1) {
            if (board.getPieces()[attackSquare] && board.getPieces()[attackSquare]->getColor() == opponent) {
                attacks.setBit(attackSquare);
            } else if (attackSquare == board.getEnPassantSquare()) {
                attacks.setBit(attackSquare);
            }
        }
    }
    return attacks;
}

