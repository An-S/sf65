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
    sf65CMDArg_t *cmdarg = & ( sf65CMDArg_t ) {};

    sf65_SetDefaultCMDOptions ( CMDOptions );
    sf65_CMDOpt_InitParser ( cmdarg, argc, argv );

    sf65_ProcessCmdArgs ( CMDOptions, cmdarg );


    /*
    ** Validate constraints
    */

    validateCMDLineSwitches ( CMDOptions );



    return cmdarg -> argIdx;
}
