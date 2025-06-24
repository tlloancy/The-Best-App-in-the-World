#pragma once
#include <concepts>
#include <functional>
#include "../core/Board.hpp"

template<typename T>
concept SearchAlgorithm = requires(T t, const Board& board, int depth) {
    { t.search(board, depth) } -> std::convertible_to<float>;
};

class Search {
public:
    virtual ~Search() = default;
    virtual float search(const Board& board, int depth) = 0;
};

template<typename Derived>
class SearchBase : public Search {
public:
    float search(const Board& board, int depth) override {
        return static_cast<Derived*>(this)->impl_search(board, depth);
    }
};

class MinimaxSearch : public SearchBase<MinimaxSearch> {
public:
    float impl_search(const Board& board, int depth);
};

class MCTSSearch : public SearchBase<MCTSSearch> {
public:
    float impl_search(const Board& board, int depth);
};