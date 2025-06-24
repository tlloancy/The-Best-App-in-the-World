#include "../../include/core/Piece.hpp"

Bitboard PieceFactory::generatePawnMoves(Bitboard board, uint8_t square) {
    if (square >= 8 && square < 56) {
        return Bitboard(1ULL << (square + 8));
    }
    return Bitboard(0);
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

Bitboard PieceFactory::generateBishopMoves(Bitboard board, uint8_t square) { return Bitboard(0); }
Bitboard PieceFactory::generateRookMoves(Bitboard board, uint8_t square) { return Bitboard(0); }
Bitboard PieceFactory::generateQueenMoves(Bitboard board, uint8_t square) { return Bitboard(0); }
Bitboard PieceFactory::generateKingMoves(Bitboard board, uint8_t square) { return Bitboard(0); }