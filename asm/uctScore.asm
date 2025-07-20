; uctScore.asm
; Optimized assembly implementation of UCT score calculation
section .text
    global _mcts_uct_score

; Arguments:
; rdi: pointer to MCTSNode (contains visits and totalScore)
; rsi: parentVisits (int)
; Returns: float result in xmm0
_mcts_uct_score:
    push rbp
    mov rbp, rsp
    sub rsp, 16          ; Space for local float

    ; Load visits and totalScore from MCTSNode
    mov eax, [rdi + 12]  ; visits (assuming offset 12 for visits in MCTSNode)
    test eax, eax
    jz .zero_visits      ; If visits == 0, return max float

    ; Calculate totalScore / visits (float division)
    cvtsi2ss xmm0, [rdi + 8]  ; totalScore (assuming offset 8 for totalScore)
    cvtsi2ss xmm1, eax        ; visits
    divss xmm0, xmm1          ; xmm0 = totalScore / visits

    ; Calculate sqrt(log(parentVisits) / visits)
    cvtsi2ss xmm2, esi        ; parentVisits
    movss xmm3, [rel .log_const]  ; Load log constant (approx ln(2))
    mulss xmm2, xmm3          ; Approximate log(parentVisits) * ln(2)
    divss xmm2, xmm1          ; xmm2 = log(parentVisits) / visits
    sqrtss xmm2, xmm2         ; xmm2 = sqrt(log(parentVisits) / visits)

    ; Multiply by 1.414f (sqrt(2))
    movss xmm3, [rel .sqrt2_const]  ; Load 1.414f
    mulss xmm2, xmm3

    ; Add to initial score
    addss xmm0, xmm2

    ; Return result in xmm0
    jmp .done

.zero_visits:
    mov eax, 0x7F800000      ; Load max float value (infinity)
    movd xmm0, eax

.done:
    leave
    ret

section .data
    align 4
    .log_const dd 0.69314718056  ; ln(2) approximation
    .sqrt2_const dd 1.41421356237 ; sqrt(2) approximation
