#include "sf65.h"

/*
 * Open input file, check error
 */
FILE *sf65_openInputFile(char *filename){
    FILE *input = fopen (filename, "rb");
    if (input == NULL) {
        fprintf (stderr, "Unable to open input file: %s\n", filename);
        exit (1);
    }
}

/*
 * Open output file, check error
 */
FILE *sf65_openOutputFile(char *filename){
    FILE *output = fopen (filename, "w");
    if (output == NULL) {
        fprintf (stderr, "Unable to open output file: %s\n", filename);
        exit (1);
    }
    return output;
}
    
   