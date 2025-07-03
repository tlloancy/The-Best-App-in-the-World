#include "../include/core/Board.hpp"
#include <cassert>
#include <algorithm>
#include <iostream>
#include <ostream>
#include <bitset>
#include "../include/gui/Renderer.hpp"
#include <SDL3/SDL.h>
#include "../include/core/Pawn.hpp"
#include "../include/core/Knight.hpp"
#include "../include/core/Bishop.hpp"
#include "../include/core/Rook.hpp"
#include "../include/core/Queen.hpp"
#include "../include/core/King.hpp"

Board::Board() {
    for (int i = 0; i < 64; ++i) pieces_[i] = nullptr;

    for (int i = 8; i < 16; ++i) pieces_[i] = new Pawn(Color::White);
    for (int i = 48; i < 56; ++i) pieces_[i] = new Pawn(Color::Black);

    pieces_[0] = new Rook(Color::White);
    pieces_[1] = new Knight(Color::White);
    pieces_[2] = new Bishop(Color::White);
    pieces_[3] = new Queen(Color::White);
    pieces_[4] = new King(Color::White);
    pieces_[5] = new Bishop(Color::White);
    pieces_[6] = new Knight(Color::White);
    pieces_[7] = new Rook(Color::White);

    pieces_[56] = new Rook(Color::Black);
    pieces_[57] = new Knight(Color::Black);
    pieces_[58] = new Bishop(Color::Black);
    pieces_[59] = new Queen(Color::Black);
    pieces_[60] = new King(Color::Black);
    pieces_[61] = new Bishop(Color::Black);
    pieces_[62] = new Knight(Color::Black);
    pieces_[63] = new Rook(Color::Black);

    occupied_ = Bitboard(0);
    for (int i = 0; i < 64; ++i) {
        if (pieces_[i]) occupied_ |= Bitboard(1ULL << i);
    }
    if (globalRenderer && globalRenderer->getDebugEnabled()) {
        std::cout << "[DEBUG] Initial occupied_: " << std::hex << occupied_.getValue() << std::dec << std::endl;
        for (int i = 0; i < 64; ++i) {
            if (occupied_ & Bitboard(1ULL << i)) {
                int displayRow = 7 - (i / 8);
                int col = i % 8;
                std::cout << "[DEBUG] Initial occupied bit set at square " << i << " (" << displayRow << ", " << col << ")" << std::endl;
            }
        }
    }
    whiteToMove_ = true;
    enPassantTarget_ = -1;
    halfMoveClock_ = 0;
    fullMoveNumber_ = 1;
}

Board::Board(const Board& other) {
    for (int i = 0; i < 64; ++i) {
        if (other.pieces_[i]) {
            pieces_[i] = other.pieces_[i]->clone();
        } else {
            pieces_[i] = nullptr;
        }
    }
    occupied_ = other.occupied_;
    whiteToMove_ = other.whiteToMove_;
    enPassantTarget_ = other.enPassantTarget_;
    halfMoveClock_ = other.halfMoveClock_;
    fullMoveNumber_ = other.fullMoveNumber_;
}

Board::~Board() {
    for (int i = 0; i < 64; ++i) delete pieces_[i];
}

Bitboard Board::getOccupied() const {
    return occupied_;
}

const std::array<Piece*, 64>& Board::getPieces() const {
    return pieces_;
}

bool Board::movePiece(uint8_t from, uint8_t to) {
    if (from >= 64 || to >= 64) return false;
    if (!pieces_[from]) {
        if (globalRenderer && globalRenderer->getDebugEnabled()) {
            std::cout << "[DEBUG] movePiece: No piece at from " << (int)from << std::endl;
        }
        return false;
    }
    if (pieces_[from]->getColor() != (whiteToMove_ ? Color::White : Color::Black)) {
        if (globalRenderer && globalRenderer->getDebugEnabled()) {
            std::cout << "[DEBUG] movePiece: Wrong color at " << (int)from << ", expected " << (whiteToMove_ ? "White" : "Black") << std::endl;
        }
        return false;
    }

    Bitboard validMoves = pieces_[from]->generateMoves(occupied_, from);
    if (!(validMoves & Bitboard(1ULL << to))) {
        if (globalRenderer && globalRenderer->getDebugEnabled()) {
            std::cout << "[DEBUG] movePiece: Invalid move from " << (int)from << " to " << (int)to << ", valid moves: " << std::hex << validMoves.getValue() << std::dec << std::endl;
        }
        return false;
    }

    if (globalRenderer && globalRenderer->getDebugEnabled()) {
        std::cout << "[DEBUG] movePiece: occupied_ before move: " << std::hex << occupied_.getValue() << std::dec << std::endl;
        for (int i = 0; i < 64; ++i) {
            if (occupied_ & Bitboard(1ULL << i)) {
                int displayRow = 7 - (i / 8);
                int col = i % 8;
                std::cout << "[DEBUG] Before move: occupied bit set at square " << i << " (" << displayRow << ", " << col << ")" << std::endl;
            }
        }
        for (int i = 0; i < 64; ++i) {
            if (pieces_[i]) {
                std::cout << "[DEBUG] Before move: Piece at " << i << " (type: " << static_cast<int>(pieces_[i]->getType()) << ", color: " << (pieces_[i]->getColor() == Color::White ? "White" : "Black") << ")" << std::endl;
            }
        }
    }

    Board temp(*this);
    temp.pieces_[to] = temp.pieces_[from];
    temp.pieces_[from] = nullptr;
    temp.occupied_ &= ~(Bitboard(1ULL << from));
    temp.occupied_ |= (Bitboard(1ULL << to));
    if (temp.isCheck()) {
        if (globalRenderer && globalRenderer->getDebugEnabled()) {
            std::cout << "[DEBUG] movePiece: Move from " << (int)from << " to " << (int)to << " puts king in check" << std::endl;
        }
        return false;
    }

    if (pieces_[to]) {
        delete pieces_[to];
        pieces_[to] = nullptr;
        occupied_ &= ~(Bitboard(1ULL << to));
        halfMoveClock_ = 0;
    }

    if (pieces_[from]->getType() == PieceType::Pawn && to == enPassantTarget_) {
        int capturedPawn = whiteToMove_ ? to - 8 : to + 8;
        delete pieces_[capturedPawn];
        pieces_[capturedPawn] = nullptr;
        occupied_ &= ~(Bitboard(1ULL << capturedPawn));
        halfMoveClock_ = 0;
    }

    pieces_[to] = pieces_[from];
    pieces_[from] = nullptr;
    occupied_ &= ~(Bitboard(1ULL << from));
    occupied_ |= (Bitboard(1ULL << to));

    if (globalRenderer && globalRenderer->getDebugEnabled()) {
        std::cout << "[DEBUG] movePiece: occupied_ after move: " << std::hex << occupied_.getValue() << std::dec << std::endl;
        for (int i = 0; i < 64; ++i) {
            if (occupied_ & Bitboard(1ULL << i)) {
                int displayRow = 7 - (i / 8);
                int col = i % 8;
                std::cout << "[DEBUG] After move: occupied bit set at square " << i << " (" << displayRow << ", " << col << ")" << std::endl;
            }
        }
        for (int i = 0; i < 64; ++i) {
            if (pieces_[i]) {
                std::cout << "[DEBUG] After move: Piece at " << i << " (type: " << static_cast<int>(pieces_[i]->getType()) << ", color: " << (pieces_[i]->getColor() == Color::White ? "White" : "Black") << ")" << std::endl;
            }
        }
    }

    enPassantTarget_ = -1;
    if (pieces_[to]->getType() == PieceType::Pawn) {
        int pawnMove = to - from;
        if ((whiteToMove_ && pawnMove == -16) || (!whiteToMove_ && pawnMove == 16)) {
            enPassantTarget_ = from + (pawnMove / 2);
        }
    }

    whiteToMove_ = !whiteToMove_;
    if (!whiteToMove_) fullMoveNumber_++;

    if (pieces_[to]->getType() == PieceType::Pawn || pieces_[to] != pieces_[from]) halfMoveClock_ = 0;
    else halfMoveClock_++;

    if (globalRenderer && globalRenderer->getDebugEnabled()) {
        std::cout << "[DEBUG] movePiece: Move from " << (int)from << " to " << (int)to << " successful" << std::endl;
    }
    return true;
}

bool Board::isCheck() const {
    int kingSquare = -1;
    Color kingColor = whiteToMove_ ? Color::White : Color::Black;
    if (globalRenderer && globalRenderer->getDebugEnabled()) {
        std::cout << "[DEBUG] isCheck: Searching for king of color " << (kingColor == Color::White ? "White" : "Black") << " (whiteToMove_: " << whiteToMove_ << ")" << std::endl;
    }
    for (int i = 0; i < 64; ++i) {
        if (globalRenderer && globalRenderer->getDebugEnabled()) {
            if (pieces_[i]) {
                std::cout << "[DEBUG] isCheck: Checking square " << i << " (type: " << static_cast<int>(pieces_[i]->getType()) << ", color: " << (pieces_[i]->getColor() == Color::White ? "White" : "Black") << ")" << std::endl;
            } else {
                std::cout << "[DEBUG] isCheck: Checking square " << i << " (null)" << std::endl;
            }
        }
        if (pieces_[i] && pieces_[i]->getType() == PieceType::King && pieces_[i]->getColor() == kingColor) {
            kingSquare = i;
            break;
        }
    }
    if (kingSquare == -1) {
        if (globalRenderer && globalRenderer->getDebugEnabled()) {
            std::cout << "[DEBUG] isCheck: No king found for color " << (kingColor == Color::White ? "White" : "Black") << std::endl;
        }
        return false;
    }

    Bitboard opponentMoves = Bitboard(0);
    Color opponentColor = (kingColor == Color::White) ? Color::Black : Color::White;
    for (int i = 0; i < 64; ++i) {
        Piece* piece = pieces_[i];
        if (piece && piece->getColor() == opponentColor) {
            Bitboard moves = piece->generateMoves(occupied_, i);
            opponentMoves |= moves;
        }
    }
    bool isCheckResult = (opponentMoves & Bitboard(1ULL << kingSquare)).getValue() != 0; // Conversion explicite via getValue()
    if (globalRenderer && globalRenderer->getDebugEnabled() && isCheckResult) {
        std::cout << "[DEBUG] isCheck for king at " << kingSquare << ": true" << std::endl;
    }
    return isCheckResult;
}

bool Board::isCheckmate() const {
    if (!isCheck()) return false;
    for (int i = 0; i < 64; ++i) {
        if (pieces_[i] && pieces_[i]->getColor() == (whiteToMove_ ? Color::White : Color::Black)) {
            Bitboard moves = pieces_[i]->generateMoves(occupied_, i);
            while (moves.getValue()) {
                int to = __builtin_ctzll(moves.getValue());
                moves &= ~(Bitboard(1ULL << to));
                Board temp(*this);
                if (temp.movePiece(static_cast<uint8_t>(i), static_cast<uint8_t>(to)) && !temp.isCheck()) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool Board::isStalemate() const {
    if (isCheck()) return false;
    for (int i = 0; i < 64; ++i) {
        if (pieces_[i] && pieces_[i]->getColor() == (whiteToMove_ ? Color::White : Color::Black)) {
            Bitboard moves = pieces_[i]->generateMoves(occupied_, i);
            if (moves.getValue()) return false;
        }
    }
    return true;
}

bool Board::isDraw() const {
    if (halfMoveClock_ >= 100) return true;
    return false;
}
