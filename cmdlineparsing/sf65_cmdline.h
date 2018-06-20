#ifndef __SF65CMDL_H__
#define __SF65CMDL_H__

#define SF65_CMDLERRLIST ER(NOERR), ER(NULLPTR), ER(INVALIDARGERR)

#define ER(x) SF65_CMDERR_##x
typedef enum {
    SF65_ERRLIST
} sf65CMDErrCode_t;
#undef ER

typedef enum {
    sf65_CMDSwitchPresent, sf65_CMDSwitchNotPresent
} sf65CMDSwitchPresence_t;

typedef struct {
    // Example: -l123, where '-' is the switchCh, 'l' is the optCh and 123 is the numArg

    // Store a copy of the argv pointer to the original command line arguments list
    char **argv;
    int argc;
    int argIdx;

    char *currentPtr; //Pointer to char ought to be read next
    int optIdx;     // Index into list of allowed cmd options
    char optCh;     // The char which determines a cmd option
    int numArg;     // The numeric argument after an optCh
    sf65CMDSwitchPresence_t hasSwitchCh; // Indicator if '-' is present before argument
} sf65CMDArg_t;

// Format: command line switch character, callbackFnc, argmin, argmax
#define SF65_CMDOPTLIST \
    CO ( v, Verbosity,    -1, -1 )\
    CO ( s, Style,         0,  1 )\
    /*CO ( n, NestingLevel,  0, -1 )\
    CO ( m, Mnemonic,      0, -1 )\
    CO ( d, Directive,     0, -1 )\
    CO ( c, Comment,       0, -1 )\
    CO ( p, Processor,     0,  0 )\*/\
    CO ( h, Help,          0,  0 ) \
    /*CO ( t, Tabs,          0,  8 )\
    CO ( a, Align,         0,  1 )\
    CO ( l, LabelPlacement, 0,  2 )\*/\
    CO ( e, ScopePadding,  0,  1 )

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

    int verbosity;

    char locallabelch;
    char *infilename;       // filename of the unformatted source
    char *outfilename;      // filename of the formatted source
} sf65Options_t;

extern sf65Options_t *CMDOptions;



/*
 * Procedure to process command line arguments given to sf65
 * Fills given struct with values of command line options and/or default values
 */
int processCMDArgs ( int argc, char **argv, sf65Options_t *CMDOptions );

typedef sf65CMDErrCode_t
sf65OptionsModifierFnc_t ( sf65Options_t *, sf65CMDArg_t *arg );

#endif
