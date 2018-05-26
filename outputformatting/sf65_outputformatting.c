#include "sf65.h"

int sf65_SetOutputXPositionInLine ( sf65ParsingData_t *pData, int xpos ) {
    NOT_NULL ( pData, -1 ) {
        pData->request = xpos;
        return xpos;
    }
    return -1;
}

int sf65_GetOutputXPositionInLine ( sf65ParsingData_t *pData ) {
    NOT_NULL ( pData, -1 ) {
        return pData->request;
    }
    return -1;
}

int sf65_IncOutputXPositionInLine ( sf65ParsingData_t *pData, int add ) {
    NOT_NULL ( pData, -1 ) {

        pData->request += add;
        return pData->request;
    }
    return -1;
}

int sf65_IncOutputXPositionByNestingLevel ( sf65ParsingData_t * pData, int nestingSpace ) {
    NOT_NULL ( pData, -1 ) {
        if ( ! ( pData->current_expr.exprType == SF65_EMPTYLINE ) ) {
            pData->request +=
                pData -> current_level *
                nestingSpace;
            return pData->request;
        } else {
            return 0;
        }
    }
    return -1;
}

int sf65_IncCurrentColumnCounter ( sf65ParsingData_t *pData, int inc ) {
    NOT_NULL ( pData, -1 ) {
        return pData -> current_column += inc;
    }
    return -1;
}

int sf65_GetCurrentColumnCounter ( sf65ParsingData_t *pData ) {
    NOT_NULL ( pData, -1 ) {
        return pData -> current_column;
    }
    return -1;
}

int sf65_ResetCurrentColumnCounter ( sf65ParsingData_t *pData ) {
    NOT_NULL ( pData, -1 ) {
        return pData -> current_column = 0;
    }
    return -1;
}

int sf65_SetCurrentColumnCounter ( sf65ParsingData_t *pData, int col ) {
    NOT_NULL ( pData, -1 ) {
        return pData -> current_column = col;
    }
    return -1;
}

int sf65_AlignCurrentColumn ( sf65ParsingData_t *pData, int tabs ) {
    int currentColumn = sf65_GetCurrentColumnCounter ( pData );
    return sf65_SetCurrentColumnCounter ( pData, ( currentColumn + tabs ) / tabs * tabs );
}

sf65ErrCode_t sf65_SetParserFlag ( sf65ParsingData_t *pData, sf65ParserFlagsEnum_t flag  ) {
    NOT_NULL ( pData, SF65_NULLPTR ) {
        switch ( flag ) {
#   define PF(x,y) case SF65_##y: pData -> x=1; break;
            SF65_PARSERFLAGS
#   undef PF
        default:
            assert ( flag < SF65_NOT_A_PARSERFLAG );
            return SF65_SETERR ( SF65_INVALIDARGERR );
        }
        return SF65_NOERR;
    }
}

unsigned int sf65_GetParserFlag ( sf65ParsingData_t *pData, sf65ParserFlagsEnum_t flag ) {
    NOT_NULL ( pData, SF65_NULLPTR ) {
        switch ( flag ) {
#   define PF(x,y) case SF65_##y: return pData -> x;
            SF65_PARSERFLAGS
#   undef PF
        default:
            SF65_SETERR ( SF65_INVALIDARGERR );
            assert ( flag < SF65_NOT_A_PARSERFLAG );
            break;
        }
    }
    return -1;
}

sf65ErrCode_t sf65_SetParserFlags ( sf65ParsingData_t *pData, sf65ParserFlagsEnum_t flag1, ... ) {
    NOT_NULL ( pData, SF65_NULLPTR ) {
        va_list va;
        va_start ( va, flag1 );

        while ( flag1 != SF65_NOT_A_PARSERFLAG ) {
            assert ( flag1 < SF65_NOT_A_PARSERFLAG );
            sf65_SetParserFlag ( pData, flag1 );
            flag1 = va_arg ( va, sf65ParserFlagsEnum_t );
        }
        va_end ( va );
        return SF65_NOERR;
    }
}

sf65ErrCode_t sf65_ClearParserFlag ( sf65ParsingData_t *pData, sf65ParserFlagsEnum_t flag ) {
    NOT_NULL ( pData, SF65_NULLPTR ) {
        switch ( flag ) {
#       define PF(x,y) case SF65_##y: pData -> x=0; break;
            SF65_PARSERFLAGS
#       undef PF
        default:
            assert ( flag < SF65_PARSERFLAGCNT );
            return SF65_SETERR ( SF65_INVALIDARGERR );
        }
        return SF65_NOERR;
    }
}

sf65ErrCode_t sf65_ClearParserFlags ( sf65ParsingData_t *pData, sf65ParserFlagsEnum_t flag1, ... ) {
    NOT_NULL ( pData, SF65_NULLPTR ) {
        va_list va;
        va_start ( va, flag1 );

        while ( flag1 != SF65_NOT_A_PARSERFLAG ) {
            assert ( flag1 < SF65_PARSERFLAGCNT );
            sf65_ClearParserFlag ( pData, flag1 );
            flag1 = va_arg ( va, sf65ParserFlagsEnum_t );
        }
        va_end ( va );
        return SF65_NOERR;
    }
}

sf65ErrCode_t sf65_ResetParserFlags ( sf65ParsingData_t * pData ) {
    NOT_NULL ( pData, SF65_NULLPTR ) {
        pData -> allParserFlags = 0;

        return SF65_NOERR;
    }
    return SF65_SETERR ( SF65_NULLPTR );
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

sf65ErrCode_t sf65_SetLinefeedType ( sf65ParsingData_t *pData, sf65LinefeedEnum_t lf_type ) {
    NOT_NULL ( pData, SF65_NULLPTR ) {
        sf65_ClearParserFlags (
            pData, SF65_INSTANT_ADDITIONAL_LINEFEED, SF65_ADDITIONAL_LINEFEED,
            SF65_NOT_A_PARSERFLAG
        );

        switch ( lf_type ) {

        case SF65_ADD_LF:
            sf65_SetParserFlag ( pData, SF65_ADDITIONAL_LINEFEED );
            break;

        case SF65_INSTANT_ADD_LF:
            sf65_SetParserFlag ( pData, SF65_INSTANT_ADDITIONAL_LINEFEED );
            break;

        default:
            assert ( lf_type < SF65_NOT_A_LF_CONST );

            if ( lf_type >= SF65_NOT_A_LF_CONST ) {
                return SF65_SETERR ( SF65_INVALIDARGERR );
            }
            break;
        }

        return SF65_NOERR;
    }
}

sf65ErrCode_t sf65_ResetLinefeedFlag ( sf65ParsingData_t *pData, sf65LinefeedEnum_t lf_type ) {
    NOT_NULL ( pData, SF65_NULLPTR ) {
        switch ( lf_type ) {

        case SF65_ADD_LF:
            sf65_ClearParserFlag ( pData, SF65_ADDITIONAL_LINEFEED );
            break;

        case SF65_INSTANT_ADD_LF:
            sf65_ClearParserFlag ( pData, SF65_INSTANT_ADDITIONAL_LINEFEED );
            break;

        default:
            assert ( lf_type < SF65_NOT_A_LF_CONST );

            if ( lf_type >= SF65_NOT_A_LF_CONST ) {
                return SF65_SETERR ( SF65_INVALIDARGERR );
            }
            break;
        }
        return SF65_NOERR;
    }
}

sf65ErrCode_t sf65_SetPaddingSpaceFlag ( sf65ParsingData_t *pData ) {
    return sf65_SetParserFlag ( pData, SF65_FORCE_SEPARATING_SPACE );
}

sf65ErrCode_t sf65_ClearPaddingSpaceFlag ( sf65ParsingData_t *pData ) {
    return sf65_ClearParserFlag ( pData, SF65_FORCE_SEPARATING_SPACE );
}

// Insert padding line if,
// - there was no empty line, before
// - there was no changing of level, before
// - the command line option is set
void conditionallyAddPaddingLineBeforeSection ( sf65Options_t * CMDOptions, sf65ParsingData_t * ParserData ) {
    if ( CMDOptions -> pad_directives && ParserData -> flags & LEVEL_IN &&
            ! ( sf65_GetParserFlag ( ParserData -> prev, SF65_LEVEL_CHANGED ) ) ) {
        if ( ParserData -> prev -> current_expr.exprType != SF65_EMPTYLINE ) {
            //sf65_SetLinefeedType ( ParserData, SF65_INSTANT_ADD_LF );
            sf65_fputc ( '\n', output );
            //ParserData -> prev -> currentExprcurrentExpr.exprType = SF65_EMPTYLINE;
        }
    }
}

void conditionallyAddPaddingLineAfterSection ( sf65Options_t * CMDOptions, sf65ParsingData_t * ParserData ) {
    if ( CMDOptions -> pad_directives && ParserData -> flags & LEVEL_OUT ) {
        //ParserData -> additional_linefeed = true;
        sf65_SetLinefeedType ( ParserData, SF65_ADD_LF );
    }
}

void conditionallyInsertAdditionalLinefeed ( sf65ParsingData_t * ParserData ) {
    static bool checkedEmptyLine = false;

    if ( checkedEmptyLine ) {
        if ( ParserData -> current_expr.exprType != SF65_EMPTYLINE &&
                ParserData -> additional_linefeed &&
                ! ( ParserData -> flags & LEVEL_OUT ) ) {
            sf65_fputc ( '\n', output );
        }

        sf65_ResetLinefeedFlag ( ParserData, SF65_ADD_LF );
        checkedEmptyLine = false;
    } else {
        if ( ParserData -> additional_linefeed ) {
            checkedEmptyLine = true;
        }
    }
}

void sf65_correctOutputColumnForFlags ( sf65ParsingData_t * ParserData, const sf65Options_t * CMDOptions ) {

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
    sf65_ChangeCase ( p1, p2, CMDOptions -> mnemonics_case );
    sf65_SetOutputXPositionInLine ( ParserData, CMDOptions -> start_mnemonic );
}

/*
 * This function sets correct case for directive and sets requested x position to start_directive
 * It also indicates that a directive was found and clears the mnemonic found flag
 */
void sf65_PlaceDirectiveInLine ( char * p1, char * p2, sf65Options_t * CMDOptions,
                                 sf65ParsingData_t * ParserData ) {
    sf65_ChangeCase ( p1, p2, CMDOptions -> directives_case );
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
    sf65_SetPaddingSpaceFlag ( ParserData );
}
