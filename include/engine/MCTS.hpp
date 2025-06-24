#pragma once
#include "../core/Board.hpp"

class MCTS {
public:
    float evaluate(const Board& board, int iterations);
};