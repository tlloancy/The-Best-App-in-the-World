#ifndef MCTS_HPP
#define MCTS_HPP

#include "../core/Board.hpp"
#include "../core/Move.hpp"
#include "../engine/Search.hpp"

class MCTS {
public:
    SearchResult evaluate(const Board& board, int iterations);
};

#endif
