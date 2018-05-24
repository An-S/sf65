#include "sf65.h"

#define ET(x) #x
char *sf65StrExprTypes[] = {EXPRTYPES};
#undef ET

extern sf65Options_t *CMDOptions;

void sf65_InitializeParser ( sf65ParsingData_t *ParserData ) {
    ParserData -> request = 0;
    ParserData -> prev_comment_original_location = 0;
    ParserData -> prev_comment_final_location = 0;
    ParserData -> current_level = 0;

    ParserData -> current_expr.exprType = SF65_OTHEREXPR;
}

void sf65_StartParsingNewLine ( sf65ParsingData_t *pData ) {
    sf65ErrCode_t currentErr = SF65_NOERR;

    // Reset determined expression type
    if ( pData -> current_expr.exprType != SF65_EMPTYLINE ) {
        pData -> current_expr.exprType = pData -> prev -> current_expr.exprType = SF65_OTHEREXPR;
    } else {
        pData -> current_expr.exprType = SF65_OTHEREXPR;
    }

    // Start with column at left
    sf65_ResetCurrentColumnCounter ( pData );

    // Reset parser flags for labels, spaces, linefeeds from state of previous line
    currentErr =
        sf65_ClearParserFlags (
            pData,
            SF65_LABEL_DETECTED,
            SF65_FORCE_SEPARATING_SPACE,
            //SF65_LEVEL_CHANGED,
            SF65_NOT_A_PARSERFLAG
        );
    assert ( currentErr == SF65_NOERR );

    // Indicate, that we are at
    // the beginning of a line by setting first_expression flag.
    // Do not enforce separating space after parts of expressions as a default
    currentErr =
        sf65_SetParserFlags (
            pData,
            SF65_FIRST_EXPRESSION, SF65_BEGINNING_OF_LINE,
            SF65_NOT_A_PARSERFLAG
        );

    assert ( currentErr == SF65_NOERR );
}

sf65ErrCode_t sf65_InitExpressionDetermination ( sf65ParsingData_t *pData ) {
    NOT_NULL ( pData, SF65_NULLPTR ) {
        pData -> flags = 0;
        sf65_ResetLinefeedFlag ( pData, SF65_INSTANT_ADD_LF );
        sf65_ClearParserFlag ( pData, SF65_FORCE_SEPARATING_SPACE );

        return SF65_NOERR;
    }
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
                sf65_Memcmpcase ( p1 + 1, directives_dasm[c].directive, p2 - p1 - 1 ) == 0
             ) ||
                ( length == p2 - p1     &&
                  sf65_Memcmpcase ( p1,     directives_dasm[c].directive, p2 - p1 )     == 0
                )
           ) {

            return c + 1;
        }
    }

    //Maybe this would be a good point to check for mnemonics which have operands not separated by space
    for ( c = 0; mnemonics_6502[c] != NULL; c++ ) {
        length = strlen ( mnemonics_6502[c] );
        if ( length == p2 - p1 && sf65_Memcmpcase ( p1, mnemonics_6502[c], p2 - p1 ) == 0 )
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



sf65Expression_t *sf65DetermineExpression ( char *p1, char *p2, sf65ParsingData_t *pData,
        sf65Options_t *pOpt ) {

    sf65Expression_t *expr = & ( pData->current_expr );
    int c = 0;

    expr->exprType = SF65_INVALIDEXPR;

    expr->index = 0;

    if ( p2 > p1 ) {
        expr->rightmostChar = * ( p2 - 1 );
    } else {
        expr->rightmostChar = *p1;
    }

    // Mnemonics start with a-z, directives start with . and labels start with '_' or a-z or @
    if ( *p1 == '.' || isalpha ( *p1 ) || *p1 == '_' ) {
        // p1 points to start of codeword, p2 be moved to end of word
        c = detectOpcode ( p1, p2, CMDOptions -> processor, &pData -> request, &pData -> flags );

        // Use p3 to iterate over codeword and eventually change case

        switch ( sgn ( c ) ) {
        case -1:
            expr->exprType = SF65_MNEMONIC;
            expr->index = c;
            break;
        case 1:
            expr->exprType = SF65_DIRECTIVE;
            expr->index = c;
            break;
        default:
            switch ( pData -> prev -> current_expr.exprType ) {
            case SF65_DIRECTIVE:
                expr->exprType = SF65_OPERAND;
                break;

            case SF65_MNEMONIC:
                expr->exprType = SF65_OPERAND;
                break;

            default:
                // Here, no matching mnemonic or directive was found
                if ( pData -> first_expression ) {
                    if ( *p1 == '.' ) {
                        expr->exprType = SF65_DIRECTIVE;
                    }
                    //Check for equation character
                    else if ( *p2 == '=' ) {
                        expr->exprType = SF65_IDENTIFIER;
                    }
                    //No equation character found, but may be after whitespace
                    //so check for blank characters
                    else if ( isblank ( *p2 ) ) {
                        char *p3 = sf65_SkipWhiteSpace ( p2 );
                        //Ok, found equation in the second run
                        if ( *p3 == '=' ) {
                            expr->exprType = SF65_IDENTIFIER;
                        }
                        //Did not find equation.
                        else {
                            expr->exprType = SF65_LABEL;
                        }
                    } else {
                        if ( pData -> beginning_of_line || * ( p2 - 1 ) == ':' ) {
                            expr->exprType = SF65_LABEL;
                            pData -> label_detected = 1;
                        } else {
                            expr->exprType = SF65_MACRONAME; //maybe macro name
                        }
                    }
                } else {
                    expr->exprType = SF65_IDENTIFIER;
                }
                break;
            }
        }
    } else {
        switch ( *p1 ) {
            // Comments always start with ';' and proceed until rest of line
        case ';':
            expr->exprType = SF65_COMMENT;
            break;
        case ',' :
            expr->exprType = SF65_COMMASEP;
            pData -> request = 0;
            break;
        case '\0':
        case '\n':
            if ( pData -> first_expression ) {
                expr->exprType = SF65_EMPTYLINE;
            } else {
                expr->exprType = SF65_OTHEREXPR;
            }
            break;
        case '=':
            expr->exprType = SF65_ASSIGNMENT;
            break;
        default:
            switch ( pData -> prev -> current_expr.exprType ) {
            case SF65_DIRECTIVE:
                expr->exprType = SF65_OPERAND;
                break;

            case SF65_MNEMONIC:
                expr->exprType = SF65_OPERAND;

                break;
            default:
                if ( *p1 == '\\' && *p2 == '\n' ) {
                    sf65_SetParserFlag ( pData, SF65_LINE_CONTINUATION );
                }
                expr->exprType = SF65_OTHEREXPR;
                break;
            }
        }
    }
    return expr;
}

bool isExpressionCharacter ( char ch ) {
    bool flag;

    if ( ch == '.' || ch == '_' ) {
        flag = true;
    } else {
        flag = ( ch != ':' &&
                 ch != ';' &&
                 ch != '\'' && ch != '"' &&
                 ch != '#' &&
                 ch != '$' &&
                 ch != '%' &&
                 ch != ',' &&
                 ch != '\\' &&
                 ch != '=' );
    }
    return flag;
}

char *sf65_DetectCodeWord ( char *p ) {
    char ch;
    //p += strcspn ( p, ";'\"#$%,\\= \t\v\0" );
    while ( ch = *p, ch && !isspace ( ch ) && isExpressionCharacter ( ch ) ) {
        //  echoChar ( ch );
        ++p;
    }

    //Rewind pointer to last non delimiting char
    //--p;
    return p;
}

char *detectOperand ( char *p ) {
    //
    char ch;

    while ( ch = *p, ch && !isspace ( ch ) && !isExpressionCharacter ( ch ) ) {
        echoChar ( ch );
        ++p;
    }

    //Rewind pointer to last non delimiting char
    //--p;
    return p;
}
