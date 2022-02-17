
;	.setcpu "65C02"
	.export _closeall, _open, _close, _read512, _chdir, _chdirroot, _opendir, _readdir
	.export	_attachd81

	.include "zeropage.inc"
	
.SEGMENT "CODE"

;	.p4510

mega65_io_enable:
	lda #$47
	sta $d02f
	lda #$53
	sta $d02f
	rts
	
cc65_args_read_ptr1_16:	
        ;; sp here is the ca65 sp ZP variable, not the stack pointer of a 4510
;        .p02
	
        lda (sp),y
        sta ptr1
        iny
        lda (sp),y
;        .p4510
        sta ptr1+1
	iny
	rts
        
cc65_args_read_tmp1_8:	
        ;; sp here is the ca65 sp ZP variable, not the stack pointer of a 4510
;        .p02
        lda (sp),y
        sta tmp1
        iny
;        .p4510
	rts
        

cc65_copy_ptr1_string_to_0100:	
        ;; Copy file name
		
;		phy
		
		TYA
		PHA

        ldy #0
@NameCopyLoop:
        lda (ptr1),y
        sta $0100,y
        iny
        cmp #0
        bne @NameCopyLoop
	
	
;		ply
		PLA	
		TAY

		rts	

setname_0100:
        ;;  Call dos_setname()
        ldy #>$0100
        ldx #<$0100
	jsr mega65_io_enable
        lda #$2E                ; dos_setname Hypervisor trap
        STA $D640               ; Do hypervisor trap
        NOP                     ; Wasted instruction slot required following hyper trap instruction
        ;; XXX Check for error (carry would be clear)
	bcs setname_ok
	lda #$ff
	rts
setname_ok:
	RTS

	
	;; closeall
_closeall:
	jsr mega65_io_enable
	LDA #$22
	STA $D640
	NOP
	LDX #$00
	RTS

_read512:

	;;  Get pointer to buffer
	sta ptr1+0
	stx ptr1+1

	;; Select current file
	;; XXX - Not currently implemented
	
	;; Read the next sector of data
	jsr mega65_io_enable
	LDA #$1A
	STA $D640
	NOP
	LDX #$00

	;; Number of bytes read returned in X and Y
	;; Store these for returning
	stx tmp2
	sty tmp3

	;; Make sure SD buffer is selected, not FDC buffer
;	lda #$80
;	tsb $D689
	LDA	$D689
	ORA	#$80
	STA	$D689	

	;; Copy the full 512 bytes from the sector buffer at $FFD6E00
	;; (This saves the need to mess with mapping/unmapping the sector
	;; buffer).

	;; Get address to save to
	lda ptr1+0
	sta copysectorbuffer_destaddr+0
	lda ptr1+1
	sta copysectorbuffer_destaddr+1

	;; Execute DMA job
	lda #$00
	sta $d702
	sta $d704
	lda #>dmalist_copysectorbuffer
	sta $d701
	lda #<dmalist_copysectorbuffer
	sta $d705

	;; Retrieve the return value
	lda tmp2
	ldx tmp3
	RTS	

	.data

dmalist_copysectorbuffer:
	;; Copy $FFD6E00 - $FFD6FFF down to low memory 
	;; MEGA65 Enhanced DMA options
        .byte $0A  ;; Request format is F018A
        .byte $80,$FF ;; Source is $FFxxxxx
        .byte $81,$00 ;; Destination is $FF
        .byte $00  ;; No more options
        ;; F018A DMA list
        ;; (MB offsets get set in routine)
        .byte $00 ;; copy + last request in chain
        .word $0200 ;; size of copy is 512 bytes
        .word $6E00 ;; starting at $6E00
        .byte $0D   ;; of bank $D
copysectorbuffer_destaddr:	
        .word $8000 ;; destination address is $8000
        .byte $00   ;; of bank $0
        .word $0000 ;; modulo (unused)

	
	.code
	
_open:

        ;; Get pointer to file name
	sta ptr1+0
	stx ptr1+1
	
	jsr cc65_copy_ptr1_string_to_0100
	jsr setname_0100	

	;; Find file
	; Look for file on FAT file system via hypervisor calls
	lda #$34
	sta $d640
	nop
	bcs open_file_exists

	;; No such file.
	lda #$ff
	tax
	rts

open_file_exists:	
	;; Actually call open
	jsr mega65_io_enable
	lda #$00
	sta $d640
	nop
	
	LDA #$18
	STA $D640
	NOP
	
	LDX #$00
	RTS

_close:
	TAX
	jsr mega65_io_enable
	LDA #$20
	STA $D640
	NOP
	LDX #$00
	RTS

_chdirroot:
	;; Change to root directory of volume

	lda #$3C
	sta $d640
	nop

	ldx #$00
	rts
	
_chdir:
	;; char mega65_dos_chdir(char *dir_name);

	;; Get pointer to file name
	;; sp here is the ca65 sp ZP variable, not the stack pointer of a 4510
	ldy #1
	.p02
	lda (sp),y
	sta ptr1+1
	sta $0441
	dey
	lda (sp),y
	.p4510
	sta ptr1
	sta $0440
	
	;; Copy file name
	ldy #0
@NameCopyLoop:
	lda (ptr1),y
	sta $0400,y
	iny
	cmp #0
	bne @NameCopyLoop
	
	;;  Call dos_setname()
	ldy #>$0400
	ldx #<$0400
	lda #$2E     		; dos_setname Hypervisor trap
	STA $D640		; Do hypervisor trap
	NOP			; Wasted instruction slot required following hyper trap instruction
	;; XXX Check for error (carry would be clear)

	;; Find the file
	LDA #$34
	STA $D640
	NOP
	BCC @direntNotFound

	;; Try to change directory to it
	LDA #$0C
	STA $D640
	NOP

@direntNotFound:
	
	;; return inverted carry flag, so result of 0 = success
	PHP

	PLA
	AND #$01
	EOR #$01
	LDX #$00
	
	RTS

_attachd81:
	;; char mega65_dos_attachd81(char *image_name);

	;; Get pointer to file name
	;; sp here is the ca65 sp ZP variable, not the stack pointer of a 4510
	ldy #1
	.p02
	lda (sp),y
	sta ptr1+1
	sta $0441
	dey
	lda (sp),y
	.p4510
	sta ptr1
	sta $0440
	
	;; Copy file name
	ldy #0
@NameCopyLoop:
	lda (ptr1),y
	sta $0400,y
	iny
	cmp #0
	bne @NameCopyLoop
	
	;;  Call dos_setname()
	ldy #>$0400
	ldx #<$0400
	lda #$2E     		; dos_setname Hypervisor trap
	STA $D640		; Do hypervisor trap
	NOP			; Wasted instruction slot required following hyper trap instruction
	
	BCC	@return

        lda #$22
        sta $d640
        nop

	BCC	@return


	LDA	#$34
	STA	$D640
	CLV

	BCC	@return

	;; XXX Check for error (carry would be clear)

	;; Try to attach it
	LDA #$40
	STA $D640
	NOP

@return:
	;; return inverted carry flag, so result of 0 = success
	PHP
	PLA
	AND #$01
	EOR #$01
	LDX #$00

	RTS


	;; Opendir takes no arguments and returns File descriptor in A
_opendir:
	LDA #$12
	STA $D640
	NOP
	LDX #$00
	RTS

	;; readdir takes the file descriptor returned by opendir as argument
	;; and gets a pointer to a MEGA65 DOS dirent structure.
	;; Again, the annoyance of the MEGA65 Hypervisor requiring a page aligned
	;; transfer area is a nuisance here. We will use $0400-$04FF, and then
	;; copy the result into a regular C dirent structure
	;;
	;; d_ino = first cluster of file
	;; d_off = offset of directory entry in cluster
	;; d_reclen = size of the dirent on disk (32 bytes)
	;; d_type = file/directory type
	;; d_name = name of file
_readdir:

	pha
	
	;; First, clear out the dirent
	ldx #0
	txa
@l1:	sta _readdir_dirent,x	
	dex
	bne @l1

	;; Third, call the hypervisor trap
	;; File descriptor gets passed in in X.
	;; Result gets written to transfer area we setup at $0400
	plx
	ldy #>$0400 		; write dirent to $0400 
	lda #$14
	STA $D640
	NOP

	bcs @readDirSuccess

	;;  Return end of directory
	lda #$00
	ldx #$00
	RTS

@readDirSuccess:
	
	;;  Copy file name
	ldx #$3f
@l2:	lda $0400,x
	sta _readdir_dirent+4+2+4+2,x
	dex
	bpl @l2
	;; make sure it is null terminated
	ldx $0400+64
	lda #$00
	sta _readdir_dirent+4+2+4+2,x

	;; Inode = cluster from offset 64+1+12 = 77
	ldx #$03
@l3:	lda $0477,x
	sta _readdir_dirent+0,x
	dex
	bpl @l3

	;; d_off stays zero as it is not meaningful here
	
	;; d_reclen we preload with the length of the file (this saves calling stat() on the MEGA65)
	ldx #3
@l4:	lda $0400+64+1+12+4,x
	sta _readdir_dirent+4+2,x
	dex
	bpl @l4

	;; File type and attributes
	;; XXX - We should translate these to C style meanings
	lda $0400+64+1+12+1+4+4
	sta _readdir_dirent+4+2+4

	;; Return address of dirent structure
	lda #<_readdir_dirent
	ldx #>_readdir_dirent
	
	RTS

	.data

_readdir_dirent:
	.dword 0   		; d_ino
	.word 0			; d_off
	.dword 0		; d_reclen
	.word 0			; d_type
	.res 256,$00