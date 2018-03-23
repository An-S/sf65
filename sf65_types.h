#ifndef __SF65_TYPES_H__
#define __SF65_TYPES_H__

/*
 * Struct to hold values of command line arguments given to sf65
 */
typedef struct{
    int tabs;               // 0 = default = use spaces not tabs, 1 = use tabs not spaces
    int style;              // 0 = four columns, 1 = three columns
    int processor;          // 0 = unknown assembler, 1 = CA65
    int start_mnemonic;     // x-position where mnemonics should be placed in output file
    int start_operand;      // x-position where operand should be placed in output file
    int start_comment;      // x-position where comments should be placed in output file
    int start_directive;    // x-position where directives should be placed in output file
    int align_comment;      // should comments be aligned with mnemonics if they are found on linestart?
    int nesting_space;      // tab width or according number of spaces to fill in output
    int labels_own_line;    // flag, if labels should be moved to own line in output file
    int mnemonics_case;     // flag, if mnemonics should be output lower/uppercase or leave unmodified
    int directives_case;    // flag, if directives should be output lower/uppercase or leave unmodified
    char *infilename;       // filename of the unformatted source
    char *outfilename;      // filename of the formatted source
} sf65Options_t;

/*
 * Struct to hold variables needed for parsing of unformatted source
 */
typedef struct{
    bool label_detected;
    bool mnemonic_detected;
    bool comment_detected;
    bool directive_detected;

    int current_column;
    int request;
    int current_level;

    int flags;
    
    int prev_comment_original_location;
    int prev_comment_final_location;
} sf65ParsingData_t;

/*
 * Struct to hold the names of the assembler directives
 * as well as the flags which define how to deal with the former
 */
typedef struct {
    char *directive;
    int flags;
} directives_t;

#endif
