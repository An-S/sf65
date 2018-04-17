#include "sf65.h"

FILE *sf65_openFile ( char *filename, char *mode ) {
    FILE *aFile = fopen ( filename, mode );
    if ( aFile == NULL ) {
        fprintf ( stderr, "Unable to open file \"%s\" as "
                  "\"%s\"\n", filename, mode );
        exit ( 1 );
    }
    return aFile;
}

char *sf65_addReplaceFileExt ( char *filename, char *ext ) {
    // add 2 to size, because we need place for the basename/ext separator "." and the "\0" str term
    char *generatedFName = calloc ( strlen ( filename ) + strlen ( ext ) + 2 , sizeof ( char ) );
    size_t separatorPos = strcspn ( filename, "." );

    strcpy ( generatedFName, filename );
    if ( separatorPos == strlen ( filename ) ) {
        generatedFName[separatorPos] = '.';
    }
    ++separatorPos;

    strcpy ( generatedFName + separatorPos, ext );
    return generatedFName;
}

/*
 * Open input file, check error
 */
FILE *sf65_openInputFile ( char *filename ) {
    FILE *input = NULL;

    sf65_fprintf ( stdout, "Trying to open input file: \"%s\"", filename );

    // This call includes error checking
    input = sf65_openFile ( filename, "rb" );

    return input;
}

/*
 * Open output file, check error
 */
FILE *sf65_openOutputFile ( char *filename ) {
    FILE *output = NULL;

    sf65_fprintf ( stdout, "Trying to open output file: \"%s\"", filename );

    // This call includes error checking
    output = sf65_openFile ( filename, "w" );
    return output;
}

FILE *sf65_openLogFile ( char *basefilename ) {
    FILE *output = NULL;
    char *logfilename = sf65_addReplaceFileExt ( basefilename, "log" );

    sf65_fprintf ( stdout, "Trying to open logfile: \"%s\"", logfilename );


    // This call includes error checking
    output = sf65_openFile ( logfilename, "w" );
    free ( logfilename );
    return output;
}

int sf65_fprintf ( FILE *file, const char *format, ... ) {
    va_list va;
    int fprintfErr;

    va_start ( va, format );
    fprintfErr = vfprintf ( file, format, va );
    if ( ferror ( file ) ) {
        sf65_pError ( strerror ( ferror ( file ) ) );
    }
    va_end ( va );
    return fprintfErr;
}
