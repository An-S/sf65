#ifndef __SF65_TYPES_H__
#define __SF65_TYPES_H__

#define SF65_ERRLIST ER(NOERR), ER(ERROR), ER(NULLPTR), ER(INVALIDARG)

#define ER(x) SF65_##x
typedef enum {
    SF65_ERRLIST
} sf65Err_t;
#undef ER

typedef enum {
    SF65_KEEPCASE, SF65_LOWERC, SF65_UPPERC
} sf65Case_t;

typedef enum {
    SF65_STREQ, SF65_STRNOTEQ
} sf65StrEq_t;

/*
 * Struct to hold values of command line arguments given to sf65
 */
typedef struct {
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
    int oversized_labels_own_line;
    int mnemonics_case;     // flag, if mnemonics should be output lower/uppercase or leave unmodified
    int directives_case;    // flag, if directives should be output lower/uppercase or leave unmodified
    int pad_directives;     // determine whether certain directives should be padded with empty lines
    char *infilename;       // filename of the unformatted source
    char *outfilename;      // filename of the formatted source
} sf65Options_t;



/*
 * Struct to hold the names of the assembler directives
 * as well as the flags which define how to deal with the former
 */
typedef struct {
    char *directive;
    int flags;
} directives_t;

#define EXPRTYPES \
    ET(MNEMONIC), ET(DIRECTIVE), ET(OPERAND), ET(LABEL), ET(COMMENT),\
    ET(EMPTYLINE), ET(MACRONAME), ET(COMMASEP), ET(ASSIGNMENT),\
    ET(OTHEREXPR), ET(IDENTIFIER), ET(STRLITERAL),\
    ET(HEXSPECIFIER), ET(BINSPECIFIER), ET(INVALIDEXPR)

#define ET(x) SF65_##x

typedef enum {
    EXPRTYPES
} sf65ExpressionEnum_t;

#undef ET

/* Use XMacro trick to generate appropriate bitmasks for Alignment types
 */
#define ALGNMTYPES A(ALIGN_LOWLIMIT), A(DONT_RELOCATE),\
    A(LEVEL_IN),\
    A(LEVEL_OUT),\
    A(LEVEL_MINUS),\
    A(ALIGN_MNEMONIC),\
    A(ALIGN_HIGHLIMIT)

/* First assign bit positions to alignment constants
 */
#define A(x) x##_BITNO
enum {
    ALGNMTYPES
};
#undef A

/* Now generate bitmasks by shifting a 1 bit by the amount of bits defined by the bit position
 * constants
 */
#define A(x) x = 1<<x##_BITNO
typedef enum {
    ALGNMTYPES
} sf65Alignment_t;
#undef A

typedef struct {
    sf65ExpressionEnum_t exprType;
    int index;
    char rightmostChar;
} sf65Expression_t;

#define SF65_PARSERFLAGS PF(label_detected, LABEL_DETECTED)\
    PF(first_expression, FIRST_EXPRESSION)\
    PF(beginning_of_line, BEGINNING_OF_LINE)\
    PF(additional_linefeed, ADDITIONAL_LINEFEED)\
    PF(instant_additional_linefeed, INSTANT_ADDITIONAL_LINEFEED)\
    PF(force_separating_space, FORCE_SEPARATING_SPACE)\
    PF(line_continuation, LINE_CONTINUATION)

#define PF(x,y) SF65_##y,
typedef enum {
    SF65_PARSERFLAGS
    SF65_PARSERFLAGCNT, SF65_NOT_A_PARSERFLAG
} sf65ParserFlagsEnum_t;
#undef PF

/*
 * Struct to hold variables needed for parsing of unformatted source
 */
typedef struct {
    union {
        struct {
#           define PF(x,y) unsigned int x: 1;
            SF65_PARSERFLAGS
#           undef PF
        };
        struct {
            unsigned int allParserFlags;
        };
    };


    int current_column;
    int last_column;
    int request;
    int current_level;

    int flags;

    int prev_comment_original_location;
    int prev_comment_final_location;

    sf65Expression_t prev_expr;
} sf65ParsingData_t;
#endif
