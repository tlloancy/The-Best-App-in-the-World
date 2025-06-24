#pragma once
#include <cstdint>
#include <functional>
#include <array>
#include <string>
#include <memory>
#include "../utils/Bitboard.hpp"

enum class PieceType { Pawn, Knight, Bishop, Rook, Queen, King };
enum class Color { White, Black };

class Piece {
public:
    virtual ~Piece() = default;
    virtual Bitboard generateMoves(Bitboard board, uint8_t square) const = 0;
    virtual std::string getSymbol() const = 0;
    PieceType getType() const { return type_; }
    Color getColor() const { return color_; }

protected:
    Piece(PieceType type, Color color) : type_(type), color_(color) {}
    PieceType type_;
    Color color_;
};

class PieceFactory {
public:
    using MoveGenerator = std::function<Bitboard(Bitboard, uint8_t)>;

    static std::unique_ptr<Piece> create(PieceType type, Color color) {
        static const std::array<MoveGenerator, 6> moveGenerators = {
            &generatePawnMoves,
            &generateKnightMoves,
            &generateBishopMoves,
            &generateRookMoves,
            &generateQueenMoves,
            &generateKingMoves
        };

        struct ConcretePiece : Piece {
            ConcretePiece(PieceType type, Color color, MoveGenerator gen)
                : Piece(type, color), generator_(gen) {}
            Bitboard generateMoves(Bitboard board, uint8_t square) const override {
                return generator_(board, square);
            }
            std::string getSymbol() const override {
                static const std::array<std::string, 6> symbols = {"P", "N", "B", "R", "Q", "K"};
                return (color_ == Color::White ? "" : "b") + symbols[static_cast<size_t>(type_)];
            }
            MoveGenerator generator_;
        };

        return std::make_unique<ConcretePiece>(type, color, moveGenerators[static_cast<size_t>(type)]);
    }

private:
    static Bitboard generatePawnMoves(Bitboard board, uint8_t square);
    static Bitboard generateKnightMoves(Bitboard board, uint8_t square);
    static Bitboard generateBishopMoves(Bitboard board, uint8_t square);
    static Bitboard generateRookMoves(Bitboard board, uint8_t square);
    static Bitboard generateQueenMoves(Bitboard board, uint8_t square);
    static Bitboard generateKingMoves(Bitboard board, uint8_t square);
};