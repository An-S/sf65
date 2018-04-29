#ifndef __SF65_EXPRESSION_EVAL_H__
#define __SF65_EXPRESSION_EVAL_H__

/*
 * This function does expression evaluation in contrast to expression determination.
 * The determination of the expression type has to be done beforehand.
 * This functions sets/resets flags for output
 */
char *sf65_EvaluateExpression ( sf65ParsingData_t *ParserData,
                                sf65Options_t *CMDOpt, char *p1, char *p2 );

#endif
