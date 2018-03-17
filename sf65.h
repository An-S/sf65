#ifndef __SF65_H__
#define __SF65_h__

#define VERSION "v0.2"



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


int processCMDArgs(int argc, char** argv);
char *skipWhiteSpace(char *p);
char *modifyChars(char *p1, char *p2, int func(int));
char *changeCase(char *p1, char *p2, char _case);

#endif
