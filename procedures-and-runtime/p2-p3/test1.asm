.import print
lis $4
.word 4
lis $11
.word 1
beq $0, $0, wain
wain: 
; begin prologue
sw $1, -4($30)
sub $30, $30, $4
sw $2, -4($30)
sub $30, $30, $4
sub $29, $30, $4
; end prologue
lis $3
.word 4
sw $3, -4($30)
sub $30, $30, $4
lw $3, 8($29)
add $30, $30, $4
lw $5, -4($30)
mult $5, $4
mflo $5
add $3, $5, $3
lw $3, 0($3)
; begin epilogue
add $30, $30, $4
add $30, $30, $4
jr $31
