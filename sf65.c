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
 *      -> Fixed in the way that labels or only indented with section when they are aligned with mnemonics
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
 *      ->
 *
 * Missing command line options
 *
 * Reintegrated possibility to choose between extra operand column and
 * operand separated by single space -> 23.03.2018
 *
 * The flag that specifies alignment of comments to nearest column is ignored
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


FILE *input;
FILE *output;

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
    int line = 0;

    char linebuf[1000];

    char *p;
    char *p1;
    char *p2;
    int allocation;
    sf65Expression_t currentExpr = {};

    processCMDArgs ( argc, argv, CMDOptions );

    // Try to open input file. Procedure exits in case of error.
    // No further err checking necessary
    input = sf65_openInputFile ( CMDOptions -> infilename );

    // Tell user that processing of input file is about to be started
    fprintf ( stderr, "Processing %s...\n", CMDOptions -> infilename );

    /*
    ** Now generate output file
    */

    // Try to open output file. Procedure exits in case of error.
    // No further err checking necessary
    output = sf65_openOutputFile ( CMDOptions -> outfilename );

    // Start with debug output (Line number of 0)
    fprintf ( stdout, "%4d:", line );

    ParserData -> request = 0;
    ParserData -> prev_comment_original_location = 0;
    ParserData -> prev_comment_final_location = 0;
    ParserData -> current_level = 0;

    // Read lines from input until EOF
    // Pointer p is set to start of line for easier parsing (using p instead of linebuf all the time)
    while ( fgets ( linebuf, sizeof ( linebuf ), input ), !feof ( input ) ) {
        // Set pointer p1 to start of line
        p1 = p = linebuf;

        // Get length of current line, just read
        allocation = strlen ( linebuf );

        // If linebuf contains not more than a newline and a termination character, process next line
        if ( allocation < 2 ) {
            fputc ( '\n', output );
            ParserData -> prev_expr.exprType = SF65_EMPTYLINE;
            continue;
        }

        if ( linebuf[allocation - 1] != '\n' ) {
            fprintf ( stdout, "Error: Line %d too long: %s", line, linebuf );
            exit ( 1 );
        }

        // Output linebuf so we see if there's a line which causes parser to lockup
        fprintf ( stdout, "%04d:__", line );
        fprintf ( stdout, "%s", linebuf );


        conditionallyInsertAdditionalLinefeed ( ParserData );

        ParserData -> directive_detected =
            ParserData -> mnemonic_detected =
                ParserData -> current_column =
                    ParserData -> label_detected =
                        ParserData -> additional_linefeed = 0;

        ParserData -> first_expression =
            ParserData -> force_separating_space = 1;
        /*
         * PARSING NOTES
         *
         * Labels always at first in line but must not be at first character
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
            if ( *p1 == 0 || ( p1 - linebuf ) >= allocation ) {
                fputc ( '\n', output );
                break;
            }

            p1 = skipWhiteSpace ( p1 );

            if ( *p1 == '"' ) {
                p2 = readUntilClosingQuote ( p1 );
                ++p2; // skip closing quote
            } else {
                p2 = detectCodeWord ( p1 );
                if ( p2 == p1 ) {
                    p2 = detectOperand ( p1 );
                    //p2 = skipWhiteSpace ( p2 );
                }
            }

            ParserData -> flags = 0;
            ParserData -> prev_expr = currentExpr;
            ParserData -> instant_additional_linefeed = false;
            ParserData -> last_column = ParserData -> current_column;

            currentExpr  = sf65DetermineExpression ( p1, p2, ParserData, CMDOptions );

            if ( currentExpr.exprType == SF65_COMMENT ) {  /* Comment */
                // Get x position for output of comment
                ParserData -> request =
                    getCommentSpacing ( p, p1, ParserData );

                // Indent by level times tab width
                if ( ParserData -> request == CMDOptions -> start_mnemonic ) {
                    ParserData -> request +=
                        ParserData -> current_level *
                        CMDOptions -> nesting_space;
                }

                request_space ( output, &ParserData -> current_column,
                                ParserData -> request, 1, CMDOptions -> tabs );

                fwrite ( p1, sizeof ( char ), allocation - ( p1 - p ), output );

                //When comment if found, rest of line is also comment. So proceed to next line
                break;
            }

            switch ( currentExpr.exprType ) {
            case SF65_MNEMONIC: {
                    sf65_PlaceMnemonicInLine ( p1, p2, CMDOptions, ParserData );
                    break;
                }
            case SF65_DIRECTIVE: {
                    sf65_PlaceDirectiveInLine ( p1, p2, CMDOptions, ParserData );
                    conditionallyAddPaddingLineAfterSection ( CMDOptions, ParserData );

                    break;
                }
            case SF65_OPERAND: {
                    // int alignoffset = sf65ParsingData -> current_column - sf65ParsingData -> last_column;

                    sf65_PlaceOperandInLine ( p1, p2, CMDOptions, ParserData );
                    ParserData -> operand_detected = 1;
                    if ( !isalnum ( *p1 ) ) ParserData -> force_separating_space = 0;

                    /*sf65ParsingData -> request =
                         alignoffset % sf65Options -> nesting_space == 0 ?
                            sf65ParsingData -> current_column:
                            sf65ParsingData -> current_column + sf65Options -> nesting_space - alignoffset;
                    ; */
                    break;
                }
            case SF65_LABEL: {
                    ParserData -> request = 0;
                    ParserData -> flags = DONT_RELOCATE;

                    //Check, if p2 already at end of line
                    //Then additional cr is not needed
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
                }
            default: {
                    if ( *p1 == ',' ) {
                        currentExpr.exprType = SF65_COMMASEP;
                        ParserData -> request = 0;
                    } else if ( ParserData -> prev_expr.exprType == SF65_COMMASEP ) {
                        ParserData -> request =
                            sf65_align (
                                ParserData -> current_column,
                                CMDOptions -> nesting_space
                            );
                        ParserData -> flags = DONT_RELOCATE;
                    } else {
                        if ( ParserData -> line_continuation ) {
                            ParserData -> line_continuation = 0;
                            ParserData -> request = CMDOptions -> start_mnemonic;
                        } else {
                            ParserData -> request = 0;

                        }
                    }

                    break;
                }
            }

            conditionallyAddPaddingLineBeforeSection ( CMDOptions, ParserData );
            sf65_correctOutputColumnForFlags ( ParserData, CMDOptions );

            // Indent by level times tab width
            if ( ParserData -> flags != DONT_RELOCATE )
                ParserData -> request += ParserData -> current_level *
                                         CMDOptions -> nesting_space;

            // Add filling spaces for alignment
            if ( *p1 != ',' )
                request_space ( output, &ParserData -> current_column, ParserData -> request,
                                ParserData -> force_separating_space, CMDOptions -> tabs );

            // Write current term to output file
            fwrite ( p1, sizeof ( char ), p2 - p1, output );
            //conditionallyAddPaddingLineAfterSection(sf65Options, sf65ParsingData);
            // Increase current_column by length of current term
            ParserData -> current_column += p2 - p1;

            if ( ParserData -> instant_additional_linefeed ) {
                fputc ( '\n', output );
                ParserData -> current_column = 0;
            }

            // Set pointer p1 to the end of the expression+1 to proceed further
            p1 = p2;

            ParserData -> first_expression = false;
        }

        ++line;
    }

    fclose ( input );
    fclose ( output );
    exit ( 0 );
}
