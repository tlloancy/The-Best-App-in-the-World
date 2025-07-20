#include "../../include/core/Bishop.hpp"
#include "../../include/core/Board.hpp"
#include "../../include/utils/Logger.hpp"
#include <iostream>
#include <array>
#include <cstdint>

extern "C" void _generate_bishop_attacks(const Board* board, int square, Bitboard* attacks, const int* directions, const Piece** pieces);

Bishop::Bishop(Color c) : color_(c) {}

PieceType Bishop::getType() const {
    return PieceType::Bishop;
}

Color Bishop::getColor() const {
    return color_;
}

Bitboard Bishop::generateMoves(const Board& board, int square) const {
    Bitboard moves = generateAttacks(board, square);
    // Debug bitboard output
    static int debug_count = 0;
    if (debug_count++ < 5 && board.getPieces()[square] && board.getPieces()[square]->getType() == PieceType::Bishop) {
        std::string log = "Bishop moves from " + std::string(1, 'a' + (square % 8)) + std::to_string(8 - (square / 8)) + ": ";
        bool has_moves = false;
        for (int i = 0; i < 64; ++i) {
            if (moves.testBit(i)) {
                log += std::string(1, 'a' + (i % 8)) + std::to_string(8 - (i / 8)) + " ";
                has_moves = true;
            }
        }
        if (!has_moves) log += "(none)";
        log += "(Bitboard: 0x" + std::to_string(moves.getValue()) + ")";
        Logger::log(log);
        // Verify bitboard coherence
        Bitboard asm_moves(0);
        static const int bishopDirs[4][2] = {{1,1}, {1,-1}, {-1,1}, {-1,-1}};
        std::array<const Piece*, 64> piece_ptrs;
        for (size_t i = 0; i < 64; ++i) {
            piece_ptrs[i] = board.getPieces()[i].get();
        }
        _generate_bishop_attacks(&board, square, &asm_moves, &bishopDirs[0][0], piece_ptrs.data());
        if (asm_moves.getValue() != moves.getValue()) {
            Logger::log("Bitboard mismatch! Expected: 0x" + std::to_string(moves.getValue()) + ", Got from ASM: 0x" + std::to_string(asm_moves.getValue()));
        }
        // Debug direction array
        std::string dir_log = "Directions: ";
        for (int i = 0; i < 4; ++i) {
            dir_log += "(" + std::to_string(bishopDirs[i][0]) + "," + std::to_string(bishopDirs[i][1]) + ") ";
        }
        Logger::log(dir_log);
    }
    return moves;
}

Bitboard Bishop::generateAttacks(const Board& board, int square) const {
    Bitboard attacks(0);
    static const int bishopDirs[4][2] = {{1,1}, {1,-1}, {-1,1}, {-1,-1}};
    std::array<const Piece*, 64> piece_ptrs;
    for (size_t i = 0; i < 64; ++i) {
        piece_ptrs[i] = board.getPieces()[i].get();
    }
    _generate_bishop_attacks(&board, square, &attacks, &bishopDirs[0][0], piece_ptrs.data());
    return attacks;
}

/*
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
*/
