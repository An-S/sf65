#include "sf65.h"

extern sf65Options_t *CMDOptions;

/*
 * Echo a char to stdout instead of outputting to file
 * May be used for debug purposes
 */
char echoChar ( char ch ) {
    //fputc (ch, stdout);
    return ch;
}

/*
 * Determines, if a certain char is a valid expression(statement)
 * character
 */
bool isExpressionCharacter ( char ch ) {
    bool flag;

    if ( ch == '.' || ch == '_' ) {
        flag = true;
    } else {
        flag = ( ch != ';' &&
                 ch != '\'' && ch != '"' &&
                 ch != '#' &&
                 ch != '$' &&
                 ch != '%' &&
                 ch != ',' &&
                 ch != '\\' ) ;
    }
    return flag;
}

char *detectCodeWord ( char *p ) {
    //
    char ch;

    while ( ch = *p, ch && !isspace ( ch ) && isExpressionCharacter ( ch ) ) {
        echoChar ( ch );
        ++p;
    }

    //Rewind pointer to last non delimiting char
    //--p;
    return p;
}

char *detectOperand ( char *p ) {
    //
    char ch;

    while ( ch = *p, ch && !isspace ( ch ) && !isExpressionCharacter ( ch ) ) {
        echoChar ( ch );
        ++p;
    }

    //Rewind pointer to last non delimiting char
    //--p;
    return p;
}


