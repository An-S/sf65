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

/*
 * Wrapper for standard fprintf function with checking of ferror
 */
int sf65_fprintf ( FILE *file, const char *format, ... );

/*
 * Wrapper for fopen with checking of error
 */
FILE *sf65_openFile ( char *filename, char *mode );

/*
 * Function to strip a base filename from filename input and add or replace the extension
 */
char *sf65_addReplaceFileExt ( char *filename, char *ext );

#endif
