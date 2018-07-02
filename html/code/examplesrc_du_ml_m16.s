; example sourcecode for sf65
; by Stefan Andree, 01.07.2018
; 
; code is provided as is. This - exacly this - example source may be used 
; for anything else

.FEATURE        labels_without_colons
.ORG            $1000-2

scrptr          = $d1
scrcol          = $d3
colptr          = $f3

clrscr          = $e544
chrout          = $ffd2
get             = $ffe4

ch_square       = 160
ch_right        = 29

magenta         = 4
cyan            = 3

                .ADDR *+2

                jsr clrscr
                jsr draw
                jsr waitkey
                jmp clrscr

;---------------------------------------

                .PROC draw
                    jsr drawsquare
                    jsr movecursor

                    lda scrptr+1
                    cmp #$07
                    bcc draw
                    lda scrptr
                    clc
                    adc scrcol
                    cmp #$e7
                    bcc draw
                    jmp drawsquare
                .ENDPROC

;---------------------------------------

                .PROC drawsquare
                    ldx #cyan

                    lda scrcol
                    cmp #20
                    bcc mkCyan
                    ldx #magenta
                mkCyan:
                    lda #ch_square
                    jmp cputc
                .ENDPROC

;---------------------------------------

                .PROC cputc
                    ldy scrcol
                    sta (scrptr),y
                    txa
                    sta (colptr),y
                    rts
                .ENDPROC

;---------------------------------------

                .PROC movecursor
                    lda #ch_right
                    jmp chrout
                .ENDPROC

;---------------------------------------

                .PROC waitkey
                    jsr get
                    beq waitkey
                    rts
                .ENDPROC

