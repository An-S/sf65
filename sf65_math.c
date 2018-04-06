#include "sf65.h"

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
