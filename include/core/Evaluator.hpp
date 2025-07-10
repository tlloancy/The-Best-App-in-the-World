#ifndef EVALUATOR_HPP
#define EVALUATOR_HPP

#include "Board.hpp"

class Evaluator {
public:
    float evaluate(const Board& board) const;
};

#endif
