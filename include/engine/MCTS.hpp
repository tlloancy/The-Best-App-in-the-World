#ifndef MCTS_HPP
#define MCTS_HPP

#include "../../include/core/Board.hpp"
#include "Search.hpp"
#include <vector>
#include <string>
#include <utility>
#include <cstdio>

class MCTS {
public:
    SearchResult evaluate(const Board& board, int iterations);
};

class MCTSSearch {
public:
    MCTSSearch(int elo = 1500, int thinkTimeMs = 0);
    ~MCTSSearch();
    SearchResult search(const Board& board, int depth, std::string* uciMove = nullptr);
    void setSkillLevel(int elo, int thinkTimeMs);
    int getElo() const { return elo_; }
    int getThinkTime() const { return thinkTimeMs_; }
private:
    void startStockfish();
    std::string getBestMoveFromStockfish(const Board& board);
    int elo_;
    int thinkTimeMs_;
    FILE* writePipe_ = nullptr;
    FILE* readPipe_ = nullptr;
    pid_t childPid_ = -1;
};

#endif // MCTS_HPP
