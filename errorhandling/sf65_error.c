#include "sf65.h"

sf65ErrorDesc_t sf65_Error = {};
FILE *errLog = NULL;

#define ER(x) SF65_MKSTR(x##_ERR)
char *errStrings[] = {
    SF65_ERRLIST
};
#undef ER

/*
 * Prints a formatted string to stderr but take va_list object as second arg
 */
void sf65_vpError ( char *format, va_list va ) {
    vfprintf ( stderr, format, va );
}

/*
 * Prints a formatted string to the stderr taking a variable number of args
 */
void sf65_pError ( char *format, ... ) {
    va_list va;
    va_start ( va, format );

    vfprintf ( stderr, format, va );
    va_end ( va );
}

sf65ErrCode_t _sf65_SetError ( sf65ErrCode_t err, const char *const msg, char *file, unsigned long int line ) {
    static unsigned int count = 0;

    sf65_Error.errcode = err;
    strncpy ( sf65_Error.msg, msg, sizeof ( sf65_Error.msg ) );
    if ( errLog ) {
        sf65_fprintf ( errLog, "%04d: Err. #%d, %s with msg: %s in line %lu, %s\n", count, err, errStrings[err], msg, line, file );
        ++count;
    }
    return err;
}

sf65ErrCode_t sf65_GetError ( void ) {
    return sf65_Error.errcode;
}

bool sf65_ErrorOccured ( void ) {
    return sf65_Error.errcode != SF65_NOERR;
}

FILE *sf65_OpenErrLog ( char *basefilename ) {
    // Try to open file and check for error

    NOT_NULL ( basefilename, NULL ) {
        FILE *file = NULL;
        char *errLogFName = sf65_addReplaceFileExt ( basefilename, "err" );

        sf65_fprintf ( stdout, "Trying to open error logfile: \"%s\"\n", errLogFName );

        // This call includes error checking
        file = sf65_openFile ( errLogFName, "w" );
        free ( errLogFName );
        return errLog = file;
    }
    return NULL;
}

sf65ErrCode_t sf65_CloseErrLog ( void ) {
    sf65_CloseFile ( errLog );

    return SF65_NOERR;
}
