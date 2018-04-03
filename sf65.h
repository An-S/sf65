#ifndef __SF65_H__
#define __SF65_h__

#define VERSION "v0.2"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "sf65_types.h"



/*
 * Globals
 */
extern FILE *input;
extern FILE *output;

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
FILE *sf65_openInputFile ( char *filename );

/*
 * Open output file (formatted source), check error
 */
FILE *sf65_openOutputFile ( char *filename );


/* ************************************************************
 * Prototypes for functions dealing with command line args
 * ************************************************************
 */
int processCMDArgs ( int argc, char **argv, sf65Options_t *CMDOptions );
void sf65_pError ( char *format, ... );

/* ************************************************************
 * Prototypes for parsing functions
 * ************************************************************
 */

void sf65_initializeParser ( sf65ParsingData_t *ParserData );

/*
 * Evaluate expression with start at p1 and end at p2. Return type of expression and index into mnemonic or
 * directive list where appropriate. Else index will be set to 0
 */
sf65Expression_t sf65DetermineExpression ( char *p1, char *p2, sf65ParsingData_t *pData, sf65Options_t *pOpt );

/*
 * These functions help with empty padding lines before and after indented sections of code
 */
void conditionallyAddPaddingLineBeforeSection ( sf65Options_t *CMDOptions, sf65ParsingData_t *ParserData );
void conditionallyAddPaddingLineAfterSection ( sf65Options_t *CMDOptions, sf65ParsingData_t *ParserData );
void conditionallyInsertAdditionalLinefeed ( sf65ParsingData_t *ParserData );

int sf65_align ( int val, int align );
char *readUntilClosingQuote ( char *p );

/*
 * Evaluate flag belonging to certain assembler directive
 * Depend on flag, increase/decrease indent or dismiss indent
 */
void sf65_correctOutputColumnForFlags ( sf65ParsingData_t *pData, const sf65Options_t *options );

/*
 * This function sets correct case for mnemonic and sets requested x position to start_mnemonic
 * It also indicates that a mnemonic was found and clears the directive found flag
 */
void sf65_PlaceMnemonicInLine ( char *p1, char *p2, sf65Options_t *CMDOptions,
                                sf65ParsingData_t *ParserData );

/*
 * This function sets correct case for directive and sets requested x position to start_directive
 * It also indicates that a directive was found and clears the mnemonic found flag
 */
void sf65_PlaceDirectiveInLine ( char *p1, char *p2, sf65Options_t *CMDOptions,
                                 sf65ParsingData_t *ParserData );

/*
 * This function sets x position for operand. It evaluates the 3/4 column style flag.
 * If 4 columns then operand is placed at start_operand, else it is placed directly behind mnemonic
 */
void sf65_PlaceOperandInLine ( char *p1, char *p2, sf65Options_t *CMDOptions,
                               sf65ParsingData_t *ParserData );


/*
 * Procedure to process command line arguments given to sf65
 * Fills given struct with values of command line options and/or default values
 */
int processCMDArgs ( int argc, char **argv, sf65Options_t *CMDOptions );


// **************************************************************************************

/*
 * Check for opcode or directive
 * c > 0 -> directive detected
 * c < 0 -> opcode detected
 */
int check_opcode ( char *p1, char *p2 );

/* Detects mnemonic or directive and returns corresponding index, in found.
 * Returns output column by ref.
 * Takes account of the processor flag and the directive flags
 */
int detectOpcode ( char *p1, char *p2, int processor, int *outputColumn, int *flags );

/*
 * Read array pointed to by p as long as whitespace is found.
 * Stop at first non whitespace character or string terminator
 */
char *skipWhiteSpace ( char *p );

/*
 * Iterate over char array from p1 to p2.
 * Call modificator function for each of the chars of the array
 * and write back modificated char.
 */
char *modifyChars ( char *p1, char *p2, int func ( int ) );

/*
 * Iterate over char array from p1 to p2.
 * Dependent on the value of the _case parameter
 * leave array as is(0), change to uppercase(2), change to lowercase(1)
 */
char *changeCase ( char *p1, char *p2, char _case );

/*
 * Detect a word limited by whitespace but always stop at comment symbol ';'
 */
char *detectCodeWord ( char *p );

char *detectOperand ( char *p );

/*
 * Comparison without case
 */
int memcmpcase ( char *p1, char *p2, int size );

/*
** Request space in line
*/
int request_space ( FILE *output, int *current, int new, int force, int tabs );

/* Tests, if a pointer is in range between a start pointer and and end pointer
 * defined by the size of the range
 */
bool inRange ( const char *p, const char *first, int size );

/*
 * Starts of the memory location pointed to by *data
 * Processes allocation bytes. If newline is found, replace by \0.
 * Carriage return characters are dismissed from the output.
 */
char *convertLinefeedsToStringSeparator ( char *data, int allocation );


int getCommentSpacing ( char *p /*linestart*/, char *p1 /*commentstart*/, sf65ParsingData_t *pData );

int sgn ( int x );
#endif
