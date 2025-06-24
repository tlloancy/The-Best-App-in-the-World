#include "../../include/core/Board.hpp"

void Board::initialize() {
    // Pions
    for (int i = 0; i < 8; ++i) {
        pieces_[8 + i] = PieceFactory::create(PieceType::Pawn, Color::White);
        pieces_[48 + i] = PieceFactory::create(PieceType::Pawn, Color::Black);
    }
    // Tours
    pieces_[0] = PieceFactory::create(PieceType::Rook, Color::White);
    pieces_[7] = PieceFactory::create(PieceType::Rook, Color::White);
    pieces_[56] = PieceFactory::create(PieceType::Rook, Color::Black);
    pieces_[63] = PieceFactory::create(PieceType::Rook, Color::Black);
    // Cavaliers
    pieces_[1] = PieceFactory::create(PieceType::Knight, Color::White);
    pieces_[6] = PieceFactory::create(PieceType::Knight, Color::White);
    pieces_[57] = PieceFactory::create(PieceType::Knight, Color::Black);
    pieces_[62] = PieceFactory::create(PieceType::Knight, Color::Black);
    // Fous
    pieces_[2] = PieceFactory::create(PieceType::Bishop, Color::White);
    pieces_[5] = PieceFactory::create(PieceType::Bishop, Color::White);
    pieces_[58] = PieceFactory::create(PieceType::Bishop, Color::Black);
    pieces_[61] = PieceFactory::create(PieceType::Bishop, Color::Black);
    // Reines
    pieces_[3] = PieceFactory::create(PieceType::Queen, Color::White);
    pieces_[59] = PieceFactory::create(PieceType::Queen, Color::Black);
    // Rois
    pieces_[4] = PieceFactory::create(PieceType::King, Color::White);
    pieces_[60] = PieceFactory::create(PieceType::King, Color::Black);

    // Mettre à jour occupied_ et piecesByType
    occupied_ = 0;
    for (int i = 0; i < 64; ++i) {
        if (pieces_[i]) {
            occupied_ |= Bitboard(1ULL << i);
            piecesByType[static_cast<size_t>(pieces_[i]->getType()) * 2 + (pieces_[i]->getColor() == Color::White ? 0 : 1)] |= Bitboard(1ULL << i);
        }
    }
}

bool Board::movePiece(uint8_t from, uint8_t to) {
    if (from >= 64 || to >= 64 || !pieces_[from]) return false;
    pieces_[to] = std::move(pieces_[from]);
    pieces_[from] = nullptr;
    occupied_ &= ~(1ULL << from);
    occupied_ |= (1ULL << to);
    // Mettre à jour piecesByType (à implémenter plus tard)
    return true;
}