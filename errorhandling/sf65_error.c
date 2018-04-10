#include "sf65.h"

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
