#ifndef __SF65_EXPRPARSE_H__
#define __SF65_EXPRPARSE_H__

/*
 * Array which holds the names of the mnemonics of the 6502 processor
 * Unintended opcodes are included
 */
extern char *mnemonics_6502[];

/*
 * Array holding a list of directives for ca65 and corresponding output flags
 */
extern directives_t directives_dasm[];

/* ************************************************************
 * Prototypes for parsing functions
 * ************************************************************
 */

/*
 * Sets default values for parser, before entering reading of lines
 */
void sf65_InitializeParser ( sf65ParsingData_t *ParserData );

/*
 * Sets default values for parser, but only those which have to be reset at
 * the start of a line
 */
void sf65_StartParsingNewLine ( sf65ParsingData_t *pData );

/*
 * Evaluate expression with start at p1 and end at p2. Return type of expression and index into mnemonic or
 * directive list where appropriate. Else index will be set to 0
 */
sf65Expression_t *sf65DetermineExpression ( char *p1, char *p2, sf65ParsingData_t *pData, sf65Options_t *pOpt );
sf65Err_t sf65_InitExpressionDetermination ( sf65ParsingData_t *pData );




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
 * Detect a word limited by whitespace but always stop at comment symbol ';'
 */
char *detectCodeWord ( char *p );

char *detectOperand ( char *p );

bool isExpressionCharacter ( char ch );

extern char *sf65StrExprTypes[];
#endif
