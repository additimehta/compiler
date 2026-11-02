.import printHex
.import readWord

lis $14
.word 4
lis $7
.word 12

sub $30, $30, $14
sw  $31, 0($30)

lis $2
.word printHex
lis $16
.word readWord

jalr $16
add $20, $0, $3

jalr $16
jalr $16
add $24, $0, $3

jalr $16
add $5, $0, $3

sub $6, $5, $7
sub $10, $24, $5
sub $9, $20, $7

lis $8
.word 16
sub $23, $20, $8

lw  $25, 0($30)
sw  $25, 0($23)
sw  $20, 4($23)
sw  $6, 8($23)
sw  $2, 12($23)

add $21, $0, $20
add $22, $0, $6

loadloop:
beq $22, $0, relocstart
jalr $16
sw  $3, 0($21)
add $1, $0, $3
jalr $2
add $21, $21, $14
sub $22, $22, $14
beq $0, $0, loadloop

relocstart:
beq $10, $0, execsetup

relocloop:
beq $10, $0, execsetup
jalr $16
add $11, $0, $3
jalr $16
add $12, $0, $3
sub $13, $12, $7
add $13, $13, $20
lw  $18, 0($13)
add $19, $18, $9
sw  $19, 0($13)
sub $10, $10, $14
sub $10, $10, $14
beq $0, $0, relocloop

execsetup:
lw  $25, 0($23)
lw  $20, 4($23)
lw  $6, 8($23)
lw  $2, 12($23)

lis $8
.word 0x10000
add $30, $20, $8

add $3, $0, $20
jalr $3

lis $8
.word 16
sub $23, $20, $8

lw  $25, 0($23)
lw  $20, 4($23)
lw  $6, 8($23)
lw  $2, 12($23)

add $21, $0, $20
add $22, $0, $6

printloop:
beq $22, $0, end
lw  $1, 0($21)
jalr $2
add $21, $21, $14
sub $22, $22, $14
beq $0, $0, printloop

end:
lis $8
.word 16
add $30, $23, $8
jr  $25
