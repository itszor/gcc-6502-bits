        .importzp _sp0, _sp1, _fp0, _fp1
        .importzp _r0, _r1, _r2, _r3, _r4, _r5, _r6, _r7
        .importzp _s0, _s1, _s2, _s3, _s4, _s5, _s6, _s7
        .importzp _tmp0, _tmp1

	.importzp _m65x_fpe0_mant, _m65x_fpe0_exp, _m65x_fpe0_sign
	.importzp _m65x_fpe1_mant, _m65x_fpe1_exp, _m65x_fpe1_sign
	
#define s_mant _m65x_fpe0_mant
#define s_exp _m65x_fpe0_exp
#define s_sign _m65x_fpe0_sign

#define s2_mant _m65x_fpe1_mant
#define s2_exp _m65x_fpe1_exp
#define s2_sign _m65x_fpe1_sign


	.segment "ZEROPAGE"
bufptr:
	.word 0

	.segment "DATA"
b_mant:
	.dword 0
b_exp:
	.byte 0
b_sign:
	.byte 0
fpnum:
	.dword 0
dec_exp:
	.byte 0

sigfigs:
	.byte 8
bufidx:
	.byte 0
print_exp:
	.byte 0
place:
	.byte 0
lastplace:
	.byte 0
	
	.segment "CODE"
	; Arguments:
	;  - r0-r3: floating-point value to unpack.
unpack:
	.scope
	lda #0
	sta s_mant
	sta s_mant+4
	sta s_exp+1
	lda _r0
	sta s_mant+1
	lda _r1
	sta s_mant+2
	lda _r2
	tax
	and #$7f
	sta s_mant+3
	; sign is stored in high bit of s_sign.
	txa
	and #$80
	sta s_sign
	
	lda _r3
	sta s_exp
	beq exp_zero
	lda s_mant+3
	ora #$80
	sta s_mant+3
exp_zero:
	rts
	.endscope

copy_s_to_s2:
	.scope
	lda s_mant
	sta s2_mant
	lda s_mant+1
	sta s2_mant+1
	lda s_mant+2
	sta s2_mant+2
	lda s_mant+3
	sta s2_mant+3
	lda s_mant+4
	sta s2_mant+4
	lda s_exp
	sta s2_exp
	lda s_sign
	sta s2_sign
	rts
	.endscope

copy_bias_to_s2:
	lda b_mant
	sta s2_mant
	lda b_mant+1
	sta s2_mant+1
	lda b_mant+2
	sta s2_mant+2
	lda b_mant+3
	sta s2_mant+3
	lda #0
	sta s2_mant+4
	lda b_exp
	sta s2_exp
	lda b_sign
	sta s2_sign
	rts

add_s2_mant_to_s:
	lda s_mant
	clc
	adc s2_mant
	sta s_mant
	lda s_mant+1
	adc s2_mant+1
	sta s_mant+1
	lda s_mant+2
	adc s2_mant+2
	sta s_mant+2
	lda s_mant+3
	adc s2_mant+3
	sta s_mant+3
	lda s_mant+4
	adc s2_mant+4
	sta s_mant+4
	rts

renormalize_right:
	.scope
	lda s_mant+4
loop:
	cmp #0
	beq done
	lsr a
	ror s_mant+3
	ror s_mant+2
	ror s_mant+1
	ror s_mant
	inc s_exp
	jmp loop
done:
	sta s_mant+4
	rts
	.endscope

div10:
	.scope
	lda s_exp
	sec
	sbc #4
	sta s_exp
	
	; s2 = s >> 1
	lda s_mant+4
	lsr a
	sta s2_mant+4
	lda s_mant+3
	ror a
	sta s2_mant+3
	lda s_mant+2
	ror a
	sta s2_mant+2
	lda s_mant+1
	ror a
	sta s2_mant+1
	lda s_mant
	ror a
	sta s2_mant
	
	; s = s + s2
	jsr add_s2_mant_to_s

	jsr copy_s_to_s2
	
	; divide the s2 mantissa by 16
	lda s2_mant+4
	ldx #4
div16:
	lsr a
	ror s2_mant+3
	ror s2_mant+2
	ror s2_mant+1
	ror s2_mant
	dex
	bne div16
	sta s2_mant+4
	
	jsr add_s2_mant_to_s
	
	;   s_mant+4 s_mant+3 s_mant+2 s_mant+1 s_mant
	; +          s_mant+4 s_mant+3 s_mant+2 s_mant+1
	;-> s_mant+4 s_mant+3 s_mant+2 s_mant+1 s_mant
	
	lda s_mant
	clc
	adc s_mant+1
	sta s_mant
	lda s_mant+1
	adc s_mant+2
	sta s_mant+1
	lda s_mant+2
	adc s_mant+3
	sta s_mant+2
	lda s_mant+3
	adc s_mant+4
	sta s_mant+3
	bcc :+
	inc s_mant+4
	:
	
	;   s_mant+4 s_mant+3 s_mant+2 s_mant+1 s_mant
	; +                   s_mant+4 s_mant+3 s_mant+2
	;->    <n/a> s_mant+3 s_mant+2 s_mant+1 s_mant
	
	lda s_mant
	clc
	adc s_mant+2
	sta s_mant
	lda s_mant+1
	adc s_mant+3
	sta s_mant+1
	lda s_mant+2
	adc s_mant+4
	sta s_mant+2
	bcc :+
	inc s_mant+3
	:
	
	jmp renormalize_right
	.endscope

ensure_less_than_10:
	; Divide input value until input value < 10.0.
	; continue while:
	; s_exp > 130 || (s_exp == 130 && s_mant >= 0xa0000000)
	.scope
div10_loop:
	lda s_exp
	cmp #131
	bcs do_div
	cmp #130
	bne less_than_10
	lda s_mant+3
	cmp #$a0
	bcc less_than_10
do_div:
	jsr div10
	inc dec_exp
	jmp div10_loop
less_than_10:
	rts
	.endscope

mul10:
	.scope
	jsr copy_s_to_s2

	lda s_exp
	clc
	adc #3
	sta s_exp
	
	lda s2_mant+3
	lsr a
	ror s2_mant+2
	ror s2_mant+1
	ror s2_mant
	lsr a
	ror s2_mant+2
	ror s2_mant+1
	ror s2_mant
	sta s2_mant+3
	
	jsr add_s2_mant_to_s
	jmp renormalize_right
	.endscope

add_bias:
	.scope
	jsr copy_bias_to_s2

	; exit if s2_exp+31 < s_exp
	lda s2_exp
	clc
	adc #31
	cmp s_exp
	bcc out
	
	lda s_exp
	sec
	sbc s2_exp
	tax
	beq done_shifting
	lda s2_mant+4
rshift_s2:
	lsr a
	ror s2_mant+3
	ror s2_mant+2
	ror s2_mant+1
	ror s2_mant
	dex
	bne rshift_s2
	sta s2_mant+4
done_shifting:

	jsr add_s2_mant_to_s
	jmp renormalize_right
out:
	rts
	.endscope

denormalize:
	.scope
	ldx s_exp
	lda s_mant
loop:
	cpx #130
	bcs done
	lsr s_mant+4
	ror s_mant+3
	ror s_mant+2
	ror s_mant+1
	ror a
	inx
	jmp loop
done:
	sta s_mant
	stx s_exp
	rts
	.endscope

	.export __m65x_ftoa
	; Arguments:
	;   - r0,r1: pointer to output string.
	;   - r2-r5: floating-point value to print.
__m65x_ftoa:
	.scope

	lda _r0
	sta bufptr
	lda _r1
	sta bufptr+1
	lda _r2
	sta fpnum
	lda _r3
	sta fpnum+1
	lda _r4
	sta fpnum+2
	lda _r5
	sta fpnum+3

	; Unpack the constant 5.0.
	lda #0
	sta _r0
	sta _r1
	lda #32
	sta _r2
	lda #129
	sta _r3
	jsr unpack
	
	; Divide by 10 "sigfigs" times.
	.scope
	lda sigfigs
	lda #8
	sta _r6
loop:
	jsr div10
	dec _r6
	bne loop
	.endscope
	
	; Copy to "bias", b_mant/b_exp/b_sign
	lda s_mant
	sta b_mant
	lda s_mant+1
	sta b_mant+1
	lda s_mant+2
	sta b_mant+2
	lda s_mant+3
	sta b_mant+3
	lda s_exp
	sta b_exp
	; Copy the sign of the input number.
	lda fpnum+2
	and #$80
	sta b_sign
		
	lda #0
	sta dec_exp
	
	; Unpack input value
	lda fpnum
	sta _r0
	lda fpnum+1
	sta _r1
	lda fpnum+2
	sta _r2
	lda fpnum+3
	sta _r3
	jsr unpack
	
	ldy #0
	sty bufidx
	
	lda s_sign
	bpl no_sign_char
	lda #'-'
	sta (bufptr),y
	inc bufidx
no_sign_char:

	ldy bufidx
	lda s_exp
	ora s_mant
	ora s_mant+1
	ora s_mant+2
	ora s_mant+3
	bne not_zero
	lda #'0'
	sta (bufptr),y
	inc bufidx
	jmp finished
not_zero:
	
	; Multiply input value until s_exp >= 127 (input value >= 1.0).
	.scope
mul10_loop:
	lda s_exp
	cmp #127
	bcs greater_than_1
	jsr mul10
	dec dec_exp
	jmp mul10_loop
greater_than_1:
	.endscope
	
	jsr ensure_less_than_10
	jsr add_bias
	jsr ensure_less_than_10

	; if dec_exp < -3 || dec_exp > 8 then print ...E...
	
	lda #0
	sta print_exp
	lda dec_exp
	clc
	adc #4
	cmp #12
	bcc no_exp
	lda #1
	sta print_exp
no_exp:

	; Output '0.' + <-dec_exp-1> x '0'
	.scope
	lda dec_exp
	bpl no_zero_dot_leading_zeros
	lda print_exp
	bne no_zero_dot_leading_zeros
	ldy bufidx
	lda #'0'
	sta (bufptr),y
	iny
	lda #'.'
	sta (bufptr),y
	iny
	ldx dec_exp
	lda #'0'
emit_zeros:
	cpx #255
	beq done
	sta (bufptr),y
	inx
	iny
	jmp emit_zeros
done:
	sty bufidx
no_zero_dot_leading_zeros:
	.endscope
	
	; The main digit output loop!
	.scope
	lda #0
	sta place
emit_digits:
	lda place
	cmp sigfigs
	bcc loop_body
	lda print_exp
	beq :+
	jmp done
:	lda dec_exp
	sec
	sbc place
	bvc :+
	eor #$80
:	bmi done
loop_body:
	
	.scope
	lda #0
	sta lastplace
	lda sigfigs
	sec
	sbc place
	cmp #1
	bne notlastplace
	sta lastplace
notlastplace:
	.endscope
	
	jsr denormalize
	
	lda s_mant+3
	lsr a
	lsr a
	lsr a
	lsr a
	ora #$30
	ldy bufidx
	sta (bufptr),y
	iny
	sty bufidx
	
	ldx #4
	lda s_mant
renormalize:
	asl a
	rol s_mant+1
	rol s_mant+2
	rol s_mant+3
	dec s_exp
	dex
	bne renormalize
	sta s_mant
	
	; print decimal point if appropriate
	.scope
	lda print_exp
	beq no_exponent
	lda place
	bne done
	lda lastplace
	beq emit_point
	jmp done
no_exponent:
	lda place
	cmp dec_exp
	bne done
	lda lastplace
	bne done
emit_point:
	ldy bufidx
	lda #'.'
	sta (bufptr),y
	iny
	sty bufidx
done:
	.endscope
	
	jsr mul10
	
	inc place
	jmp emit_digits
done:
	.endscope
	
	; Strip trailing zeros and decimal point
	.scope
	dec place
	ldy bufidx
	dey
strip_zeros:
	cpy #1
	bcc done_zeros
	lda (bufptr),y
	cmp #'0'
	bne done_zeros
	lda dec_exp
	cmp place
	bcs done_zeros
	sty bufidx
	dey
	dec place
	jmp strip_zeros
done_zeros:
	
	lda (bufptr),y
	cmp #'.'
	bne done_point
	sty bufidx
done_point:
	
	.endscope
	
finished:
	lda #0
	ldy bufidx
	sta (bufptr),y
	
	.scope
	lda print_exp
	beq no_exp
	lda dec_exp
	sta _r0
	rts
no_exp:
	lda #0
	sta _r0
	rts
	.endscope

	.endscope
