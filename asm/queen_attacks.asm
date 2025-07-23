; queen_attacks.asm
; Generate queen attacks with obstacle detection and color handling
section .text
global _generate_queen_attacks

; Arguments: rdi = const Board*, rsi = int square, rdx = Bitboard&, rcx = const int* (directions), r8 = const Piece** (pieces)
_generate_queen_attacks:
    push rbp
    mov rbp, rsp
    sub rsp, 48         ; Aligned space for local variables
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
    mov r11b, [r10 + 8]     ; Color at offset 8

.loop:
    cmp ecx, 8          ; 8 directions for queen
    jge .done

    ; Load directions pointer
    mov r12, [rbp-32]   ; r12 = directions
    mov r14, rcx
    shl r14, 3          ; r14 = rcx * 8 (since each pair is 8 bytes)
    mov eax, [r12 + r14] ; dy = rank change = directions[rcx*2 + 0]
    movsxd rax, eax     ; Sign-extend dy
    mov [rbp-48], eax   ; Save dy
    mov ebx, [r12 + r14 + 4] ; dx = file change = directions[rcx*2 + 1]
    movsxd rbx, ebx     ; Sign-extend dx
    mov [rbp-44], ebx   ; Save dx

    mov r9d, edx        ; newRank = rank
    mov r10d, r8d       ; newFile = file

.step:
    add r9d, [rbp-48]   ; newRank += dy
    add r10d, [rbp-44]  ; newFile += dx

    ; Check board boundaries
    cmp r9d, 0
    jl .next_dir
    cmp r9d, 7
    jg .next_dir
    cmp r10d, 0
    jl .next_dir
    cmp r10d, 7
    jg .next_dir

    mov r13d, r9d       ; temp = newRank
    imul r13d, 8        ; temp *= 8
    add r13d, r10d      ; newSquare = temp + newFile
    mov r14, [rbp-40]   ; Reload pieces pointer
    mov r15, [r14 + r13*8] ; Load piece at newSquare

    ; Set bit for valid move
    mov rdi, [rbp-24]   ; Load attacks Bitboard
    mov rax, [rdi]
    bts rax, r13
    mov [rdi], rax

    ; Check for piece at newSquare
    test r15, r15       ; Check if piece exists
    jnz .check_color    ; If piece exists, check color
    jmp .step           ; No piece, continue in direction

.check_color:
    mov r14b, [r15 + 8]     ; Load color of piece at newSquare (offset 8)
    cmp r14b, r11b      ; Compare with own color
    je .next_dir        ; Same color, stop
    ; Opposite color, bit already set, stop
    jmp .next_dir

.next_dir:
    inc ecx
    jmp .loop

.done:
    leave
    ret
