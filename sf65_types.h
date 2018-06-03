#ifndef __SF65_TYPES_H__
#define __SF65_TYPES_H__

/*
 * This typedef is used for
 * 1. padding empty lines before and after certain directives, if wished.
 * 2. Breaking oversized labels, if wished
 */
typedef enum {
    SF65_DEFAULT_LF,
    SF65_ADD_LF,
    SF65_INSTANT_ADD_LF,
    SF65_NOT_A_LF_CONST
} sf65LinefeedEnum_t;

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

/* There are flags which change every syntax element:
 *
 * first_expression
 * force_separating_space
 * instant_additional_linefeed
 * beginning of line
 *
 *
 * and flags which value is kept until the next line (if changed once):
 *
 * label_detected
 * additional_linefeed
 * level_changed
 * line_continuation
 */
#define SF65_PARSERFLAGS PF(label_detected, LABEL_DETECTED)\
    PF(first_expression, FIRST_EXPRESSION)\
    PF(beginning_of_line, BEGINNING_OF_LINE)\
    PF(additional_linefeed, ADDITIONAL_LINEFEED)\
    PF(level_changed, LEVEL_CHANGED)\
    PF(indent_request, INDENT_REQUEST)\
    PF(unindent_request, UNINDENT_REQUEST)\
    PF(instant_additional_linefeed, INSTANT_ADDITIONAL_LINEFEED)\
    PF(force_separating_space, FORCE_SEPARATING_SPACE)\
    PF(line_continuation, LINE_CONTINUATION)

#define PF(x,y) SF65_##y,
typedef enum {
    SF65_PARSERFLAGS
    SF65_PARSERFLAGCNT, SF65_NOT_A_PARSERFLAG
} sf65ParserFlagsEnum_t;
#undef PF

typedef     union {
    // These flags are assumed to be needed to be kept during expression parsing
    struct {
#           define PF(x,y) unsigned int x: 1;
        SF65_PARSERFLAGS
#           undef PF
    };
    struct {
        unsigned int allParserFlags;
    };
} sf65Parserflags_t;

typedef struct {
    // These fields needs to be kept between expressions
    int current_column;
    int request;
    int current_level;
    int prev_comment_original_location;
    int prev_comment_final_location;

    // These fields are overwritten for each expression
    int flags;

    sf65Expression_t current_expr;
} sf65ParserState_t;

typedef struct {
    // Needs to be kept for whole line parsing/formatting procedure
    char linebuf[1000]; // Input file is read line by line into this buffer
    int linesize;
} sf65Linebuffer_t;

/*
 * Struct to hold variables needed for parsing of unformatted source
 * This struct has been broken down into smaller structs which are
 * included into the bigger struct via anonymous types
 */
typedef struct sf65ParsingData_Tag {
    //anon structs
    sf65Linebuffer_t;  // Char array and linesize
    sf65ParserState_t; // Variables which hold state information for parser
    sf65Parserflags_t; // Flags which indicate special occurences and requests

    struct sf65ParsingData_Tag *prev;
} sf65ParsingData_t;

#endif
