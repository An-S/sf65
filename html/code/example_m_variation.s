; example sourcecode fo|; example sourcecode fo|; example sourcecode fo|; example sourcecode fo|; example sourcecode fo
; by Stefan Andree, 01.|; by Stefan Andree, 01.|; by Stefan Andree, 01.|; by Stefan Andree, 01.|; by Stefan Andree, 01.
;		       |;		       |;		       |;		       |;
; code is provided as i|; code is provided as i|; code is provided as i|; code is provided as i|; code is provided as i
; for anything else    |; for anything else    |; for anything else    |; for anything else    |; for anything else
		       |		       |		       |		       |
.FEATURE	labels_|.FEATURE	labels_|.FEATURE	labels_|.FEATURE	labels_|.FEATURE	labels_
.ORG		$1000-2|.ORG		$1000-2|.ORG		$1000-2|.ORG		$1000-2|.ORG		$1000-2
		       |		       |		       |		       |
scrptr	= $d1	       |scrptr	  = $d1	       |scrptr	    = $d1      |scrptr	      = $d1    |scrptr		= $d1
scrcol	= $d3	       |scrcol	  = $d3	       |scrcol	    = $d3      |scrcol	      = $d3    |scrcol		= $d3
colptr	= $f3	       |colptr	  = $f3	       |colptr	    = $f3      |colptr	      = $f3    |colptr		= $f3
		       |		       |		       |		       |
clrscr	= $e544	       |clrscr	  = $e544      |clrscr	    = $e544    |clrscr	      = $e544  |clrscr		= $e544
chrout	= $ffd2	       |chrout	  = $ffd2      |chrout	    = $ffd2    |chrout	      = $ffd2  |chrout		= $ffd2
get	= $ffe4	       |get	  = $ffe4      |get	    = $ffe4    |get	      = $ffe4  |get		= $ffe4
		       |		       |		       |		       |
ch_square = 160	       |ch_square = 160	       |ch_square   = 160      |ch_square     = 160    |ch_square	= 160
ch_right = 29	       |ch_right  = 29	       |ch_right    = 29       |ch_right      = 29     |ch_right	= 29
		       |		       |		       |		       |
magenta = 4	       |magenta	  = 4	       |magenta	    = 4	       |magenta	      = 4      |magenta		= 4
cyan	= 3	       |cyan	  = 3	       |cyan	    = 3	       |cyan	      = 3      |cyan		= 3
		       |		       |		       |		       |
	.ADDR	*+2    |	  .ADDR *+2    |	    .ADDR *+2  |	      .ADDR *+2|		.ADDR *
		       |		       |		       |		       |
	jsr	clrscr |	  jsr	clrscr |	    jsr clrscr |	      jsr clrsc|		jsr clr
	jsr	draw   |	  jsr	draw   |	    jsr draw   |	      jsr draw |		jsr dra
	jsr	waitkey|	  jsr	waitkey|	    jsr waitkey|	      jsr waitk|		jsr wai
	jmp	clrscr |	  jmp	clrscr |	    jmp clrscr |	      jmp clrsc|		jmp clr
		       |		       |		       |		       |
	.PROC	draw   |	  .PROC draw   |	    .PROC draw |	      .PROC dra|		.PROC d
	    jsr	    dra|	      jsr   dra|		jsr dra|		  jsr d|		    jsr
	    jsr	    mov|	      jsr   mov|		jsr mov|		  jsr m|		    jsr
		       |		       |		       |		       |
	    lda	    scr|	      lda   scr|		lda scr|		  lda s|		    lda
	    cmp	    #$0|	      cmp   #$0|		cmp #$0|		  cmp #|		    cmp
	    bcc	    dra|	      bcc   dra|		bcc dra|		  bcc d|		    bcc
	    lda	    scr|	      lda   scr|		lda scr|		  lda s|		    lda
	    clc	       |	      clc      |		clc    |		  clc  |		    clc
	    adc	    scr|	      adc   scr|		adc scr|		  adc s|		    adc
	    cmp	    #$e|	      cmp   #$e|		cmp #$e|		  cmp #|		    cmp
	    bcc	    dra|	      bcc   dra|		bcc dra|		  bcc d|		    bcc
	    jmp	    dra|	      jmp   dra|		jmp dra|		  jmp d|		    jmp
	.ENDPROC       |	  .ENDPROC     |	    .ENDPROC   |	      .ENDPROC |		.ENDPRO
		       |		       |		       |		       |
	.PROC	drawsqu|	  .PROC drawsqu|	    .PROC draws|	      .PROC dra|		.PROC d
	    ldx	    #cy|	      ldx   #cy|		ldx #cy|		  ldx #|		    ldx
		       |		       |		       |		       |
	    lda	    scr|	      lda   scr|		lda scr|		  lda s|		    lda
	    cmp	    #20|	      cmp   #20|		cmp #20|		  cmp #|		    cmp
	    bcc	    mkC|	      bcc   mkC|		bcc mkC|		  bcc m|		    bcc
	    ldx	    #ma|	      ldx   #ma|		ldx #ma|		  ldx #|		    ldx
	mkCyan:	       |	  mkCyan:      |	    mkCyan:    |	      mkCyan:  |		mkCyan:
	    lda	    #ch|	      lda   #ch|		lda #ch|		  lda #|		    lda
	    jmp	    cpu|	      jmp   cpu|		jmp cpu|		  jmp c|		    jmp
	.ENDPROC       |	  .ENDPROC     |	    .ENDPROC   |	      .ENDPROC |		.ENDPRO
		       |		       |		       |		       |
	.PROC	cputc  |	  .PROC cputc  |	    .PROC cputc|	      .PROC cpu|		.PROC c
	    ldy	    scr|	      ldy   scr|		ldy scr|		  ldy s|		    ldy
	    sta	    (sc|	      sta   (sc|		sta (sc|		  sta (|		    sta
	    txa	       |	      txa      |		txa    |		  txa  |		    txa
	    sta	    (co|	      sta   (co|		sta (co|		  sta (|		    sta
	    rts	       |	      rts      |		rts    |		  rts  |		    rts
	.ENDPROC       |	  .ENDPROC     |	    .ENDPROC   |	      .ENDPROC |		.ENDPRO
		       |		       |		       |		       |
	.PROC	movecur|	  .PROC movecur|	    .PROC movec|	      .PROC mov|		.PROC m
	    lda	    #ch|	      lda   #ch|		lda #ch|		  lda #|		    lda
	    jmp	    chr|	      jmp   chr|		jmp chr|		  jmp c|		    jmp
	.ENDPROC       |	  .ENDPROC     |	    .ENDPROC   |	      .ENDPROC |		.ENDPRO
		       |		       |		       |		       |
	.PROC	waitkey|	  .PROC waitkey|	    .PROC waitk|	      .PROC wai|		.PROC w
	    jsr	    get|	      jsr   get|		jsr get|		  jsr g|		    jsr
	    beq	    wai|	      beq   wai|		beq wai|		  beq w|		    beq
	    rts	       |	      rts      |		rts    |		  rts  |		    rts
	.ENDPROC       |	  .ENDPROC     |	    .ENDPROC   |	      .ENDPROC |		.ENDPRO
		       |		       |		       |		       |
