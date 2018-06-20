#include "sf65.h"

/*
 * Reads characters from input string until a non white-space character or line-end or '\0'
 * is found. Returns pointer to first non-whitespace char or '\n' or '\0'
 */
char *sf65_SkipWhiteSpace ( char *p ) {
    int n = strspn ( p, " \t\f\v" );
    return p + n;
}

/*
 * Reads characters from input string until a terminating quote is found
 * Returns pointer to the terminating quote
 */
char *sf65_ReadUntilClosingQuote ( char *p ) {
    // Skip quote at p. Find closing quote
    return strpbrk ( p + 1, "\"" );
}

/*
 * Starts reading chars at p1 and calls modificator func with every char until p2 is reached
 * Mimicks a foreach loop of more modern programming languages
 */
char *sf65_ModifyChars ( char *p1, char *p2, int func ( int ) ) {
    char ch;

    while ( p1 < p2 ) {
        ch = *p1;
        echoChar ( ch );
        *p1 = func ( ch );
        ++p1;
    }
    return p1;
}

/*
 * Modify case of chars between p1 and p2. The desired case can be given as third arg.
 * 0: Leave case unchanged, 1: tolower, 2: toupper
 */
char *sf65_ChangeCase ( char *p1, char *p2, sf65Case_t _case ) {
    switch ( _case ) {
    case SF65_LOWERC:
        sf65_ModifyChars ( p1, p2, tolower );
        break;
    case SF65_UPPERC:
        sf65_ModifyChars ( p1, p2, toupper );
        break;
    default:
        break;
    }
    return p2;
}

/*
 * Compares substrings of certain size starting at p1 and p2 without case
 * Returns 0, if substrings considered equal and 1 if they are considered different
*/
sf65StrEq_t sf65_Memcmpcase ( char *p1, char *p2, int size ) {
    while ( size-- ) {
        if ( tolower ( *p1 ) != tolower ( *p2 ) )
            return SF65_STRNOTEQ;
        p1++;
        p2++;
    }

    return SF65_STREQ;
}
