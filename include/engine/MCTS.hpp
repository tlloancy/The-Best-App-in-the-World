#ifndef MCTS_HPP
#define MCTS_HPP

#include "../../include/core/Board.hpp"
#include "../../include/core/Move.hpp"
#include "Search.hpp"
#include <vector>
#include <string>
#include <cstdio>

class MCTS {
public:
    SearchResult evaluate(const Board& board, int iterations);
};

class MCTSSearch {
public:
    MCTSSearch(int skillLevel = 10);
    ~MCTSSearch();
    SearchResult search(const Board& board, int depth, std::string* uciMove = nullptr, std::vector<SearchResult>* topResults = nullptr);
private:
    void startStockfish();
    std::string getBestMoveFromStockfish(const Board& board);
    int skillLevel_;
    FILE* writePipe_ = nullptr;
    FILE* readPipe_ = nullptr;
    pid_t childPid_ = -1;
};

#endif
