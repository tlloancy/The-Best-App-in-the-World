; popcount.asm
; Enhanced function to count set bits in a 64-bit integer
section .data
    mask_pairs    dq 0x5555555555555555
    mask_quads    dq 0x3333333333333333
    mask_octets   dq 0x0F0F0F0F0F0F0F0F
    magic_number  dq 0x0101010101010101

section .text
global _bitboard_popcount
global _bitboard_popcount_fallback

; Main function using popcnt (x86-64 with POPCNT support)
_bitboard_popcount:
    mov rax, rdi         ; Load input into rax
    popcnt rax, rax      ; Count set bits
    ret

; Fallback function using SWAR algorithm (no POPCNT support)
_bitboard_popcount_fallback:
    push rbx
    mov rax, rdi         ; Load input into rax
    mov rbx, rax         ; Copy for parallel processing
    shr rbx, 1           ; Shift right by 1
    mov rcx, [rel mask_pairs] ; RIP-relative access to mask_pairs
    and rbx, [rcx]       ; Mask for pairs
    sub rax, rbx         ; Subtract shifted bits (count pairs)
    mov rbx, rax         ; Copy result
    mov rcx, [rel mask_quads] ; RIP-relative access to mask_quads
    and rax, [rcx]       ; Mask for quads
    shr rbx, 2           ; Shift right by 2
    and rbx, [rcx]       ; Mask for quads
    add rax, rbx         ; Add quads
    mov rbx, rax         ; Copy result
    mov rcx, [rel mask_octets] ; RIP-relative access to mask_octets
    and rax, [rcx]       ; Mask for octets
    shr rbx, 4           ; Shift right by 4
    and rbx, [rcx]       ; Mask for octets
    add rax, rbx         ; Add octets
    mov rcx, [rel magic_number] ; RIP-relative access to magic_number
    imul rax, [rcx]      ; Multiply by magic number
    shr rax, 56          ; Shift right to get final count
    pop rbx
    ret
