        .importzp _r0, _r1, _r2

	.segment "CODE"
	.export strlen
strlen:
	ldy #0
	sty _r2
page_loop:
	lda (_r0),y
	beq done
	iny
	bne page_loop

	inc _r1
	inc _r2
#ifdef __CMOS__
	bra page_loop
#else
	jmp page_loop
#endif
done:
	sty _r0
	lda _r2
	sta _r1
	rts
