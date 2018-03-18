#ifndef __SF65_H__
#define __SF65_h__

#define VERSION "v0.2"

#include <stdlib.h>

#define DONT_RELOCATE_LABEL 0x01
#define LEVEL_IN        0x02
#define LEVEL_OUT       0x04
#define LEVEL_MINUS     0x08


extern int tabs;

extern int style;
extern int processor;
extern int start_mnemonic;
extern int start_operand;
extern int start_comment;
extern int start_directive;
extern int align_comment;
extern int nesting_space;
extern int labels_own_line;
extern int prev_comment_original_location;
extern int prev_comment_final_location;
extern int mnemonics_case;
extern int directives_case;

typedef struct {
    char *directive;
    int flags;
} directives_t;

extern char *mnemonics_6502[];
extern directives_t directives_dasm[];

int processCMDArgs(int argc, char** argv);

/*
 * Read array pointed to by p as long as whitespace is found.
 * Stop at first non whitespace character or string terminator
 */
char *skipWhiteSpace(char *p);

/*
 * Iterate over char array from p1 to p2.
 * Call modificator function for each of the chars of the array
 * and write back modificated char.
 */
char *modifyChars(char *p1, char *p2, int func(int));
char *changeCase(char *p1, char *p2, char _case);

/*
 * Detect a word limited by whitespace but always stop at comment symbol ';'
 */
char *detectCodeWord(char *p);

#endif
