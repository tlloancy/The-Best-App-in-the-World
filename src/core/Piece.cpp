#include "../../include/core/Piece.hpp"
#include "../../include/utils/Logger.hpp"

Bitboard PieceFactory::generatePawnMoves(Bitboard board, uint8_t square) {
    Bitboard moves = 0;
    int row = square / 8;
    Color color = (row < 4) ? Color::White : Color::Black;
    int direction = (color == Color::White) ? 8 : -8;
    Logger::log("Generating moves for pawn at " + std::to_string(square) + ", row: " + std::to_string(row) + ", color: " + (color == Color::White ? "White" : "Black"));
    if (row > 0 && row < 7 && !(board & Bitboard(1ULL << (square + direction)))) {
        Logger::log("Adding simple move to " + std::to_string(square + direction));
        moves |= Bitboard(1ULL << (square + direction)); // Avance simple
        if ((row == 1 && color == Color::White && !(board & (Bitboard(1ULL << (square + 8)) | Bitboard(1ULL << (square + 16))))) ||
            (row == 6 && color == Color::Black && !(board & (Bitboard(1ULL << (square - 8)) | Bitboard(1ULL << (square - 16)))))) {
            Logger::log("Adding double move to " + std::to_string(square + 2 * direction));
            moves |= Bitboard(1ULL << (square + 2 * direction)); // Double avance
        }
    }
    Logger::log("Pawn moves for " + std::to_string(square) + ": " + std::to_string(moves));
    // Captures (à implémenter)
    return moves;
}

Bitboard PieceFactory::generateKnightMoves(Bitboard board, uint8_t square) {
    Bitboard moves = 0;
    int row = square / 8;
    int col = square % 8;
    int movesList[8][2] = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        {1, -2}, {1, 2}, {2, -1}, {2, 1}
    };
    for (int i = 0; i < 8; ++i) {
        int newRow = row + movesList[i][0];
        int newCol = col + movesList[i][1];
        if (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8) {
            moves |= Bitboard(1ULL << (newRow * 8 + newCol));
        }
    }
    return moves;
}

Bitboard PieceFactory::generateBishopMoves(Bitboard board, uint8_t square) {
    Bitboard moves = 0;
    int row = square / 8;
    int col = square % 8;
    int steps[] = {-1, 1};
    for (int dr : steps) for (int dc : steps) {
        for (int i = 1; i < 8; ++i) {
            int newRow = row + i * dr;
            int newCol = col + i * dc;
            if (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8) {
                moves |= Bitboard(1ULL << (newRow * 8 + newCol));
                if (board & Bitboard(1ULL << (newRow * 8 + newCol))) break;
            } else break;
        }
    }
    return moves;
}

Bitboard PieceFactory::generateRookMoves(Bitboard board, uint8_t square) {
    Bitboard moves = 0;
    int row = square / 8;
    int col = square % 8;
    for (int dr : {-1, 0, 1}) for (int dc : {-1, 0, 1}) {
        if (dr == 0 && dc == 0) continue;
        if (dr != 0 && dc != 0) continue;
        for (int i = 1; i < 8; ++i) {
            int newRow = row + i * dr;
            int newCol = col + i * dc;
            if (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8) {
                moves |= Bitboard(1ULL << (newRow * 8 + newCol));
                if (board & Bitboard(1ULL << (newRow * 8 + newCol))) break;
            } else break;
        }
    }
    return moves;
}

Bitboard PieceFactory::generateQueenMoves(Bitboard board, uint8_t square) {
    return generateBishopMoves(board, square) | generateRookMoves(board, square);
}

Bitboard PieceFactory::generateKingMoves(Bitboard board, uint8_t square) {
    Bitboard moves = 0;
    int row = square / 8;
    int col = square % 8;
    for (int dr : {-1, 0, 1}) for (int dc : {-1, 0, 1}) {
        if (dr == 0 && dc == 0) continue;
        int newRow = row + dr;
        int newCol = col + dc;
        if (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8) {
            moves |= Bitboard(1ULL << (newRow * 8 + newCol));
        }
    }
    return moves;
}