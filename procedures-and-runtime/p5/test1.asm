.import print
.import init
.import new
.import delete
lis $4
.word 4
lis $11
.word 1
beq $0, $0, wain
wain: 
sw $1, -4($30)
sub $30, $30, $4
sw $2, -4($30)
sub $30, $30, $4
sw $31, -4($30)
sub $30, $30, $4
lis $5
.word init
jalr $5
add $30, $30, $4
lw $31, -4($30)
add $30, $30, $4
lw $2, -4($30)
add $30, $30, $4
lw $1, -4($30)
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
sw $0, -4($30)
sub $30, $30, $4
lis $3
.word 1
sw $3, -4($30)
sub $30, $30, $4
loop0:
lw $3, -4($29)
sw $3, -4($30)
sub $30, $30, $4
lis $3
.word 32
add $30, $30, $4
lw $5, -4($30)
slt $3, $5, $3
beq $3, $0, endloop0
lis $3
.word 1023
add $1, $3, $0
sw $31, -4($30)
sub $30, $30, $4
lis $5
.word new
jalr $5
add $30, $30, $4
lw $31, -4($30)
bne $3, $0, NEWOK0
add $3, $11, $0
NEWOK0:
sw $3, -12($29)
lis $3
.word 0
sw $3, -8($29)
loop1:
lw $3, -8($29)
sw $3, -4($30)
sub $30, $30, $4
lis $3
.word 1023
add $30, $30, $4
lw $5, -4($30)
slt $3, $5, $3
beq $3, $0, endloop1
lw $3, -12($29)
sw $3, -4($30)
sub $30, $30, $4
lw $3, -8($29)
add $30, $30, $4
lw $5, -4($30)
mult $3, $4
mflo $3
add $3, $5, $3
sw $3, -4($30)
sub $30, $30, $4
lw $3, -4($29)
sw $3, -4($30)
sub $30, $30, $4
lw $3, -8($29)
add $30, $30, $4
lw $5, -4($30)
mult $5, $3
mflo $3
add $30, $30, $4
lw $5, -4($30)
sw $3, 0($5)
lw $3, -8($29)
sw $3, -4($30)
sub $30, $30, $4
lis $3
.word 1
add $30, $30, $4
lw $5, -4($30)
add $3, $5, $3
sw $3, -8($29)
beq $0, $0, loop1
endloop1:
lw $3, -12($29)
beq $3, $11, DELETESKIP0
add $1, $3, $0
sw $31, -4($30)
sub $30, $30, $4
lis $5
.word delete
jalr $5
add $30, $30, $4
lw $31, -4($30)
DELETESKIP0:
lw $3, -4($29)
sw $3, -4($30)
sub $30, $30, $4
lis $3
.word 1
add $30, $30, $4
lw $5, -4($30)
add $3, $5, $3
sw $3, -4($29)
beq $0, $0, loop0
endloop0:
lw $3, 4($29)
add $1, $3, $0
sw $31, -4($30)
sub $30, $30, $4
lis $5
.word new
jalr $5
add $30, $30, $4
lw $31, -4($30)
bne $3, $0, NEWOK1
add $3, $11, $0
NEWOK1:
sw $3, 0($29)
lis $3
.word 0
sw $3, -4($29)
loop2:
lw $3, -4($29)
sw $3, -4($30)
sub $30, $30, $4
lw $3, 4($29)
add $30, $30, $4
lw $5, -4($30)
slt $3, $5, $3
beq $3, $0, endloop2
lw $3, 0($29)
sw $3, -4($30)
sub $30, $30, $4
lw $3, -4($29)
add $30, $30, $4
lw $5, -4($30)
mult $3, $4
mflo $3
add $3, $5, $3
sw $3, -4($30)
sub $30, $30, $4
lw $3, 8($29)
sw $3, -4($30)
sub $30, $30, $4
lw $3, -4($29)
add $30, $30, $4
lw $5, -4($30)
mult $3, $4
mflo $3
add $3, $5, $3
lw $3, 0($3)
add $30, $30, $4
lw $5, -4($30)
sw $3, 0($5)
lw $3, -4($29)
sw $3, -4($30)
sub $30, $30, $4
lis $3
.word 1
add $30, $30, $4
lw $5, -4($30)
add $3, $5, $3
sw $3, -4($29)
beq $0, $0, loop2
endloop2:
lis $3
.word 0
sw $3, -8($29)
loop3:
lw $3, -8($29)
sw $3, -4($30)
sub $30, $30, $4
lis $3
.word 1024
add $30, $30, $4
lw $5, -4($30)
slt $3, $5, $3
beq $3, $0, endloop3
lis $3
.word 1
add $1, $3, $0
sw $31, -4($30)
sub $30, $30, $4
lis $5
.word new
jalr $5
add $30, $30, $4
lw $31, -4($30)
bne $3, $0, NEWOK2
add $3, $11, $0
NEWOK2:
sw $3, -12($29)
lw $3, -12($29)
sw $3, -4($30)
sub $30, $30, $4
lis $3
.word 241
add $30, $30, $4
lw $5, -4($30)
sw $3, 0($5)
lw $3, -12($29)
beq $3, $11, DELETESKIP1
add $1, $3, $0
sw $31, -4($30)
sub $30, $30, $4
lis $5
.word delete
jalr $5
add $30, $30, $4
lw $31, -4($30)
DELETESKIP1:
lw $3, -8($29)
sw $3, -4($30)
sub $30, $30, $4
lis $3
.word 1
add $30, $30, $4
lw $5, -4($30)
add $3, $5, $3
sw $3, -8($29)
beq $0, $0, loop3
endloop3:
lis $3
.word 0
sw $3, -4($29)
loop4:
lw $3, -4($29)
sw $3, -4($30)
sub $30, $30, $4
lw $3, 4($29)
add $30, $30, $4
lw $5, -4($30)
slt $3, $5, $3
beq $3, $0, endloop4
lw $3, 8($29)
sw $3, -4($30)
sub $30, $30, $4
lw $3, -4($29)
add $30, $30, $4
lw $5, -4($30)
mult $3, $4
mflo $3
add $3, $5, $3
lw $3, 0($3)
add $1, $3, $0
sw $31, -4($30)
sub $30, $30, $4
lis $5
.word print
jalr $5
add $30, $30, $4
lw $31, -4($30)
lw $3, 0($29)
sw $3, -4($30)
sub $30, $30, $4
lw $3, -4($29)
add $30, $30, $4
lw $5, -4($30)
mult $3, $4
mflo $3
add $3, $5, $3
lw $3, 0($3)
add $1, $3, $0
sw $31, -4($30)
sub $30, $30, $4
lis $5
.word print
jalr $5
add $30, $30, $4
lw $31, -4($30)
lw $3, -4($29)
sw $3, -4($30)
sub $30, $30, $4
lis $3
.word 1
add $30, $30, $4
lw $5, -4($30)
add $3, $5, $3
sw $3, -4($29)
beq $0, $0, loop4
endloop4:
lw $3, 0($29)
beq $3, $11, DELETESKIP2
add $1, $3, $0
sw $31, -4($30)
sub $30, $30, $4
lis $5
.word delete
jalr $5
add $30, $30, $4
lw $31, -4($30)
DELETESKIP2:
lis $3
.word 0
; begin epilogue
add $30, $30, $4
add $30, $30, $4
add $30, $30, $4
add $30, $30, $4
add $30, $30, $4
add $30, $30, $4
jr $31
