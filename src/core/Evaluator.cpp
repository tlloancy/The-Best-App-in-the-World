#include "../../include/core/Evaluator.hpp"

float Evaluator::materialScore(const Board& board) const {
    float score = 0.0f;
    for (const auto& piece : board.getPieces()) {
        if (piece) {
            float value = 0.0f;
            switch (piece->getType()) {
                case PieceType::Pawn: value = 1.0f; break;
                case PieceType::Knight: value = 3.0f; break;
                case PieceType::Bishop: value = 3.0f; break;
                case PieceType::Rook: value = 5.0f; break;
                case PieceType::Queen: value = 9.0f; break;
                case PieceType::King: value = 0.0f; break;
            }
            score += (piece->getColor() == Color::White) ? value : -value;
        }
    }
    return score;
}

float Evaluator::positionalScore(const Board& board) const {
    float score = 0.0f;
    for (int square = 0; square < 64; ++square) {
        if (board.getPieces()[square]) {
            int rank = square / 8;
            int file = square % 8;
            float centerBonus = (file >= 3 && file <= 4 && rank >= 3 && rank <= 4) ? 0.5f : 0.0f;
            score += (board.getPieces()[square]->getColor() == Color::White) ? centerBonus : -centerBonus;
        }
    }
    return score;
}

float Evaluator::kingSafetyScore(const Board& board) const {
    float score = 0.0f;
    int whiteKingSquare = board.findKing(Color::White);
    int blackKingSquare = board.findKing(Color::Black);
    if (whiteKingSquare != -1) {
        int rank = whiteKingSquare / 8;
        score += (rank == 0 || rank == 1) ? 0.2f : -0.2f;
    }
    if (blackKingSquare != -1) {
        int rank = blackKingSquare / 8;
        score += (rank == 7 || rank == 6) ? -0.2f : 0.2f;
    }
    return score;
}

float Evaluator::evaluate(const Board& board) const {
    if (board.isCheckmate()) {
        return board.isWhiteToMove() ? -1000.0f : 1000.0f;
    }
    if (board.isStalemate() || board.isDraw()) {
        return 0.0f;
    }
    return materialScore(board) + positionalScore(board) + kingSafetyScore(board);
}
