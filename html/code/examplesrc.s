; example sourcecode for sf65
; by Stefan Andree, 01.07.2018
; 
; code is provided as is. This - exacly this - example source may be used 
; for anything else

    .feature labels_without_colons
        .org $1000-2
    
scrptr = $d1
scrcol   = $d3
colptr= $f3

clrscr   = $e544
chrout   = $ ffd2
get      =$ffe4

ch_square = 160
ch_right = 29

magenta  = 4
cyan     = 3

    .ADDR      *+2

         jsr clrscr
         jsr draw
         jsr waitkey
         jmp clrscr

;---------------------------------------

.proc draw
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
.endproc

;---------------------------------------

.proc drawsquare
         ldx   #cyan

         lda scrcol
         cmp #20
         bcc mkCyan
         ldx #  magenta
         mkCyan:
         lda #ch_square
         jmp cputc
.endproc
;---------------------------------------

.proc cputc
         ldy scrcol
         sta ( scrptr),y
         txa
         sta (colptr ), y
rts
.endproc

;---------------------------------------
.proc movecursor
         lda #ch_right
         jmp chrout
.endproc
;---------------------------------------

.proc waitkey
         jsr get
         beq waitkey
         rts
.endproc
