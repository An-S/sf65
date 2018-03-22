#include "sf65.h"

extern sf65Options_t *sf65Options;

/*
** Check for opcode or directive
* c > 0 -> directive detected
* c < 0 -> opcode detected
*/
int check_opcode (char *p1, char *p2) {
    int c;
    int length;

    for (c = 0; directives_dasm[c].directive != NULL; c++) {
        length = strlen (directives_dasm[c].directive);
        if ( (*p1 == '.' &&
               length == p2 - p1 - 1 &&
               memcmpcase (p1 + 1, directives_dasm[c].directive, p2 - p1 - 1) == 0
             ) ||
             ( length == p2 - p1     &&
               memcmpcase (p1,     directives_dasm[c].directive, p2 - p1)     == 0
             )
           ) {

               return c + 1;
        }
    }

    //Maybe this would be a good point to check for mnemonics which have operands not separated by space
    for (c = 0; mnemonics_6502[c] != NULL; c++) {
        length = strlen (mnemonics_6502[c]);
        if (length == p2 - p1 && memcmpcase (p1, mnemonics_6502[c], p2 - p1) == 0)
            return - (c + 1);
    }
    return 0;
}

/* Detects mnemonic or directive and returns corresponding index, in found.
 * Returns output column by ref.
 * Takes account of the processor flag and the directive flags
 */
int detectOpcode(char *p1, char *p2, int processor, int *outputColumn, int *flags){
    int opIndex = -65000; //invalidate opIndex

// For 6502 processor check codeword against opcode list
    if (processor == 1) {
        opIndex = check_opcode (p1, p2);
        if (opIndex == 0) {
            *outputColumn = 0; //Set output column to start_mnemonic column
        } else if (opIndex < 0) {
            *outputColumn = sf65Options -> start_mnemonic;
        } else {
            *flags = directives_dasm[opIndex - 1].flags;
            if (*flags & DONT_RELOCATE_LABEL)
                *outputColumn = 0;
            else
                *outputColumn = sf65Options -> start_directive;
        }
    // For other processors just assume a mnemonic wtho checking
    } else {
        *outputColumn = sf65Options -> start_mnemonic;
        opIndex = 0;
    }

    return opIndex;
}
