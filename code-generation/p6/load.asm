.import printHex
.import readWord

lis $14
.word 4

; Save return address
sub $30, $30, $14
sw $31, 0($30)

lis $2
.word printHex

lis $16
.word readWord

; Read the load address a
jalr $16
add $20, $0, $3     ; $20 = a (load address)

; Skip first word of MERL (beq $0, $0, 2)
jalr $16

; Skip second word (endModule)
jalr $16

; Read third word (endCode)
jalr $16
add $5, $0, $3      ; $5 = endCode

; Calculate code size: endCode - 12
lis $6
.word 12
sub $6, $5, $6      ; $6 = code size in bytes

; Initialize for first loop
add $21, $0, $20    ; $21 = current address in memory (starts at a)
add $22, $0, $6     ; $22 = counter for first loop

loop1:
beq $22, $0, loop2setup
jalr $16            ; read word into $3
add $8, $0, $3      ; save word in $8
sw $3, 0($21)       ; store word at memory address $21
add $1, $0, $8      ; put word in $1 for printHex
jalr $2             ; print the word
add $21, $21, $14   ; increment memory address
sub $22, $22, $14   ; decrement counter
beq $0, $0, loop1

loop2setup:
add $21, $0, $20    ; reset to start address a
add $22, $0, $6     ; reset counter

loop2:
beq $22, $0, done
lw $3, 0($21)       ; load word from memory
add $1, $0, $3      ; put word in $1 for printHex
jalr $2             ; print the word
add $21, $21, $14   ; increment memory address
sub $22, $22, $14   ; decrement counter
beq $0, $0, loop2

done:
lw $31, 0($30)
add $30, $30, $14
jr $31
