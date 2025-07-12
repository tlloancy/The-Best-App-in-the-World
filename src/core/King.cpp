#include "../../include/core/King.hpp"
#include "../../include/core/Board.hpp"
#include <iostream>

constexpr int WHITE_CASTLE_KINGSIDE_KING_TO = 6;
constexpr int WHITE_CASTLE_KINGSIDE_PASS = 5;
constexpr int WHITE_CASTLE_QUEENSIDE_KING_TO = 2;
constexpr int WHITE_CASTLE_QUEENSIDE_PASS = 3;
constexpr int BLACK_CASTLE_KINGSIDE_KING_TO = 62;
constexpr int BLACK_CASTLE_KINGSIDE_PASS = 61;
constexpr int BLACK_CASTLE_QUEENSIDE_KING_TO = 58;
constexpr int BLACK_CASTLE_QUEENSIDE_PASS = 59;

King::King(Color c) : color_(c) {}

PieceType King::getType() const {
    return PieceType::King;
}

Color King::getColor() const {
    return color_;
}

Bitboard King::generateMoves(const Board& board, int square) const {
    Bitboard moves = generateAttacks(board, square);
    if (board.isCheck()) return moves;
    if (getColor() == Color::White) {
        if (board.canCastleWhiteKingside() && !board.getPieces()[WHITE_CASTLE_KINGSIDE_PASS] && !board.getPieces()[WHITE_CASTLE_KINGSIDE_KING_TO]) {
            Board temp(board);
            temp.performMove(square, WHITE_CASTLE_KINGSIDE_PASS);
            if (!temp.isKingInCheck(getColor())) {
                moves.setBit(WHITE_CASTLE_KINGSIDE_KING_TO);
            }
        }
        if (board.canCastleWhiteQueenside() && !board.getPieces()[WHITE_CASTLE_QUEENSIDE_KING_TO + 1] && !board.getPieces()[WHITE_CASTLE_QUEENSIDE_PASS] && !board.getPieces()[WHITE_CASTLE_QUEENSIDE_KING_TO]) {
            Board temp(board);
            temp.performMove(square, WHITE_CASTLE_QUEENSIDE_PASS);
            if (!temp.isKingInCheck(getColor())) {
                moves.setBit(WHITE_CASTLE_QUEENSIDE_KING_TO);
            }
        }
    } else {
        if (board.canCastleBlackKingside() && !board.getPieces()[BLACK_CASTLE_KINGSIDE_PASS] && !board.getPieces()[BLACK_CASTLE_KINGSIDE_KING_TO]) {
            Board temp(board);
            temp.performMove(square, BLACK_CASTLE_KINGSIDE_PASS);
            if (!temp.isKingInCheck(getColor())) {
                moves.setBit(BLACK_CASTLE_KINGSIDE_KING_TO);
            }
        }
        if (board.canCastleBlackQueenside() && !board.getPieces()[BLACK_CASTLE_QUEENSIDE_KING_TO + 1] && !board.getPieces()[BLACK_CASTLE_QUEENSIDE_PASS] && !board.getPieces()[BLACK_CASTLE_QUEENSIDE_KING_TO]) {
            Board temp(board);
            temp.performMove(square, BLACK_CASTLE_QUEENSIDE_PASS);
            if (!temp.isKingInCheck(getColor())) {
                moves.setBit(BLACK_CASTLE_QUEENSIDE_KING_TO);
            }
        }
    }
    return moves;
}

Bitboard King::generateAttacks(const Board& board, int square) const {
    Bitboard attacks(0);
    int rank = square / 8;
    int file = square % 8;
    Color ownColor = getColor();
    int kingMoves[8][2] = {{1, 0}, {1, 1}, {1, -1}, {0, 1}, {0, -1}, {-1, 0}, {-1, 1}, {-1, -1}};

    for (auto& move : kingMoves) {
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

Piece* King::clone() const {
    return new King(*this);
}
