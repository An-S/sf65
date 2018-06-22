#include "sf65.h"

#   define CO(x,y,v,w) sf65CMDErrCode_t sf65_Opt##y(sf65Options_t *opt, sf65CMDArg_t *cmdarg );
SF65_CMDOPTLIST
#   undef CO

// When entering this function, it is assumed that currentOptPtr points to character after '-'
// and that there was a '-' present
void detectCMDLineSwitches ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
    NOT_NULL ( CMDOptions, ); NOT_NULL ( cmdarg, ) {

        // Define a list of allowed switches by concatenating a string using x macro
        // stringification
#   define CO(w,x,y,z) #w
        char switches[] = SF65_CMDOPTLIST ; // '\0' is appended automatically
#   undef CO

        // Define an array of pointers to callback functions which are called for
        // a set switch at the same array position as in the switches string
#   define CO(w,x,y,z) sf65_Opt##x,
        sf65OptionsModifierFnc_t *modifierFncList[] = {SF65_CMDOPTLIST NULL};
#   undef CO

        // Define min/max values for numeric command line parameters
#   define CO(w,x,y,z) y,
        int optMinList[] = {SF65_CMDOPTLIST - 1};
#   undef CO

#   define CO(w,x,y,z) z,
        int optMaxList[] = {SF65_CMDOPTLIST - 1};
#   undef CO

        // = cmdarg->optCh;// = *currentOptPtr++;

        sf65_CMDOpt_ReadNextCh ( cmdarg );

        if ( isdigit ( cmdarg -> currentCh ) ) {
            cmdarg -> numArg = sf65_ConvertStrToInt ( cmdarg -> currentPtr );
            //checkIf0Or1 ( cmdNumArg );
        } else {
            cmdarg -> numArg = -1;
        }

        // If optCh not found, strchr returns NULL and then the expression becomes negative
        cmdarg -> optIdx = strchr ( switches, cmdarg -> optCh ) - switches;

        // This if clause checks indirectly the NULL return value of strchr, if optCh was not found
        if ( cmdarg -> optIdx < 0 ) {
            // Indicate invalid command line option
            sf65_pError ( "Unknown option '%c'\n", cmdarg -> optCh );
            exit ( 1 );
        }

        // Check, that unknown options were catched successfully and we can continue
        // with known options
        assert ( cmdarg -> optIdx >= 0 );

        // Assert that list of valid cmd options do not contain one option more than one time
        assert ( cmdarg -> optIdx == strrchr ( switches, cmdarg -> optCh ) - switches );
        sf65_printfUserInfo ( "Detected switch '%c' with arg '%d'.\n", cmdarg -> optCh, cmdarg -> numArg );

        // Check range of numeric parameter for cmd option and exit if range exceeded
        conditionallyFailWthMsg (
            sf65_checkRange ( cmdarg -> numArg, optMinList[cmdarg -> optIdx],
                              optMaxList[cmdarg -> optIdx]
                            ) ,
            "Numeric option parameter out of range: %c in [%d..%d] but given %d\n", cmdarg -> optCh,
            optMinList[cmdarg -> optIdx], optMaxList[cmdarg -> optIdx], cmdarg -> numArg
        );
        // assert non NULL function pointer
        assert ( modifierFncList[cmdarg -> optIdx] );
        // call function to set members in CMDOptions struct
        modifierFncList[cmdarg -> optIdx] ( CMDOptions, cmdarg );

////modifierFncList[optidx)(CMDOptions,
//// If come here, option after switch character was given
//        switch ( cmdSwitchCh ) {
//            /*case 'v':
//                ++CMDOptions -> verbosity;
//                break;
//            case 'h':
//                showCMDOptionsHelp();
//                exit ( 1 );
//            case 'e':
//                CMDOptions -> pad_directives = cmdNumArg;
//
//                conditionallyFailWthMsg (
//                    cmdNumArgIs0Or1,
//                    "Bad sf65Options -> pad directives: %d\n", cmdNumArg
//                );
//                break;
//            case 's':   // sf65Options -> Style
//                CMDOptions -> style = cmdNumArg;
//
//                conditionallyFailWthMsg (
//                    cmdNumArgIs0Or1,
//                    "Bad sf65Options -> style code: %d\n", cmdNumArg
//                );
//                break;*/
//
//        case 'p':   /* Processor */
//            CMDOptions -> processor = cmdNumArg;
//
//            conditionallyFailWthMsg (
//                cmdNumArgIs0Or1,
//                "Bad sf65Options -> processor code: %d\n", cmdNumArg
//            );
//            break;
//        case 'm':   /* Mnemonic start */
//            if ( *currentOptPtr == 'l' ) {
//                CMDOptions -> mnemonics_case = 1;
//            } else if ( *currentOptPtr == 'u' ) {
//                CMDOptions -> mnemonics_case = 2;
//            } else {
//                CMDOptions -> start_mnemonic = cmdNumArg;
//            }
//            break;
//        case 'o':   /* Operand start */
//            CMDOptions -> start_operand = cmdNumArg;
//            break;
//        case 'c':   /* Comment start */
//            CMDOptions -> start_comment = cmdNumArg;
//            break;
//        case 't':   /* Tab size */
//            CMDOptions -> tabs = cmdNumArg;
//            break;
//        case 'a':   /* Comment alignment */
//            CMDOptions -> align_comment = cmdNumArg;
//            conditionallyFailWthMsg (
//                cmdNumArgIs0Or1, "Bad comment alignment: %d\n", cmdNumArg
//            );
//
//            break;
//        case 'n':   /* Nesting space */
//            CMDOptions -> nesting_space = cmdNumArg;
//            break;
//        case 'l':   /* Labels in own line. l0 = labels in existing line
//                           l1 = oversized labels own line
//                           l2 = all labels own line*/
//            if ( strlen ( currentOptPtr ) > 0 ) {
//                CMDOptions -> oversized_labels_own_line = cmdNumArg;
//
//                conditionallyFailWthMsg (
//                    sf65_checkRange ( cmdNumArg, 0, 2 ) ,
//                    "Bad label line placement: %d\n", cmdNumArg
//                );
//
//                if ( CMDOptions -> oversized_labels_own_line == 2 ) {
//                    CMDOptions -> labels_own_line = 1;
//                }
//            } else {
//                CMDOptions -> labels_own_line = 1;
//            }
//            break;
//        case 'd':   /* Directives */
//            if ( *currentOptPtr == 'l' ) {
//                CMDOptions -> directives_case = 1;
//            } else if ( *currentOptPtr == 'u' ) {
//                CMDOptions -> directives_case = 2;
//            } else {
//                sf65_pError ( "Unknown argument: %c%c\n", cmdSwitchCh, *currentOptPtr );
//            }
//            break;
//        default:    /* Other */
//            sf65_pError ( "Unknown argument: %c\n", cmdSwitchCh );
//            exit ( 1 );
//        }

    }
}

/*
 * checks some constraints on given arguments to cmd line switches
 * f.e. start_mnemonic should not be greater than start_operand and such...
 */
void validateCMDLineSwitches ( sf65Options_t * CMDOptions ) {
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
