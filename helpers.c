#include "sf65.h"
#include "stdlib.h"
#include "ctype.h"

char *skipWhiteSpace(char *p){
    while (*p && isspace (*p))
        p++;

    return p;
}

char *modifyChars(char *p1, char *p2, int func(int)){
    while (p1 < p2) {
        *p1 = func (*p1);
        p1++;
    }
    return p1;
}

char *changeCase(char *p1, char *p2, char _case){
    switch (_case) {
        case 1:
            modifyChars(p1, p2, tolower);
            break;
        case 2:
            modifyChars(p1, p2, toupper);
            break;
        default:
            break;
    }
    return p2;
}


char *detectCodeWord(char *p){
    while (*p && !isspace (*p) && *p != ';' && *p != '\'' && *p != '"' && *p != '#' && *p != '$')
        p++;

    return p;
}
