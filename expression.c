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
                (length == p2 - p1     &&
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
int detectOpcode (char *p1, char *p2, int processor, int *outputColumn, int *flags) {
    int opIndex = -65000; //invalidate opIndex

    // For 6502 processor check codeword against opcode list
    if (processor == 1) {
        opIndex = check_opcode (p1, p2);
        if (opIndex == 0) {
            *outputColumn = 0;  // Did not find mnemonic nor directive. -> Request output at line start
        } else if (opIndex < 0) {
            *outputColumn = sf65Options -> start_mnemonic;
            *flags = ALIGN_MNEMONIC;
        } else {
            *flags = directives_dasm[opIndex - 1].flags;
            if (*flags & DONT_RELOCATE)
                *outputColumn = 0;
            else
                *outputColumn = sf65Options -> start_directive;
        }
        // For other processors just assume a mnemonic wtho checking
    } else {
        *outputColumn = sf65Options -> start_mnemonic;
        opIndex = 0;
        *flags = ALIGN_MNEMONIC;
    }

    return opIndex;
}


void sf65_correctOutputColumnForFlags(sf65ParsingData_t *sf65ParsingData, const sf65Options_t *sf65Options){
    if (sf65ParsingData -> current_column != 0 && sf65Options -> labels_own_line != 0 && (sf65ParsingData -> flags & DONT_RELOCATE) == 0) {
        fputc ('\n', output);
        sf65ParsingData -> current_column = 0;
    }

    if (sf65ParsingData -> flags & LEVEL_IN) {
        sf65ParsingData -> current_level++;
        sf65ParsingData -> request = sf65Options -> start_mnemonic - 4;
    }

    if (sf65ParsingData -> flags & LEVEL_OUT) {
        if (sf65ParsingData -> current_level > 0)
            sf65ParsingData -> current_level--;
        sf65ParsingData -> request = sf65Options -> start_mnemonic;
    }

    if (sf65ParsingData -> flags & ALIGN_MNEMONIC) {
        sf65ParsingData -> request = sf65Options -> start_mnemonic;
    }
    
    // Unindent by one level
    if (sf65ParsingData -> flags & LEVEL_MINUS)
        if (sf65ParsingData -> request > sf65Options -> nesting_space) 
            sf65ParsingData -> request -= sf65Options -> nesting_space;
}

/*
 * This function sets correct case for mnemonic and sets requested x position to start_mnemonic
 * It also indicates that a mnemonic was found and clears the directive found flag
 */
void sf65_PlaceMnemonicInLine(char *p1, char *p2, sf65Options_t *sf65Options, 
                              sf65ParsingData_t *sf65ParsingData){
    changeCase (p1, p2, sf65Options -> mnemonics_case);
    sf65ParsingData -> request = sf65Options -> start_mnemonic;
    sf65ParsingData -> directive_detected = 0;
    sf65ParsingData -> mnemonic_detected = 1;
}

/*
 * This function sets correct case for directive and sets requested x position to start_directive
 * It also indicates that a directive was found and clears the mnemonic found flag
 */
void sf65_PlaceDirectiveInLine(char *p1, char *p2, sf65Options_t *sf65Options, 
                              sf65ParsingData_t *sf65ParsingData){
    changeCase (p1, p2, sf65Options -> directives_case);
    sf65ParsingData -> request = sf65Options -> start_directive;
    sf65ParsingData -> directive_detected = 1;
    sf65ParsingData -> mnemonic_detected = 0;
}

/*
 * This function sets correct case for directive and sets requested x position to start_directive
 * It also indicates that a directive was found and clears the mnemonic found flag
 */
void sf65_PlaceOperandInLine(char *p1, char *p2, sf65Options_t *sf65Options, 
                              sf65ParsingData_t *sf65ParsingData){
    if ( sf65Options -> style != 0){
        sf65ParsingData -> request = 0;
    }else{
        sf65ParsingData -> request = sf65Options -> start_operand;
    }
    sf65ParsingData -> mnemonic_detected = 0;
}

sf65Expression_t sf65DetermineExpression(char *p1, char *p2, sf65ParsingData_t *pData, sf65Options_t *pOpt){
    sf65Expression_t expr;
    int c = 0;
    
    expr.exprType = SF65_INVALIDEXPR;
    
    expr.index = 0;
    
    // Comments always start with ';' and proceed until rest of line
    if (*p1 == ';') {
        expr.exprType = SF65_COMMENT;
        return expr;
    }
    
    // Mnemonics start with a-z, directives start with . and labels start with '_' or a-z
    if (*p1 == '_' || *p1 == '.' || isalnum (*p1)) {
        // p1 points to start of codeword, p2 be moved to end of word
        c = detectOpcode (p1, p2, sf65Options -> processor, &pData -> request, &pData -> flags);

        // Use p3 to iterate over codeword and eventually change case
        
        switch (sgn(c)){
        case -1:
            expr.exprType = SF65_MNEMONIC;
            expr.index = c;
            break;
        case 1:
            expr.exprType = SF65_DIRECTIVE;
            expr.index = c;
            break;
        default:
            expr.exprType = SF65_OTHEREXPR;
            break;
        }
    }
    return expr;
}
