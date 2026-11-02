.import printHex
.import readWord

lis $14
.word 4

sub $30, $30, $14
sw $31, 0($30)

lis $2
.word printHex

lis $16
.word readWord

jalr $16
jalr $16
jalr $16
add $5, $0, $3

lis $6
.word 12
sub $6, $5, $6

loop:
beq $6, $0, done
jalr $16            ; read word into $3
add $7, $0, $6      ; save counter in $7
add $8, $0, $3      ; save the word we want to print in $8
add $1, $0, $8      ; put word in $1 for printHex
jalr $2             ; printHex prints value from $1
add $6, $0, $7      ; restore counter
sub $6, $6, $14
beq $0, $0, loop

done:
lw $31, 0($30)
add $30, $30, $14
jr $31