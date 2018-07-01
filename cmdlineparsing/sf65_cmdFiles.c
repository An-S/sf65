#include "sf65.h"


char *sf65_setInFilename ( sf65Options_t * cmdopt, char * fname ) {
    NOT_NULL ( cmdopt, NULL ); NOT_NULL ( fname, NULL ) {
        return cmdopt -> infilename = fname;
    }
}


char *sf65_setOutFilename ( sf65Options_t * cmdopt, char * fname ) {
    NOT_NULL ( cmdopt, NULL ); NOT_NULL ( fname, NULL ) {
        return cmdopt -> outfilename = fname;
    }
}
