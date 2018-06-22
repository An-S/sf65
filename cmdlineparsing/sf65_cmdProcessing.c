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

sf65CMDErrCode_t sf65_ProcessCmdArgs ( sf65CMDArg_t *cmdarg ) {
    static char predefinedInFilename[]  = "in.src";
    static char predefinedOutFilename[]  = "out.src";

    NOT_NULL ( cmdarg, SF65_CMDERR_NULLPTR ) {
        int filenamePositions[2] = {};
        int filenameCount = 0;

        while ( cmdarg -> argIdx < cmdarg -> argc ) {
            if ( sf65_CMDOpt_GetNextArg ( cmdarg ) != SF65_CMDERR_NOERR ) {
                return -1;
            }

            if ( sf65_CMDSwitchNotPresent == cmdarg -> hasOpt ) {
                // If no switch character '-' is present, argument must be
                // filename or error. Therefore it is meaningless to continue
                // reading chars one by one like in the else branch

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

                // Should not read over end of cmd argument
                //assert ( '0' != cmdarg -> currentCh );

                // Test, if option is given after '-' switch character.
                // However, if cmdArgIdx > argc-2, single '-' is allowed to specify
                // stdin /stdout
                if ( !cmdarg -> currentCh && cmdarg -> argIdx < cmdarg -> argc - 2 ) {
                    // If no option specified after '-', output err msg and indicate
                    // position of arg neglecting arg 0 (filename of executable)
                    // That's why "cmdArgIdx - 1"
                    sf65_pError ( "Missing option after switch at arg: %d\n", cmdarg -> argIdx - 1 );
                    exit ( 1 );
                }

                if ( !*cmdarg -> currentPtr && cmdarg -> argIdx >= cmdarg -> argc - 2 ) {
                    /*if ( cmdArgIdx < argc - 2 ) {
                        sf65_pError ( "'-' for stdin/stdout can be given only for last two parameters\n" );
                        exit ( 1 );
                    }*/
                    //if ( filenameCount < 2 ) {

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
                } else {
                    // Determine which options have been given on command line and set variables
                    // in CMDOptions struct accordingly
                    detectCMDLineSwitches ( CMDOptions, cmdarg );
                }
            }
        }
        return SF65_CMDERR_NOERR;
    }
}
