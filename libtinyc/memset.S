        .importzp _r0, _r1, _r2, _r3, _r4, _r5

	.segment "CODE"
	.export memset
	; args:
	; _r1, _r0: dst pointer
	; _r3 (unused), _r2: character to write
	; _r5, _r4: number of bytes to fill
	; returns:
	; _r1, _r0: dst pointer
memset:
	lda _r1
	sta _r3

	lda _r2

	ldy #0
	ldx _r5
	beq lastpage

fullpage:
	sta (_r0),y
	iny
	bne fullpage
	inc _r1
	dex
	bne fullpage

lastpage:
	ldy _r4
	beq done
lploop:
	dey
	sta (_r0),y
	bne lploop

done:
	lda _r3
	sta _r1
	rts
