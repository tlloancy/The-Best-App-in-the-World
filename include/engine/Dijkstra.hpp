#pragma once
#include "../core/Board.hpp"

class Dijkstra {
public:
    void findPath(const Board& board, uint8_t start, uint8_t end);
};