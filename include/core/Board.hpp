#ifndef BOARD_HPP
#define BOARD_HPP

#include "Piece.hpp"
#include "../utils/Bitboard.hpp"
#include <memory>
#include <array>
#include <string>
#include <vector>

class Board {
public:
    Board();
    Board(const Board& other);
    Board& operator=(const Board& other);
    ~Board();
    bool movePiece(uint8_t from, uint8_t to);
    bool isCheck() const;
    bool isKingInCheck(Color color) const;
    bool isCheckmate() const;
    bool isStalemate() const;
    bool isDraw() const;
    bool isWhiteToMove() const { return isWhiteToMove_; }
    Bitboard getOccupied() const;
    Bitboard getOccupiedByColor(Color color) const;
    const std::array<std::unique_ptr<Piece>, 64>& getPieces() const { return pieces_; }
    uint8_t getEnPassantSquare() const { return enPassantSquare_; }
    float evaluate() const;
    std::vector<std::pair<uint8_t, uint8_t>> getLegalMoves(Color color) const;
    int findKing(Color color) const;
    bool canCastleWhiteKingside() const { return whiteCanCastleKingside_; }
    bool canCastleWhiteQueenside() const { return whiteCanCastleQueenside_; }
    bool canCastleBlackKingside() const { return blackCanCastleKingside_; }
    bool canCastleBlackQueenside() const { return blackCanCastleQueenside_; }
    void performMove(uint8_t from, uint8_t to);

private:
    std::array<std::unique_ptr<Piece>, 64> pieces_;
    bool isWhiteToMove_ = true;
    uint8_t enPassantSquare_ = 64;
    int whiteKingSquare_;
    int blackKingSquare_;
    bool whiteCanCastleKingside_;
    bool whiteCanCastleQueenside_;
    bool blackCanCastleKingside_;
    bool blackCanCastleQueenside_;
    int halfmoveClock_;
    int fullmoveNumber_;
};

#endif
