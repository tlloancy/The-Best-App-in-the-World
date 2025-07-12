#ifndef EVALUATOR_HPP
#define EVALUATOR_HPP

#include "Board.hpp"

class Evaluator {
public:
    float evaluate(const Board& board) const;
private:
    float materialScore(const Board& board) const;
    float positionalScore(const Board& board) const;
    float kingSafetyScore(const Board& board) const;
};

#endif
