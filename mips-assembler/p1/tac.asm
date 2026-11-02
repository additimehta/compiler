; TAC implementation
; Registers being used
; $20 - used for stdout
; $21 - used for stdin
; $11 - constant -1 used to check if reading is complete
; $4 - constant 4 used for pushing data into stack and restoring
; $5 - temp register
; $1 - works as a temp stack pointer
; $3 - word counter (also total bytes)


; a chain of chars work very similar to how an array works
; so when accessing a char we must move up by 4


lis $20
.word 0xffff000c
lis $21
.word 0xffff0004
lis $11
.word -1
lis $4
.word 4

add $3, $0, $0
add $1, $30, $0



read:
	lw $2, 0($21)		; read from stdin into $3
	beq $2, $11, write	; if the encoding is -1 then reading is complete
	sw $2, -4($30)		; store byte that was just read good practice if the procedure was called somwhere else
	sub $30, $30, $4	; adjust stack
	add $1,  $30, $0	; adjust the stack pointer
	add $3, $3, $4		; incremeent the word count (total bytes)
	beq $0, $0, read	; loop back to read



write:
	beq $3, $0, done	; word count = 0, no more chars to print!
	lw $2, 0($1)		; load character from wherever the stack pointer is pointing to, into $2
	sw $2, 0($20)		; write the character just loaded to stdout
	add $1, $1, $4		; move the stack pointer to the next character
	sub $3, $3, $4		; decrease the word count
	beq $0, $0, write	; loop back to write

done:
	jr $31				; printing complete

; Citations:
; A lot of the comments and I/O methods were from the textbook
; Module 4 : Assembly Language, page 30