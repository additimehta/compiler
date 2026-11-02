; increasing.asm
; Check if an array is strictly increasing using compare

; Registers being used
; $1  - A[i], first parameter to compare
; $2  - A[i+1], second parameter to compare
; $3  - output (1 if strictly increasing, 0 otherwise)
; $4  - constant 4 (word size)
; $5  - temporary for i+1
; $6  - base address of array
; $7  - length of array
; $8  - address of compare procedure
; $9  - loop index i
; $10 - constant 1
; $11 - result of slt check
; $12 - address of A[i]
; $14 - address of A[i+1]
; $30 - stack pointer
; $31 - return address


lis   $4
.word 4                  ; $4 = word size
lis   $10
.word 1                  ; $10 = constant 1

add   $6, $1, $0         ; save base address into $6
add   $7, $2, $0         ; save length into $7
add   $9, $0, $0         ; i = 0
lis   $8
.word compare

loop:
    sub   $11, $7, $10   ; $11 = n - 1
    slt   $11, $9, $11   ; check if i < n - 1
    beq   $11, $0, end   ; if not, end

    ; load A[i] into $1
    mult  $9, $4         ; i * 4
    mflo  $12
    add   $12, $12, $6   ; base + offset
    lw    $1, 0($12)

    ; load A[i+1] into $2
    add   $5, $9, $10    ; i+1
    mult  $5, $4
    mflo  $14
    add   $14, $14, $6   ; base + offset
    lw    $2, 0($14)

    ; call compare(A[i], A[i+1])
    sub   $30, $30, $4   ; save return address
    sw    $31, 0($30)

    jalr  $8
    add   $0, $0, $0     ; nop in delay slot

    lw    $31, 0($30)    ; restore return address
    add   $30, $30, $4   ; pop stack

    beq   $3, $0, fail   ; if compare returned 0 -> fail

    add   $9, $9, $10    ; i = i+1
    beq   $0, $0, loop   ; repeat


end:
    lis   $3
    .word 1              ; result = 1 (strictly increasing)
    jr    $31

fail:
    add   $3, $0, $0     ; result = 0 (not strictly increasing)
    jr    $31



; CITATIONS:
; - Module 4: Assembly Lanuage pg30,33