#include "sf65.h"

void showCMDOptionsHelp ( void );

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

void setCMDOptionsDefaults ( sf65Options_t *CMDOptions ) {
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
    case 'h':
        showCMDOptionsHelp();
        exit ( 1 );
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

/*
 * checks some constraints on given arguments to cmd line switches
 * f.e. start_mnemonic should not be greater than start_operand and such...
 */
void validateCMDLineSwitches ( sf65Options_t *CMDOptions ) {
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
}

sf65CMDSwitchPresence_t stripSwitchCharacter ( char **arg ) {
    if ( **arg == '-' ) {
        ++*arg;
        return sf65_CMDSwitchPresent;
    }
    return sf65_CMDSwitchNotPresent;
}

// Gets entry from argv array with index argc. Changes case if switch character is present
char *getOpt ( int argc, char ** argv ) {
    //protect from accidently given NULL ptr
    NOT_NULL ( argv, NULL ) {
        char *arg = argv[argc];

        // Assert, that arg is not NULL ptr (must point to argv entry)
        assert ( arg );

        // Assert, that *arg points to non empty string
        // -> Remark: User can pass empty string by using "". Therefore the
        // assertion must be replaced by runtime checks
        // assert ( *arg );

        //change only to lowercase if switch character is present
        //This way, filename parameters are protected from case switching
        if ( *arg == '-' ) {
            // Missing Option after switch character
            if ( !* ( arg + 1 ) ) {
                return NULL;
            }
            sf65_ChangeCase ( arg, strchr ( arg, '\0' ), SF65_LOWERC );
        }
        return arg;//NULL; // No arg specified after '-'
    }
}

void showCMDOptionsHelp ( void ) {
    fprintf ( stderr, "\n" );
    fprintf ( stderr, "SF65 by Monte Carlos / Cascade, Mar 2018. Based on:\n\n" );
    fprintf ( stderr, "Pretty6502 " VERSION " by Oscar Toledo G. http://nanochess.org/\n" );
    fprintf ( stderr, "For more information about the changes since forking, see release notes.\n" );
    fprintf ( stderr, "\n" );
    fprintf ( stderr, "Usage:\n" );
    fprintf ( stderr, "    [./]sf65 [args] [input.asm] [output.asm]\n" );
    fprintf ( stderr, "\n" );
    fprintf ( stderr, "DON'T USE SAME OUTPUT FILE AS INPUT, though it's possible,\n" );
    fprintf ( stderr, "you can DAMAGE YOUR SOURCE if this program has bugs.\n" );
    fprintf ( stderr, "\n" );
    fprintf ( stderr, "You can specify '-' for input and/or output file\n\n" );

    fprintf ( stderr, "Arguments:\n" );
    fprintf ( stderr, "    -s0       Code in four columns (default)\n" );
    fprintf ( stderr, "              label: mnemonic operand comment\n" );
    fprintf ( stderr, "    -s1       Code in three columns\n" );
    fprintf ( stderr, "              label: mnemonic+operand comment\n" );
    /*fprintf ( stderr, "    -p0       Processor unknown\n" );
    fprintf ( stderr, "    -p1       Processor 6502 + DASM syntax (default)\n" );*/
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
    fprintf ( stderr, "    -l0       Do not put labels in own line\n" );
    fprintf ( stderr, "    -l[1]     Put oversized labels in own line\n" );
    fprintf ( stderr, "    -l2       Put all labels in own line\n" );
    fprintf ( stderr, "    -dl       Change directives to lowercase\n" );
    fprintf ( stderr, "    -du       Change directives to uppercase\n" );
    fprintf ( stderr, "    -ml       Change mnemonics to lowercase\n" );
    fprintf ( stderr, "    -mu       Change mnemonics to uppercase\n" );
    fprintf ( stderr, "    -e0|1     Pad sectioning directives with empty lines\n" );
    fprintf ( stderr, "\n" );
    fprintf ( stderr, "Assumes all your labels are at start of line and there is space\n" );
    fprintf ( stderr, "before mnemonic.\n" );
    fprintf ( stderr, "\n" );
    fprintf ( stderr, "Accepts any assembler file where ; means comment\n" );
    fprintf ( stderr, "[label] mnemonic [operand] ; comment\n" );
    sf65_fputnl ( stderr );
    sf65_fputnl ( stderr );
}

char *sf65_setInFilename ( sf65Options_t *cmdopt, char *fname ) {
    NOT_NULL ( cmdopt, NULL ); NOT_NULL ( fname, NULL ) {
        return cmdopt -> infilename = fname;
    }
}


char *sf65_setOutFilename ( sf65Options_t *cmdopt, char *fname ) {
    NOT_NULL ( cmdopt, NULL ); NOT_NULL ( fname, NULL ) {
        return cmdopt -> outfilename = fname;
    }
}


/*
 * The format to call sf65 is one of the three cases:
 *
 * ./sf65 -x -y -z [infile] [outfile]  //argc = 6
 * ./sf65 -x -y -z [infile] //argc = 5
 * ./sf65 -x -y -z // argc = 4
 *
 * -x -y -z is a sequence of command line options which is not necessarily a sequence
 * of three
 *
 * After the command line options switches the filenames for the infile and/or outfile
 * may be given.
 * However, filenames given before argc-2 will be considered as error
 *
 * It must be detected, if
 * 1.) none filename is present
 * 2.) one filename is present
 * 3.) two filenames are present
 */
int processCMDArgs ( int argc, char** argv, sf65Options_t *CMDOptions ) {
    static char predefinedInFilename[]  = "in.src";
    static char predefinedOutFilename[]  = "out.src";

    int cmdArgIdx;
    char *currentOptPtr;
    int filenamePositions[2] = {};
    int filenameCount = 0;

    setCMDOptionsDefaults ( CMDOptions );

    /*
     * Process arguments
     *
     * quoted filenames are handled differently in windows and linux
     *
     * windows:
     *
     * quoting with double quotes lead to arguments passed without quotes
     * quoting with single quotes passes the arguments inclusive quotes
     * quoting of empty string leads to omitted argument
     *
     *
     * linux:
     *
     * quoting with double quotes lead to arguments passed without quotes
     * quoting with single quotes -~-
     * quoting of empty string leads to -~-
     */
    for ( cmdArgIdx = 1; cmdArgIdx < argc; ++cmdArgIdx ) {
        // Detect switch character '-' and return pointer to char directly following '-'
        currentOptPtr = getOpt ( cmdArgIdx, argv );

        // Test, if option is given after '-' switch character.
        // However, if cmdArgIdx > argc-2, single '-' is allowed to specify
        // stdin /stdout
        if ( !*currentOptPtr && cmdArgIdx < argc - 2 ) {
            // If no option specified after '-', output err msg and indicate
            // position of arg neglecting arg 0 (filename of executable)
            // That's why "cmdArgIdx - 1"
            sf65_pError ( "Missing option after switch at arg: %d\n", cmdArgIdx - 1 );
            exit ( 1 );
        }

        // If switch character '-' is present, inc arg pointer to succeeding char
        // Return value indicates presence of switch
        switch ( stripSwitchCharacter ( &currentOptPtr ) ) {
        case sf65_CMDSwitchPresent:
            if ( !*currentOptPtr && cmdArgIdx >= argc - 2 ) {
                if ( cmdArgIdx < argc - 2 ) {
                    sf65_pError ( "'-' for stdin/stdout can be given only for last two parameters\n" );
                    exit ( 1 );
                }
                //if ( filenameCount < 2 ) {

                // Assert that there are not more than 2 filenames present
                // which should always be true if above condition fails
                assert ( filenameCount < 2 );
                filenamePositions[filenameCount++] = cmdArgIdx;
            } else {
                // Determine which options have been given on command line and set variables
                // in CMDOptions struct accordingly
                detectCMDLineSwitches ( CMDOptions, currentOptPtr );
            }
            break;
        case sf65_CMDSwitchNotPresent:
            if ( cmdArgIdx < argc - 2 ) {
                sf65_pError ( "In/out filenames should be given as last two parameters\n" );
                exit ( 1 );
            }
            //if ( filenameCount < 2 ) {

            // Assert that there are not more than 2 filenames present
            // which should always be true if above condition fails
            assert ( filenameCount < 2 );
            filenamePositions[filenameCount++] = cmdArgIdx;
            break;
        default:
            // Should not come here
            assert ( false );
            break;
        }
    }


    /*
    ** Validate constraints
    */

    validateCMDLineSwitches ( CMDOptions );

    switch ( filenameCount ) {
    case 0:
        sf65_printfUserInfo ( "Neither input nor output file given. Using default filenames\n" );
        sf65_setInFilename ( CMDOptions, predefinedInFilename );
        sf65_setOutFilename ( CMDOptions, predefinedOutFilename );
        break;
    case 1:
        sf65_printfUserInfo ( "No output file given. Using Default filename\n" );
        if ( *argv[filenamePositions[0]] ) {
            sf65_setInFilename ( CMDOptions, argv[filenamePositions[0]] );
        } else {
            sf65_setInFilename ( CMDOptions, predefinedInFilename );
        }
        sf65_setOutFilename ( CMDOptions, predefinedOutFilename );

        break;
    case 2:
        if ( *argv[filenamePositions[0]] ) {
            sf65_setInFilename ( CMDOptions, argv[filenamePositions[0]] );
        } else {
            sf65_printfUserInfo ( "No input file given. Using Default filename\n" );

            sf65_setInFilename ( CMDOptions, predefinedInFilename );
        }
        if ( *argv[filenamePositions[1]] ) {
            sf65_setOutFilename ( CMDOptions, argv[filenamePositions[1]] );
        } else {
            sf65_printfUserInfo ( "No output file given. Using Default filename\n" );

            sf65_setOutFilename ( CMDOptions, predefinedOutFilename );
        }
        break;
    default:
        assert ( false ); //Should not come here, because max 2 files may be specified
    }

    return cmdArgIdx;
}
