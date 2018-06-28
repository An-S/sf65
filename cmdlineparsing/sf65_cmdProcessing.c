#include "sf65.h"

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

#   define CO(x,y,v,w) sf65CMDErrCode_t sf65_Opt##y(sf65Options_t *opt, sf65CMDArg_t *cmdarg );
SF65_CMDOPTLIST
#   undef CO

sf65CMDErrCode_t sf65_ProcessCmdArgs ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
    static char predefinedInFilename[]  = "in.src";
    static char predefinedOutFilename[]  = "out.src";
    // Define a list of allowed switches by concatenating a string using x macro
    // stringification
#   define CO(w,x,y,z) #w
    static char switches[] = SF65_CMDOPTLIST ; // '\0' is appended automatically
#   undef CO

    // Define an array of pointers to callback functions which are called for
    // a set switch at the same array position as in the switches string
#   define CO(w,x,y,z) sf65_Opt##x,
    static sf65OptionsModifierFnc_t *modifierFncList[] = {SF65_CMDOPTLIST NULL};
#   undef CO

    NOT_NULL ( cmdarg, SF65_CMDERR_NULLPTR ) {
        int filenamePositions[2] = {};
        int filenameCount = 0;

        while ( cmdarg -> argIdx < cmdarg -> argc ) {
            if ( sf65_CMDOpt_GetNextArg ( cmdarg ) != SF65_CMDERR_NOERR ) {
                break;
            }

            if ( sf65_CMDSwitchNotPresent == cmdarg -> hasOpt ) {
                // If no switch character '-' is present, argument must be
                // filename or error. Therefore it is meaningless to continue
                // reading chars one by one like in the else branch

                // Reset option character for the next run
                cmdarg -> optCh = '\0';

                if ( cmdarg -> argIdx < cmdarg -> argc - 2 ) {
                    sf65_pError ( "In/out filenames should be given as last two parameters\n" );
                    exit ( 1 );
                }

                // Assert that there are not more than 2 filenames present
                // which should always be true if above condition fails
                assert ( filenameCount < 2 );
                filenamePositions[filenameCount++] = cmdarg -> argIdx;
            } else {
                sf65_CMDOpt_ReadNextCh ( cmdarg );
                cmdarg -> optCh = cmdarg -> currentCh;

                // Should not read over end of cmd argument
                //assert ( '0' != cmdarg -> currentCh );

                // Check on isolated switch character '-'
                if ( !cmdarg -> currentCh ) {
                    // Test, if option is given after '-' switch character.
                    // However, if cmdArgIdx > argc-2, single '-' is allowed to specify
                    // stdin /stdout
                    if ( cmdarg -> argIdx < cmdarg -> argc - 2 ) {
                        // If no option specified after '-', output err msg and indicate
                        // position of arg neglecting arg 0 (filename of executable)
                        // That's why "cmdArgIdx - 1"
                        sf65_pError ( "Missing option after switch at arg: %d\n", cmdarg -> argIdx - 1 );
                        exit ( 1 );
                    } else {
                        // Assert that there are not more than 2 filenames present
                        // which should always be true if above condition fails
                        assert ( filenameCount < 2 );
                        filenamePositions[filenameCount++] = cmdarg -> argIdx;

                        // If output is beeing redirected to stdout, set verbosity to 0
                        // to prevent mixing diagnostic output and output of formatted
                        // source
                        if ( filenameCount > 1 ) {
                            CMDOptions -> verbosity = 0;
                        }
                    }
                } else {
                    // Determine which options have been given on command line and set variables
                    //d in CMDOptions struct accordingly
                    sf65_DetectMatchingOption ( CMDOptions, cmdarg,
                                                switches, modifierFncList );
                }
            }
        }

        switch ( filenameCount ) {
        case 0:
            sf65_printfUserInfo ( "Neither input nor output file given. Using default filenames\n" );
            sf65_setInFilename ( CMDOptions, predefinedInFilename );
            sf65_setOutFilename ( CMDOptions, predefinedOutFilename );
            break;
        case 1:
            sf65_printfUserInfo ( "No output file given. Using Default filename\n" );
            if ( *cmdarg -> argv[filenamePositions[0]] ) {
                sf65_setInFilename ( CMDOptions, cmdarg -> argv[filenamePositions[0]] );
            } else {
                sf65_setInFilename ( CMDOptions, predefinedInFilename );
            }
            sf65_setOutFilename ( CMDOptions, predefinedOutFilename );

            break;
        case 2:
            if ( *cmdarg -> argv[filenamePositions[0]] ) {
                sf65_setInFilename ( CMDOptions, cmdarg -> argv[filenamePositions[0]] );
            } else {
                sf65_printfUserInfo ( "No input file given. Using Default filename\n" );

                sf65_setInFilename ( CMDOptions, predefinedInFilename );
            }
            if ( *cmdarg -> argv[filenamePositions[1]] ) {
                sf65_setOutFilename ( CMDOptions, cmdarg -> argv[filenamePositions[1]] );
            } else {
                sf65_printfUserInfo ( "No output file given. Using Default filename\n" );

                sf65_setOutFilename ( CMDOptions, predefinedOutFilename );
            }
            break;
        default:
            DO_NOT_REACH ( "Should not come here, because max 2 files may be specified\n" );
        }

        return SF65_CMDERR_NOERR;
    }
}
