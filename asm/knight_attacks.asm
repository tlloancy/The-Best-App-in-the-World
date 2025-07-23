; knight_attacks.asm
; Generate knight attacks with color handling (non-sliding)
section .text
 global _generate_knight_attacks
_generate_knight_attacks:
    push rbp
    mov rbp, rsp
    sub rsp, 48
    mov [rbp-8], rdi
    mov [rbp-16], rsi
    mov [rbp-24], rdx
    mov [rbp-32], rcx
    mov [rbp-40], r8
    mov rcx, [rbp-24]
    xor rax, rax
    mov [rcx], rax
    mov ecx, 0
    mov edx, [rbp-16]
    mov r8d, edx
    shr edx, 3
    and r8d, 7
    mov r9, [rbp-40]
    mov r10, [r9 + rsi*8]
    test r10, r10
    jz .done
    mov r11b, [r10 + 8]
.loop:
    cmp ecx, 8
    jge .done
    mov r12, [rbp-32]
    mov r14, rcx
    shl r14, 3
    mov eax, [r12 + r14]
    movsxd rax, eax
    mov [rbp-48], eax
    mov ebx, [r12 + r14 +4]
    movsxd rbx, ebx
    mov [rbp-44], ebx
    mov r9d, edx
    mov r10d, r8d
.step:
    add r9d, [rbp-48]
    add r10d, [rbp-44]
    cmp r9d, 0
    jl .next_dir
    cmp r9d, 7
    jg .next_dir
    cmp r10d, 0
    jl .next_dir
    cmp r10d, 7
    jg .next_dir
    mov r13d, r9d
    imul r13d, 8
    add r13d, r10d
    mov r14, [rbp-40]
    mov r15, [r14 + r13*8]
    test r15, r15
    jz .set_and_continue
.check_color:
    mov r14b, [r15 + 8]
    cmp r14b, r11b
    je .next_dir
    mov rdi, [rbp-24]
    mov rax, [rdi]
    bts rax, r13
    mov [rdi], rax
    jmp .next_dir
.set_and_continue:
    mov rdi, [rbp-24]
    mov rax, [rdi]
    bts rax, r13
    mov [rdi], rax
    jmp .next_dir
.next_dir:
    inc ecx
    jmp .loop
.done:
    leave
    ret