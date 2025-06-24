#pragma once
#include <cstdint>

class Bitboard {
public:
    Bitboard(uint64_t value = 0) : value_(value) {}
    operator uint64_t() const { return value_; }
    Bitboard operator|(const Bitboard& other) const { return Bitboard(value_ | other.value_); }
    Bitboard operator&(const Bitboard& other) const { return Bitboard(value_ & other.value_); }
    Bitboard operator^(const Bitboard& other) const { return Bitboard(value_ ^ other.value_); }
    Bitboard operator~() const { return Bitboard(~value_); }
    Bitboard operator<<(int shift) const { return Bitboard(value_ << shift); }
    Bitboard operator>>(int shift) const { return Bitboard(value_ >> shift); }
    Bitboard& operator|=(const Bitboard& other) { value_ |= other.value_; return *this; }
    Bitboard& operator&=(const Bitboard& other) { value_ &= other.value_; return *this; }

private:
    uint64_t value_;
};