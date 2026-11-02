print:
    ; Save first to negative offsets (preserve caller regs)
    sw    $1,  -4($30)
    sw    $2,  -8($30)
    sw    $3, -12($30)
    sw    $4, -16($30)
    sw    $5, -20($30)
    sw    $6, -24($30)
    sw    $8, -28($30)
    sw    $10,-32($30)

    ; Allocate 32-byte frame
    lis   $2
    .word 32
    sub   $30, $30, $2

    ; constants
    lis   $4
    .word 4              ; word size for push/pop
    lis   $10
    .word 10             ; base 10
    lis   $8
    .word 0xffff000c     ; putchar (prints low byte)

    ; If negative, print '-' but DO NOT negate $1 (INT_MIN-safe)
    slt   $3, $1, $0
    beq   $3, $0, notneg
    lis   $3
    .word 45             ; '-'
    sw    $3, 0($8)

notneg:
    ; If zero, print '0'
    beq   $1, $0, printzero

    ; gather digits onto tiny stack buffer (bytes pushed in $5)
    add   $5, $0, $0

digitloop:
    div   $1, $10
    mfhi  $3             ; remainder (can be negative)

    ; make remainder positive if needed
    slt   $6, $3, $0
    beq   $6, $0, remok
    sub   $3, $0, $3
remok:
    lis   $6
    .word 48             ; '0'
    add   $3, $3, $6     ; to ASCII

    ; push this ASCII digit
    sub   $30, $30, $4
    sw    $3, 0($30)
    add   $5, $5, $4

    mflo  $1             ; quotient
    bne   $1, $0, digitloop

printdigits:
    ; pop/print until count is zero
    beq   $5, $0, afterdigits
    lw    $3, 0($30)
    sw    $3, 0($8)
    add   $30, $30, $4
    sub   $5,  $5,  $4
    beq   $0,  $0,  printdigits

printzero:
    lis   $3
    .word 48             ; '0'
    sw    $3, 0($8)

afterdigits:
    lis   $3
    .word 10             ; '\n'
    sw    $3, 0($8)

    ; Deallocate frame back to original SP
    lis   $2
    .word 32
    add   $30, $30, $2

    ; Restore regs from the same negative offsets
    lw    $1,  -4($30)
    lw    $2,  -8($30)
    lw    $3, -12($30)
    lw    $4, -16($30)
    lw    $5, -20($30)
    lw    $6, -24($30)
    lw    $8, -28($30)
    lw    $10,-32($30)

    jr    $31
