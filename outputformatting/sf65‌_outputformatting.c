#include "sf65.h"

int sf65_SetOutputXPositionInLine ( sf65ParsingData_t *pData, int xpos ) {
    NOT_NULL ( pData ) {
        pData->request = xpos;
        return xpos;
    }
    return -1;
}

int sf65_GetOutputXPositionInLine ( sf65ParsingData_t *pData ) {
    NOT_NULL ( pData ) {
        return pData->request;
    }
    return -1;
}

int sf65_IncOutputXPositionInLine ( sf65ParsingData_t *pData, int add ) {
    NOT_NULL ( pData ) {

        pData->request += add;
        return pData->request;
    }
    return -1;
}

int sf65_IncOutputXPositionByNestingLevel ( sf65ParsingData_t * pData, int nestingSpace ) {
    NOT_NULL ( pData ) {

        pData->request +=
            pData -> current_level *
            nestingSpace;
        return pData->request;
    }
    return -1;
}

int sf65_IncCurrentColumnCounter ( sf65ParsingData_t *pData, int inc ) {
    NOT_NULL ( pData ) {
        return pData -> current_column += inc;
    }
    return -1;
}

int sf65_GetCurrentColumnCounter ( sf65ParsingData_t *pData ) {
    NOT_NULL ( pData ) {
        return pData -> current_column;
    }
    return -1;
}

int sf65_ResetCurrentColumnCounter ( sf65ParsingData_t *pData ) {
    NOT_NULL ( pData ) {
        return pData -> current_column = 0;
    }
    return -1;
}

int sf65_SetCurrentColumnCounter ( sf65ParsingData_t *pData, int col ) {
    NOT_NULL ( pData ) {
        return pData -> current_column = col;
    }
    return -1;
}

int sf65_AlignCurrentColumn ( sf65ParsingData_t *pData, int tabs ) {
    int currentColumn = sf65_GetCurrentColumnCounter ( pData );
    return sf65_SetCurrentColumnCounter ( pData, ( currentColumn + tabs ) / tabs * tabs );
}

sf65Err_t sf65_SetParserFlag ( sf65ParsingData_t *pData, sf65ParserFlagsEnum_t flag  ) {
    NOT_NULL ( pData ) {
        switch ( flag ) {
#   define PF(x,y) case SF65_##y: pData -> x=1; break;
            SF65_PARSERFLAGS
#   undef PF
        default:
            return SF65_INVALIDARG;
        }
        return SF65_NOERR;
    }
    return SF65_NULLPTR;
}

int sf65_GetParserFlag ( sf65ParsingData_t *pData, sf65ParserFlagsEnum_t flag ) {
    NOT_NULL ( pData ) {
        switch ( flag ) {
#   define PF(x,y) case SF65_##y: return pData -> x;
            SF65_PARSERFLAGS
#   undef PF
        default:
            break;
        }
    }
    return -1;
}

sf65Err_t sf65_SetParserFlags ( sf65ParsingData_t *pData, sf65ParserFlagsEnum_t flag1, ... ) {
    NOT_NULL ( pData ) {
        va_list va;
        va_start ( va, flag1 );

        while ( flag1 != SF65_NOT_A_PARSERFLAG ) {
            sf65_SetParserFlag ( pData, flag1 );
            flag1 = va_arg ( va, sf65ParserFlagsEnum_t );
        }
        va_end ( va );
        return SF65_NOERR;
    }
    return SF65_NULLPTR;
}

sf65Err_t sf65_ClearParserFlag ( sf65ParsingData_t *pData, sf65ParserFlagsEnum_t flag ) {
    NOT_NULL ( pData ) {
        switch ( flag ) {
#       define PF(x,y) case SF65_##y: pData -> x=0;
            SF65_PARSERFLAGS
#       undef PF
        default:
            return SF65_NOT_A_PARSERFLAG;
        }
        return SF65_NOERR;
    }
    return SF65_NULLPTR;
}

sf65Err_t sf65_ClearParserFlags ( sf65ParsingData_t *pData, sf65ParserFlagsEnum_t flag1, ... ) {
    NOT_NULL ( pData ) {
        va_list va;
        va_start ( va, flag1 );

        while ( flag1 != SF65_NOT_A_PARSERFLAG ) {
            sf65_ClearParserFlag ( pData, flag1 );
            flag1 = va_arg ( va, sf65ParserFlagsEnum_t );
        }
        va_end ( va );
        return SF65_NOERR;
    }
    return SF65_NULLPTR;
}

sf65Err_t sf65_ResetParserFlags ( sf65ParsingData_t * pData ) {
    NOT_NULL ( pData ) {
        pData -> allParserFlags = 0;

        return SF65_NOERR;
    }
    return SF65_NULLPTR;
}

/*
** Request space in line
*  Return number of spaces actually written
*/
int request_space ( FILE * output, int * current, int new, int force, int tabs ) {

    /*
    ** If already exceeded space...
    */
    if ( *current > new ) {

        // If force is true, a single space is always written to output
        if ( force ) {
            fputc ( ' ', output );
            ( *current ) ++;

            return 1;
        }
        return 0;
    }

    // From here on, *current < new

    /*
    ** Advance one step at a time
    */
    if ( new ) {
        // Write spaces to output, only if new != 0
        // Assume, new is non negative

        while ( *current < new ) {
            if ( tabs == 0 ) {
                // Use spaces instead of tabs

                int i = 0;

                // Write number of spaces calculated from the
                // difference between *current and new
                for ( ; i < new - *current; ++i ) {
                    fputc ( ' ', output );
                }

                // Update the current_column variable to the new x position
                *current = new;
                return i;
            } else {
                // Use tabs, not spaces. Assume tab has a width of <<tabs>>
                fputc ( '\t', output );

                // Quantize current output column to value of <<tabs>>
                *current = ( *current + tabs ) / tabs * tabs;
            }
        }

        return 0;
    }

    return 0;
}

int sf65_PadOutputWithSpaces ( FILE * output, sf65ParsingData_t *pData, int tabs ) {
    int new = sf65_GetOutputXPositionInLine ( pData );

    /*
    ** If already exceeded space...
    */
    if ( sf65_GetCurrentColumnCounter ( pData ) > new ) {
        // If force is true, a single space is always written to output
        if ( sf65_GetParserFlag ( pData, SF65_FORCE_SEPARATING_SPACE ) ) {
            sf65_fputspc ( output );
            sf65_IncCurrentColumnCounter ( pData, 1 ) ;
            return 1;
        }
        return 0;
    } else {
        // From here on, *current < new

        /*
        ** Advance one step at a time
        */
        if ( new ) {
            // Write spaces to output, only if new != 0
            // Assume, new is non negative

            if ( tabs == 0 ) {
                // Calculate number of spaces to output
                int n = new - sf65_GetCurrentColumnCounter ( pData );

                // Use spaces instead of tabs
                // Write number of spaces calculated from the
                // difference between *current and new
                sf65_fputnspc ( output, n );

                // Update the current_column variable to the new x position
                sf65_IncCurrentColumnCounter ( pData, n );

                return n;
            } else {
                while ( sf65_GetCurrentColumnCounter ( pData ) < new ) {
                    // Use tabs, not spaces. Assume tab has a width of <<tabs>>
                    sf65_fputc ( '\t', output );

                    // Quantize current output column to value of <<tabs>>
                    // int sf65_align ( int val, int align )
                    sf65_AlignCurrentColumn ( pData, tabs );
                }
            }
        }

        return 0;
    }

    return 0;
}

int getCommentSpacing ( char * p /*linestart*/, char * p1 /*commentstart*/, sf65ParsingData_t * pData ) {
    /*
    ** Try to keep comments horizontally aligned (only works
    ** if spaces were used in source file)
    */
    //p2 = p1;
    //while (p2 - 1 >= p && isspace (* (p2 - 1)))
    //    p2--;
    int request = 0;


    // If original comment x position in unformatted src is the same as that of the
    // previous comment, then request the same final location as of the previous comment
    if ( p1 - p == pData -> prev_comment_original_location ) {
        request = pData -> prev_comment_final_location;
    } else {
        // Here, we have found another x postion of current comment vs previous comment
        // Remember the x position of the current comment in unformatted src
        pData -> prev_comment_original_location = p1 - p;

        if ( !CMDOptions -> align_comment ) {
            request = pData -> prev_comment_original_location / CMDOptions -> nesting_space *
                      CMDOptions -> nesting_space;
            return request;
        }

        if ( pData -> current_column <= CMDOptions -> start_mnemonic )
            request = CMDOptions -> start_mnemonic;
        else
            request = CMDOptions -> start_comment;

        //if (current_column == 0 && align_comment == 1)
        //    request = start_mnemonic;

        // Remember the final location of currently processed comment
        pData -> prev_comment_final_location = request;
    }

    return request;
}

void conditionallyAddPaddingLineBeforeSection ( sf65Options_t * CMDOptions, sf65ParsingData_t * ParserData ) {
    if ( CMDOptions -> pad_directives && ParserData -> flags & LEVEL_IN ) {
        if ( ParserData -> prev_expr.exprType != SF65_EMPTYLINE ) {
            sf65_fputc ( '\n', output );
        }
    }
}

void conditionallyAddPaddingLineAfterSection ( sf65Options_t * CMDOptions, sf65ParsingData_t * ParserData ) {
    if ( CMDOptions -> pad_directives && ParserData -> flags & LEVEL_OUT ) {
        ParserData -> additional_linefeed = true;
    }
}

void conditionallyInsertAdditionalLinefeed ( sf65ParsingData_t * ParserData ) {
    if ( ParserData -> prev_expr.exprType != SF65_EMPTYLINE &&
            ParserData -> additional_linefeed ) {
        sf65_fputc ( '\n', output );
    }
}

void sf65_correctOutputColumnForFlags ( sf65ParsingData_t * ParserData, const sf65Options_t * CMDOptions ) {
    if ( ParserData -> current_column != 0 && CMDOptions -> labels_own_line != 0 && ( ParserData -> flags & DONT_RELOCATE ) == 0 ) {
        sf65_fputc ( '\n', output );

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
void sf65_PlaceMnemonicInLine ( char * p1, char * p2, sf65Options_t * CMDOptions,
                                sf65ParsingData_t * ParserData ) {
    changeCase ( p1, p2, CMDOptions -> mnemonics_case );
    ParserData -> request = CMDOptions -> start_mnemonic;
}

/*
 * This function sets correct case for directive and sets requested x position to start_directive
 * It also indicates that a directive was found and clears the mnemonic found flag
 */
void sf65_PlaceDirectiveInLine ( char * p1, char * p2, sf65Options_t * CMDOptions,
                                 sf65ParsingData_t * ParserData ) {
    changeCase ( p1, p2, CMDOptions -> directives_case );
    ParserData -> request = CMDOptions -> start_directive;
}

/*
 * This function sets correct case for directive and sets requested x position to start_directive
 * It also indicates that a directive was found and clears the mnemonic found flag
 */
void sf65_PlaceOperandInLine ( char * p1, char * p2, sf65Options_t * CMDOptions,
                               sf65ParsingData_t * ParserData ) {
    if ( CMDOptions -> style != 0 ) {
        ParserData -> request = 0;
    } else {
        ParserData -> request = CMDOptions -> start_operand;
    }
}
