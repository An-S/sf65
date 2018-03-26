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
    int c, line = 0;

    char linebuf[1000];

    char *p;
    char *p1;
    char *p2;
    int allocation;

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
        // Output linebuf so we see if there's a line which causes parser to lockup
        fprintf (stdout, "%04d:__", line);
        fprintf (stdout, "%s\n", linebuf);

        // Set pointer p1 to start of line
        p1 = p = linebuf;

        // Get length of current line, just read
        allocation = strlen (linebuf);

        // If linebuf contains not more than a newline and a termination character, process next line
        if (allocation < 2) {
            fputc ('\n', output);
            continue;
        }

        if (linebuf[allocation - 1] != '\n') {
            fprintf (stdout, "Line too long");
            exit (1);
        }

        sf65ParsingData -> directive_detected = 0;
        sf65ParsingData -> mnemonic_detected = 0;
        sf65ParsingData -> current_column = 0;
        sf65ParsingData -> label_detected = 0;

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

            if (sf65ParsingData -> directive_detected) {
                fputc (' ', output);
                ++sf65ParsingData -> current_column;
            }

            if (*p1 == ';') {   /* Comment */
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

            p2 = detectCodeWord (p1);
            if (p2 == p1) {
                p2 = detectOperand (p1);
            }

            sf65ParsingData -> flags = 0;

            if (*p1 == '_' || *p1 == '.' || isalnum (*p1)) {
                // p1 points to start of codeword, p2 be moved to end of word
                c = detectOpcode (p1, p2, sf65Options -> processor, &sf65ParsingData -> request, &sf65ParsingData -> flags);

                // Use p3 to iterate over codeword and eventually change case
                
                switch (sgn(c)){
                case -1:
                    sf65_PlaceMnemonicInLine(p1, p2, sf65Options, sf65ParsingData);
                    break;
                case 1:
                    sf65_PlaceDirectiveInLine(p1, p2, sf65Options, sf65ParsingData);
                    break;
                default:
                    if (sf65ParsingData -> mnemonic_detected) {
                        //Previous term was mnemonic -> assume operand, here
                        sf65_PlaceOperandInLine(p1, p2, sf65Options, sf65ParsingData);
                        c = -1;
                    }else if (sf65ParsingData -> directive_detected){
                        sf65ParsingData -> directive_detected = 0;
                        c = 1;
                    }else if (sf65ParsingData -> label_detected == 0){
                        // If label has not been detected in line, it may still be possible
                        // that current term is a label
                        if ( (*p1 == '_' || isalpha (*p1)) ) {
                            // If term starts with valid label characters, then assume label
                            sf65ParsingData -> request = 0;
                            sf65ParsingData -> label_detected = 1;
                        }else{
                            //sf65_PlaceOperandInLine(p1, p2, sf65Options, sf65ParsingData);
                            sf65ParsingData -> request = 0;
                        }
                    } else{
                        sf65ParsingData -> request = 0;
                    }   
                    break;
                }
            } else {
                // Neither label, nor directive, nor mnemonic
                // Catch terms starting with non alnum characters 
                if (sf65ParsingData -> mnemonic_detected) {
                    sf65_PlaceOperandInLine(p1, p2, sf65Options, sf65ParsingData);
                    c = -1;
                } else {
                    sf65ParsingData -> request = 0;
                }

            }

            sf65_correctOutputColumnForFlags(sf65ParsingData, sf65Options);
            
            // Indent by level times tab width
            if (c != 0) sf65ParsingData -> request += sf65ParsingData -> current_level * sf65Options -> nesting_space;

            // Add filling spaces for alignment
            request_space (output, &sf65ParsingData -> current_column, sf65ParsingData -> request, 1, sf65Options -> tabs);

            // Write current term to output file
            fwrite (p1, sizeof (char), p2 - p1, output);

            // Increase current_column by length of current term
            sf65ParsingData -> current_column += p2 - p1;

            // Set pointer p1 to the end of the expression+1 to proceed further
            p1 = p2;
        }
        ++line;
    }

    fclose (input);
    fclose (output);
    exit (0);
}
