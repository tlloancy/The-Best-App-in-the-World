#include "../../include/gui/Renderer.hpp"
#include "../../include/core/King.hpp"
#include <iostream>

Bitboard King::generateMoves(Bitboard occupied, int square) const {
    Bitboard moves = Bitboard(0);
    int displayRow = 7 - (square / 8);
    int col = square % 8;

    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue;
            int newDisplayRow = displayRow + dr;
            int newCol = col + dc;
            if (newDisplayRow >= 0 && newDisplayRow < 8 && newCol >= 0 && newCol < 8) {
                int newSquare = (7 - newDisplayRow) * 8 + newCol;
                moves |= Bitboard(1ULL << newSquare);
                if (globalRenderer && globalRenderer->getDebugEnabled()) {
                    std::cout << "[DEBUG] King at " << square << " can move to " << newSquare << std::endl;
                }
            }
        }
    }
    if (globalRenderer && globalRenderer->getDebugEnabled()) {
        std::cout << "[DEBUG] King moves for " << square << ": " << std::hex << moves.getValue() << std::dec << std::endl;
    }
    return moves;
}
