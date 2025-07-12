#include "../../include/core/Board.hpp"
#include "../../include/core/Pawn.hpp"
#include "../../include/core/Knight.hpp"
#include "../../include/core/Bishop.hpp"
#include "../../include/core/Rook.hpp"
#include "../../include/core/Queen.hpp"
#include "../../include/core/King.hpp"
#include <iostream>
#include <vector>

constexpr int WHITE_KING_START = 4;
constexpr int BLACK_KING_START = 60;
constexpr int WHITE_ROOK_QUEENSIDE = 0;
constexpr int WHITE_ROOK_KINGSIDE = 7;
constexpr int BLACK_ROOK_QUEENSIDE = 56;
constexpr int BLACK_ROOK_KINGSIDE = 63;
constexpr int WHITE_PROMOTION_RANK = 7;
constexpr int BLACK_PROMOTION_RANK = 0;
constexpr int HALFMOVE_DRAW = 100;

Board::Board() {
    pieces_[WHITE_ROOK_QUEENSIDE] = std::make_unique<Rook>(Color::White);
    pieces_[1] = std::make_unique<Knight>(Color::White);
    pieces_[2] = std::make_unique<Bishop>(Color::White);
    pieces_[3] = std::make_unique<Queen>(Color::White);
    pieces_[WHITE_KING_START] = std::make_unique<King>(Color::White);
    pieces_[5] = std::make_unique<Bishop>(Color::White);
    pieces_[6] = std::make_unique<Knight>(Color::White);
    pieces_[WHITE_ROOK_KINGSIDE] = std::make_unique<Rook>(Color::White);
    for (int i = 8; i < 16; ++i) {
        pieces_[i] = std::make_unique<Pawn>(Color::White);
    }
    for (int i = 16; i < 48; ++i) {
        pieces_[i] = nullptr;
    }
    for (int i = 48; i < 56; ++i) {
        pieces_[i] = std::make_unique<Pawn>(Color::Black);
    }
    pieces_[BLACK_ROOK_QUEENSIDE] = std::make_unique<Rook>(Color::Black);
    pieces_[57] = std::make_unique<Knight>(Color::Black);
    pieces_[58] = std::make_unique<Bishop>(Color::Black);
    pieces_[59] = std::make_unique<Queen>(Color::Black);
    pieces_[BLACK_KING_START] = std::make_unique<King>(Color::Black);
    pieces_[61] = std::make_unique<Bishop>(Color::Black);
    pieces_[62] = std::make_unique<Knight>(Color::Black);
    pieces_[BLACK_ROOK_KINGSIDE] = std::make_unique<Rook>(Color::Black);
    enPassantSquare_ = 64;
    whiteKingSquare_ = WHITE_KING_START;
    blackKingSquare_ = BLACK_KING_START;
    whiteCanCastleKingside_ = true;
    whiteCanCastleQueenside_ = true;
    blackCanCastleKingside_ = true;
    blackCanCastleQueenside_ = true;
    halfmoveClock_ = 0;
    fullmoveNumber_ = 1;
}

Board::Board(const Board& other) : isWhiteToMove_(other.isWhiteToMove_), enPassantSquare_(other.enPassantSquare_), whiteKingSquare_(other.whiteKingSquare_), blackKingSquare_(other.blackKingSquare_), whiteCanCastleKingside_(other.whiteCanCastleKingside_), whiteCanCastleQueenside_(other.whiteCanCastleQueenside_), blackCanCastleKingside_(other.blackCanCastleKingside_), blackCanCastleQueenside_(other.blackCanCastleQueenside_), halfmoveClock_(other.halfmoveClock_), fullmoveNumber_(other.fullmoveNumber_) {
    for (int i = 0; i < 64; ++i) {
        if (other.pieces_[i]) {
            pieces_[i] = std::unique_ptr<Piece>(other.pieces_[i]->clone());
        } else {
            pieces_[i] = nullptr;
        }
    }
}

Board& Board::operator=(const Board& other) {
    if (this == &other) return *this;
    isWhiteToMove_ = other.isWhiteToMove_;
    enPassantSquare_ = other.enPassantSquare_;
    whiteKingSquare_ = other.whiteKingSquare_;
    blackKingSquare_ = other.blackKingSquare_;
    whiteCanCastleKingside_ = other.whiteCanCastleKingside_;
    whiteCanCastleQueenside_ = other.whiteCanCastleQueenside_;
    blackCanCastleKingside_ = other.blackCanCastleKingside_;
    blackCanCastleQueenside_ = other.blackCanCastleQueenside_;
    halfmoveClock_ = other.halfmoveClock_;
    fullmoveNumber_ = other.fullmoveNumber_;
    for (int i = 0; i < 64; ++i) {
        if (other.pieces_[i]) {
            pieces_[i] = std::unique_ptr<Piece>(other.pieces_[i]->clone());
        } else {
            pieces_[i] = nullptr;
        }
    }
    return *this;
}

Board::~Board() {}

void Board::performMove(uint8_t from, uint8_t to) {
    Color movingColor = pieces_[from]->getColor();
    bool isPawn = pieces_[from]->getType() == PieceType::Pawn;
    bool isDoublePawnMove = isPawn && std::abs(static_cast<int>(from) - static_cast<int>(to)) == 16;
    bool isEnPassantCapture = isPawn && to == enPassantSquare_;
    bool isPromotion = isPawn && ((movingColor == Color::White && to / 8 == WHITE_PROMOTION_RANK) || (movingColor == Color::Black && to / 8 == BLACK_PROMOTION_RANK));
    bool isCastling = pieces_[from]->getType() == PieceType::King && std::abs(static_cast<int>(from) - static_cast<int>(to)) == 2;
    pieces_[to] = std::move(pieces_[from]);
    pieces_[from] = nullptr;
    if (isEnPassantCapture) {
        int captureDir = (movingColor == Color::White) ? -8 : 8;
        pieces_[to + captureDir] = nullptr;
    }
    enPassantSquare_ = isDoublePawnMove ? (from + to) / 2 : 64;
    if (pieces_[to]->getType() == PieceType::King) {
        if (movingColor == Color::White) whiteKingSquare_ = to;
        else blackKingSquare_ = to;
    }
    if (isCastling) {
        bool kingside = (to > from);
        uint8_t rookFrom = movingColor == Color::White ? (kingside ? WHITE_ROOK_KINGSIDE : WHITE_ROOK_QUEENSIDE) : (kingside ? BLACK_ROOK_KINGSIDE : BLACK_ROOK_QUEENSIDE);
        uint8_t rookTo = movingColor == Color::White ? (kingside ? 5 : 3) : (kingside ? 61 : 59);
        pieces_[rookTo] = std::move(pieces_[rookFrom]);
        pieces_[rookFrom] = nullptr;
    }
    if (isPromotion) {
        pieces_[to] = std::make_unique<Queen>(movingColor);
    }
    if (from == WHITE_KING_START || to == WHITE_KING_START) {
        whiteCanCastleKingside_ = false;
        whiteCanCastleQueenside_ = false;
    }
    if (from == BLACK_KING_START || to == BLACK_KING_START) {
        blackCanCastleKingside_ = false;
        blackCanCastleQueenside_ = false;
    }
    if (from == WHITE_ROOK_QUEENSIDE || to == WHITE_ROOK_QUEENSIDE) whiteCanCastleQueenside_ = false;
    if (from == WHITE_ROOK_KINGSIDE || to == WHITE_ROOK_KINGSIDE) whiteCanCastleKingside_ = false;
    if (from == BLACK_ROOK_QUEENSIDE || to == BLACK_ROOK_QUEENSIDE) blackCanCastleQueenside_ = false;
    if (from == BLACK_ROOK_KINGSIDE || to == BLACK_ROOK_KINGSIDE) blackCanCastleKingside_ = false;
}

bool Board::movePiece(uint8_t from, uint8_t to) {
    if (!pieces_[from] || pieces_[from]->getColor() != (isWhiteToMove_ ? Color::White : Color::Black)) return false;
    Bitboard validMoves = pieces_[from]->generateMoves(*this, from);
    if (!validMoves.testBit(to)) return false;
    Board temp(*this);
    temp.performMove(from, to);
    Color movingColor = temp.pieces_[to]->getColor();
    if (temp.isKingInCheck(movingColor)) return false;
    bool isPawn = pieces_[from]->getType() == PieceType::Pawn;
    bool isCapture = temp.pieces_[to] != nullptr || (isPawn && to == enPassantSquare_);
    performMove(from, to);
    halfmoveClock_ = (isPawn || isCapture) ? 0 : halfmoveClock_ + 1;
    if (!isWhiteToMove_) fullmoveNumber_++;
    isWhiteToMove_ = !isWhiteToMove_;
    return true;
}

Bitboard Board::getOccupied() const {
    Bitboard occupied;
    for (int i = 0; i < 64; ++i) {
        if (pieces_[i]) occupied.setBit(i);
    }
    return occupied;
}

Bitboard Board::getOccupiedByColor(Color color) const {
    Bitboard occupied;
    for (int i = 0; i < 64; ++i) {
        if (pieces_[i] && pieces_[i]->getColor() == color) occupied.setBit(i);
    }
    return occupied;
}

int Board::findKing(Color color) const {
    return color == Color::White ? whiteKingSquare_ : blackKingSquare_;
}

bool Board::isKingInCheck(Color color) const {
    Color opponent = (color == Color::White) ? Color::Black : Color::White;
    int kingSquare = findKing(color);
    for (int i = 0; i < 64; ++i) {
        if (pieces_[i] && pieces_[i]->getColor() == opponent) {
            Bitboard attacks = pieces_[i]->generateAttacks(*this, i);
            if (attacks.testBit(kingSquare)) return true;
        }
    }
    return false;
}

bool Board::isCheck() const {
    return isKingInCheck(isWhiteToMove_ ? Color::White : Color::Black);
}

std::vector<std::pair<uint8_t, uint8_t>> Board::getLegalMoves(Color color) const {
    std::vector<std::pair<uint8_t, uint8_t>> legalMoves;
    for (int from = 0; from < 64; ++from) {
        if (pieces_[from] && pieces_[from]->getColor() == color) {
            Bitboard moves = pieces_[from]->generateMoves(*this, from);
            for (int to = 0; to < 64; ++to) {
                if (moves.testBit(to)) {
                    Board temp(*this);
                    temp.performMove(from, to);
                    if (!temp.isKingInCheck(color)) {
                        legalMoves.emplace_back(from, to);
                    }
                }
            }
        }
    }
    return legalMoves;
}

bool Board::isCheckmate() const {
    if (!isCheck()) return false;
    Color currentColor = isWhiteToMove_ ? Color::White : Color::Black;
    return getLegalMoves(currentColor).empty();
}

bool Board::isStalemate() const {
    if (isCheck()) return false;
    Color currentColor = isWhiteToMove_ ? Color::White : Color::Black;
    return getLegalMoves(currentColor).empty();
}

bool Board::isDraw() const {
    return halfmoveClock_ >= HALFMOVE_DRAW || isStalemate();
}

float Board::evaluate() const {
    float score = 0.0f;
    for (const auto& piece : pieces_) {
        if (piece) {
            float value = 0.0f;
            switch (piece->getType()) {
                case PieceType::Pawn: value = 1.0f; break;
                case PieceType::Knight: value = 3.0f; break;
                case PieceType::Bishop: value = 3.0f; break;
                case PieceType::Rook: value = 5.0f; break;
                case PieceType::Queen: value = 9.0f; break;
                case PieceType::King: value = 0.0f; break;
            }
            score += (piece->getColor() == Color::White) ? value : -value;
        }
    }
    return score;
}
