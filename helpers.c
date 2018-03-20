#include "sf65.h"
#include "stdlib.h"
#include "ctype.h"

char echoChar(char ch){
    fputc(ch, stdout);
    return ch;
}

char *skipWhiteSpace(char *p){
    char ch;
    while (ch = *p, ch && isspace (ch)){
        echoChar(ch);
        ++p;
    }
        
    // p points to first char after whitespace or to '\0' character
    return p;
}

char *modifyChars(char *p1, char *p2, int func(int)){
    char ch;
    
    while (p1 < p2) {
        ch = *p1;
        echoChar(ch);
        *p1 = func (ch);
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
    // && *p != '\'' && *p != '"' && *p != '#' && *p != '$'
    char ch;
    
    while (ch = *p, ch && !isspace (ch) /*&& *p != ';' */){
        echoChar(ch);
        ++p;
    }
    
    //Rewind pointer to last non delimiting char
    --p;
    return p;
}

/*
** Comparison without case
*/
int memcmpcase (char *p1, char *p2, int size) {
//    char ch;
    
    while (size--) {
        if (tolower (*p1) != tolower (*p2))
            return 1;
        p1++;
        p2++;
    }
    return 0;
}

/*
** Request space in line
*/
void request_space (FILE *output, int *current, int new, int force, int tabs) {

    /*
    ** If already exceeded space...
    */
    if (*current >= new) {
        if (force){
            fputc (' ', output);
            (*current)++;
        }
        return;
    }

    /*
    ** Advance one step at a time
    */
    if (new){
        while (1) {
            if (tabs == 0) {
                int i = 0;
                for (; i<new-*current; ++i){
                    fprintf(output, " ");
                }
                //fprintf (output, "%*s", new - *current, "");
                *current = new;
            } else {
                fputc ('\t', output);
                *current = (*current + tabs) / tabs * tabs;
            }
            if (*current >= new)
                return;
        }
    }
}

/* Tests, if a pointer is in range between a start pointer and and end pointer
 * defined by the size of the range
 */
bool inRange(const char *p, const char *first, int size){
    return p < (first + size);
}

/*
 * Starts of the memory location pointed to by *data
 * Processes allocation bytes. If newline is found, replace by \0.
 * Carriage return characters are dismissed from the output.
 */
char *convertLinefeedsToStringSeparator(char* data, int allocation){
    char *p1 = data;
    char *p2 = p1;
    char request = 0;

    while ( inRange(p1, data, allocation) ) {
        if (*p1 == '\r') {  /* Ignore \r characters */
            p1++; // Here, we increase only p1 but not p2 !
            continue;
        }
        if (*p1 == '\n') {
            p1++;
            *p2++ = '\0';   /* Break line by replacing line terminator with string terminator*/
            request = 1;
            continue;
        }
        *p2++ = *p1++; //Remove \r chars out of code (only case which does not inc p2)
        request = 0;
    }

    if (request == 0)
        *p2++ = '\0';   /* Force line break */

    return p2;
}
