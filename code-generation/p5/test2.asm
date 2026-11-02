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
.word 65
lis $5
.word 0xffff000c
sw $3, 0($5)
lis $3
.word 0
; begin epilogue
add $30, $30, $4
add $30, $30, $4
add $29, $30, $0
jr $31
