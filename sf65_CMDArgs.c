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
int sf65_ParseCMDArgs ( int argc, char** argv, sf65Options_t * CMDOptions ) {
    static char predefinedInFilename[]  = "in.src";
    static char predefinedOutFilename[]  = "out.src";

    sf65CMDArg_t *cmdarg = & ( sf65CMDArg_t ) {};

    int filenamePositions[2] = {};
    int filenameCount = 0;

    sf65_SetDefaultCMDOptions ( CMDOptions );
    sf65_CMDOpt_InitParser ( cmdarg, argc, argv );




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

    return cmdarg -> argIdx;
}
