#ifndef __SF65CMDL_H__
#define __SF65CMDL_H__

/*
 * Procedure to process command line arguments given to sf65
 * Fills given struct with values of command line options and/or default values
 */
int processCMDArgs ( int argc, char **argv, sf65Options_t *CMDOptions );


#endif
