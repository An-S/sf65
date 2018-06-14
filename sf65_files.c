#include "sf65.h"

void sf65_conditionallyPrintFError ( FILE *file ) {
    NOT_NULL ( file, ) {
        if ( ferror ( file ) ) {
            sf65_pError ( strerror ( ferror ( file ) ) );
            clearerr ( file );
        }
    }
}

FILE *sf65_openFile ( char *filename, char *mode ) {
    NOT_NULL ( filename, NULL  ) {
        NOT_NULL ( mode, NULL ) {
            FILE *aFile = fopen ( filename, mode );
            if ( aFile == NULL ) {
                fprintf ( stderr, "Unable to open file \"%s\" as "
                          "\"%s\"\n", filename, mode );
                exit ( 1 );
            }
            return aFile;
        }
        return NULL;
    }
}

char *sf65_addReplaceFileExt ( char * filename, char * ext ) {
    NOT_NULL ( filename, NULL ); NOT_NULL ( ext, NULL ) {
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
    return NULL;
}

/*
 * Open input file, check error
 */
FILE *sf65_openInputFile ( char * filename ) {
    NOT_NULL ( filename, NULL ) {
        FILE *input = NULL;
        if ( strlen ( filename ) == 1 && *filename == '-' ) {
            sf65_fprintf ( stdout, "Reading from stdin\n" );
            return stdin;
        }

        sf65_fprintf ( stdout, "Trying to open input file: \"%s\"\n", filename );

        // This call includes error checking
        input = sf65_openFile ( filename, "r" );

        return input;
    }
    return NULL;
}

/*
 * Open output file, check error
 */
FILE *sf65_openOutputFile ( char * filename ) {
    NOT_NULL ( filename, NULL ) {
        FILE *output = NULL;
        if ( strlen ( filename ) == 1 && *filename == '-' ) {
            sf65_fprintf ( stdout, "Writing to stdout\n" );
            return stdout;
        }

        sf65_fprintf ( stdout, "Trying to open output file: \"%s\"\n", filename );

        // This call includes error checking
        output = sf65_openFile ( filename, "w" );
        return output;
    }
    return NULL;
}

FILE *sf65_openLogFile ( char * basefilename ) {
    NOT_NULL ( basefilename, NULL ) {
        FILE *output = NULL;
        char *logfilename = sf65_addReplaceFileExt ( basefilename, "log" );

        sf65_fprintf ( stdout, "Trying to open logfile: \"%s\"\n", logfilename );


        // This call includes error checking
        output = sf65_openFile ( logfilename, "w" );
        free ( logfilename );
        return output;
    }
    return NULL;
}

int sf65_conditionalVPrintf ( bool cond, const char * format, va_list va ) {
    if ( cond ) {
        NOT_NULL ( format, -1 ) {
            int printfErr;

            printfErr = vprintf ( format, va );

            return printfErr;
        }
    }
    return 0;
}

int sf65_conditionalPrintf ( bool cond, const char * format, ... ) {
    va_list va;
    int printfErr;

    va_start ( va, format );
    printfErr = sf65_conditionalVPrintf ( cond, format, va );
    va_end ( va );
    return printfErr;
}

int sf65_fprintf ( FILE * file, const char * format, ... ) {
    NOT_NULL ( file, -1 ); NOT_NULL ( format, -1 ) {
        va_list va;
        int fprintfErr;

        va_start ( va, format );
        fprintfErr = vfprintf ( file, format, va );
        sf65_conditionallyPrintFError ( file );
        va_end ( va );
        return fprintfErr;
    }
    return -1;
}

int sf65_printfUserInfo ( const char * format, ... ) {
    va_list va;
    int printfErr;

    va_start ( va, format );
    printfErr = vprintf ( format, va );
    va_end ( va );

    return printfErr;
}

int sf65_printfVerbose ( int min_verbosity, sf65Options_t *cmdOpt, const char * format, ... ) {
    va_list va;
    int printfErr;

    va_start ( va, format );
    printfErr = sf65_conditionalVPrintf ( cmdOpt -> verbosity >= min_verbosity, format, va );
    va_end ( va );

    return printfErr;
}

size_t sf65_fwrite ( char * startPtr, char * endPtr, FILE * file ) {
    NOT_NULL ( file, -1 ) {
        size_t bytesWritten = fwrite ( startPtr, sizeof ( char ), endPtr - startPtr, file );
        sf65_conditionallyPrintFError ( file );
        return bytesWritten;
    }
    return -1;
}

size_t sf65_fputc ( char ch, FILE * file ) {
    NOT_NULL ( file, 0 ) {
        fputc ( ch, file );
        sf65_conditionallyPrintFError ( file );
        return 1;
    }
    return -1;
}

size_t sf65_fwriteCountChars ( char * startPtr, size_t count, FILE * file ) {
    NOT_NULL ( file, 0 ); NOT_NULL ( startPtr, 0 ) {
        size_t bytesWritten = fwrite ( startPtr, sizeof ( char ), count, file );
        sf65_conditionallyPrintFError ( file );
        return bytesWritten;
    }
    return -1;
}

char *sf65_fgets ( FILE * file, char * buf, size_t sz ) {

    // Use binary or instead of logical or, because we really want to get bitor
    NOT_NULL ( file, NULL ); NOT_NULL ( buf, NULL ) {
        char *ptr = fgets ( buf, sz, file );

        // EOF must be passed back to caller,
        // so that "he" knows when it's time to terminate sf65
        if ( !feof ( file ) ) {
            sf65_conditionallyPrintFError ( file );
        }
        return ptr;
    }
    return NULL;
}

size_t sf65_fputnl ( FILE * file ) {
    NOT_NULL ( file, 0 ) {
        fputc ( '\n', file );
        sf65_conditionallyPrintFError ( file );
        return 1;
    }
    return -1;
}

size_t sf65_fputnspc ( FILE * file, int n ) {
    NOT_NULL ( file, 0 ) {
        int i;

        for ( i = n; i > 0; --i ) {
            fputc ( ' ', file );
            sf65_conditionallyPrintFError ( file );
        }

        return n;
    }
    return -1;
}

size_t sf65_fputspc ( FILE * file ) {
    return sf65_fputnspc ( file, 1 );
}


void sf65_CloseFile ( FILE *file ) {
    if ( file ) {
        fclose ( file );
    }
}
