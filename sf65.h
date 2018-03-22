#ifndef __SF65_H__
#define __SF65_h__

#define VERSION "v0.2"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "sf65_types.h"

#define DONT_RELOCATE_LABEL 0x01
#define LEVEL_IN        0x02
#define LEVEL_OUT       0x04
#define LEVEL_MINUS     0x08
#define ALIGN_MNEMONIC  0x10

extern int prev_comment_original_location;
extern int prev_comment_final_location;

/*
 * Array which holds the names of the mnemonics of the 6502 processor
 * Unintended opcodes are included
 */
extern char *mnemonics_6502[];

/// Instance holding a list of directives for ca65
extern directives_t directives_dasm[];


/* ************************************************************
 * Prototypes for functions dealing with input and output files
 * ************************************************************
 */

/*
 * Open input file (unformatted source), check error
 */
FILE *sf65_openInputFile(char *filename);

/*
 * Open output file (formatted source), check error
 */
FILE *sf65_openOutputFile(char *filename);


/* ************************************************************
 * Prototypes for functions dealing with command line args
 * ************************************************************
 */
int processCMDArgs(int argc, char** argv, sf65Options_t *sf65Options);

/* ************************************************************
 * Prototypes for parsing functions
 * ************************************************************
 */

/*
 * Procedure to process command line arguments given to sf65
 * Fills given struct with values of command line options and/or default values
 */
int processCMDArgs(int argc, char** argv, sf65Options_t *sf65Options);


// **************************************************************************************

/*
 * Check for opcode or directive
 * c > 0 -> directive detected
 * c < 0 -> opcode detected
 */
int check_opcode (char *p1, char *p2);

/* Detects mnemonic or directive and returns corresponding index, in found.
 * Returns output column by ref. 
 * Takes account of the processor flag and the directive flags
 */
int detectOpcode(char *p1, char *p2, int processor, int *outputColumn, int *flags);
 
/*
 * Read array pointed to by p as long as whitespace is found.
 * Stop at first non whitespace character or string terminator
 */
char *skipWhiteSpace(char *p);

/*
 * Iterate over char array from p1 to p2.
 * Call modificator function for each of the chars of the array
 * and write back modificated char.
 */
char *modifyChars(char *p1, char *p2, int func(int));

/*
 * Iterate over char array from p1 to p2.
 * Dependent on the value of the _case parameter
 * leave array as is(0), change to uppercase(2), change to lowercase(1)
 */
char *changeCase(char *p1, char *p2, char _case);

/*
 * Detect a word limited by whitespace but always stop at comment symbol ';'
 */
char *detectCodeWord(char *p);

char *detectOperand(char *p);

/*
 * Comparison without case
 */
int memcmpcase (char *p1, char *p2, int size);

/*
** Request space in line
*/
int request_space (FILE *output, int *current, int new, int force, int tabs);

/* Tests, if a pointer is in range between a start pointer and and end pointer
 * defined by the size of the range
 */
bool inRange(const char *p, const char *first, int size);

/*
 * Starts of the memory location pointed to by *data
 * Processes allocation bytes. If newline is found, replace by \0.
 * Carriage return characters are dismissed from the output.
 */
char *convertLinefeedsToStringSeparator(char* data, int allocation);


int getCommentSpacing(char* linestart, char *commentstart, int current_column);

#endif
