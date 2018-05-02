#include "sf65.h"

sf65ErrorDesc_t sf65_Error = {};
FILE *errLog = NULL;

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

sf65ErrCode_t sf65_SetError ( sf65ErrCode_t err, const char *const msg ) {
    sf65_Error.errcode = err;
    strncpy ( sf65_Error.msg, msg, sizeof ( sf65_Error.msg ) );
    if ( errLog ) {
        sf65_fprintf ( errLog, "%s", msg );
    }
    return err;
}

sf65ErrCode_t sf65_GetError ( void ) {
    return sf65_Error.errcode;
}

bool sf65_ErrorOccured ( void ) {
    return sf65_Error.errcode != SF65_NOERR;
}

FILE *sf65_OpenErrLog ( char *filename ) {
    // Try to open file and check for error
    FILE *file = sf65_openFile ( filename, "w" );
    return errLog = file;
}
