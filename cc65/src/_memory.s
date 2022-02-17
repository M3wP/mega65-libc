	.export	_mega65_save_zp, _mega65_restore_zp
	.export	_toggle_rom_write_protect

	.import	_mega65_io_enable

.SEGMENT "DATA"
 savezp:
	.res	256


.SEGMENT "CODE"

	.p4510


_toggle_rom_write_protect:
	jsr _mega65_io_enable
	lda #$70
	STA $d640
	nop
	ldx #0
	rts


_mega65_save_zp:
	LDA	#$00
	LDX	#$00
	LDY	#$00
	LDZ	#$00
	MAP
	EOM

	LDY	#$01
	TYS

	LDA	#$00
	TAB

	LDX	#$02
@loop:
	LDA	$00, X
	STA	savezp, X
	INX
	BNE	@loop

	RTS

_mega65_restore_zp:
	LDX	#$02
@loop:
	LDA	savezp, X
	STA	$00, X
	INX
	BNE	@loop

	RTS


	
