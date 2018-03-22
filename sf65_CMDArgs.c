#include "sf65.h"

int processCMDArgs(int argc, char** argv, sf65Options_t *sf65Options){
    /*
    ** Show usage if less than 3 arguments (program name counts as one)
    */
    int c = 0;

    if (argc < 3) {
        fprintf (stderr, "\n");
        fprintf (stderr, "Pretty6502 " VERSION " by Oscar Toledo G. http://nanochess.org/\n");
        fprintf (stderr, "\n");
        fprintf (stderr, "Usage:\n");
        fprintf (stderr, "    pretty6502 [args] input.asm output.asm\n");
        fprintf (stderr, "\n");
        fprintf (stderr, "DON'T USE SAME OUTPUT FILE AS INPUT, though it's possible,\n");
        fprintf (stderr, "you can DAMAGE YOUR SOURCE if this program has bugs.\n");
        fprintf (stderr, "\n");
        fprintf (stderr, "Arguments:\n");
        fprintf (stderr, "    -s0       Code in four columns (default)\n");
        fprintf (stderr, "              label: mnemonic operand comment\n");
        fprintf (stderr, "    -s1       Code in three columns\n");
        fprintf (stderr, "              label: mnemonic+operand comment\n");
        fprintf (stderr, "    -p0       Processor unknown\n");
        fprintf (stderr, "    -p1       Processor 6502 + DASM syntax (default)\n");
        fprintf (stderr, "    -m8       Start of mnemonic column (default)\n");
        fprintf (stderr, "    -o16      Start of operand column (default)\n");
        fprintf (stderr, "    -c32      Start of comment column (default)\n");
        fprintf (stderr, "    -t8       Use tabs of size 8 to reach column\n");
        fprintf (stderr, "    -t0       Use spaces to align (default)\n");
        fprintf (stderr, "    -a0       Align comments to nearest column\n");
        fprintf (stderr, "    -a1       Comments at line start are aligned\n");
        fprintf (stderr, "              to mnemonic (default)\n");
        fprintf (stderr, "    -n4       Nesting spacing (can be any number\n");
        fprintf (stderr, "              of spaces or multiple of tab size)\n");
        fprintf (stderr, "    -l        Puts labels in its own line\n");
        fprintf (stderr, "    -dl       Change directives to lowercase\n");
        fprintf (stderr, "    -du       Change directives to uppercase\n");
        fprintf (stderr, "    -ml       Change mnemonics to lowercase\n");
        fprintf (stderr, "    -mu       Change mnemonics to uppercase\n");
        fprintf (stderr, "\n");
        fprintf (stderr, "Assumes all your labels are at start of line and there is space\n");
        fprintf (stderr, "before mnemonic.\n");
        fprintf (stderr, "\n");
        fprintf (stderr, "Accepts any assembler file where ; means comment\n");
        fprintf (stderr, "[label] mnemonic [operand] ; comment\n");
        exit (1);
    }

    /*
    ** Default settings
    */
    sf65Options -> style = 0;
    sf65Options -> processor = 1;
    sf65Options -> start_mnemonic = 8;
    sf65Options -> start_operand = 16;
    sf65Options -> start_comment = 32;
    sf65Options -> start_directive = 0;//7;
    sf65Options -> tabs = 0;
    sf65Options -> align_comment = 1;
    sf65Options -> nesting_space = 4;
    sf65Options -> labels_own_line = 0;
    sf65Options -> mnemonics_case = 0;
    sf65Options -> directives_case = 0;
    
    // Default to stdin
    sf65Options -> infilename = "-";
    // Default to stdout
    sf65Options -> outfilename = "-";
    
    prev_comment_final_location = 4;

    /*
    ** Process arguments
    */
    c = 1;
    while (c < argc - 2) {
        if (argv[c][0] != '-') {
            fprintf (stderr, "Bad argument\n");
            exit (1);
        }
        switch (tolower (argv[c][1])) {
            case 's':   /* sf65Options -> Style */
                sf65Options -> style = atoi (&argv[c][2]);
                if (sf65Options -> style != 0 && sf65Options -> style != 1) {
                    fprintf (stderr, "Bad sf65Options -> style code: %d\n", sf65Options -> style);
                    exit (1);
                }
                break;
            case 'p':   /* Processor */
                sf65Options -> processor = atoi (&argv[c][2]);
                if (sf65Options -> processor < 0 || sf65Options -> processor > 1) {
                    fprintf (stderr, "Bad sf65Options -> processor code: %d\n", sf65Options -> processor);
                    exit (1);
                }
                break;
            case 'm':   /* Mnemonic start */
                if (tolower (argv[c][2]) == 'l') {
                    sf65Options -> mnemonics_case = 1;
                } else if (tolower (argv[c][2]) == 'u') {
                    sf65Options -> mnemonics_case = 2;
                } else {
                    sf65Options -> start_mnemonic = atoi (&argv[c][2]);
                }
                break;
            case 'o':   /* Operand start */
                sf65Options -> start_operand = atoi (&argv[c][2]);
                break;
            case 'c':   /* Comment start */
                sf65Options -> start_comment = atoi (&argv[c][2]);
                break;
            case 't':   /* Tab size */
                sf65Options -> tabs = atoi (&argv[c][2]);
                break;
            case 'a':   /* Comment alignment */
                sf65Options -> align_comment = atoi (&argv[c][2]);
                if (sf65Options -> align_comment != 0 && sf65Options -> align_comment != 1) {
                    fprintf (stderr, "Bad comment alignment: %d\n", sf65Options -> align_comment);
                    exit (1);
                }
                break;
            case 'n':   /* Nesting space */
                sf65Options -> nesting_space = atoi (&argv[c][2]);
                break;
            case 'l':   /* Labels in own line */
                sf65Options -> labels_own_line = 1;
                break;
            case 'd':   /* Directives */
                if (tolower (argv[c][2]) == 'l') {
                    sf65Options -> directives_case = 1;
                } else if (tolower (argv[c][2]) == 'u') {
                    sf65Options -> directives_case = 2;
                } else {
                    fprintf (stderr, "Unknown argument: %c%c\n", argv[c][1], argv[c][2]);
                }
                break;
            default:    /* Other */
                fprintf (stderr, "Unknown argument: %c\n", argv[c][1]);
                exit (1);
        }
        c++;
    }

    /*
    ** Validate constraints
    */
    if (sf65Options -> style == 1) {
        if (sf65Options -> start_mnemonic > sf65Options -> start_comment) {
            fprintf (stderr, "Operand error: -m%d > -c%d\n", sf65Options -> start_mnemonic, sf65Options -> start_comment);
            exit (1);
        }
        sf65Options -> start_operand = sf65Options -> start_mnemonic;
    } else if (sf65Options -> style == 0) {
        if (sf65Options -> start_mnemonic > sf65Options -> start_operand) {
            fprintf (stderr, "Operand error: -m%d > -o%d\n", sf65Options -> start_mnemonic, sf65Options -> start_operand);
            exit (1);
        }
        if (sf65Options -> start_operand > sf65Options -> start_comment) {
            fprintf (stderr, "Operand error: -o%d > -c%d\n", sf65Options -> start_operand, sf65Options -> start_comment);
            exit (1);
        }
    }
    if (sf65Options -> tabs > 0) {
        if (sf65Options -> start_mnemonic % sf65Options -> tabs) {
            fprintf (stderr, "Operand error: -m%d isn't a multiple of %d\n", sf65Options -> start_mnemonic, sf65Options -> tabs);
            exit (1);
        }
        if (sf65Options -> start_operand % sf65Options -> tabs) {
            fprintf (stderr, "Operand error: -m%d isn't a multiple of %d\n", sf65Options -> start_operand, sf65Options -> tabs);
            exit (1);
        }
        if (sf65Options -> start_comment % sf65Options -> tabs) {
            fprintf (stderr, "Operand error: -m%d isn't a multiple of %d\n", sf65Options -> start_comment, sf65Options -> tabs);
            exit (1);
        }
        if (sf65Options -> nesting_space % sf65Options -> tabs) {
            fprintf (stderr, "Operand error: -n%d isn't a multiple of %d\n", sf65Options -> nesting_space, sf65Options -> tabs);
            exit (1);
        }
    }
    
    sf65Options -> infilename = argv[c];
    sf65Options -> outfilename = argv[c+1];
    
    return c;
}
