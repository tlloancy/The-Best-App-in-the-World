#include "../../include/gui/Renderer.hpp"
#include "../../include/core/Pawn.hpp"
#include <iostream>

Bitboard Pawn::generateMoves(Bitboard occupied, int square) const {
    Bitboard moves = Bitboard(0);
    int displayRow = 7 - (square / 8);
    int col = square % 8;
    Color color = getColor();
    int direction = (color == Color::White) ? 8 : -8;

    if (globalRenderer && globalRenderer->getDebugEnabled()) {
        std::cout << "[DEBUG] Pawn at " << square << " (display row: " << displayRow << ", col: " << col << ") - occupied_: " << std::hex << occupied.getValue() << std::dec << std::endl;
    }

    int advanceSquare = square + direction;
    if (advanceSquare >= 0 && advanceSquare < 64) {
        if (!(occupied & Bitboard(1ULL << advanceSquare))) {
            moves |= Bitboard(1ULL << advanceSquare);
            if (globalRenderer && globalRenderer->getDebugEnabled()) {
                std::cout << "[DEBUG] Pawn at " << square << " can advance to " << advanceSquare << std::endl;
            }
            if (((displayRow == 6 && color == Color::White) || (displayRow == 1 && color == Color::Black)) &&
                !(occupied & Bitboard(1ULL << (advanceSquare + direction)))) {
                moves |= Bitboard(1ULL << (advanceSquare + direction));
                if (globalRenderer && globalRenderer->getDebugEnabled()) {
                    std::cout << "[DEBUG] Pawn at " << square << " can double advance to " << (advanceSquare + direction) << std::endl;
                }
            }
        }
    }
    if (col > 0) {
        int leftCapture = advanceSquare - 1;
        if (leftCapture >= 0 && leftCapture < 64 && (occupied & Bitboard(1ULL << leftCapture))) {
            moves |= Bitboard(1ULL << leftCapture);
            if (globalRenderer && globalRenderer->getDebugEnabled()) {
                std::cout << "[DEBUG] Pawn at " << square << " can capture left to " << leftCapture << std::endl;
            }
        }
    }
    if (col < 7) {
        int rightCapture = advanceSquare + 1;
        if (rightCapture >= 0 && rightCapture < 64 && (occupied & Bitboard(1ULL << rightCapture))) {
            moves |= Bitboard(1ULL << rightCapture);
            if (globalRenderer && globalRenderer->getDebugEnabled()) {
                std::cout << "[DEBUG] Pawn at " << square << " can capture right to " << rightCapture << std::endl;
            }
        }
    }
    if (globalRenderer && globalRenderer->getDebugEnabled()) {
        std::cout << "[DEBUG] Pawn moves for " << square << ": " << std::hex << moves.getValue() << std::dec << std::endl;
    }
    return moves;
}
