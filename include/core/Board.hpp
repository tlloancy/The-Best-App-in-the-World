#ifndef BOARD_HPP
#define BOARD_HPP

#include <array>
#include "Piece.hpp"
#include "../utils/Bitboard.hpp"

class Board {
public:
    Board();
    Board(const Board& other);
    ~Board();
    Bitboard getOccupied() const;
    const std::array<Piece*, 64>& getPieces() const;
    bool movePiece(uint8_t from, uint8_t to);
    bool isCheck() const;
    bool isCheckmate() const;
    bool isStalemate() const;
    bool isDraw() const;
    bool isWhiteToMove() const { return whiteToMove_; }

private:
    std::array<Piece*, 64> pieces_;
    Bitboard occupied_;
    bool whiteToMove_;
    int enPassantTarget_;
    int halfMoveClock_;
    int fullMoveNumber_;
};

#endif
