#include "sf65.h"

int sf65_ConvertStrToInt ( char* arg ) {
    long int num = -1;
    char *endarg;

    num = strtol ( arg, &endarg, 0 );

    // There should be only numeric until end of arg. If not, return -1 to indicate error
    if ( *endarg != '\0' ) {
        return -1;
    }

    return num;
}

bool checkIf0Or1 ( int val ) {
    return sf65_checkRange ( val, 0, 1 );
}

/*
 * Checks whether val lies in range [min, max]
 */
bool sf65_checkRange ( int val, int min, int max ) {
    if ( val < min ) return false;
    if ( val > max ) return false;
    return true;
}

/*
 * Returns -1,0,1 dependent on sign of argument
 */
int sgn ( int x ) {
    if ( x ) {
        return x / abs ( x );
    }
    return 0;
}

/* Tests, if a pointer is in range between a start pointer and and end pointer
 * defined by the size of the range
 */
bool inRange ( const char *p, const char *first, int size ) {
    return p < ( first + size );
}

/*
 * Returns the value aligned to the next multiple of align or
 * keep value if it is already a multiple of align
 */
int sf65_align ( int val, int align ) {
    return ( val + align - 1 ) / align * align;
}
