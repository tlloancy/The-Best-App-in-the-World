#include "../../include/gui/Renderer.hpp"
#include "../../include/core/Bishop.hpp"
#include <iostream>

Bitboard Bishop::generateMoves(Bitboard occupied, int square) const {
    Bitboard moves = Bitboard(0);
    int displayRow = 7 - (square / 8);
    int col = square % 8;
    int directions[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

    for (auto& dir : directions) {
        for (int i = 1; i < 8; ++i) {
            int newDisplayRow = displayRow + i * dir[0];
            int newCol = col + i * dir[1];
            if (newDisplayRow < 0 || newDisplayRow >= 8 || newCol < 0 || newCol >= 8) break;
            int newSquare = (7 - newDisplayRow) * 8 + newCol;
            moves |= Bitboard(1ULL << newSquare);
            if (occupied & Bitboard(1ULL << newSquare)) break;
            if (globalRenderer && globalRenderer->getDebugEnabled()) {
                std::cout << "[DEBUG] Bishop at " << square << " can move to " << newSquare << std::endl;
            }
        }
    }
    if (globalRenderer && globalRenderer->getDebugEnabled()) {
        std::cout << "[DEBUG] Bishop moves for " << square << ": " << std::hex << moves.getValue() << std::dec << std::endl;
    }
    return moves;
}
