#ifndef SEARCH_HPP
#define SEARCH_HPP

#include "../../include/core/Board.hpp"
#include "../../include/core/Move.hpp"
#include <vector>
#include <string>
#include <cstdio>

struct SearchResult {
    float score;
    Move bestMove;
    std::vector<Move> topMoves;
};

class Search {
public:
    virtual SearchResult search(const Board& board, int depth) = 0;
    virtual ~Search() = default;
};

class StockfishSearch : public Search {
public:
    StockfishSearch(int skillLevel = 10);
    ~StockfishSearch();
    SearchResult search(const Board& board, int depth) override;
private:
    void startStockfish();
    std::string getBestMoveFromStockfish(const Board& board);
    int skillLevel_;
    FILE* writePipe_ = nullptr;
    FILE* readPipe_ = nullptr;
    pid_t childPid_ = -1;
};

#endif
