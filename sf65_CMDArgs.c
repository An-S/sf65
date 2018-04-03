#include "sf65.h"

bool checkRange ( int val, int min, int max ) {
    if ( val < min ) return false;
    if ( val > max ) return false;
    return true;
}

bool checkIf0Or1 ( int val ) {
    return checkRange ( val, 0, 1 );
}

bool conditionallyFailWthMsg ( bool succeed, char *format, ... ) {
    va_list va;
    va_start ( va, format );

    if ( !succeed ) {
        sf65_vpError ( format, va );
        exit ( 1 );
    }

    va_end ( va );
    return succeed;
}

/* Attempts to convert the string from arg into a decimal integer
 */
int getIntArg ( char* arg ) {
    long int num = -1;
    char *endarg;

    num = strtol ( arg, &endarg, 0 );

    // There should be only numeric until end of arg. If not, return -1 to indicate error
    if ( *endarg != '\0' ) {
        return -1;
    }

    return num;
}

void detectCMDLineSwitches ( sf65Options_t * CMDOptions, char *currentOptPtr ) {
    int cmdNumArg;
    bool cmdNumArgIs0Or1;
    char cmdSwitchCh;

    cmdSwitchCh = *currentOptPtr++;

    if ( isdigit ( *currentOptPtr ) ) {
        cmdNumArg = getIntArg ( currentOptPtr );
        cmdNumArgIs0Or1 = checkIf0Or1 ( cmdNumArg );
    } else {
        cmdNumArg = -1;
        cmdNumArgIs0Or1 = false;
    }

    // If come here, option after switch character was given
    switch ( cmdSwitchCh ) {
    case 'e':   /* sf65Options -> pad lines */
        CMDOptions -> pad_directives = cmdNumArg;

        conditionallyFailWthMsg (
            cmdNumArgIs0Or1,
            "Bad sf65Options -> pad directives: %d\n", cmdNumArg
        );
        break;
    case 's':   /* sf65Options -> Style */
        CMDOptions -> style = cmdNumArg;

        conditionallyFailWthMsg (
            cmdNumArgIs0Or1,
            "Bad sf65Options -> style code: %d\n", cmdNumArg
        );
        break;
    case 'p':   /* Processor */
        CMDOptions -> processor = cmdNumArg;

        conditionallyFailWthMsg (
            cmdNumArgIs0Or1,
            "Bad sf65Options -> processor code: %d\n", cmdNumArg
        );
        break;
    case 'm':   /* Mnemonic start */
        if ( *currentOptPtr == 'l' ) {
            CMDOptions -> mnemonics_case = 1;
        } else if ( *currentOptPtr == 'u' ) {
            CMDOptions -> mnemonics_case = 2;
        } else {
            CMDOptions -> start_mnemonic = cmdNumArg;
        }
        break;
    case 'o':   /* Operand start */
        CMDOptions -> start_operand = cmdNumArg;
        break;
    case 'c':   /* Comment start */
        CMDOptions -> start_comment = cmdNumArg;
        break;
    case 't':   /* Tab size */
        CMDOptions -> tabs = cmdNumArg;
        break;
    case 'a':   /* Comment alignment */
        CMDOptions -> align_comment = cmdNumArg;
        conditionallyFailWthMsg (
            cmdNumArgIs0Or1, "Bad comment alignment: %d\n", cmdNumArg
        );

        break;
    case 'n':   /* Nesting space */
        CMDOptions -> nesting_space = cmdNumArg;
        break;
    case 'l':   /* Labels in own line. l0 = labels in existing line
                           l1 = oversized labels own line
                           l2 = all labels own line*/
        if ( strlen ( currentOptPtr ) > 0 ) {
            CMDOptions -> oversized_labels_own_line = cmdNumArg;

            conditionallyFailWthMsg (
                checkRange ( cmdNumArg, 0, 2 ) ,
                "Bad label line placement: %d\n", cmdNumArg
            );

            if ( CMDOptions -> oversized_labels_own_line == 2 ) {
                CMDOptions -> labels_own_line = 1;
            }
        } else {
            CMDOptions -> labels_own_line = 1;
        }
        break;
    case 'd':   /* Directives */
        if ( *currentOptPtr == 'l' ) {
            CMDOptions -> directives_case = 1;
        } else if ( *currentOptPtr == 'u' ) {
            CMDOptions -> directives_case = 2;
        } else {
            sf65_pError ( "Unknown argument: %c%c\n", cmdSwitchCh, *currentOptPtr );
        }
        break;
    default:    /* Other */
        sf65_pError ( "Unknown argument: %c\n", cmdSwitchCh );
        exit ( 1 );
    }
}

// Detect an option specified by '-' and returns pointer to first character after
// '-'
// Converts all chars after '-' to lowercase, if any
char *getOpt ( int argc, char ** argv ) {
    char *arg = argv[argc];

    if ( arg[0] != '-' ) {
        fprintf ( stderr, "Bad argument\n" );
        exit ( 1 );
    }

    ++arg;

    if ( *arg ) {
        changeCase ( arg, strchr ( arg, '\0' ), SF65_LOWERC );

        return arg;//skipWhiteSpace ( arg );

    }
    return NULL; // No arg specified after '-'
}

int processCMDArgs ( int argc, char** argv, sf65Options_t *CMDOptions ) {
    /*
    ** Show usage if less than 3 arguments (program name counts as one)
    */
    int c = 0, cmdNumArg;
    bool cmdNumArgIs0Or1;
    char *currentOptPtr;
    char cmdSwitchCh;

    if ( argc < 3 ) {
        fprintf ( stderr, "\n" );
        fprintf ( stderr, "SF65 by Monte Carlos / Cascade, Mar 2018. Based on:\n\n" );
        fprintf ( stderr, "Pretty6502 " VERSION " by Oscar Toledo G. http://nanochess.org/\n" );
        fprintf ( stderr, "For more information about the changes since forking, see release notes.\n" );
        fprintf ( stderr, "\n" );
        fprintf ( stderr, "Usage:\n" );
        fprintf ( stderr, "    pretty6502 [args] input.asm output.asm\n" );
        fprintf ( stderr, "\n" );
        fprintf ( stderr, "DON'T USE SAME OUTPUT FILE AS INPUT, though it's possible,\n" );
        fprintf ( stderr, "you can DAMAGE YOUR SOURCE if this program has bugs.\n" );
        fprintf ( stderr, "\n" );
        fprintf ( stderr, "Arguments:\n" );
        fprintf ( stderr, "    -s0       Code in four columns (default)\n" );
        fprintf ( stderr, "              label: mnemonic operand comment\n" );
        fprintf ( stderr, "    -s1       Code in three columns\n" );
        fprintf ( stderr, "              label: mnemonic+operand comment\n" );
        fprintf ( stderr, "    -p0       Processor unknown\n" );
        fprintf ( stderr, "    -p1       Processor 6502 + DASM syntax (default)\n" );
        fprintf ( stderr, "    -m8       Start of mnemonic column (default)\n" );
        fprintf ( stderr, "    -o16      Start of operand column (default)\n" );
        fprintf ( stderr, "    -c32      Start of comment column (default)\n" );
        fprintf ( stderr, "    -t8       Use tabs of size 8 to reach column\n" );
        fprintf ( stderr, "    -t0       Use spaces to align (default)\n" );
        fprintf ( stderr, "    -a0       Align comments to nearest column\n" );
        fprintf ( stderr, "    -a1       Comments at line start are aligned\n" );
        fprintf ( stderr, "              to mnemonic (default)\n" );
        fprintf ( stderr, "    -n4       Nesting spacing (can be any number\n" );
        fprintf ( stderr, "              of spaces or multiple of tab size)\n" );
        fprintf ( stderr, "    -l        Puts labels in its own line\n" );
        fprintf ( stderr, "    -dl       Change directives to lowercase\n" );
        fprintf ( stderr, "    -du       Change directives to uppercase\n" );
        fprintf ( stderr, "    -ml       Change mnemonics to lowercase\n" );
        fprintf ( stderr, "    -mu       Change mnemonics to uppercase\n" );
        fprintf ( stderr, "    -e1       Pad sectioning directives with empty lines\n" );
        fprintf ( stderr, "\n" );
        fprintf ( stderr, "Assumes all your labels are at start of line and there is space\n" );
        fprintf ( stderr, "before mnemonic.\n" );
        fprintf ( stderr, "\n" );
        fprintf ( stderr, "Accepts any assembler file where ; means comment\n" );
        fprintf ( stderr, "[label] mnemonic [operand] ; comment\n" );
        exit ( 1 );
    }

    /*
    ** Default settings
    */
    CMDOptions -> style = 0;
    CMDOptions -> processor = 1;
    CMDOptions -> start_mnemonic = 8;
    CMDOptions -> start_operand = 16;
    CMDOptions -> start_comment = 32;
    CMDOptions -> start_directive = 0;//7;
    CMDOptions -> tabs = 0;
    CMDOptions -> align_comment = 1;
    CMDOptions -> nesting_space = 4;
    CMDOptions -> labels_own_line = 0;
    CMDOptions -> oversized_labels_own_line = 1;
    CMDOptions -> mnemonics_case = 0;
    CMDOptions -> directives_case = 0;
    CMDOptions -> pad_directives = 1;

    // Default to stdin
    CMDOptions -> infilename = "-";
    // Default to stdout
    CMDOptions -> outfilename = "-";

    /*
     * Process arguments
     */
    c = 1;
    while ( c < argc - 2 ) {
        currentOptPtr = getOpt ( c, argv );

        // Detect forgotton option after switch character
        if ( !currentOptPtr ) {
            sf65_pError ( "Missing option after switch!" );
            exit ( 1 );
        }

        cmdSwitchCh = *currentOptPtr++;

        if ( isdigit ( *currentOptPtr ) ) {
            cmdNumArg = getIntArg ( currentOptPtr );
            cmdNumArgIs0Or1 = checkIf0Or1 ( cmdNumArg );
        } else {
            cmdNumArg = -1;
            cmdNumArgIs0Or1 = false;
        }

        detectCMDLineSwitches ( CMDOptions, currentOptPtr );

        // If come here, option after switch character was given
        switch ( cmdSwitchCh ) {
        case 'e':   /* sf65Options -> pad lines */
            CMDOptions -> pad_directives = cmdNumArg;

            conditionallyFailWthMsg (
                cmdNumArgIs0Or1,
                "Bad sf65Options -> pad directives: %d\n", cmdNumArg
            );
            break;
        case 's':   /* sf65Options -> Style */
            CMDOptions -> style = cmdNumArg;

            conditionallyFailWthMsg (
                cmdNumArgIs0Or1,
                "Bad sf65Options -> style code: %d\n", cmdNumArg
            );
            break;
        case 'p':   /* Processor */
            CMDOptions -> processor = cmdNumArg;

            conditionallyFailWthMsg (
                cmdNumArgIs0Or1,
                "Bad sf65Options -> processor code: %d\n", cmdNumArg
            );
            break;
        case 'm':   /* Mnemonic start */
            if ( *currentOptPtr == 'l' ) {
                CMDOptions -> mnemonics_case = 1;
            } else if ( *currentOptPtr == 'u' ) {
                CMDOptions -> mnemonics_case = 2;
            } else {
                CMDOptions -> start_mnemonic = cmdNumArg;
            }
            break;
        case 'o':   /* Operand start */
            CMDOptions -> start_operand = cmdNumArg;
            break;
        case 'c':   /* Comment start */
            CMDOptions -> start_comment = cmdNumArg;
            break;
        case 't':   /* Tab size */
            CMDOptions -> tabs = cmdNumArg;
            break;
        case 'a':   /* Comment alignment */
            CMDOptions -> align_comment = cmdNumArg;
            conditionallyFailWthMsg (
                cmdNumArgIs0Or1, "Bad comment alignment: %d\n", cmdNumArg
            );

            break;
        case 'n':   /* Nesting space */
            CMDOptions -> nesting_space = cmdNumArg;
            break;
        case 'l':   /* Labels in own line. l0 = labels in existing line
                           l1 = oversized labels own line
                           l2 = all labels own line*/
            if ( strlen ( currentOptPtr ) > 0 ) {
                CMDOptions -> oversized_labels_own_line = cmdNumArg;

                conditionallyFailWthMsg (
                    checkRange ( cmdNumArg, 0, 2 ) ,
                    "Bad label line placement: %d\n", cmdNumArg
                );

                if ( CMDOptions -> oversized_labels_own_line == 2 ) {
                    CMDOptions -> labels_own_line = 1;
                }
            } else {
                CMDOptions -> labels_own_line = 1;
            }
            break;
        case 'd':   /* Directives */
            if ( *currentOptPtr == 'l' ) {
                CMDOptions -> directives_case = 1;
            } else if ( *currentOptPtr == 'u' ) {
                CMDOptions -> directives_case = 2;
            } else {
                sf65_pError ( "Unknown argument: %c%c\n", argv[c][1], argv[c][2] );
            }
            break;
        default:    /* Other */
            sf65_pError ( "Unknown argument: %c\n", argv[c][1] );
            exit ( 1 );
        }
        c++;
    }

    /*
    ** Validate constraints
    */
    if ( CMDOptions -> style == 1 ) {
        conditionallyFailWthMsg (
            CMDOptions -> start_mnemonic <= CMDOptions -> start_comment,
            "Operand error: -m%d > -c%d\n", CMDOptions -> start_mnemonic, CMDOptions -> start_comment
        );

        CMDOptions -> start_operand = CMDOptions -> start_mnemonic;
    } else if ( CMDOptions -> style == 0 ) {
        conditionallyFailWthMsg (
            CMDOptions -> start_mnemonic <= CMDOptions -> start_operand ,
            "Operand error: -m%d > -o%d\n", CMDOptions -> start_mnemonic, CMDOptions -> start_operand
        );
        conditionallyFailWthMsg (
            CMDOptions -> start_operand <= CMDOptions -> start_comment ,
            "Operand error: -o%d > -c%d\n", CMDOptions -> start_operand, CMDOptions -> start_comment
        );
    }

    if ( CMDOptions -> tabs > 0 ) {
        conditionallyFailWthMsg (
            !CMDOptions -> start_mnemonic % CMDOptions -> tabs ,
            "Operand error: -m%d isn't a multiple of %d\n", CMDOptions -> start_mnemonic, CMDOptions -> tabs
        );

        conditionallyFailWthMsg (
            CMDOptions -> start_operand % CMDOptions -> tabs,
            "Operand error: -m%d isn't a multiple of %d\n", CMDOptions -> start_operand, CMDOptions -> tabs
        );

        conditionallyFailWthMsg (
            CMDOptions -> start_comment % CMDOptions -> tabs,
            "Operand error: -m%d isn't a multiple of %d\n", CMDOptions -> start_comment, CMDOptions -> tabs
        );

        conditionallyFailWthMsg (
            CMDOptions -> nesting_space % CMDOptions -> tabs,
            "Operand error: -n%d isn't a multiple of %d\n", CMDOptions -> nesting_space, CMDOptions -> tabs
        );

    }

    if ( c < argc ) {
        CMDOptions -> infilename = argv[c];
    } else {
        CMDOptions -> infilename = "test.src";
    }

    if ( c + 1 < argc ) {
        CMDOptions -> outfilename = argv[c + 1];
    } else {
        CMDOptions -> outfilename = "test.out";
    }

    return c;
}
