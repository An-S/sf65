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
//(Avoid replacement of -> by . or -> by .)
sf65ParsingData_t _sf65ParsingData;

//Create pointer to instance of sf65Options_t, so we can use -> throughout
sf65ParsingData_t *ParserData = &_sf65ParsingData;

/*
** Main program
*/
int main ( int argc, char *argv[] ) {
    int line = 0;   // Line counter for the current line of input to be processed

    char linebuf[1000]; // Input file is read line by line into this buffer

    char *p;        // Pointer for line start
    char *p1;       // Pointer for start of expression
    char *p2;       // Pointer for end of expression
    //char *p3;       // Pointer for forward peeks

    int allocation; // Holds the length the currently processed line of the input file
    // counted from start to a '\n' or EOF

    sf65Expression_t currentExpr = {};

    // Parse command line options and set corresponding variables in CMDOptions struct
    processCMDArgs ( argc, argv, CMDOptions );

    // Try to open input file. Procedure exits in case of error.
    // No further err checking necessary
    input = sf65_openInputFile ( CMDOptions -> infilename );

    // Tell user that processing of input file is about to be started
    sf65_printfUserInfo ( "Processing %s...\n", CMDOptions -> infilename );

    /*
    ** Now generate output file
    */

    // Try to open output file. Procedure exits in case of error.
    // No further err checking necessary
    output = sf65_openOutputFile ( CMDOptions -> outfilename );
    logoutput = sf65_openLogFile ( CMDOptions -> outfilename );

    // Start with debug output (Line number of 0)
    sf65_printfUserInfo ( "%4d:", line );

    sf65_initializeParser ( ParserData );

    // Read lines from input until EOF
    // Pointer p is set to start of line for easier parsing (using p instead of linebuf all the time)
    do {
        fgets ( linebuf, sizeof ( linebuf ), input );
        if ( !feof ( input ) ) {
            if ( ferror ( input ) ) {
                sf65_printfUserInfo ( "Error reading line\n" );
                exit ( 1 );
            }
        }

        // Output linebuf so we see if there's a line which causes parser to lockup
        sf65_printfUserInfo ( "%04d:__", line );
        sf65_printfUserInfo ( "%s", linebuf );
        sf65_fprintf ( logoutput, "%04d:__", line );
        sf65_fprintf ( logoutput, linebuf );
        sf65_fprintf ( logoutput, "%04d:__", line );

        // Set pointer p1 to start of line
        p1 = p = linebuf;

        // Get length of current line, just read
        allocation = strlen ( linebuf );

        // If linebuf contains not more than a newline and a termination character, process next line
        if ( allocation < 2 ) {
            sf65_fputc ( '\n', output );
            sf65_fprintf ( logoutput, "%s\n", sf65StrExprTypes[SF65_EMPTYLINE] );

            ParserData -> prev_expr.exprType = SF65_EMPTYLINE;
            ParserData -> additional_linefeed = 0;

            ++line;
            continue;
        }

        currentExpr.exprType = SF65_OTHEREXPR;

        // Check, if termination end of line character is read. If not,
        // the input buffer is too small to hold the complete line and was therefor
        // truncated upon reading
        if ( linebuf[allocation - 1] != '\n' && !feof ( input ) ) {
            sf65_printfUserInfo ( "Error: Line %d too long: %s", line, linebuf );
            exit ( 1 );
        }

        // Output linebuf so we see if there's a line which causes parser to lockup
        sf65_printfUserInfo ( "%04d:__", line );
        sf65_printfUserInfo ( "%s", linebuf );

        // If parser requested additional linefeed on parsing prev line, then insert
        // the requested additional linefeed. However, if there is already an
        // empty line in the input, additional linefeed is suppressed
        conditionallyInsertAdditionalLinefeed ( ParserData );

        // Reset flags for detected expression types from last line of input
        ParserData -> directive_detected =
            ParserData -> mnemonic_detected =
                ParserData -> current_column =
                    ParserData -> label_detected =
                        ParserData -> operand_detected =
                            ParserData -> additional_linefeed = false;

        // Indicate, that we are at
        // the beginning of a line by setting first_expression flag.
        // Enforce separating space after parts of expressions as a default
        ParserData -> first_expression =
            ParserData -> beginning_of_line = true;
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

            // Check termination condition for current line by comparing running pointers
            // with total length of current line
            if ( *p1 == 0 || ( p1 - linebuf ) >= allocation - 1 ) {
                fputc ( '\n', output );
                sf65_fputc (  '\n', logoutput );

                break;
            }

            // Overread white space at beginning of line
            p1 = skipWhiteSpace ( p1 );
            if ( p1 - p ) { ParserData -> beginning_of_line = false;}

            // Detect quotes and return whole string at once, if quote is found
            if ( *p1 == '"' ) {
                p2 = readUntilClosingQuote ( p1 );
                ++p2; // skip closing quote
            } else {
                // Read a white space or special character separated section of input file
                p2 = detectCodeWord ( p1 );

                // Sanity check to prevent lockups from pointers beeing not increased anymore
                if ( p2 == p1 ) {
                    // Read rest of expression
                    ++p2; //= detectOperand ( p1 );
                }
            }

            ParserData -> flags = 0;
            ParserData -> instant_additional_linefeed = false;

            // Analyze expression determined by the start and end pointers p1 and p2
            // Except for currentExpr all values are returned in ParserData struct
            currentExpr  = sf65DetermineExpression ( p1, p2, ParserData, CMDOptions );

            // Use extra if construct so that we can break inner loop, which wouldn't be
            // possible with switch contruct
            if ( currentExpr.exprType == SF65_COMMENT ) {  /* Comment */
                // Get x position for output of comment
                ParserData -> request =
                    getCommentSpacing ( p, p1, ParserData );

                // Indent by level times nesting space/tab width
                if ( ParserData -> request == CMDOptions -> start_mnemonic ) {
                    ParserData -> request +=
                        ParserData -> current_level *
                        CMDOptions -> nesting_space;
                }

                // Align output by filling up with spaces
                request_space ( output, &ParserData -> current_column,
                                ParserData -> request, 1, CMDOptions -> tabs );

                // Store formatted expression into output
                sf65_fwriteCountChars ( p1, allocation - ( p1 - p ), output );
                sf65_fprintf ( logoutput, "%s / \n", sf65StrExprTypes[SF65_COMMENT] );
                //When comment is found, rest of line is also comment. So proceed to next line
                break;
            }

            switch ( currentExpr.exprType ) {
            case SF65_MACRONAME:
            case SF65_MNEMONIC:
                sf65_PlaceMnemonicInLine ( p1, p2, CMDOptions, ParserData );
                break;

            case SF65_DIRECTIVE:
                sf65_PlaceDirectiveInLine ( p1, p2, CMDOptions, ParserData );
                conditionallyAddPaddingLineAfterSection ( CMDOptions, ParserData );

                break;

            case SF65_OPERAND:
                sf65_PlaceOperandInLine ( p1, p2, CMDOptions, ParserData );

                // Is operand does not start with a variable or label character or a number
                // directly attach operand to mnemonic. f.e. "lda #$ 00" is not desired but
                // "lda #$00". However, "lda label1" or "sta 1" is ok.
                if ( !isExpressionCharacter ( *p1 ) ) ParserData -> force_separating_space = 0;

                break;

            case SF65_IDENTIFIER:
                ParserData -> force_separating_space = true;

                switch ( ParserData->prev_expr.exprType ) {
                case SF65_ASSIGNMENT:
                    ParserData -> request = 0;
                    break;
                case SF65_MNEMONIC:
                    ParserData -> request = CMDOptions->start_operand;
                    break;
                default:
                    ParserData -> request = 0; // Do not put identifier at special position
                    break;
                }


                break;

            case SF65_LABEL:
                // Leave label at start of line
                ParserData -> request = 0;
                ParserData -> flags = DONT_RELOCATE;

                // Detect oversized labels.
                // Check, if p2 already at end of line
                // Then additional cr is not needed
                if ( allocation > p2 - p1 ) {
                    if ( CMDOptions -> oversized_labels_own_line ) {
                        if ( p2 - p1 >= CMDOptions -> start_mnemonic ) {
                            ParserData -> instant_additional_linefeed = true;
                        }
                    }
                    if ( CMDOptions -> labels_own_line ) {
                        ParserData -> instant_additional_linefeed = true;
                    }
                }
                break;

            case SF65_EMPTYLINE:
                ParserData -> additional_linefeed = false;
                break;

            case SF65_ASSIGNMENT:
                ParserData -> force_separating_space = true;
                ParserData -> request = CMDOptions -> start_mnemonic;
                break;

            default:
                // Detect separator for comma separated list of values
                switch ( ParserData -> prev_expr.exprType ) {
                case SF65_COMMASEP:
                    // Align comma separated list of values
                    ParserData -> request =
                        sf65_align (
                            ParserData -> current_column,
                            CMDOptions -> nesting_space
                        );
                    ParserData -> flags = DONT_RELOCATE;
                    break;

                case SF65_ASSIGNMENT:
                    ParserData -> force_separating_space = true;
                    break;

                default:
                    // No comma separated list of values.
                    if ( isExpressionCharacter ( *p1 ) &&
                            isExpressionCharacter ( ParserData->prev_expr.rightmostChar ) ) {
                        ParserData -> force_separating_space = true;
                    }
                    // Detect line continuation character and eventually indent line accordingly
                    if ( ParserData -> first_expression && ParserData -> line_continuation ) {
                        ParserData -> line_continuation = 0;
                        ParserData -> request = CMDOptions -> start_operand;
                    } else {
                        // Standard output
                        ParserData -> request = 0;
                    }
                    break;
                }

                break;
            }


            // For scope enclosing directives, add padding lines if requested by cmd options
            conditionallyAddPaddingLineBeforeSection ( CMDOptions, ParserData );

            // Evaluate flags given for certain directives and correct output column accordingly
            sf65_correctOutputColumnForFlags ( ParserData, CMDOptions );

            // Indent sections of code according to nesting depth
            if ( ParserData -> flags != DONT_RELOCATE )
                ParserData -> request += ParserData -> current_level *
                                         CMDOptions -> nesting_space;

            // Add filling spaces for alignment but not for a comma delimiter
            if ( *p1 != ',' )
                request_space ( output, &ParserData -> current_column, ParserData -> request,
                                ParserData -> force_separating_space, CMDOptions -> tabs );

            ParserData -> force_separating_space = false;

            // Write current term to output file
            sf65_fwrite ( p1, p2, output );
            //sf65_fwrite ( p1, p2, logoutput );

            sf65_fwriteCountChars ( sf65StrExprTypes[currentExpr.exprType],
                                    strlen ( sf65StrExprTypes[currentExpr.exprType] ), logoutput );
            sf65_fprintf ( logoutput, " / " );

            // Increase current_column by length of current term
            ParserData -> current_column += p2 - p1;

            // For breaking oversized labels, insert instant additional linefeed
            if ( ParserData -> instant_additional_linefeed ) {
                fputc ( '\n', output );
                ParserData -> current_column = 0;
            }

            // Set pointer p1 to the end of the expression+1 to proceed further
            p1 = p2;

            // If come here, at least one expression has been evaluated so unset
            // first_expression flag
            ParserData -> first_expression = false;

            // Propagate current values from last run to the variables holding prev values
            ParserData -> prev_expr = currentExpr;
            ParserData -> last_column = ParserData -> current_column;
        }

        ++line;
    } while ( !feof ( input ) );
    sf65_printfUserInfo ( "\n" );
    sf65_fprintf ( logoutput, "\n" );

    fclose ( input );
    fclose ( output );
    fclose ( logoutput );

    exit ( 0 );
}
