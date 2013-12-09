	.org 0xe00
start:
	ldx #0
loop:
	lda message,x
	sta 0xfff0
	beq done
	inx
	jmp loop
done:
	; Exit emulator.
	jmp 0

message:
	.asc "Hello world\n",0
