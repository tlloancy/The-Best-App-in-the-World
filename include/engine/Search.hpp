#ifndef SEARCH_HPP
#define SEARCH_HPP

#include <concepts>
#include <functional>
#include <vector>
#include "../core/Board.hpp"
#include "../core/Move.hpp"

template<typename T>
concept SearchAlgorithm = requires(T t, const Board& board, int depth) {
    { t.search(board, depth) } -> std::convertible_to<float>;
};

struct SearchResult {
    float score;
    Move bestMove;
    std::vector<Move> topMoves;
};

class Search {
public:
    virtual ~Search() = default;
    virtual SearchResult search(const Board& board, int depth) = 0;
};

template<typename Derived>
class SearchBase : public Search {
public:
    SearchResult search(const Board& board, int depth) override {
        return static_cast<Derived*>(this)->impl_search(board, depth);
    }
};

class MinimaxSearch : public SearchBase<MinimaxSearch> {
public:
    SearchResult impl_search(const Board& board, int depth);
};

class MCTSSearch : public SearchBase<MCTSSearch> {
public:
    SearchResult impl_search(const Board& board, int depth);
};

#endif
