#ifndef __SF65FILEIO_H__
#define __SF65FILEIO_H__

/*
 * Globals
 */
extern FILE *input;
extern FILE *output;

/*
 * Open input file (unformatted source), check error
 */
FILE *sf65_openInputFile ( char *filename );

/*
 * Open output file (formatted source), check error
 */
FILE *sf65_openOutputFile ( char *filename );


#endif
