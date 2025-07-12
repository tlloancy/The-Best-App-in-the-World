#include "../../include/engine/Search.hpp"
#include "../../include/core/Evaluator.hpp"
#include <algorithm>
#include <limits>

float evaluateMove(const Board& board, const std::pair<uint8_t, uint8_t>& move) {
    Board temp(board);
    temp.movePiece(move.first, move.second);
    Evaluator evaluator;
    return evaluator.evaluate(temp);
}

SearchResult MinimaxSearch::impl_search(const Board& board, int depth) {
    SearchResult result;
    result.score = std::numeric_limits<float>::lowest();
    result.bestMove = {0, 0};
    result.topMoves.clear();

    if (depth == 0 || board.isCheckmate() || board.isStalemate() || board.isDraw()) {
        Evaluator evaluator;
        result.score = evaluator.evaluate(board);
        return result;
    }

    Color color = board.isWhiteToMove() ? Color::White : Color::Black;
    auto moves = board.getLegalMoves(color);
    if (moves.empty()) {
        result.score = board.isCheckmate() ? (color == Color::White ? -1000.0f : 1000.0f) : 0.0f;
        return result;
    }

    std::vector<std::pair<std::pair<uint8_t, uint8_t>, float>> moveScores;
    for (const auto& move : moves) {
        moveScores.emplace_back(move, evaluateMove(board, move));
    }
    std::sort(moveScores.begin(), moveScores.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    float alpha = std::numeric_limits<float>::lowest();
    float beta = std::numeric_limits<float>::max();
    for (size_t i = 0; i < moveScores.size() && i < 3; ++i) {
        Board temp(board);
        temp.movePiece(moveScores[i].first.first, moveScores[i].first.second);
        MinimaxSearch subSearch;
        SearchResult subResult = subSearch.impl_search(temp, depth - 1);
        float score = -subResult.score;
        if (score > result.score) {
            result.score = score;
            result.bestMove = {moveScores[i].first.first, moveScores[i].first.second};
        }
        result.topMoves.push_back({moveScores[i].first.first, moveScores[i].first.second});
        alpha = std::max(alpha, score);
        if (alpha >= beta) break;
    }

    return result;
}
