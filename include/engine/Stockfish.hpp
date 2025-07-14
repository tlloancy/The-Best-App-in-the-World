#ifndef STOCKFISH_HPP
#define STOCKFISH_HPP

#include <string>
#include <cstdio>
#include <utility>

class Stockfish {
public:
    Stockfish();
    ~Stockfish();
    std::pair<std::string, float> getBestMoveAndScore(const std::string& fen, int depth);
private:
    FILE* pipe;
};

#endif
