#ifndef __SF65_OUTPUTFORM_H__
#define __SF65_OUTPUTFORM_H__

/* This function requests a certain xposition in output
 * It wraps access to request variable in parsing data.
 */
int sf65_SetOutputXPositionInLine ( sf65ParsingData_t *pData, int xpos );
int sf65_GetOutputXPositionInLine ( sf65ParsingData_t *pData );
int sf65_IncOutputXPositionInLine ( sf65ParsingData_t *pData, int add );

/*
 * These functions help with empty padding lines before and after indented sections of code
 */
void conditionallyAddPaddingLineBeforeSection ( sf65Options_t *CMDOptions, sf65ParsingData_t *ParserData );
void conditionallyAddPaddingLineAfterSection ( sf65Options_t *CMDOptions, sf65ParsingData_t *ParserData );
void conditionallyInsertAdditionalLinefeed ( sf65ParsingData_t *ParserData );

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
** Request space in line
*/
int request_space ( FILE *output, int *current, int new, int force, int tabs );

/*
 * Place comment in line according to scanning position and pData parameters
 */
int getCommentSpacing ( char *linestart, char *commentstart, sf65ParsingData_t *pData );

#endif
