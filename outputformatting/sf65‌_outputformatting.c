#include "sf65.h"

/*
** Request space in line
*  Return number of spaces actually written
*/
int request_space ( FILE *output, int *current, int new, int force, int tabs ) {

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


int getCommentSpacing ( char *p /*linestart*/, char *p1 /*commentstart*/, sf65ParsingData_t *pData ) {
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

void conditionallyAddPaddingLineBeforeSection ( sf65Options_t *CMDOptions, sf65ParsingData_t *ParserData ) {
    if ( CMDOptions -> pad_directives && ParserData -> flags & LEVEL_IN ) {
        if ( ParserData -> prev_expr.exprType != SF65_EMPTYLINE ) {
            fputc ( '\n', output );
        }
    }
}

void conditionallyAddPaddingLineAfterSection ( sf65Options_t *CMDOptions, sf65ParsingData_t *ParserData ) {
    if ( CMDOptions -> pad_directives && ParserData -> flags & LEVEL_OUT ) {
        ParserData -> additional_linefeed = true;
    }
}

void conditionallyInsertAdditionalLinefeed ( sf65ParsingData_t *ParserData ) {
    if ( ParserData -> prev_expr.exprType != SF65_EMPTYLINE &&
            ParserData -> additional_linefeed ) {

        fputc ( '\n', output );
    }
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
