
; Temporaries used here:
;   $5  : literal 1 when needed
;   $6  : n-1
;   $7  : product (n-1)*f(n-1,k) after mflo
;   $8  : saved original n
;   $9  : saved original k
;   $10 : holds address of stirling for jalr
;   $11 : f(n-1,k)
;   $12 : frame size constant (and later f(n-1,k-1))
;
; Stack frame (allocated 48 bytes):
;   [SP+0]   : (unused inside frame; we saved regs at negative offsets first)
;   ...
;   We first store caller regs at negative offsets, then move SP down by 48,
;   and on return move SP back up and load from the same negative offsets.



stirling:
    ; Save caller regs to negative offsets (before touching any temp).
    ; This guarantees we don't clobber any caller value while loading constants.
    sw    $1,  -4($30)
    sw    $2,  -8($30)
    sw    $4, -12($30)
    sw    $5, -16($30)
    sw    $6, -20($30)
    sw    $7, -24($30)
    sw    $8, -28($30)
    sw    $9, -32($30)
    sw    $10,-36($30)
    sw    $11,-40($30)
    sw    $12,-44($30)
    sw    $31,-48($30)

    ; Allocate 48-byte frame (we saved 12 words x 4 bytes).
    lis   $12
    .word 48
    sub   $30, $30, $12

    ; Base cases:
    ; if n==0 and k==0 -> 1
    ; else if n==0 -> 0
    ; else if k==0 -> 0
    ; otherwise recurse.
    beq   $1, $0, basen0
    beq   $2, $0, basek0
    beq   $0, $0, recursestep   ; both nonzero -> recursive case

basen0:
    ; n==0
    beq   $2, $0, bothzero      ; f(0,0)=1
    lis   $3
    .word 0                     ; f(0,k)=0 for k>0
    beq   $0, $0, end

basek0:
    ; k==0
    beq   $1, $0, bothzero      ; f(0,0)=1 (handled above for completeness)
    lis   $3
    .word 0                     ; f(n,0)=0 for n>0
    beq   $0, $0, end

bothzero:
    ; (n,k)==(0,0) -> 1
    lis   $3
    .word 1
    beq   $0, $0, end

recursestep:
    ; Compute: (n-1)*f(n-1,k) + f(n-1,k-1)
    ; Save originals (n,k) since we'll overwrite $1,$2 for recursive calls.
    add   $8, $1, $0            ; $8 <- n
    add   $9, $2, $0            ; $9 <- k

    ; $6 = n-1
    lis   $5
    .word 1
    sub   $6, $1, $5

    ; First recursive call: f(n-1, k)
    add   $1, $6, $0            ; $1 <- n-1
    add   $2, $9, $0            ; $2 <- k
    lis   $10
    .word stirling
    jalr  $10
    add   $11, $3, $0           ; $11 <- f(n-1,k)

    ; Second recursive call: f(n-1, k-1)
    add   $1, $6, $0            ; $1 <- n-1
    lis   $5
    .word 1
    sub   $2, $9, $5            ; $2 <- k-1
    lis   $10
    .word stirling
    jalr  $10
    add   $12, $3, $0           ; $12 <- f(n-1,k-1)

    ; Combine: (n-1)*f(n-1,k) + f(n-1,k-1)
    ; mult writes product to hi/lo; we read the low 32 bits (overflow ignored by spec).
    mult  $6, $11               ; (n-1)*f(n-1,k)
    mflo  $7                    ; $7 <- product low 32 bits
    add   $3, $7, $12           ; return value in $3

    ; Restore argument regs to originals (not strictly required since we restore later,
    ; but keeps locals readable and harmlessly reinstates n,k here).
    add   $1, $8, $0            ; $1 <- original n
    add   $2, $9, $0            ; $2 <- original k

end:
    ; Deallocate frame and restore saved registers.
    lis   $12
    .word 48
    add   $30, $30, $12

    ; Restore caller regs (exactly the same negative offsets we used to save).
    lw    $1,  -4($30)
    lw    $2,  -8($30)
    lw    $4, -12($30)
    lw    $5, -16($30)
    lw    $6, -20($30)
    lw    $7, -24($30)
    lw    $8, -28($30)
    lw    $9, -32($30)
    lw    $10,-36($30)
    lw    $11,-40($30)
    lw    $12,-44($30)
    lw    $31,-48($30)

    jr    $31                   ; return to caller
