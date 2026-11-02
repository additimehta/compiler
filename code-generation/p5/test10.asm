.import print
; begin prologue
lis $4
.word 4
lis $11
.word 1
sw $1, -4($30)
sub $30, $30, $4
sw $2, -4($30)
sub $30, $30, $4
sub $29, $30, $4
; end prologue
lis $3
.word 3
sw $3, -4($30)
sub $30, $30, $4
lis $3
.word 4
sw $3, -4($30)
sub $30, $30, $4
lis $3
.word 5
sw $3, -4($30)
sub $30, $30, $4
lis $3
.word 6
sw $3, -4($30)
sub $30, $30, $4
; DEBUG: x at 0($29)
; DEBUG: y at -4($29)
; DEBUG: c at -8($29)
; DEBUG: d at -12($29)
lw $3, 8($29)
sw $3, 0($29)
lw $3, 8($29)
add $1, $3, $0
sw $31, -4($30)
sub $30, $30, $4
lis $5
.word print
jalr $5
add $30, $30, $4
lw $31, -4($30)
lw $3, 0($29)
sw $3, 4($29)
lw $3, 4($29)
add $1, $3, $0
sw $31, -4($30)
sub $30, $30, $4
lis $5
.word print
jalr $5
add $30, $30, $4
lw $31, -4($30)
lw $3, -4($29)
sw $3, -4($29)
lw $3, -8($29)
add $1, $3, $0
sw $31, -4($30)
sub $30, $30, $4
lis $5
.word print
jalr $5
add $30, $30, $4
lw $31, -4($30)
lw $3, -12($29)
sw $3, -8($29)
lw $3, -12($29)
add $1, $3, $0
sw $31, -4($30)
sub $30, $30, $4
lis $5
.word print
jalr $5
add $30, $30, $4
lw $31, -4($30)
lw $3, -4($29)
sw $3, -12($29)
lw $3, -12($29)
; begin epilogue
add $30, $30, $4
add $30, $30, $4
add $30, $30, $4
add $30, $30, $4
add $30, $30, $4
add $30, $30, $4
jr $31
