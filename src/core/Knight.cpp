#include "../../include/gui/Renderer.hpp"
#include "../../include/core/Knight.hpp"
#include <iostream>

Bitboard Knight::generateMoves(Bitboard occupied, int square) const {
    Bitboard moves = Bitboard(0);
    int displayRow = 7 - (square / 8);
    int col = square % 8;
    int knightMoves[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}};

    for (auto& move : knightMoves) {
        int newDisplayRow = displayRow + move[0];
        int newCol = col + move[1];
        if (newDisplayRow >= 0 && newDisplayRow < 8 && newCol >= 0 && newCol < 8) {
            int newSquare = (7 - newDisplayRow) * 8 + newCol;
            moves |= Bitboard(1ULL << newSquare);
            if (globalRenderer && globalRenderer->getDebugEnabled()) {
                std::cout << "[DEBUG] Knight at " << square << " can move to " << newSquare << std::endl;
            }
        }
    }
    if (globalRenderer && globalRenderer->getDebugEnabled()) {
        std::cout << "[DEBUG] Knight moves for " << square << ": " << std::hex << moves.getValue() << std::dec << std::endl;
    }
    return moves;
}
