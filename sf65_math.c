#include "sf65.h"

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
