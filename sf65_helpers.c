#include "sf65.h"

extern sf65Options_t *CMDOptions;

int sgn ( int x ) {
    if ( x ) {
        return x / abs ( x );
    }
    return 0;
}
char echoChar ( char ch ) {
    //fputc (ch, stdout);
    return ch;
}

void sf65_vpError ( char *format, va_list va ) {
    vfprintf ( stderr, format, va );
}

void sf65_pError ( char *format, ... ) {
    va_list va;
    va_start ( va, format );

    vfprintf ( stderr, format, va );
    va_end ( va );
}
/*
 * Reads characters from input until a non white-space character or line-end or '\0'
 * is found. Returns pointer to first non-whitespace char or '\n' or '\0'
 */
char *skipWhiteSpace ( char *p ) {
    char ch;
    while ( ch = *p, ch && isspace ( ch ) && ch != '\n' ) {
        echoChar ( ch );
        ++p;
    }

    // p points to first char after whitespace or to '\0' character
    return p;
}

char *readUntilClosingQuote ( char *p ) {

    // Skip quote at p. Find closing quote
    return strpbrk ( p + 1, "\"" );
}

char *modifyChars ( char *p1, char *p2, int func ( int ) ) {
    char ch;

    while ( p1 < p2 ) {
        ch = *p1;
        echoChar ( ch );
        *p1 = func ( ch );
        p1++;
    }
    return p1;
}

char *changeCase ( char *p1, char *p2, char _case ) {
    switch ( _case ) {
    case 1:
        modifyChars ( p1, p2, tolower );
        break;
    case 2:
        modifyChars ( p1, p2, toupper );
        break;
    default:
        break;
    }
    return p2;
}

int sf65_align ( int val, int align ) {
    return ( val + align - 1 ) / align * align;
}

bool isExpressionCharacter ( char ch ) {
    bool flag;

    if ( ch == '.' || ch == '_' ) {
        flag = true;
    } else {
        flag = ( ch != ';' &&
                 ch != '\'' && ch != '"' &&
                 ch != '#' &&
                 ch != '$' &&
                 ch != '%' &&
                 ch != ',' &&
                 ch != '\\' ) ;
    }
    return flag;
}

char *detectCodeWord ( char *p ) {
    //
    char ch;

    while ( ch = *p, ch && !isspace ( ch ) && isExpressionCharacter ( ch ) ) {
        echoChar ( ch );
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

/*
** Comparison without case
*/
int memcmpcase ( char *p1, char *p2, int size ) {
    //    char ch;

    while ( size-- ) {
        if ( tolower ( *p1 ) != tolower ( *p2 ) )
            return 1;
        p1++;
        p2++;
    }
    return 0;
}

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

/* Tests, if a pointer is in range between a start pointer and and end pointer
 * defined by the size of the range
 */
bool inRange ( const char *p, const char *first, int size ) {
    return p < ( first + size );
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
