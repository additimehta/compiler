; begin prologue
lis $4
.word 4
sw $1, -4($30)
sub $30, $30, $4
sw $2, -4($30)
sub $30, $30, $4
sub $29, $30, $4
; end prologue
lw $3, 4($29)
sw $3, 8($29)
lw $3, 8($29)
; begin epilogue
add $30, $30, $4
add $30, $30, $4
jr $31
