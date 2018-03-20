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
 * Alignment of .proc/.scope/.endproc/.endscope is not satisfying
 * Mnemonics/directives after labels are recognized as operands
 * (However, comments are correctly recignized and aligned with mnemonics)
 * Labels are indented with section(.proc, .scope ...) directives
 */

/* MISSING FEATURES
 *
 * Colons cannot be added/removed from labels
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "sf65.h"

int tabs;

int style;
int processor;
int start_mnemonic;
int start_operand;
int start_comment;
int start_directive;
int align_comment;
int nesting_space;
int labels_own_line;
int prev_comment_original_location;
int prev_comment_final_location;
int mnemonics_case;
int directives_case;



/*
** Main program
*/
int main (int argc, char *argv[]) {
    int c, line = 0;

    FILE *input;
    FILE *output;
    char *data;
    char linebuf[200];
    
    char *p;
    char *p1;
    char *p2;
    int allocation;
    int current_column;
    int request;
    int current_level;

    int flags;
    
    bool label_detected;
    bool mnemonic_detected;
    bool comment_detected;
    bool directive_detected;
    
    c = processCMDArgs(argc, argv);


    /*
    ** Open input file, measure it and read it into buffer
    */
    input = fopen (argv[c], "rb");
    if (input == NULL) {
        fprintf (stderr, "Unable to open input file: %s\n", argv[c]);
        exit (1);
    }
    fprintf (stderr, "Processing %s...\n", argv[c]);
    fseek (input, 0, SEEK_END);
    allocation = ftell (input);
    data = malloc (allocation + sizeof (char));
    if (data == NULL) {
        fprintf (stderr, "Unable to allocate memory\n");
        fclose (input);
        exit (1);
    }
    fseek (input, 0, SEEK_SET);

    // Read file at once into data
    /*if (fread (data, sizeof (char), allocation, input) != allocation) {
        fprintf (stderr, "Something went wrong reading the input file\n");
        fclose (input);
        free (data);
        exit (1);
    }
    fclose (input);
    */
    /*
    ** Ease processing of input file
    */
    request = 0;

    //@Todo: Is this code which only task is to remove \r chars really necessary?

    //p = convertLinefeedsToStringSeparator(data, allocation);
    //allocation = p - data; // Correct size of data by using p2

    /*
    ** Now generate output file
    */
    c++;
    output = fopen (argv[c], "w");
    if (output == NULL) {
        fprintf (stderr, "Unable to open output file: %s\n", argv[c]);
        exit (1);
    }
    prev_comment_original_location = 0;
    prev_comment_final_location = start_mnemonic;
    current_level = 0;

    /*
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
    // Init p with start of data
    p = data;
    fprintf(stdout, "%4d:", line);
    
    // Loop until p out of range
    // p points to line start
    
    while ( fgets(linebuf, sizeof(linebuf), input ), !feof(input) ) {
        // Set pointer p1 to start of line, which p points to
        p1 = linebuf;
        
        // Loop over all chars in a line
        while(true){
            p1 = skipWhiteSpace(p1);
            if ( '\0' == *p1 ) {
                ++line;
                fputc ('\n', output);
                
                fputc ('\n', stdout);
                fprintf(stdout, "%04d:____", line);
    
                break;
            }
            
            p2 = detectCodeWord(p1);
            
            if (*p1 == ';') {   /* Comment */
                while (*p2++); // Increase expr end ptr to end of line
                --p2;
                
                /*
                ** Try to keep comments horizontally aligned (only works
                ** if spaces were used in source file)
                */
                //p2 = p1;
                //while (p2 - 1 >= p && isspace (* (p2 - 1)))
                //    p2--;
                
                
                if (p1 - p == prev_comment_original_location) {
                    request = prev_comment_final_location;
                } else {
                    prev_comment_original_location = p1 - p;
                
                    if (current_column == 0)
                        request = 0;
                    else if (current_column < start_mnemonic)
                        request = start_mnemonic;
                    else
                        request = start_comment;
                    if (current_column == 0 && align_comment == 1)
                        request = start_mnemonic;
                    prev_comment_final_location = request;
                }

                // Indent by level times tab width
                request += current_level * nesting_space;

                request_space (output, &current_column, request, 0, tabs);
                //p2 = p1;
                //while (*p2)
                 //   p2++;
                //while (p2 > p1 && isspace (* (p2 - 1)))
                 //   p2--;
                

                
                fwrite (p1, sizeof (char), p2 - p1, output);
                fputc ('\n', output);
                //current_column += p2 - p1;
            }
            p1 = p2+1;
        }
        p = p1;
        ++p; //Always inc to avoid dead locking on reading chars again and again
    }
//        
    //exit(0);
//    
//        current_column = 0;
//
//        // Loop over all chars which are whitespace but not \0
//        p1 = skipWhiteSpace(p1);
//        if ( !(*p1)) {
//            ++p1;
//            fputc ('\n', output);
//            continue;
//        }
//        
//        p2 = detectCodeWord(p1);
//        
//        //p1 points to first char other than space or \0 after start of line or end of label
//        //p2 point to end of word
//        flags = 0;
//        
//        if (*p1 == ';') {   /* Comment */
//            while (*p2++) // Increase expr end ptr to end of line
//            
//            /*
//            ** Try to keep comments horizontally aligned (only works
//            ** if spaces were used in source file)
//            */
//            //p2 = p1;
//            //while (p2 - 1 >= p && isspace (* (p2 - 1)))
//            //    p2--;
//            
//            
//            if (p2 == p && p1 - p == prev_comment_original_location) {
//                request = prev_comment_final_location;
//            } else {
//                prev_comment_original_location = p1 - p;
//            
//                if (current_column == 0)
//                    request = 0;
//                else if (current_column < start_mnemonic)
//                    request = start_mnemonic;
//                else
//                    request = start_comment;
//                if (current_column == 0 && align_comment == 1)
//                    request = start_mnemonic;
//                prev_comment_final_location = request;
//            }
//
//            // Indent by level times tab width
//            request += current_level * nesting_space;
//
//            request_space (output, &current_column, request, 0, tabs);
//            //p2 = p1;
//            //while (*p2)
//             //   p2++;
//            //while (p2 > p1 && isspace (* (p2 - 1)))
//             //   p2--;
//
//            
//            
//            fwrite (p1, sizeof (char), p2 - p1, output);
//            fputc ('\n', output);
//            //current_column += p2 - p1;
//            p1 = p2;
//            continue;  // Proceed with next line without further parsing of this line
//        }
//        //while (*p1 && *p1 != ';') {    /* Mnemonic or directive*/
//        if (*p1 =='_' || isalpha(*p1)){
//            // p1 points to start of codeword, p2 be moved to end of word
//            c = detectOpcode(p1, p2, processor, &request, &flags);
//
//            // Use p3 to iterate over codeword and eventually change case
//            if (c < 0) {
//                changeCase(p1, p2, mnemonics_case);
//            } else if ( c > 0) {
//                changeCase(p1, p2, directives_case);
//            } else{
//                //Label
//                request = 0;
//                label_detected = 1;
//            }
//        }else{
//            request = start_operand;
//        }
//        
//        /*
//        ** Move label to own line
//        */
//        if (current_column != 0 && labels_own_line != 0 && (flags & DONT_RELOCATE_LABEL) == 0) {
//            fputc ('\n', output);
//            current_column = 0;
//        }
//        
//        if (flags & LEVEL_IN) {
//            current_level++;
//            request = start_mnemonic-4;
//        }
//        
//        if (flags & LEVEL_OUT) {
//            if (current_level > 0)
//                current_level--;
//            request = start_mnemonic;
//        }
//
//        // Indent by level times tab width
//        if ( c != 0 ) request += current_level * nesting_space;
//
//        // Unindent by one level
//        if (flags & LEVEL_MINUS)
//            if (request > nesting_space) request -= nesting_space;
//        
//        // Insert space into output dependent on indent
//        request_space (output, &current_column, request, 0, tabs);
//        if (p2-p1) fwrite (p1, sizeof (char), p2 - p1, output);
//
//        current_column += p2 - p1;
//
//            
//            //p1 = skipWhiteSpace(p1);
//
//            // It is wrong to assume operand, here. Could aswell be mnemonic or directive
//            if (*p1 && *p1 != ';') {    /* Operand */
//                request = start_operand;
//                request += current_level * nesting_space;
//                request_space (output, &current_column, request, 1, tabs);
//                p2 = p1;
//
//                // Detect argument types:
//                // - Immediate addressing with #
//                // - Absolute addressing with $
//                // - char argument with " or '
//                while (*p2 && *p2 != ';') {
//                    if (*p2 == '"') {
//                        p2++;
//                        while (*p2 && *p2 != '"')
//                            p2++;
//                        p2++;
//                    } else if (*p2 == '\'') {
//                        p2++;
//                        while (*p2 && *p2 != '"')
//                            p2++;
//                        p2++;
//                    } else if (*p2 == '$') {
//                        p2++;
//                        while (*p2 && !isspace(*p2) )
//                            p2++;
//                        //p2++;
//                    } else if (*p2 == '#') {
//                        p2++;
//                        while (*p2 && *p2 != '"')
//                            p2++;
//                        //p2++;
//                    } else {
//                        p2++;
//                    }
//                }
//                while (p2 > p1 && isspace (* (p2 - 1)))
//                    p2--;
//                fwrite (p1, sizeof (char), p2 - p1, output);
//                current_column += p2 - p1;
//                p1 = p2;
//                while (*p1 && isspace (*p1))
//                    p1++;
//            }

        //}
        
        //while (*p++) ;
    //}
    fclose (output);
    free (data);
    exit (0);
}


int processCMDArgs(int argc, char** argv){
    /*
    ** Show usage if less than 3 arguments (program name counts as one)
    */
    int c = 0;

    if (argc < 3) {
        fprintf (stderr, "\n");
        fprintf (stderr, "Pretty6502 " VERSION " by Oscar Toledo G. http://nanochess.org/\n");
        fprintf (stderr, "\n");
        fprintf (stderr, "Usage:\n");
        fprintf (stderr, "    pretty6502 [args] input.asm output.asm\n");
        fprintf (stderr, "\n");
        fprintf (stderr, "DON'T USE SAME OUTPUT FILE AS INPUT, though it's possible,\n");
        fprintf (stderr, "you can DAMAGE YOUR SOURCE if this program has bugs.\n");
        fprintf (stderr, "\n");
        fprintf (stderr, "Arguments:\n");
        fprintf (stderr, "    -s0       Code in four columns (default)\n");
        fprintf (stderr, "              label: mnemonic operand comment\n");
        fprintf (stderr, "    -s1       Code in three columns\n");
        fprintf (stderr, "              label: mnemonic+operand comment\n");
        fprintf (stderr, "    -p0       Processor unknown\n");
        fprintf (stderr, "    -p1       Processor 6502 + DASM syntax (default)\n");
        fprintf (stderr, "    -m8       Start of mnemonic column (default)\n");
        fprintf (stderr, "    -o16      Start of operand column (default)\n");
        fprintf (stderr, "    -c32      Start of comment column (default)\n");
        fprintf (stderr, "    -t8       Use tabs of size 8 to reach column\n");
        fprintf (stderr, "    -t0       Use spaces to align (default)\n");
        fprintf (stderr, "    -a0       Align comments to nearest column\n");
        fprintf (stderr, "    -a1       Comments at line start are aligned\n");
        fprintf (stderr, "              to mnemonic (default)\n");
        fprintf (stderr, "    -n4       Nesting spacing (can be any number\n");
        fprintf (stderr, "              of spaces or multiple of tab size)\n");
        fprintf (stderr, "    -l        Puts labels in its own line\n");
        fprintf (stderr, "    -dl       Change directives to lowercase\n");
        fprintf (stderr, "    -du       Change directives to uppercase\n");
        fprintf (stderr, "    -ml       Change mnemonics to lowercase\n");
        fprintf (stderr, "    -mu       Change mnemonics to uppercase\n");
        fprintf (stderr, "\n");
        fprintf (stderr, "Assumes all your labels are at start of line and there is space\n");
        fprintf (stderr, "before mnemonic.\n");
        fprintf (stderr, "\n");
        fprintf (stderr, "Accepts any assembler file where ; means comment\n");
        fprintf (stderr, "[label] mnemonic [operand] ; comment\n");
        exit (1);
    }

    /*
    ** Default settings
    */
    style = 0;
    processor = 1;
    start_mnemonic = 8;
    start_operand = 16;
    start_comment = 32;
    start_directive = 0;//7;
    tabs = 0;
    align_comment = 1;
    nesting_space = 4;
    labels_own_line = 0;
    mnemonics_case = 0;
    directives_case = 0;
    prev_comment_final_location = 4;
    
    /*
    ** Process arguments
    */
    c = 1;
    while (c < argc - 2) {
        if (argv[c][0] != '-') {
            fprintf (stderr, "Bad argument\n");
            exit (1);
        }
        switch (tolower (argv[c][1])) {
            case 's':   /* Style */
                style = atoi (&argv[c][2]);
                if (style != 0 && style != 1) {
                    fprintf (stderr, "Bad style code: %d\n", style);
                    exit (1);
                }
                break;
            case 'p':   /* Processor */
                processor = atoi (&argv[c][2]);
                if (processor < 0 || processor > 1) {
                    fprintf (stderr, "Bad processor code: %d\n", processor);
                    exit (1);
                }
                break;
            case 'm':   /* Mnemonic start */
                if (tolower (argv[c][2]) == 'l') {
                    mnemonics_case = 1;
                } else if (tolower (argv[c][2]) == 'u') {
                    mnemonics_case = 2;
                } else {
                    start_mnemonic = atoi (&argv[c][2]);
                }
                break;
            case 'o':   /* Operand start */
                start_operand = atoi (&argv[c][2]);
                break;
            case 'c':   /* Comment start */
                start_comment = atoi (&argv[c][2]);
                break;
            case 't':   /* Tab size */
                tabs = atoi (&argv[c][2]);
                break;
            case 'a':   /* Comment alignment */
                align_comment = atoi (&argv[c][2]);
                if (align_comment != 0 && align_comment != 1) {
                    fprintf (stderr, "Bad comment alignment: %d\n", align_comment);
                    exit (1);
                }
                break;
            case 'n':   /* Nesting space */
                nesting_space = atoi (&argv[c][2]);
                break;
            case 'l':   /* Labels in own line */
                labels_own_line = 1;
                break;
            case 'd':   /* Directives */
                if (tolower (argv[c][2]) == 'l') {
                    directives_case = 1;
                } else if (tolower (argv[c][2]) == 'u') {
                    directives_case = 2;
                } else {
                    fprintf (stderr, "Unknown argument: %c%c\n", argv[c][1], argv[c][2]);
                }
                break;
            default:    /* Other */
                fprintf (stderr, "Unknown argument: %c\n", argv[c][1]);
                exit (1);
        }
        c++;
    }

    /*
    ** Validate constraints
    */
    if (style == 1) {
        if (start_mnemonic > start_comment) {
            fprintf (stderr, "Operand error: -m%d > -c%d\n", start_mnemonic, start_comment);
            exit (1);
        }
        start_operand = start_mnemonic;
    } else if (style == 0) {
        if (start_mnemonic > start_operand) {
            fprintf (stderr, "Operand error: -m%d > -o%d\n", start_mnemonic, start_operand);
            exit (1);
        }
        if (start_operand > start_comment) {
            fprintf (stderr, "Operand error: -o%d > -c%d\n", start_operand, start_comment);
            exit (1);
        }
    }
    if (tabs > 0) {
        if (start_mnemonic % tabs) {
            fprintf (stderr, "Operand error: -m%d isn't a multiple of %d\n", start_mnemonic, tabs);
            exit (1);
        }
        if (start_operand % tabs) {
            fprintf (stderr, "Operand error: -m%d isn't a multiple of %d\n", start_operand, tabs);
            exit (1);
        }
        if (start_comment % tabs) {
            fprintf (stderr, "Operand error: -m%d isn't a multiple of %d\n", start_comment, tabs);
            exit (1);
        }
        if (nesting_space % tabs) {
            fprintf (stderr, "Operand error: -n%d isn't a multiple of %d\n", nesting_space, tabs);
            exit (1);
        }
    }

    return c;
}
