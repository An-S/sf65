#ifndef __SF65CMDL_H__
#define __SF65CMDL_H__

#include <stdbool.h>
#include <sf65_macros.h>

#define CHECK_NULL(arg, errorCode) if(!(arg)){assert( (arg) != NULL );return errorCode;} else

#define SF65_CMDERRLIST ER(NOERR), ER(NULLPTR), ER(NOMOREARGS), ER(INVALIDARGERR)

#define ER(x) SF65_CMDERR_##x
typedef enum {
    SF65_CMDERRLIST
} sf65CMDErrCode_t;
#undef ER

typedef enum {
    sf65_CMDSwitchPresent = true, sf65_CMDSwitchNotPresent = false
} sf65CMDSwitchPresence_t;



typedef struct {
    // Example: -l123, where '-' is the switchCh, 'l' is the optCh and 123 is the numArg

    // Store a copy of the argv pointer to the original command line arguments list
    char **argv;
    char *arg;
    int argc;
    int argIdx;

    char currentCh;
    char *currentPtr; //Pointer to char ought to be read next
    int optIdx;     // Index into list of allowed cmd options
    char optCh;     // The char which determines a cmd option
    int numArg;     // The numeric argument after an optCh
    sf65CMDSwitchPresence_t hasOpt; // Indicator if '-' is present before argument
} sf65CMDArg_t;

// Format: command line switch character, callbackFnc, argmin, argmax
#define SF65_CMDOPTLIST \
    CO ( v, Verbosity,    -1, -1 )\
    CO ( s, Style,         0,  1 )\
    CO ( n, NestingLevel,  0, 12 )\
    CO ( m, Mnemonic,      -1, 20 )\
    CO ( d, Directive,     -1, 20 )\
    CO ( c, Comment,       0, 40 )\
    CO ( p, Processor,     0,  1 )\
    CO ( o, Operand,       0, 40 )\
    CO ( h, Help,          -1,  -1 )\
    CO ( t, Tabs,          0,  8 )\
    CO ( a, Align,         0,  1 )\
    CO ( l, LabelPlacement, 0,  2 )\
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

typedef sf65CMDErrCode_t
sf65OptionsModifierFnc_t ( sf65Options_t *, sf65CMDArg_t * );

typedef struct {
    const char *switches;
    sf65OptionsModifierFnc_t **optModifierFncList;
    int *llimits;
    int *hlimits;
} gfgfgfg_t;


void showCMDOptionsHelp ( void );

/*
 * Inits parser. Retrieves first arg and stores state.
 */
sf65CMDErrCode_t sf65_CMDOpt_InitParser ( sf65CMDArg_t *arg, int argc, char **argv );

/*
 * Sets default values for variables holding the value parameters
 * of the command line arguments
 */
void sf65_SetDefaultCMDOptions ( sf65Options_t *CMDOptions );

int sf65_ParseCMDArgs ( int argc, char** argv, sf65Options_t * CMDOptions );

void validateCMDLineSwitches ( sf65Options_t * CMDOptions );

/* This function compares a given cmd switch with a list of allowed switches
 * and then calls the according callback function to act upon a set switch.
 */
void sf65_DetectMatchingOption ( sf65Options_t *CMDOptions,
                                 sf65CMDArg_t *cmdarg,
                                 const char *switches,
                                 sf65OptionsModifierFnc_t **fncList );
char *sf65_setInFilename ( sf65Options_t * cmdopt, char * fname );
char *sf65_setOutFilename ( sf65Options_t * cmdopt, char * fname );

/*
 * Procedure to process command line arguments given to sf65
 * Fills given struct with values of command line options and/or default values
 */
sf65CMDErrCode_t  sf65_ProcessCmdArgs ( sf65Options_t *, sf65CMDArg_t * );

char sf65_CMDOpt_ReadNextCh ( sf65CMDArg_t *arg );
sf65CMDErrCode_t sf65_CMDOpt_GetNextArg ( sf65CMDArg_t * );


#endif
