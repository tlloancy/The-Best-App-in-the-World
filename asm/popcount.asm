; popcount.asm
; Function to count the number of set bits in a 64-bit integer using popcnt
section .text
global _bitboard_popcount

_bitboard_popcount:
    ; Input: rdi contains the 64-bit value
    ; Output: eax contains the popcount result
    mov rax, rdi
    popcnt rax, rax
    ret
