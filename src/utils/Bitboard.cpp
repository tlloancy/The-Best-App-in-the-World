#include "../../include/utils/Bitboard.hpp"

#ifdef __x86_64__
#include <x86intrin.h>
int Bitboard::popcount() const {
    return _popcnt64(value_);
}
#else
int Bitboard::popcount() const {
    uint64_t v = value_;
    v = v - ((v >> 1) & 0x5555555555555555ULL);
    v = (v & 0x3333333333333333ULL) + ((v >> 2) & 0x3333333333333333ULL);
    v = (v + (v >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
    return (v * 0x0101010101010101ULL) >> 56;
}
#endif
