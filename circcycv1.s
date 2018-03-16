
.INCLUDE "circcyc.s.h"
;.INCLUDE ""

.EXPORT _cycler_coltab = CYCLER::COLTAB1
.EXPORT _cycler_coltab2 = CYCLER::COLTAB2

.CODE


    jmp main

.PROC irqcode

.PROC start_effect
    jsr irqlib::irqenter


    dbg_incborder
    jsr CYCLER::cycleframe
    pha

    ;jsr pusha
    ;lda circcyc_scrnhi,x
    ;jsr _sprites_initShape_CascadeLogo
    lda #$3b
    sta VIC_CTRL1
    lda #$c0
    jsr CYCLER::getVICBankBits
    sta CIA2_PRA
    jsr CYCLER::getVICScrnBits
    sta VIC_ADDR

    dbg_incborder
.IF 0
    lda #0
    jsr basic_move
    clc
    adc #100
    sta xbase
    lda #0
    jsr basic_move

    ldx #0
    jsr sprmove_pollNextXYPair
    clc
xbase = *+1
    adc #0
    pha
    txa
    lsr
    clc
    adc #40
    tax
    pla
.ENDIF
    ldx #0
    jsr superposeOszillations
    ;lda #100
    ;ldx #70
    jsr sprites_showBlockAt
    pla
    jsr setsprdef

    dbg_incborder
	lda #scrolly_DesiredVICCtrl2Bits
	jsr scrolly_SoftShift
    bcc noHardScroll
    dbg_incborder
	jsr scrolly_Copy
    dbg_decborder
noHardScroll:

    jsr scrolly_ColorFlash
    dbg_decborder
    dbg_decborder
    dbg_decborder
    jsr chainIrq
    jmp irqlib::irqexit

.ENDPROC


.PROC scrollersplit1
    jsr irqlib::irqenter

    lda #$3f
    sta VIC_CTRL1
    lda #scrolly_DesiredVICCtrl2Bits
    jsr scrolly_getXShift
    sta VIC_CTRL2
    lda #$c4
    jsr scrolly_getVICAddrBits
    sta VIC_ADDR
    stx CIA2_PRA

    jsr chainIrq
    cli
    dbg_decborder
    ldx #1
    jsr superposeOszillations
.IF 0
    jsr sprmove_pollNextXYPair
    clc
    adc #100
    pha
    txa
    lsr
    clc
    adc #156
    tax
    pla

    ;lda #150
    ;ldx #180
.ENDIF
    jsr sprites_showBlockAt
    jsr setsprdef

    dbg_incborder
    jmp irqlib::irqexit
.ENDPROC

.PROC scrollersplit2
    jsr irqlib::irqenter

    lda #$3b
    sta VIC_CTRL1
    lda #8
    sta VIC_CTRL2

    lda #$c4
    jsr CYCLER::getVICBankBits
    sta CIA2_PRA
    jsr CYCLER::getVICScrnBits
    sta VIC_ADDR

    jsr chainIrq

    jmp irqlib::irqexit
.ENDPROC

.PROC chainIrq
    ldy irqpos
    lda rasterlineloTbl,y
    sta VIC_HLINE

    lda #$7f
    and VIC_CTRL1
    ldx rasterlinehiTbl,y
    beq leaveRasterlineHiBit0

    ora #$80
leaveRasterlineHiBit0:
    sta VIC_CTRL1

    lda irqadrhiTbl,y
    sta IRQRamVec+1
    lda irqadrloTbl,y
    sta IRQRamVec

    iny
    cpy #rasterlineloTbl_end-rasterlineloTbl
    bne :+
    ldy #0
:   sty irqpos

    rts
irqpos: .BYTE 0

.define irq_rasterlines \
    circcyc_irqline_effect, circcyc_irqline_split1, circcyc_irqline_split2

.define irq_procs \
    start_effect, scrollersplit1, scrollersplit2

rasterlineloTbl:
    .lobytes irq_rasterlines
rasterlineloTbl_end:
rasterlinehiTbl:
    .hibytes irq_rasterlines
irqadrloTbl:
    .lobytes irq_procs
irqadrhiTbl:
    .hibytes irq_procs

.ENDPROC
.ENDPROC

.PROC main
.EXPORT _ccyc_start = *
	sei
	lda #$35
    sta CPUPORT
	jsr scrninit
    jsr scrolly_Init
    jsr irq_init
    jsr sprites_initParameters
    lda #1
    jsr basic_move

    lda #$ff
    sta VIC_SPR_ENA

 	cli
	:
	jmp :-
.ENDPROC
;-------------------------------------------------------------------------------------------
.PROC setsprdef
    lda CPUPORT
    pha
    lda #$34
    sta CPUPORT

    ldy frameno
    lda circcyc_scrnhi,y
    sta scrnhi
    lda #7
    tax
    clc
    adc sprdeftable,y
    tay
setSprDef:
    tya
scrnhi = *+2
    sta CYCLER::SCRN1+$3f8,x
    dey
    dex
    bpl setSprDef

    ldy frameno
    iny
    cpy #8
    bne :+
    ldy #0
:    sty frameno
    pla
    sta CPUPORT
    rts
circcyc_scrnhi
    .BYTE  >CYCLER::SCRN1+3, >CYCLER::SCRN1+3, >CYCLER::SCRN2+3,>CYCLER::SCRN2+3
    .BYTE  >CYCLER::SCRN3+3, >CYCLER::SCRN3+3, >CYCLER::SCRN4+3,>CYCLER::SCRN4+3

    ;cycleframe returns screen no+1
sprdeftable:
    .REPEAT 4, SPRNUM
        .BYTE <( .IDENT(.SPRINTF("circcyc_sprites%d",SPRNUM+1)) / $40 )
        .BYTE <( .IDENT(.SPRINTF("circcyc_sprites%d",SPRNUM+5)) / $40 )
    .ENDREP
frameno: .BYTE 0
.ENDPROC

;-------------------------------------------------------------------------------------------
scrninit:
	lda #$3b
	sta VIC_CTRL1
	lda #$08
	sta VIC_CTRL2
	lda #<CYCLER::D018W1
	sta $d018

    jsr CYCLER::cycler_init

	rts
;-------------------------------------------------------------------------------------------

.PROC irq_init
    jsr irqlib::initirqregs
    lda #$3b
    sta $d011
    jmp irqcode::chainIrq
.ENDPROC
;-------------------------------------------------------------------------------------------
