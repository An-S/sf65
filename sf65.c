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
sf65Options_t *sf65Options = &_sf65Options;

//Create instance of sf65ParsingData_t, but do not use directly
//(Avoid replacement of -> by . or -> by .)
sf65ParsingData_t _sf65ParsingData;

//Create pointer to instance of sf65Options_t, so we can use -> throughout
sf65ParsingData_t *sf65ParsingData = &_sf65ParsingData;

/*
** Main program
*/
int main (int argc, char *argv[]) {
    int line = 0;

    char linebuf[1000];

    char *p;
    char *p1;
    char *p2;
    int allocation;
    sf65Expression_t currentExpr = {};
    
    processCMDArgs (argc, argv, sf65Options);
    
    // Try to open input file. Procedure exits in case of error.
    // No further err checking necessary
    input = sf65_openInputFile (sf65Options -> infilename);

    // Tell user that processing of input file is about to be started
    fprintf (stderr, "Processing %s...\n", sf65Options -> infilename);

    /*
    ** Now generate output file
    */
    
    // Try to open output file. Procedure exits in case of error.
    // No further err checking necessary
    output = sf65_openOutputFile (sf65Options -> outfilename);

    // Start with debug output (Line number of 0)
    fprintf (stdout, "%4d:", line);

    sf65ParsingData -> request = 0;
    sf65ParsingData -> prev_comment_original_location = 0;
    sf65ParsingData -> prev_comment_final_location = 0;
    sf65ParsingData -> current_level = 0;

    // Read lines from input until EOF
    // Pointer p is set to start of line for easier parsing (using p instead of linebuf all the time)
    while (fgets (linebuf, sizeof (linebuf), input), !feof (input)) {
        // Set pointer p1 to start of line
        p1 = p = linebuf;

        // Get length of current line, just read
        allocation = strlen (linebuf);

        // If linebuf contains not more than a newline and a termination character, process next line
        if (allocation < 2) {
            fputc ('\n', output);
            currentExpr.exprType = SF65_EMPTYLINE;
            
            continue;
        }

        if (linebuf[allocation - 1] != '\n') {
            fprintf (stdout, "Error: Line %d too long: %s", line, linebuf);
            exit (1);
        }
        
        // Output linebuf so we see if there's a line which causes parser to lockup
        fprintf (stdout, "%04d:__", line);
        fprintf (stdout, "%s", linebuf);
        
        sf65ParsingData -> directive_detected = 0;
        sf65ParsingData -> mnemonic_detected = 0;
        sf65ParsingData -> current_column = 0;
        sf65ParsingData -> label_detected = 0;

        sf65ParsingData -> additional_linefeed = 0;
        sf65ParsingData -> first_expression = 1;
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
        while (true) {
            if (*p1 == 0 || (p1 - linebuf) >= allocation) {
                fputc ('\n', output);
                break;
            }

            p1 = skipWhiteSpace (p1);

            p2 = detectCodeWord (p1);
            if (p2 == p1) {
                p2 = detectOperand (p1);
            }
            
            sf65ParsingData -> flags = 0;
            sf65ParsingData -> prev_expr = currentExpr;
            sf65ParsingData -> instant_additional_linefeed = false;
            
            currentExpr  = sf65DetermineExpression(p1, p2, sf65ParsingData, sf65Options);
            
            if ( currentExpr.exprType == SF65_COMMENT) {   /* Comment */
                // Get x position for output of comment
                sf65ParsingData -> request = 
                    getCommentSpacing (p, p1, sf65ParsingData);

                // Indent by level times tab width
                if (sf65ParsingData -> request == sf65Options -> start_mnemonic) {
                    sf65ParsingData -> request += 
                        sf65ParsingData -> current_level * 
                        sf65Options -> nesting_space;
                }

                request_space (output, &sf65ParsingData -> current_column, 
                                        sf65ParsingData -> request, 1, sf65Options -> tabs);

                fwrite (p1, sizeof (char), allocation - (p1 - p), output);

                //When comment if found, rest of line is also comment. So proceed to next line
                break;
            }

            switch (currentExpr.exprType){
                case SF65_MNEMONIC: {
                    sf65_PlaceMnemonicInLine(p1, p2, sf65Options, sf65ParsingData);
                    break;
                }
                case SF65_DIRECTIVE: {
                    sf65_PlaceDirectiveInLine(p1, p2, sf65Options, sf65ParsingData);
                    conditionallyAddPaddingLineAfterSection(sf65Options, sf65ParsingData);
            
                    break;
                }
                case SF65_OPERAND: {
                    sf65_PlaceOperandInLine(p1, p2, sf65Options, sf65ParsingData);
                    sf65ParsingData -> operand_detected = 1;
                    break;
                }
                case SF65_LABEL: {
                    sf65ParsingData -> request = 0;
                    sf65ParsingData -> flags = DONT_RELOCATE;
                    
                    //Check, if p2 already at end of line
                    //Then additional cr is not needed
                    if (allocation > p2-p){
                        if (sf65Options -> oversized_labels_own_line){
                            if ( p2-p1 >= sf65Options -> start_mnemonic){
                                sf65ParsingData -> instant_additional_linefeed = true;
                            }
                        }
                        if (sf65Options -> labels_own_line){
                            sf65ParsingData -> instant_additional_linefeed = true;
                        }
                    }
                    break;
                }
                default: {
                    if (*p1 == ','){
                        sf65ParsingData -> request = 
                            (sf65ParsingData -> current_column + 
                             1)
                            / sf65Options -> nesting_space *
                            sf65Options -> nesting_space;
                    }else{
                        sf65ParsingData -> request = 0;
                    }
                }
            }
            
            conditionallyAddPaddingLineBeforeSection(sf65Options, sf65ParsingData);
            sf65_correctOutputColumnForFlags(sf65ParsingData, sf65Options);
            
            // Indent by level times tab width
            if (sf65ParsingData -> flags != DONT_RELOCATE) sf65ParsingData -> request += sf65ParsingData -> current_level * sf65Options -> nesting_space;

            // Add filling spaces for alignment
            if (*p1 != ',') 
                request_space (output, &sf65ParsingData -> current_column, sf65ParsingData -> request, 1, sf65Options -> tabs);

            // Write current term to output file
            fwrite (p1, sizeof (char), p2 - p1, output);
            //conditionallyAddPaddingLineAfterSection(sf65Options, sf65ParsingData);
            if (sf65ParsingData -> instant_additional_linefeed){
                fputc('\n', output);
                //sf65ParsingData -> current_column = 0;
            }
            // Increase current_column by length of current term
            sf65ParsingData -> current_column += p2 - p1;

            // Set pointer p1 to the end of the expression+1 to proceed further
            p1 = p2;
            
            sf65ParsingData -> first_expression = false;
        }
        ++line;
        conditionallyInsertAdditionalLinefeed(sf65ParsingData);
        
    }

    fclose (input);
    fclose (output);
    exit (0);
}
