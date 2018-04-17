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

/*
 * Open input file, check error
 */
FILE *sf65_openInputFile ( char *filename ) {
    // This call includes error checking
    FILE *input = sf65_openFile ( filename, "rb" );

    return input;
}

/*
 * Open output file, check error
 */
FILE *sf65_openOutputFile ( char *filename ) {
    // This call includes error checking
    FILE *output = sf65_openFile ( filename, "w" );
    return output;
}

void sf65_fprintf ( FILE *file, const char *format, ... ) {
    va_list va;
    va_start ( va, format );
    vfprintf ( file, format, va );
    if ( ferror ( file ) ) {
        sf65_pError ( strerror ( ferror ( file ) ) );
    }
    va_end ( va );
}
