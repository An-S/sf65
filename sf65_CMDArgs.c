#include "sf65.h"

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
int processCMDArgs ( int argc, char** argv, sf65Options_t * CMDOptions ) {
    static char predefinedInFilename[]  = "in.src";
    static char predefinedOutFilename[]  = "out.src";

    sf65CMDArg_t *cmdarg = & ( sf65CMDArg_t ) {};

    int cmdArgIdx;
    char *currentOptPtr;
    int filenamePositions[2] = {};
    int filenameCount = 0;

    sf65_SetDefaultCMDOptions ( CMDOptions );
    sf65_CMDOpt_InitParser ( cmdarg, argv, argc );

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
        if ( sf65_CMDOpt_GetNextArg ( cmdarg ) != SF65_CMDERR_NOERR ) {
            return -1;
        }

        if ( cmdarg -> hasOpt ) {
            sf65_CMDOpt_ReadNextCh ( cmdarg );
        }

        // Detect switch character '-' and return pointer to char directly following '-'
        currentOptPtr = getOpt ( cmdArgIdx, argv );

        // Test, if option is given after '-' switch character.
        // However, if cmdArgIdx > argc-2, single '-' is allowed to specify
        // stdin /stdout
        if ( !* ( currentOptPtr + 1 ) && cmdArgIdx < argc - 2 ) {
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

                // If output is beeing redirected to stdout, set verbosity to 0
                // to prevent mixing diagnostic output and output of formatted
                // source
                if ( filenameCount > 1 ) {
                    CMDOptions -> verbosity = 0;
                }
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
