#include "../../include/engine/MCTS.hpp"
#include "../../include/core/Evaluator.hpp"
#include <random>
#include <cmath>

struct MCTSNode {
    Board board;
    Move move;
    int visits = 0;
    float totalScore = 0.0f;
    std::vector<MCTSNode*> children;
    MCTSNode* parent = nullptr;

    MCTSNode(const Board& b, const Move& m = {0, 0}) : board(b), move(m) {}
    ~MCTSNode() { for (auto child : children) delete child; }

    float uctScore(int parentVisits) const {
        if (visits == 0) return std::numeric_limits<float>::max();
        return (totalScore / visits) + 1.414f * std::sqrt(std::log(parentVisits) / visits);
    }
};

SearchResult MCTS::evaluate(const Board& board, int iterations) {
    SearchResult result;
    result.score = 0.0f;
    result.bestMove = {0, 0};
    result.topMoves.clear();

    MCTSNode* root = new MCTSNode(board);
    std::random_device rd;
    std::mt19937 gen(rd());

    for (int i = 0; i < iterations; ++i) {
        MCTSNode* node = root;

        // Selection
        while (!node->children.empty()) {
            node = *std::max_element(node->children.begin(), node->children.end(),
                [node](const MCTSNode* a, const MCTSNode* b) { return a->uctScore(node->visits) < b->uctScore(node->visits); });
        }

        // Expansion
        if (node->visits > 0 && !node->board.isCheckmate() && !node->board.isStalemate() && !node->board.isDraw()) {
            auto moves = node->board.getLegalMoves(node->board.isWhiteToMove() ? Color::White : Color::Black);
            for (const auto& move : moves) {
                Board temp(node->board);
                temp.movePiece(move.first, move.second);
                MCTSNode* child = new MCTSNode(temp, {move.first, move.second});
                child->parent = node;
                node->children.push_back(child);
            }
            if (!node->children.empty()) {
                std::uniform_int_distribution<size_t> dist(0, node->children.size() - 1);
                node = node->children[dist(gen)];
            }
        }

        // Simulation
        Board simBoard(node->board);
        float simScore = 0.0f;
        int simDepth = 0;
        while (simDepth < 10 && !simBoard.isCheckmate() && !simBoard.isStalemate() && !simBoard.isDraw()) {
            auto moves = simBoard.getLegalMoves(simBoard.isWhiteToMove() ? Color::White : Color::Black);
            if (moves.empty()) break;
            std::uniform_int_distribution<size_t> dist(0, moves.size() - 1);
            auto move = moves[dist(gen)];
            simBoard.movePiece(move.first, move.second);
            simDepth++;
        }
        Evaluator evaluator;
        simScore = evaluator.evaluate(simBoard);

        // Backpropagation
        while (node) {
            node->visits++;
            node->totalScore += simScore;
            simScore = -simScore;
            node = node->parent;
        }
    }

    if (!root->children.empty()) {
        auto bestChild = *std::max_element(root->children.begin(), root->children.end(),
            [](const MCTSNode* a, const MCTSNode* b) { return a->totalScore / a->visits < b->totalScore / b->visits; });
        result.score = bestChild->totalScore / bestChild->visits;
        result.bestMove = bestChild->move;

        std::vector<std::pair<MCTSNode*, float>> scoredChildren;
        for (auto child : root->children) {
            scoredChildren.emplace_back(child, child->visits > 0 ? child->totalScore / child->visits : 0.0f);
        }
        std::sort(scoredChildren.begin(), scoredChildren.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });
        for (size_t i = 0; i < scoredChildren.size() && i < 3; ++i) {
            result.topMoves.push_back(scoredChildren[i].first->move);
        }
    }

    delete root;
    return result;
}

SearchResult MCTSSearch::impl_search(const Board& board, int depth) {
    MCTS mcts;
    return mcts.evaluate(board, 100);
}
