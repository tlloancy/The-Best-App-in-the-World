#ifndef BITBOARD_HPP
#define BITBOARD_HPP

#include <cstdint>

class Bitboard {
public:
    Bitboard() : value_(0) {}
    explicit Bitboard(uint64_t v) : value_(v) {}
    Bitboard& operator=(uint64_t v) { value_ = v; return *this; }
    Bitboard& operator|=(const Bitboard& other) { value_ |= other.value_; return *this; }
    Bitboard& operator&=(const Bitboard& other) { value_ &= other.value_; return *this; }
    Bitboard operator&(const Bitboard& other) const { return Bitboard(value_ & other.value_); }
    Bitboard operator|(const Bitboard& other) const { return Bitboard(value_ | other.value_); }
    Bitboard operator~() const { return Bitboard(~value_); }
    bool operator!=(const Bitboard& other) const { return value_ != other.value_; }
    explicit operator bool() const { return value_ != 0; }
    uint64_t getValue() const { return value_; }
    void setBit(int bit) { value_ |= (1ULL << bit); }
    bool testBit(int bit) const { return (value_ & (1ULL << bit)) != 0; }
    int popcount() const;
private:
    uint64_t value_;
};

#endif
