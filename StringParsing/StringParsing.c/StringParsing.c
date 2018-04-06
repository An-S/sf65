#include "sf65.h"

/*
 * Reads characters from input until a non white-space character or line-end or '\0'
 * is found. Returns pointer to first non-whitespace char or '\n' or '\0'
 */
char *skipWhiteSpace ( char *p ) {
    char ch;
    while ( ch = *p, ch && isspace ( ch ) && ch != '\n' ) {
        //echoChar ( ch );
        ++p;
    }

    // p points to first char after whitespace or to '\0' character
    return p;
}

char *readUntilClosingQuote ( char *p ) {

    // Skip quote at p. Find closing quote
    return strpbrk ( p + 1, "\"" );
}

char *modifyChars ( char *p1, char *p2, int func ( int ) ) {
    char ch;

    while ( p1 < p2 ) {
        ch = *p1;
        //echoChar ( ch );
        *p1 = func ( ch );
        p1++;
    }
    return p1;
}

char *changeCase ( char *p1, char *p2, char _case ) {
    switch ( _case ) {
    case 1:
        modifyChars ( p1, p2, tolower );
        break;
    case 2:
        modifyChars ( p1, p2, toupper );
        break;
    default:
        break;
    }
    return p2;
}

/*
** Comparison without case
*/
int memcmpcase ( char *p1, char *p2, int size ) {
    //    char ch;

    while ( size-- ) {
        if ( tolower ( *p1 ) != tolower ( *p2 ) )
            return 1;
        p1++;
        p2++;
    }

    return 0;
}
