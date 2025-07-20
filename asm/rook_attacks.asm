; rook_attacks.asm
; Generate rook attacks with obstacle detection and color handling
section .text
global _generate_rook_attacks

; Arguments: rdi = const Board*, rsi = int square, rdx = Bitboard&, rcx = const int* (directions), r8 = const Piece** (pieces)
_generate_rook_attacks:
    push rbp
    mov rbp, rsp
    sub rsp, 48         ; Aligned space for local variables
    push rbx
    push r12
    push r13
    push r14
    push r15
    mov [rbp-8], rdi    ; Save board pointer
    mov [rbp-16], rsi   ; Save square
    mov [rbp-24], rdx   ; Save attacks reference (Bitboard&)
    mov [rbp-32], rcx   ; Save directions pointer
    mov [rbp-40], r8    ; Save pieces pointer

    ; Initialize attacks Bitboard
    mov rcx, [rbp-24]   ; Load address of attacks Bitboard
    xor rax, rax        ; Clear rax
    mov [rcx], rax      ; Set attacks to 0

    mov ecx, 0          ; Loop counter (direction index)
    mov edx, [rbp-16]   ; Load square into edx
    mov r8d, edx        ; Save original square
    shr edx, 3          ; rank = square / 8
    and r8d, 7          ; file = square % 8
    mov r9, [rbp-40]    ; Load pieces array base
    mov r10, [r9 + rsi*8] ; Load piece at square
    test r10, r10
    jz .done

    ; Get own color via virtual call
    push rdi
    mov rdi, r10        ; piece -> rdi
    mov r11, [rdi]      ; vtable
    call [r11 + 8]      ; getColor() at offset 8
    pop rdi
    mov r11d, eax       ; own color in r11d

.loop:
    cmp ecx, 4          ; Only 4 directions for rook
    jge .done

    ; Load direction offsets
    mov r12, [rbp-32]   ; r12 = directions
    movsxd r14, ecx     ; Sign-extend direction index
    shl r14, 3          ; r14 = rcx * 8 (since each pair is 8 bytes)
    mov eax, [r12 + r14] ; Load dy (rank change)
    movsxd rax, eax     ; Sign-extend dy to 64 bits
    mov ebx, [r12 + r14 + 4] ; Load dx (file change)
    movsxd rbx, ebx     ; Sign-extend dx to 64 bits

    mov r9d, edx        ; newRank = rank
    mov r10d, r8d       ; newFile = file

.step:
    add r9d, eax        ; newRank += dy
    add r10d, ebx       ; newFile += dx

    ; Check board boundaries
    cmp r9d, 0
    jl .next_dir
    cmp r9d, 8
    jge .next_dir
    cmp r10d, 0
    jl .next_dir
    cmp r10d, 8
    jge .next_dir

    ; Calculate new square
    mov r13d, r9d       ; temp = newRank
    shl r13d, 3         ; temp *= 8
    add r13d, r10d      ; newSquare = temp + newFile

    mov r14, [rbp-40]   ; Reload pieces pointer
    mov r15, [r14 + r13*8] ; Load piece at newSquare

    test r15, r15       ; Check if piece exists
    jz .set_bit_continue ; If no piece, set bit and continue

    ; Occupied, check color
    push rdi
    push rax
    push rbx
    mov rdi, r15        ; piece -> rdi
    mov r12, [rdi]      ; vtable (use r12 temp)
    call [r12 + 8]      ; getColor() at offset 8
    pop rbx
    pop rax
    pop rdi
    cmp eax, r11d       ; Compare with own color
    je .next_dir        ; Same color, stop without setting bit

    ; Opposite color, set bit and stop
    mov r12, [rbp-24]   ; Load attacks Bitboard address
    mov rsi, [r12]      ; Load current bitboard
    bts rsi, r13        ; Set bit for newSquare
    mov [r12], rsi      ; Store updated bitboard
    jmp .next_dir       ; Stop ray

.set_bit_continue:
    mov r12, [rbp-24]   ; Load attacks Bitboard address
    mov rsi, [r12]      ; Load current bitboard
    bts rsi, r13        ; Set bit for newSquare
    mov [r12], rsi      ; Store updated bitboard
    jmp .step           ; Continue in direction

.next_dir:
    inc ecx
    jmp .loop

.done:
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    add rsp, 48
    pop rbp
    ret
