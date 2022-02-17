	.export _mega65_dos_exechelper

	.import	_mega65_restore_zp

	.include "zeropage.inc"

.SEGMENT "CODE"

	.p4510

_mega65_dos_exechelper:
	;; char mega65_dos_exechelper(char *image_name);

	LDA	#$00
	LDX	#$00
	LDY	#$00
	LDZ	#$00
	MAP
	EOM

	SEI

	LDY	#$01
	TYS

	LDA	#$00
	TAB

	LDA	#$80
	TRB	$D054

	LDA	#27
	STA	$D011

	LDA	#200
	STA	$D016

	LDA	#20
	STA	$D018

	LDA	#$00
	STA	$D060
	LDA	#$04
	STA	$D061
	LDA	#$00
	STA	$D062

	LDA	#$80
	TRB	$D031

	LDA	#$05
	TRB	$D054

	LDA	#$08
	TRB	$D031

;	Set VIC line size
	LDA #$28
	STA $D058
	LDA #$00
	STA $D059

;	Set VIC window w
	LDA	#$28
	STA	$D05E

;	CHRYSCL
	LDA	#$01
	STA	$D05B

;	Set VIC window h
	LDA	#$19
	STA	$D07B

	;; Get pointer to file name
	;; sp here is the ca65 sp ZP variable, not the stack pointer of a 4510
	ldy #1
	.p02
	lda (sp),y
	sta ptr1+1
	sta $0141
	dey
	lda (sp),y
	.p4510
	sta ptr1
	sta $0140

	;; Copy file name
	ldy #0
@NameCopyLoop:
	lda (ptr1),y
	sta $0100,y
	iny
	cmp #0
	bne @NameCopyLoop

	JSR	_mega65_restore_zp

	; close all files to work around hyppo file descriptor leak bug
        lda #$22
        sta $d640
        nop

	;;  Call dos_setname()
	ldy #>$0100
	ldx #<$0100
	lda #$2E     		; dos_setname Hypervisor trap
	STA $D640		; Do hypervisor trap
	NOP			; Wasted instruction slot required following hyper trap instruction
	;; XXX Check for error (carry would be clear)

	BCC	@error

	; close all files to work around hyppo file descriptor leak bug
       lda #$22
        sta $d640
        nop
	
	;; Now copy a little routine into place in $0340 that does the actual loading and jumps into
	;; the program when loaded.
	ldx #$00
@lfr1:	lda loadfile_routine,x
	sta $0340,x
	inx
	cpx #$34
	bne @lfr1

	;; Call helper routine
	jsr $0340
	
@error:
	;; as this is effectively like exec() on unix, it can only return an error
	LDA #$01
	LDX #$00
	
	RTS

loadfile_routine:
	LDA	#$2F
	STA	$00

	LDA	#$37
	STA	$01

;	LDA	#$41
;	STA	$00

	LDA	#$00
	STA	$D01A

	LDA	$DC0D
	ASL	$D019

	LDA	#$FF
	STA	$DC0D


	; Now load the file to $0400 over the screen
        lda #$36
        ldx #$FF
        ldy #$07
        ldz #$00
        sta $d640
        nop
        bcc @error

        LDX	#$FF
        TXS

        ldz     #$00
;@halt:
;		INC	$D020
;		BRA	@halt

        CLI
		jmp $080D

	rts
@error:
		LDA	#$02
		STA	$D020
		BRA	@error