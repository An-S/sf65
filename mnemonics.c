#include "sf65.h"

/*
** 6502 mnemonics
*/
char *mnemonics_6502[] = {
    "adc", "anc", "and", "ane", "arr", "asl", "asr", "axs", "bcc",
    "bcs", "beq", "bit", "bmi", "bne", "bpl", "brk", "bvc",
    "bvs", "clc", "cld", "cli", "clv", "cmp", "cpx", "cpy",
    "dcp", "dec", "dex", "dey", "eor", "inc", "inx", "iny",
    "isb", "jmp", "jsr", "las", "lax", "lda", "ldx", "ldy",
    "lsr", "lxa", "nop", "ora", "pha", "php", "pla", "plp",
    "rla", "rol", "ror", "rra", "rti", "rts", "sax", "sbc",
    "sbx", "sec", "sed", "sei", "sha", "shs", "shx", "shy",
    "slo", "sre", "sta", "stx", "sty", "tax", "tay", "tsx",
    "txa", "txs", "tya", NULL,
};
