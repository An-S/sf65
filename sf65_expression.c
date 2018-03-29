#include "sf65.h"

extern sf65Options_t *CMDOptions;

void initializeParser ( sf65ParsingData_t *ParserData ) {
    ParserData -> request = 0;
    ParserData -> prev_comment_original_location = 0;
    ParserData -> prev_comment_final_location = 0;
    ParserData -> current_level = 0;
}

/*
** Check for opcode or directive
* c > 0 -> directive detected
* c < 0 -> opcode detected
*/
int check_opcode ( char *p1, char *p2 ) {
    int c;
    int length;

    for ( c = 0; directives_dasm[c].directive != NULL; c++ ) {
        length = strlen ( directives_dasm[c].directive );
        if ( ( *p1 == '.' &&
                length == p2 - p1 - 1 &&
                memcmpcase ( p1 + 1, directives_dasm[c].directive, p2 - p1 - 1 ) == 0
             ) ||
                ( length == p2 - p1     &&
                  memcmpcase ( p1,     directives_dasm[c].directive, p2 - p1 )     == 0
                )
           ) {

            return c + 1;
        }
    }

    //Maybe this would be a good point to check for mnemonics which have operands not separated by space
    for ( c = 0; mnemonics_6502[c] != NULL; c++ ) {
        length = strlen ( mnemonics_6502[c] );
        if ( length == p2 - p1 && memcmpcase ( p1, mnemonics_6502[c], p2 - p1 ) == 0 )
            return - ( c + 1 );
    }
    return 0;
}

/* Detects mnemonic or directive and returns corresponding index, in found.
 * Returns output column by ref.
 * Takes account of the processor flag and the directive flags
 */
int detectOpcode ( char *p1, char *p2, int processor, int *outputColumn, int *flags ) {
    int opIndex = -65000; //invalidate opIndex

    // For 6502 processor check codeword against opcode list
    if ( processor == 1 ) {
        opIndex = check_opcode ( p1, p2 );
        if ( opIndex == 0 ) {
            *outputColumn = 0;  // Did not find mnemonic nor directive. -> Request output at line start
        } else if ( opIndex < 0 ) {
            *outputColumn = CMDOptions -> start_mnemonic;
            *flags = ALIGN_MNEMONIC;
        } else {
            *flags = directives_dasm[opIndex - 1].flags;
            if ( *flags & DONT_RELOCATE )
                *outputColumn = 0;
            else
                *outputColumn = CMDOptions -> start_directive;
        }
        // For other processors just assume a mnemonic wtho checking
    } else {
        *outputColumn = CMDOptions -> start_mnemonic;
        opIndex = 0;
        *flags = ALIGN_MNEMONIC;
    }

    return opIndex;
}


void sf65_correctOutputColumnForFlags ( sf65ParsingData_t *ParserData, const sf65Options_t *CMDOptions ) {
    if ( ParserData -> current_column != 0 && CMDOptions -> labels_own_line != 0 && ( ParserData -> flags & DONT_RELOCATE ) == 0 ) {
        fputc ( '\n', output );
        ParserData -> current_column = 0;
    }

    if ( ParserData -> flags & LEVEL_IN ) {
        ParserData -> current_level++;
        ParserData -> request = CMDOptions -> start_mnemonic - 4;
    }

    if ( ParserData -> flags & LEVEL_OUT ) {
        if ( ParserData -> current_level > 0 )
            ParserData -> current_level--;
        ParserData -> request = CMDOptions -> start_mnemonic;
    }

    if ( ParserData -> flags & ALIGN_MNEMONIC ) {
        ParserData -> request = CMDOptions -> start_mnemonic;
    }

    // Unindent by one level
    if ( ParserData -> flags & LEVEL_MINUS )
        if ( ParserData -> request > CMDOptions -> nesting_space )
            ParserData -> request -= CMDOptions -> nesting_space;
}

/*
 * This function sets correct case for mnemonic and sets requested x position to start_mnemonic
 * It also indicates that a mnemonic was found and clears the directive found flag
 */
void sf65_PlaceMnemonicInLine ( char *p1, char *p2, sf65Options_t *CMDOptions,
                                sf65ParsingData_t *ParserData ) {
    changeCase ( p1, p2, CMDOptions -> mnemonics_case );
    ParserData -> request = CMDOptions -> start_mnemonic;
    ParserData -> directive_detected = 0;
    ParserData -> mnemonic_detected = 1;
}

/*
 * This function sets correct case for directive and sets requested x position to start_directive
 * It also indicates that a directive was found and clears the mnemonic found flag
 */
void sf65_PlaceDirectiveInLine ( char *p1, char *p2, sf65Options_t *CMDOptions,
                                 sf65ParsingData_t *ParserData ) {
    changeCase ( p1, p2, CMDOptions -> directives_case );
    ParserData -> request = CMDOptions -> start_directive;
    ParserData -> directive_detected = 1;
    ParserData -> mnemonic_detected = 0;
}

/*
 * This function sets correct case for directive and sets requested x position to start_directive
 * It also indicates that a directive was found and clears the mnemonic found flag
 */
void sf65_PlaceOperandInLine ( char *p1, char *p2, sf65Options_t *CMDOptions,
                               sf65ParsingData_t *ParserData ) {
    if ( CMDOptions -> style != 0 ) {
        ParserData -> request = 0;
    } else {
        ParserData -> request = CMDOptions -> start_operand;
    }
    ParserData -> mnemonic_detected = 0;
}

sf65Expression_t sf65DetermineExpression ( char *p1, char *p2, sf65ParsingData_t *pData, sf65Options_t *pOpt ) {
    sf65Expression_t expr;
    int c = 0;

    expr.exprType = SF65_INVALIDEXPR;

    expr.index = 0;

    // Comments always start with ';' and proceed until rest of line
    if ( *p1 == ';' ) {
        expr.exprType = SF65_COMMENT;
        return expr;
    }

    // Mnemonics start with a-z, directives start with . and labels start with '_' or a-z or @

    /*
        if (*p1 == '_'){
            // Probably label, because mnemonics nor directives start with '_'
            // Now check previous expression type

            switch ( pData -> prev_expr.exprType ){
                case SF65_DIRECTIVE:
                    expr.exprType = SF65_OPERAND;
                    pData -> operand_detected = 1;
                    pData -> directive_detected = 0;
                    break;

                case SF65_MNEMONIC:
                    expr.exprType = SF65_OPERAND;
                    pData -> operand_detected = 1;
                    pData -> mnemonic_detected = 0;
                    break;

                case SF65_LABEL:
                    expr.exprType = SF65_OTHEREXPR;
                    break;

                default:
                    if ( pData -> label_detected || !pData -> first_expression ){
                        // Can't be label, because only one label per line allowed
                        expr.exprType = SF65_OTHEREXPR;
                    }else{
                        expr.exprType = SF65_LABEL;
                        pData -> label_detected = 1;
                    }
            }
        }else{*/
    if ( *p1 == '.' || isalpha ( *p1 ) || *p1 == '_' ) {
        // p1 points to start of codeword, p2 be moved to end of word
        c = detectOpcode ( p1, p2, CMDOptions -> processor, &pData -> request, &pData -> flags );

        // Use p3 to iterate over codeword and eventually change case

        switch ( sgn ( c ) ) {
        case -1:
            expr.exprType = SF65_MNEMONIC;
            expr.index = c;
            pData -> mnemonic_detected = 1;
            break;
        case 1:
            expr.exprType = SF65_DIRECTIVE;
            expr.index = c;
            pData -> directive_detected = 1;
            break;
        default:
            switch ( pData -> prev_expr.exprType ) {
            case SF65_DIRECTIVE:
                expr.exprType = SF65_OPERAND;
                pData -> operand_detected = 1;
                pData -> directive_detected = 0;
                break;

            case SF65_MNEMONIC:
                expr.exprType = SF65_OPERAND;
                pData -> operand_detected = 1;
                pData -> mnemonic_detected = 0;
                break;

            case SF65_LABEL:
                expr.exprType = SF65_OTHEREXPR;
                break;

            default:
                if ( !pData -> first_expression ) {
                    expr.exprType = SF65_OTHEREXPR;
                } else {
                    expr.exprType = SF65_LABEL;
                    pData -> label_detected = 1;
                }
                break;
            }
        }
    } else {
        switch ( pData -> prev_expr.exprType ) {
        case SF65_DIRECTIVE:
            expr.exprType = SF65_OPERAND;
            pData -> operand_detected = 1;
            pData -> directive_detected = 0;
            break;

        case SF65_MNEMONIC:
            expr.exprType = SF65_OPERAND;
            pData -> operand_detected = 1;
            pData -> mnemonic_detected = 0;

            break;
        default:
            if ( *p1 == '\\' && *p2 == '\n' ) {
                pData -> line_continuation = 1;
            }
            expr.exprType = SF65_OTHEREXPR;
            break;
        }
    }
    //}
    return expr;
}
