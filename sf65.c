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
 */

/* MISSING FEATURES
 *
 * Colons cannot be added/removed from labels
 */

#include "sf65.h"

//Create instance of sf65Options_t, but do not use directly 
//(Avoid replacement of -> by . or -> by .)
sf65Options_t _sf65Options;

//Create pointer to instance of sf65Options_t, so we can use -> throughout
sf65Options_t *sf65Options = &_sf65Options;

int prev_comment_original_location;
int prev_comment_final_location;

/*
** Main program
*/
int main (int argc, char *argv[]) {
    int c, line = 0;

    FILE *input;
    FILE *output;
    char linebuf[200];

    char *p;
    char *p1;
    char *p2;
    int allocation;

    int request;
    int current_column;
    int current_level;
    
    int flags;

    bool label_detected;
    bool mnemonic_detected;
    bool comment_detected;
    bool directive_detected;

    c = processCMDArgs(argc, argv, sf65Options);

    // Try to open input file. Procedure exits in case of error. 
    // No further err checking necessary 
    input = sf65_openInputFile(sf65Options -> infilename);
    
    // Tell user that processing of input file is about to be started
    fprintf (stderr, "Processing %s...\n", argv[c]);
    
    /*
    ** Now generate output file
    */
    c++;
    
    // Try to open output file. Procedure exits in case of error. 
    // No further err checking necessary 
    output = sf65_openOutputFile(sf65Options -> outfilename);

    // Start with debug output (Line number of 0)
    fprintf(stdout, "%4d:", line);
    
    request = 0;
    prev_comment_original_location = 0;
    prev_comment_final_location = sf65Options -> start_mnemonic;
    current_level = 0;
    
    // Read lines from input until EOF
    // Pointer p is set to start of line for easier parsing (using p instead of linebuf all the time)
    while ( fgets(linebuf, sizeof(linebuf), input ), !feof(input) ) {
        // Output linebuf so we see if there's a line which causes parser to lockup
        fprintf(stdout, "%04d:__", line);
        fprintf(stdout, "%s\n", linebuf);

        // Set pointer p1 to start of line
        p1 = p = linebuf;

        // Get length of current line, just read
        allocation = strlen(linebuf);

        // If linebuf contains not more than a newline and a termination character, process next line
        if (allocation < 2) {
            fputc('\n', output);
            continue;
        }

        if (linebuf[allocation-1] != '\n'){
            fprintf(stdout, "Line too long");
            exit(1);
        }

        directive_detected = 0;
        mnemonic_detected = 0;
        current_column = 0;
        label_detected = 0;
        
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
        while( true ){
            if (*p1 == 0 || (p1 - linebuf) >= allocation){
                fputc('\n', output);
                break;
            }

            p1 = skipWhiteSpace(p1);

            if (directive_detected){
                fputc(' ', output);
                ++current_column;
            }

            if (*p1 == ';') {   /* Comment */
                // Get x position for output of comment
                request = getCommentSpacing(p, p1, current_column);

                // Indent by level times tab width
                if (request == sf65Options -> start_mnemonic){
                    request += current_level * sf65Options -> nesting_space;
                }

                request_space (output, &current_column, request, 1, sf65Options -> tabs);

                fwrite (p1, sizeof (char), allocation-(p1-p), output);

                //When comment if found, rest of line is also comment. So proceed to next line
                break;
            }

            p2 = detectCodeWord(p1);
            if (p2 == p1){
                p2 = detectOperand(p1);
            }

            flags = 0;

            if (*p1 =='_' || *p1 == '.' || isalnum(*p1)){
                // p1 points to start of codeword, p2 be moved to end of word
                c = detectOpcode(p1, p2, sf65Options -> processor, &request, &flags);

                // Use p3 to iterate over codeword and eventually change case
                if (c < 0) {
                    changeCase(p1, p2, sf65Options -> mnemonics_case);
                    request = sf65Options -> start_mnemonic;
                    mnemonic_detected = 1;
                } else if ( c > 0) {
                    changeCase(p1, p2, sf65Options -> directives_case);
                    request = sf65Options -> start_directive;
                    directive_detected = 1;
                } else{
                    //Label
                    if ( ( *p1 == '_' || isalpha(*p1) ) && label_detected == 0 ){
                        request = 0;
                        label_detected = 1;
                    }else{
                        if (mnemonic_detected){
                            request = sf65Options -> start_operand;
                        }else{
                            request = 0;
                        }
                    }
                }
            }else{
                if (mnemonic_detected){
                    request = sf65Options -> start_operand;
                    mnemonic_detected = 0;
                }else{
                    request = 0;
                }

            }

            if (current_column != 0 && sf65Options -> labels_own_line != 0 && (flags & DONT_RELOCATE_LABEL) == 0) {
                fputc ('\n', output);
                current_column = 0;
            }

            if (flags & LEVEL_IN) {
                current_level++;
                request = sf65Options -> start_mnemonic-4;
            }

            if (flags & LEVEL_OUT) {
                if (current_level > 0)
                    current_level--;
                request = sf65Options -> start_mnemonic;
            }

            if (flags & ALIGN_MNEMONIC){
                request = sf65Options -> start_mnemonic;
            }

            // Indent by level times tab width
            if ( c != 0 ) request += current_level * sf65Options -> nesting_space;

            // Unindent by one level
            if (flags & LEVEL_MINUS)
                if (request > sf65Options -> nesting_space) request -= sf65Options -> nesting_space;

            // Add filling spaces for alignment
            request_space (output, &current_column, request, 1, sf65Options -> tabs);

            // Write current term to output file
            fwrite (p1, sizeof (char), p2-p1, output);

            // Increase current_column by length of current term
            current_column += p2-p1;

            // Set pointer p1 to the end of the expression+1 to proceed further
            p1 = p2;
        }
        ++line;
    }

    fclose (input);
    fclose (output);
    exit (0);
}
