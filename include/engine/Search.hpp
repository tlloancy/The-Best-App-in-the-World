#ifndef SEARCH_HPP
#define SEARCH_HPP

#include "../../include/core/Board.hpp"
#include <vector>
#include <string>
#include <utility>

struct SearchResult {
    float score;
    std::pair<int, int> bestMove;
    std::vector<std::pair<int, int>> topMoves;
};

class Search {
public:
    virtual SearchResult search(const Board& board, int depth, std::string* uciMove = nullptr) = 0;
    virtual ~Search() = default;
};

class StockfishSearch : public Search {
public:
    StockfishSearch(int skillLevel = 10);
    ~StockfishSearch();
    SearchResult search(const Board& board, int depth, std::string* uciMove = nullptr) override;
private:
    void startStockfish();
    std::string getBestMoveFromStockfish(const Board& board);
    int skillLevel_;
    FILE* writePipe_ = nullptr;
    FILE* readPipe_ = nullptr;
    pid_t childPid_ = -1;
};

#endif // SEARCH_HPP
