#pragma once
#include <vector>
#include <memory>
#include <variant>
#include "Piece.hpp"
#include "../utils/Bitboard.hpp"

class Board {
public:
    Board() { initialize(); }
    void initialize();

    template<typename Func>
    void forEachPiece(Func&& func) const {
        for (const auto& piece : pieces_) {
            if (piece) func(*piece);
        }
    }

    using PieceVariant = std::variant<std::monostate, std::unique_ptr<Piece>>;
    void mutatePiece(uint8_t square, PieceType newType, Color color) {
        pieces_[square] = PieceFactory::create(newType, color);
    }

    Bitboard getOccupied() const { return occupied_; }
    const auto& getPieces() const { return pieces_; }
    bool movePiece(uint8_t from, uint8_t to);

private:
    std::array<std::unique_ptr<Piece>, 64> pieces_;
    Bitboard occupied_;
    Bitboard piecesByType[12];
};