#include "../../include/utils/Bitboard.hpp"

extern "C" int _bitboard_popcount(uint64_t);

int Bitboard::popcount() const {
    return _bitboard_popcount(value_);
}
