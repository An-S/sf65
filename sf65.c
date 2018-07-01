/*
** sf65 (Source formatter for CA65) by Monte Carlos / Cascade on 03-2018
** havily based on
** Pretty6502
**
** by Oscar Toledo G.
**
** © Copyright 2017 Oscar Toledo G.
**
** Creation date: Nov/03/2017.
** Revision date: Nov/06/2017. Processor selection. Indents nested IF/ENDIF.
**                             Tries to preserve vertical structure of comments.
**                             Allows label in its own line. Allows to change case
**                             of mnemonics and directives.
**
** Changes introduced further:
** - Major code refactoring to increase maintainability and extensibility
**      In Detail:
**      - Added header file containing all important definitions and prototypes
**      - Sepearated repeated code into own procedures and put into a helper.c file
**      - Rewrote main loop using calls to these procedures instead of the former inline placement of the code
**      - Added header for datatype definitions, globbered global variables into structs
**      - Provided more named constants for alignment and expression types
**
** - Replaced DASM directives list with the CA65 directives
*/


/* BUGTRACKER
 *
 * Comments at line start are not aligned with commands when requested
 *      -> Seems fixed, now (16.03.2018)
 * Mnemonics having args not separated by space are not recognized
 *      -> Is fixed for mnemonics which comes first in line
 *      -> 17./18.03.2018
 * Alignment of .proc/.scope/.endproc/.endscope is not satisfying
 *      -> Tried to solve that issue by placing scope directives 4 chars left of menemonics
 *      -> 17./18.03.2018
 * Mnemonics/directives after labels are recognized as operands
 *      -> Progress: The mnemonics are recognized as seen by the changed case. However, they are not in the correct column
 * (However, comments are correctly recignized and aligned with mnemonics)
 *         -> 19./20.03.2018
 * Labels are indented with section(.proc, .scope ...) directives
 *      -> Fixed in the way that labels are only indented with section when they are aligned with mnemonics
 *          -> 21.03.2018
 * Missing spaces after directives
 *      -> Fixed by remembering a found directive and issuing a space before processing the next term
 *          -> 21.03.2018
 * Missing spaces after labels
 *      -> Fixed for most cases but one: length of label is very large -> Fixed 21.03.2018
 * Data directives are not correctly placed with mnemonics
 *      -> Fixed by introducing ALIGN_MNEMONIC flag 19.03.2018
 * Extra linefeeds after comments
 *      -> Fixed, 20.03.2018
 * Removed linefeeds on empty lines
 *      -> Problem persists after directives in prev line -> Fixed 21.03.2018
 * If unformatted source contains mnemonics which reference to variables
 * then operands are separated by space instead of using start_operand column
 *      -> fixed, by remembering found mnemonic and formatting following statement
 *         at start_operand column
 *
 * When variables are assigned to an hexadecimal value, sf65 outputs space between $ and xdigits,
 * which cause syntax error
 *      -> Fixed, 14.04.2018
 *
 * Missing command line options
 *
 * Reintegrated possibility to choose between extra operand column and
 * operand separated by single space -> 23.03.2018
 *
 * In mode which breaks long labels and rest of line into each one line, also variable assignments
 * are beeing broken into two lines. However, this does not make sense.
 *
 * The flag that specifies alignment of comments to nearest column is ignored
 *
 * Labels starting in first line of source are aligned with mnemonics
 *
 */

/* MISSING FEATURES
 *
 * Colons cannot be added/removed from labels
 * Put overlengthy labels into own line
 * Format comma separated data tables to nearest tab stop
 * After directives like .byte, align first data entry to mnemonics
 * Add one space after each comma in data tables, remove spaces before commas
 * Format arguments of directives so they are aligned with opcodes
 */

/* UNSUPPORTED FEATURES
 *
 * cheap locals are not detected as such (they have @ as first char instead of 'a-zA-Z' or '_'
 * .LOCALCHAR, .features at_in_identifiers, dollar_in_identifiers,
 * leading_dot_in_identifiers may break things
 */
#include "sf65.h"


typedef struct {
    FILE *input;
    FILE *output;
    FILE *logoutput;
} sf65FileDescr_t;

FILE *input;
FILE *output;
FILE *logoutput;

//Create instance of sf65Options_t, but do not use directly
//(Avoid replacement of -> by . or -> by .)
sf65Options_t _sf65Options;

//Create pointer to instance of sf65Options_t, so we can use -> throughout
sf65Options_t *CMDOptions = &_sf65Options;

//Create instance of sf65ParsingData_t, but do not use directly
//(Avoid replacement of -> by . or . by ->)
//sf65ParsingData_t _sf65ParsingData;

//Create pointer to instance of sf65Options_t, so we can use -> throughout
sf65ParsingData_t *ParserData = NULL;//&_sf65ParsingData;
sf65ParsingData_t *OldParserData = NULL;//&_sf65ParsingData;

void sf65_CloseFiles ( void ) {
    sf65_CloseErrLog();
    sf65_CloseFile ( input );
    sf65_CloseFile ( output );
    sf65_CloseFile ( logoutput );
}

char *sf65_GetStartOfExpressionString ( char *p ) {
    NOT_NULL ( p, NULL ) {
        return sf65_SkipWhiteSpace ( p );
    }
}

char *sf65_GetEndOfExpressionString ( char *p ) {
    NOT_NULL ( p, NULL ) {
        // Detect quotes and return whole string at once, if quote is found
        // If no quote is found, real until eof/nl
        if ( *p == '"' ) {
            p = sf65_ReadUntilClosingQuote ( p );
            ++p; // skip closing quote
            return p;
        } else {
            char *p2 = NULL;

            // Read a white space or special character separated section of input file
            p2 = sf65_DetectCodeWord ( p );

            // Sanity check to prevent lockups from pointers beeing not increased anymore
            if ( p2 == p ) {
                // Read rest of expression
                ++p2; //= detectOperand ( p1 );
            }
            return p2;
        }
    }
}

void sf65_IndicateBeginningOfLineState ( sf65ParsingData_t *pData, char *p1 ) {
    NOT_NULL ( pData, ) {
        pData -> beginning_of_line = ! ( p1 - pData -> linebuf );
    }
}

/*
** Main program
*/
int main ( int argc, char *argv[] ) {
    int line = 0;   // Line counter for the current line of input to be processed

    char *p;        // Pointer for line start
    char *p1;       // Pointer for start of expression
    char *p2;       // Pointer for end of expression
    //char *p3;       // Pointer for forward peeks

    int allocation; // Holds the length the currently processed line of the input file
    // counted from start to a '\n' or EOF

    sf65Expression_t *currentExpr;

    atexit ( sf65_CloseFiles );

    // Parse command line options and set corresponding variables in CMDOptions struct
    if ( sf65_ParseCMDArgs ( argc, argv, CMDOptions ) < 0 ) {
        sf65_pError ( "Error parsing cmd args\n" );
        exit ( 1 );
    }

    // Try to open input file. Procedure exits in case of error.
    // No further err checking necessary
    input = sf65_openInputFile ( CMDOptions -> infilename );

    // Tell user that processing of input file is about to be started
    sf65_printfUserInfo ( "Processing %s...\n", CMDOptions -> infilename );

    /*
    ** Open or create output files
    */

    // Try to open output file. Procedure exits in case of error.
    // No further err checking necessary
    output = sf65_openOutputFile ( CMDOptions -> outfilename );
    logoutput = sf65_openLogFile ( CMDOptions -> infilename );
    sf65_OpenErrLog ( CMDOptions -> infilename );

    // Start with debug output (Line number of 0)
    sf65_printfVerbose ( 1, CMDOptions, "%4d:", line );

    sf65_GetParserDataPointers ( &ParserData, &OldParserData );

    // Read lines from input until EOF
    // Pointer p is set to start of line for easier parsing (using p instead of linebuf all the time)
    do {
        if ( !sf65_fgets ( input, ParserData -> linebuf, sizeof ( ParserData -> linebuf ) ) ) {
            // fgets read EOF so provoke EOF error to exit while loop at the end
            sf65_fputnl ( output );
            break;
        }
        if ( !feof ( input ) ) {
            if ( ferror ( input ) ) {
                sf65_printfUserInfo ( "Error reading line\n" );
                exit ( 1 );
            }
        }

        // Output linebuf so we see if there's a line which causes parser to lockup
        sf65_printfVerbose ( 1, CMDOptions, "%04d:__", line );
        sf65_printfVerbose ( 1, CMDOptions, "%s", ParserData -> linebuf );
        sf65_fprintf ( logoutput, "%04d:__", line );
        sf65_fprintf ( logoutput, ParserData -> linebuf );
        sf65_fprintf ( logoutput, "%04d:__", line );

        // Set pointer p1 to start of line
        p1 = p2 = p = ParserData -> linebuf;

        // Get length of current line, just read
        allocation = ParserData -> linesize = strlen ( ParserData -> linebuf );

        //currentExpr.exprType = SF65_OTHEREXPR;

        // Check, if termination end of line character is read. If not,
        // the input buffer is too small to hold the complete line and was therefor
        // truncated upon reading
        if ( ParserData -> linebuf[allocation - 1] != '\n' && !feof ( input ) ) {
            sf65_printfUserInfo ( "Error: Line %d too long: %s", line, ParserData -> linebuf );
            exit ( 1 );
        }

        // Output linebuf so we see if there's a line which causes parser to lockup
        sf65_printfVerbose ( 1, CMDOptions, "%04d:__", line );
        sf65_printfVerbose ( 1, CMDOptions, "%s", ParserData -> linebuf );

        // Must be inserted after call to conditionallyInsertAdditionalLinefeed(...), because
        // this function needs ADDITIONAL_LINEFEED flag from prev line
        sf65_StartParsingNewLine ( ParserData );

        /*
         * PARSING NOTES
         *
         * Labels always at first in line but need not to be at first character
         * Labels always have '_' or 'a-zA-Z' as first char
         * First term after linestart may also be mnemonic or directive or comment
         * Mnemonics are recognized by their name
         * Operands can come after mnemonic, only
         * Operands include special chars like '"', '\'', '$', '%', '#', '<', '>'...
         * Operands may be calculated
         * Directives may have parameters which also needs to be aligned
         *
         */

        // Loop over all chars in a line
        while ( true ) {
            // Allocation >= 2, here

            sf65_InitExpressionDetermination ( ParserData );
            p1 = sf65_GetStartOfExpressionString ( p1 );
            p2 = sf65_GetEndOfExpressionString ( p1 );

            // Integrate colon into statement, if present
            if ( *p2 == ':' ) {
                ++p2;
            }

            sf65_IndicateBeginningOfLineState ( ParserData, p1 );

            // Analyze expression determined by the start and end pointers p1 and p2
            // Except for currentExpr all values are returned in ParserData struct
            currentExpr = sf65DetermineExpression ( p1, p2, ParserData, CMDOptions );
            p2 = sf65_EvaluateExpression ( ParserData, CMDOptions, p1, p2 );

            // Detect line continuation character and eventually indent line accordingly
            if ( ParserData -> first_expression && ParserData -> line_continuation ) {
                ParserData -> line_continuation = 0;
                sf65_SetOutputXPositionInLine ( ParserData, CMDOptions -> start_operand );
            }

            // For scope enclosing directives, add padding lines if requested by cmd options
            conditionallyAddPaddingLineBeforeSection ( CMDOptions, ParserData );

            // Evaluate flags given for certain directives and correct output column accordingly
            sf65_correctOutputColumnForFlags ( ParserData, CMDOptions );

            // Indent sections of code according to nesting depth
            if ( ParserData -> flags != DONT_RELOCATE ) {
                sf65_IncOutputXPositionByNestingLevel ( ParserData, CMDOptions -> nesting_space );
            }

            if ( ParserData -> first_expression ) {

                if ( ! ( ParserData -> flags & LEVEL_IN  ||
                         ParserData -> flags & LEVEL_OUT ) ) {
                    sf65_ClearParserFlag ( ParserData, SF65_LEVEL_CHANGED );
                }

                // If parser requested additional linefeed on parsing prev line, then insert
                // the requested additional linefeed. However, if there is already an
                // empty line in the input, additional linefeed is suppressed
                conditionallyInsertAdditionalLinefeed ( ParserData );
            }

// Add filling spaces for alignment but not for a comma delimiter
            if ( *p1 != ',' )
                sf65_PadOutputWithSpaces (
                    output, ParserData, CMDOptions -> tabs
                );

// Write current term to output file
            sf65_fwrite ( p1, p2, output );
//sf65_fwrite ( p1, p2, logoutput );

// For breaking oversized labels, insert instant additional linefeed
            if ( ParserData -> instant_additional_linefeed ) {
                sf65_fputnl ( output );
                sf65_ResetCurrentColumnCounter ( ParserData );
            } else {
                // Increase current_column by length of current term
                sf65_IncCurrentColumnCounter ( ParserData, p2 - p1 );
            }

            sf65_fwriteCountChars ( sf65StrExprTypes[currentExpr->exprType],
                                    strlen ( sf65StrExprTypes[currentExpr->exprType] ), logoutput );
            sf65_fprintf ( logoutput, " / " );

            if ( ParserData -> flags & LEVEL_IN  ||
                    ParserData -> flags & LEVEL_OUT ) {
                sf65_SetParserFlag ( ParserData, SF65_LEVEL_CHANGED );
            }

// Set pointer p1 to the end of the expression+1 to proceed further
            p1 = p2;

// If come here, at least one expression has been evaluated so unset
// first_expression flag
            ParserData -> first_expression = false;

// Propagate current values from last run to the variables holding prev values

// Check termination condition for current line by comparing running pointers
// with total length of current line

//sf65_TestLineEvaluationTerminationCondition();
            sf65_ToggleParserDataPointers ( &ParserData, &OldParserData );

            if ( *p1 == 0 || ( p1 - p ) >= allocation - 1 || *p1 == '\n' ) {
                if ( ParserData->current_expr.exprType != SF65_EMPTYLINE && !feof ( input ) ) {
                    sf65_fputnl ( output );
                }
                sf65_fputnl ( logoutput );
                break;
            }

        }
        if ( sf65_GetParserFlag ( ParserData, SF65_INDENT_REQUEST ) ) {
            sf65_ClearParserFlag ( ParserData, SF65_INDENT_REQUEST );
            ++ParserData -> current_level;
        }
        if ( sf65_GetParserFlag ( ParserData, SF65_UNINDENT_REQUEST ) ) {
            sf65_ClearParserFlag ( ParserData, SF65_UNINDENT_REQUEST );
            --ParserData -> current_level;
        }
        ++line;
    } while ( !feof ( input ) );
    sf65_printfUserInfo ( "\n" );
    sf65_fprintf ( logoutput, "\n" );

    exit ( 0 );
}
