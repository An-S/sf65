#ifndef __SF65_STRINGFUNC_h__
#define __SF65_STRINGFUNC_h__

/*
 * Reads characters from input string until a terminating quote is found
 * Returns pointer to the terminating quote
 */
char *readUntilClosingQuote ( char *p );

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
char *changeCase ( char *p1, char *p2, sf65Case_t _case );

/*
 * Comparison without case
 */
sf65StrEq_t memcmpcase ( char *p1, char *p2, int size );

#endif
