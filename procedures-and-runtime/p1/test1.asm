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
ERROR: Invalid lvalue structure
