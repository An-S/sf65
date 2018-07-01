#include "sf65.h"

void do_not_reach ( char *file, unsigned int line, char *format, ... ) {
    va_list va;
    va_start ( va, format );
    sf65_pError ( "%s (%u) <<", file, line );
    vfprintf ( stderr, format, va );
    va_end ( va );
    abort();
}
