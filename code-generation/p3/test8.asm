; begin prologue
lis $4
.word 4
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
.word 5
add $30, $30, $4
lw $5, -4($30)
add $3, $5, $3
sw $3, -4($30)
sub $30, $30, $4
lis $3
.word 7
sw $3, -4($30)
sub $30, $30, $4
lis $3
.word 2
add $30, $30, $4
lw $5, -4($30)
sub $3, $5, $3
add $30, $30, $4
lw $5, -4($30)
mult $5, $3
mflo $3
; begin epilogue
add $30, $30, $4
add $30, $30, $4
jr $31
