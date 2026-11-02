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
.word 1
sw $3, -4($30)
sub $30, $30, $4
sw $0, -4($30)
sub $30, $30, $4
lw $3, 8($29)
sw $3, -4($30)
sub $30, $30, $4
lis $3
.word 2147483647
add $30, $30, $4
lw $5, -4($30)
mult $3, $4
mflo $3
add $3, $5, $3
sw $3, 0($29)
lw $3, 0($29)
sw $3, -4($30)
sub $30, $30, $4
lw $3, 8($29)
add $30, $30, $4
lw $5, -4($30)
sltu $6, $5, $3
sub $3, $11, $6
beq $3, $0, else0
lis $3
.word 1
sw $3, -4($29)
beq $0, $0, endif0
else0:
lis $3
.word 0
sw $3, -4($29)
endif0:
lw $3, -4($29)
; begin epilogue
add $30, $30, $4
add $30, $30, $4
add $30, $30, $4
add $30, $30, $4
jr $31
