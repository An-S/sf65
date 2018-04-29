#ifndef __SF65_OUTPUTFORM_H__
#define __SF65_OUTPUTFORM_H__

/* This function requests a certain xposition in output
 * It wraps access to request variable in parsing data.
 * It returns the value or -1 on error.
 */
int sf65_SetOutputXPositionInLine ( sf65ParsingData_t *pData, int xpos );

/* This function returns the requested x position to caller
 * or -1 on error
 */
int sf65_GetOutputXPositionInLine ( sf65ParsingData_t *pData );

/*
 * This function adds a relatoive offset to the requested x position
 * and returns the new value to the caller or -1 on error
 */
int sf65_IncOutputXPositionInLine ( sf65ParsingData_t *pData, int add );

/*
 * This function adds a relative offset to the requested output x position
 * and returns the new value to the caller or -1 on error
 */
int sf65_IncOutputXPositionByNestingLevel ( sf65ParsingData_t *pData, int nestingSpace );

/*
 * Adds an increment to the current column counter
 */
int sf65_IncCurrentColumnCounter ( sf65ParsingData_t *pData, int inc );

/*
 * Sets the current column counter back to zero
 */
int sf65_ResetCurrentColumnCounter ( sf65ParsingData_t *pData );

/*
 * Gets the current column counter from parser Data
 */
int sf65_GetCurrentColumnCounter ( sf65ParsingData_t *pData );

/*
 * Sets the current column counter from parser Data
 */
int sf65_SetCurrentColumnCounter ( sf65ParsingData_t *pData, int col );

/*
 * Aligns the current column by tabwidth which was specified by cmd options
 */
int sf65_AlignCurrentColumn ( sf65ParsingData_t *pData, int tabs );

/*
 * Clears a parser flag specified by an enumeration of parser flags
 */
sf65Err_t sf65_ClearParserFlag ( sf65ParsingData_t *pData, sf65ParserFlagsEnum_t flag );

/*
 * Clears the parser flags specified by an va_list of enumerated parser flags
 */
sf65Err_t sf65_ClearParserFlags ( sf65ParsingData_t *pData, sf65ParserFlagsEnum_t flag1, ... );

/*
 * Sets a parser flag specified by an enumeration of parser flags
 */
sf65Err_t sf65_SetParserFlag ( sf65ParsingData_t *pData, sf65ParserFlagsEnum_t flag );

/*
 * Gets a parser flag state or return -1 on error
 */
int sf65_GetParserFlag ( sf65ParsingData_t *pData, sf65ParserFlagsEnum_t flag );

/*
 * Sets the parser flags specified by an va_list of enumerated parser flags
 */
sf65Err_t sf65_SetParserFlags ( sf65ParsingData_t *pData, sf65ParserFlagsEnum_t flag1, ... );

/*
 * Clears all the parser flags
 */
sf65Err_t sf65_ResetParserFlags ( sf65ParsingData_t * pData );

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
int sf65_PadOutputWithSpaces ( FILE * output, sf65ParsingData_t *pData, int tabs );
/*
 * Place comment in line according to scanning position and pData parameters
 */
int getCommentSpacing ( char *linestart, char *commentstart, sf65ParsingData_t *pData );

/*
 * These functions set/reset requested line feed type in output.
 * Linefeed can be DEFAULT, INSTANT_ADDITIONAL, ADDITIONAL
 */
sf65Err_t sf65_SetLinefeedType ( sf65ParsingData_t *pData, sf65LinefeedEnum_t lf_type );
sf65Err_t sf65_ResetLinefeedFlag ( sf65ParsingData_t *pData, sf65LinefeedEnum_t lf_type );

/*
 * These functions set/reset flag for space padding in output, f.e. between
 * mnemonic and operand.
 */
sf65Err_t sf65_SetPaddingSpaceFlag ( sf65ParsingData_t *pData );
sf65Err_t sf65_ClearPaddingSpaceFlag ( sf65ParsingData_t *pData );

#endif
