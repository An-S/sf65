/*
** sf65 (Source formatter for CA65) by Monte Carlos / Cascade on 03-2018
** havily based on
** Pretty6502
**
** by Oscar Toledo G.
**
** © Copyright 2017 Oscar Toledo G.
**
** Creation date: Nov/03/2017.
** Revision date: Nov/06/2017. Processor selection. Indents nested IF/ENDIF.
**                             Tries to preserve vertical structure of comments.
**                             Allows label in its own line. Allows to change case
**                             of mnemonics and directives.
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define VERSION "v0.2"

int tabs;

int style;
int processor;
int start_mnemonic;
int start_operand;
int start_comment;
int start_directive = 0;
int align_comment;
int nesting_space;
int labels_own_line;

/*
** 6502 mnemonics
*/
char *mnemonics_6502[] = {
    "adc", "anc", "and", "ane", "arr", "asl", "asr", "bcc",
    "bcs", "beq", "bit", "bmi", "bne", "bpl", "brk", "bvc",
    "bvs", "clc", "cld", "cli", "clv", "cmp", "cpx", "cpy",
    "dcp", "dec", "dex", "dey", "eor", "inc", "inx", "iny",
    "isb", "jmp", "jsr", "las", "lax", "lda", "ldx", "ldy",
    "lsr", "lxa", "nop", "ora", "pha", "php", "pla", "plp",
    "rla", "rol", "ror", "rra", "rti", "rts", "sax", "sbc",
    "sbx", "sec", "sed", "sei", "sha", "shs", "shx", "shy",
    "slo", "sre", "sta", "stx", "sty", "tax", "tay", "tsx",
    "txa", "txs", "tya", NULL,
};

#define DONT_RELOCATE_LABEL 0x01
#define LEVEL_IN        0x02
#define LEVEL_OUT       0x04
#define LEVEL_MINUS     0x08

/*
** DASM directives
*/
struct {
    char *directive;
    int flags;
} directives_dasm[] = {
    {"=",         DONT_RELOCATE_LABEL},
    {"a16",  0},
    {"a8",  0},
    {"addr",  0},
    {"align",  0},
    {"asciiz",  0},
    {"assert",  0},
    {"autoimport",  0},
    {"bankbyte",  0},
    {"bankbytes",  0},
    {"blank",  0},
    {"bss",  0},
    {"byt",  0},
    {"byte",  0},
    {"case",  0},
    {"charmap",  0},
    {"code",  0},
    {"concat",  0},
    {"condes",  0},
    {"const",  0},
    {"constructor",  0},
    {"cpu",  0},
    {"data",  0},
    {"dbyt",  0},
    {"debuginfo",  0},
    {"def",  0},
    {"define",  0},
    {"defined",  0},
    {"delmac",  0},
    {"delmacro",  0},
    {"definedmacro",  0},
    {"destructor",  0},
    {"dword",  0},
    {"else",  LEVEL_MINUS},
    {"elseif",  LEVEL_OUT},
    {"end",  0},
    {"endenum",  0},
    {"endif",  LEVEL_OUT},
    {"endmac",  LEVEL_OUT},
    {"endmacro",  LEVEL_OUT},
    {"endproc",  LEVEL_OUT},
    {"endrep",  LEVEL_OUT},
    {"endrepeat",  LEVEL_OUT},
    {"endscope",  LEVEL_OUT},
    {"endstruct",  LEVEL_OUT},
    {"enum",  LEVEL_IN},
    {"error",  0},
    {"exitmac",  0},
    {"exitmacro",  0},
    {"export",  0},
    {"exportzp",  0},
    {"faraddr",  0},
    {"fatal",  0},
    {"feature",  0},
    {"fileopt",  0},
    {"fopt",  0},
    {"forceimport",  0},
    {"global",  0},
    {"globalzp",  0},
    {"hibyte",  0},
    {"hibytes",  0},
    {"hiword",  0},
    {"i16",  0},
    {"i8",  0},
    {"ident",  0},
    {"if",  LEVEL_IN},
    {"ifblank",  LEVEL_IN},
    {"ifconst",  LEVEL_IN},
    {"ifdef",  LEVEL_IN},
    {"ifnblank",  LEVEL_IN},
    {"ifndef",  LEVEL_IN},
    {"ifnref",  LEVEL_IN},
    {"ifp02",  LEVEL_IN},
    {"ifp816",  LEVEL_IN},
    {"ifpc02",  LEVEL_IN},
    {"ifpsc02",  LEVEL_IN},
    {"ifref",  LEVEL_IN},
    {"import",  0},
    {"importzp",  0},
    {"incbin",  0},
    {"include",  0},
    {"interruptor",  0},
    {"left",  0},
    {"linecont",  0},
    {"list",  0},
    {"listbytes",  0},
    {"lobyte",  0},
    {"lobytes",  0},
    {"local",  0},
    {"localchar",  0},
    {"loword",  0},
    {"mac",  LEVEL_IN},
    {"macpack",  0},
    {"macro",  LEVEL_IN},
    {"match",  0},
    {"mid",  0},
    {"org",  0},
    {"out",  0},
    {"p02",  0},
    {"p816",  0},
    {"pagelen",  0},
    {"pagelength",  0},
    {"paramcount",  0},
    {"pc02",  0},
    {"popseg",  0},
    {"proc",  LEVEL_IN},
    {"psc02",  0},
    {"pushseg",  0},
    {"ref",  0},
    {"referenced",  0},
    {"reloc",  0},
    {"repeat",  LEVEL_IN},
    {"res",  0},
    {"right",  0},
    {"rodata",  0},
    {"scope",  LEVEL_IN},
    {"segment",  0},
    {"setcpu",  0},
    {"sizeof",  0},
    {"smart",  0},
    {"sprintf",  0},
    {"strat",  0},
    {"string",  0},
    {"strlen",  0},
    {"struct",  LEVEL_IN},
    {"sunplus",  0},
    {"tag",  0},
    {"tcount",  0},
    {"time",  0},
    {"undef",  0},
    {"undefine",  0},
    {"union",  0},
    {"version",  0},
    {"warning",  0},
    {"word",  0},
    {"xmatch",  0},
    {"zeropage",  0},

    {NULL,       0}
};

/*
** Comparison without case
*/
int memcmpcase (char *p1, char *p2, int size) {
    while (size--) {
        if (tolower (*p1) != tolower (*p2))
            return 1;
        p1++;
        p2++;
    }
    return 0;
}

/*
** Check for opcode or directive
* c > 0 -> directive detected
* c < 0 -> opcode detected
*/
int check_opcode (char *p1, char *p2) {
    int c;
    int length;

    for (c = 0; directives_dasm[c].directive != NULL; c++) {
        length = strlen (directives_dasm[c].directive);
        if ( (*p1 == '.' && length == p2 - p1 - 1 && memcmpcase (p1 + 1, directives_dasm[c].directive, p2 - p1 - 1) == 0) || (length == p2 - p1 && memcmpcase (p1, directives_dasm[c].directive, p2 - p1) == 0)) {
            return c + 1;
        }
    }
    for (c = 0; mnemonics_6502[c] != NULL; c++) {
        length = strlen (mnemonics_6502[c]);
        if (length == p2 - p1 && memcmpcase (p1, mnemonics_6502[c], p2 - p1) == 0)
            return - (c + 1);
    }
    return 0;
}

/*
** Request space in line
*/
void request_space (FILE *output, int *current, int new, int force) {

    /*
    ** If already exceeded space...
    */
    if (*current >= new) {
        if (force){
            fputc (' ', output);
            (*current)++;
        }
        return;
    }

    /*
    ** Advance one step at a time
    */
    if (new){
        while (1) {
            if (tabs == 0) {
                int i = 0;
                for (; i<new-*current; ++i){
                    fprintf(output, " ");
                }
                //fprintf (output, "%*s", new - *current, "");
                *current = new;
            } else {
                fputc ('\t', output);
                *current = (*current + tabs) / tabs * tabs;
            }
            if (*current >= new)
                return;
        }
    }
}

bool inRange(const char *p, const char *first, int size){
    return p < (first + size);
}

char *convertLinefeedsToStringSeparator(char* data, int allocation){
    char *p1 = data;
    char *p2 = p1;
    char request = 0;

    while ( inRange(p1, data, allocation) ) {
        if (*p1 == '\r') {  /* Ignore \r characters */
            p1++; // Here, we increase only p1 but not p2 !
            continue;
        }
        if (*p1 == '\n') {
            p1++;
            *p2++ = '\0';   /* Break line by replacing line terminator with string terminator*/
            request = 1;
            continue;
        }
        *p2++ = *p1++; //Remove \r chars out of code (only case which does not inc p2)
        request = 0;
    }

    if (request == 0)
        *p2++ = '\0';   /* Force line break */

    return p2;
}

/*
 * Detect a word limited by whitespace but always stop at comment symbol ';'
 *
 * @param Pointer to start address of char sequence
 * @return First whitespace or comment char after
 *
 */
char *detectCodeWord(char *p){
    while (*p && !isspace (*p) && *p != ';')
        p++;

    return p;
}

/*
 * Read array pointed to by p as long as whitespace is found.
 * Stop at first non whitespace character or string terminator
 *
 * @param Pointer to start address of char sequence
 * @return First non whitespace after start
 *
 */
char *skipWhiteSpace(char *p){
    while (*p && isspace (*p))
        p++;

    return p;
}

/*
 * Iterate over char array from p1 to p2.
 * Call modificator function for each of the chars of the array
 * and write back modificated char.
 *
 * @param p1: Pointer to start
 * @param p2: Pointer to end
 * @return End of array
 *
 */
char *modifyChars(char *p1, char *p2, int func(int)){
    while (p1 < p2) {
        *p1 = func (*p1);
        p1++;
    }
    return p1;
}

char *changeCase(char *p1, char *p2, char _case){
    switch (_case) {
        case 1:
            modifyChars(p1, p2, tolower);
            break;
        case 2:
            modifyChars(p1, p2, toupper);
            break;
        default:
            break;
    }
    return p2;
}

int detectOpcode(char *p1, char *p2, int processor, int *outputColumn, int *flags){
    int opIndex = -65000; //invalidate opIndex

// For 6502 processor check codeword against opcode list
    if (processor == 1) {
        opIndex = check_opcode (p1, p2);
        if (opIndex == 0) {
            *outputColumn = 0; //Set output column to start_mnemonic column
        } else if (opIndex < 0) {
            *outputColumn = start_mnemonic;
        } else {
            *flags = directives_dasm[opIndex - 1].flags;
            if (*flags & DONT_RELOCATE_LABEL)
                *outputColumn = 0;
            else
                *outputColumn = start_directive;
        }
    // For other processors just assume a mnemonic wtho checking
    } else {
        *outputColumn = start_mnemonic;
        opIndex = 0;
    }

    return opIndex;
}

/*
** Main program
*/
int main (int argc, char *argv[]) {
    int c;

    FILE *input;
    FILE *output;
    int allocation;
    char *data;
    char *p;
    char *p1;
    char *p2;
    int current_column;
    int request;
    int current_level;
    int prev_comment_original_location;
    int prev_comment_final_location;
    int flags;
    int mnemonics_case;
    int directives_case;

    /*
    ** Show usage if less than 3 arguments (program name counts as one)
    */
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
    style = 0;
    processor = 1;
    start_mnemonic = 8;
    start_operand = 16;
    start_comment = 32;
    tabs = 0;
    align_comment = 1;
    nesting_space = 4;
    labels_own_line = 0;
    mnemonics_case = 0;
    directives_case = 0;

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
            case 's':   /* Style */
                style = atoi (&argv[c][2]);
                if (style != 0 && style != 1) {
                    fprintf (stderr, "Bad style code: %d\n", style);
                    exit (1);
                }
                break;
            case 'p':   /* Processor */
                processor = atoi (&argv[c][2]);
                if (processor < 0 || processor > 1) {
                    fprintf (stderr, "Bad processor code: %d\n", processor);
                    exit (1);
                }
                break;
            case 'm':   /* Mnemonic start */
                if (tolower (argv[c][2]) == 'l') {
                    mnemonics_case = 1;
                } else if (tolower (argv[c][2]) == 'u') {
                    mnemonics_case = 2;
                } else {
                    start_mnemonic = atoi (&argv[c][2]);
                }
                break;
            case 'o':   /* Operand start */
                start_operand = atoi (&argv[c][2]);
                break;
            case 'c':   /* Comment start */
                start_comment = atoi (&argv[c][2]);
                break;
            case 't':   /* Tab size */
                tabs = atoi (&argv[c][2]);
                break;
            case 'a':   /* Comment alignment */
                align_comment = atoi (&argv[c][2]);
                if (align_comment != 0 && align_comment != 1) {
                    fprintf (stderr, "Bad comment alignment: %d\n", align_comment);
                    exit (1);
                }
                break;
            case 'n':   /* Nesting space */
                nesting_space = atoi (&argv[c][2]);
                break;
            case 'l':   /* Labels in own line */
                labels_own_line = 1;
                break;
            case 'd':   /* Directives */
                if (tolower (argv[c][2]) == 'l') {
                    directives_case = 1;
                } else if (tolower (argv[c][2]) == 'u') {
                    directives_case = 2;
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
    if (style == 1) {
        if (start_mnemonic > start_comment) {
            fprintf (stderr, "Operand error: -m%d > -c%d\n", start_mnemonic, start_comment);
            exit (1);
        }
        start_operand = start_mnemonic;
    } else if (style == 0) {
        if (start_mnemonic > start_operand) {
            fprintf (stderr, "Operand error: -m%d > -o%d\n", start_mnemonic, start_operand);
            exit (1);
        }
        if (start_operand > start_comment) {
            fprintf (stderr, "Operand error: -o%d > -c%d\n", start_operand, start_comment);
            exit (1);
        }
    }
    if (tabs > 0) {
        if (start_mnemonic % tabs) {
            fprintf (stderr, "Operand error: -m%d isn't a multiple of %d\n", start_mnemonic, tabs);
            exit (1);
        }
        if (start_operand % tabs) {
            fprintf (stderr, "Operand error: -m%d isn't a multiple of %d\n", start_operand, tabs);
            exit (1);
        }
        if (start_comment % tabs) {
            fprintf (stderr, "Operand error: -m%d isn't a multiple of %d\n", start_comment, tabs);
            exit (1);
        }
        if (nesting_space % tabs) {
            fprintf (stderr, "Operand error: -n%d isn't a multiple of %d\n", nesting_space, tabs);
            exit (1);
        }
    }

    /*
    ** Open input file, measure it and read it into buffer
    */
    input = fopen (argv[c], "rb");
    if (input == NULL) {
        fprintf (stderr, "Unable to open input file: %s\n", argv[c]);
        exit (1);
    }
    fprintf (stderr, "Processing %s...\n", argv[c]);
    fseek (input, 0, SEEK_END);
    allocation = ftell (input);
    data = malloc (allocation + sizeof (char));
    if (data == NULL) {
        fprintf (stderr, "Unable to allocate memory\n");
        fclose (input);
        exit (1);
    }
    fseek (input, 0, SEEK_SET);

    // Read file at once into data
    if (fread (data, sizeof (char), allocation, input) != allocation) {
        fprintf (stderr, "Something went wrong reading the input file\n");
        fclose (input);
        free (data);
        exit (1);
    }
    fclose (input);

    /*
    ** Ease processing of input file
    */
    request = 0;

    // Set read pointers to begin of data
    p1 = data;

    //@Todo: Is this code which only task is to remove \r chars really necessary?

    p2 = convertLinefeedsToStringSeparator(data, allocation);
    allocation = p2 - data; // Correct size of data by using p2

    /*
    ** Now generate output file
    */
    c++;
    output = fopen (argv[c], "w");
    if (output == NULL) {
        fprintf (stderr, "Unable to open output file: %s\n", argv[c]);
        exit (1);
    }
    prev_comment_original_location = 0;
    prev_comment_final_location = 0;
    current_level = 0;

    // Init p with start of data
    p = data;

    // Loop until p out of range
    while (inRange(p, data, allocation) ) {
        current_column = 0;

        // Here, all pointers are set to the same address
        p1 = p;

        // Loop, if *p2 is not \0 and not \s and not ";" -> Label detection
        p2 = detectCodeWord(p);

        // Ok, p2 was increased (while run at lease 1 iteration) -> Label detected
        if (p2 - p1) {  /* Label or mnemonic or directive */
            c = detectOpcode(p1, p2, processor, &request, &flags);
            if (c < 0) {
                changeCase(p1, p2, mnemonics_case);
            } else if (c > 0) {
                changeCase(p1, p2, directives_case);
            }

            if (current_column != 0 && labels_own_line != 0 && (flags & DONT_RELOCATE_LABEL) == 0) {
                fputc ('\n', output);
                current_column = 0;
            }
            if (flags & LEVEL_OUT) {
                if (current_level > 0)
                    current_level--;
            }

            if (request > 0){
                // Indent by level times tab width
                request += current_level * nesting_space;

                // Unindent by one level
                if (flags & LEVEL_MINUS)
                    if (request >= nesting_space) request -= nesting_space;

                // Insert space into output dependent on indent
                request_space (output, &current_column, request, 0);
            }
            //Write Label to output (p2 points to end of label, p1 points to start of line)
            fwrite (p1, sizeof (char), p2 - p1, output);
            current_column += p2 - p1;

            //Now put p1 also to the end of the label
            p1 = p2;
        } else {
            current_column = 0;
        }

        // Loop over all chars which are whitespace but not \0
        p1 = skipWhiteSpace(p1);

        //p1 points to first char other than space or \0 after start of line or end of label
        flags = 0;

        if (*p1 && *p1 != ';') {    /* Mnemonic or directive*/
            // p1 points to start of codeword, p2 be moved to end of word
            p2 = detectCodeWord(p1);
            c = detectOpcode(p1, p2, processor, &request, &flags);

            // Use p3 to iterate over codeword and eventually change case
            if (c < 0) {
                changeCase(p1, p2, mnemonics_case);
            } else if ( c > 0) {
                changeCase(p1, p2, directives_case);
                current_column = 0;
            } else { }

            /*
            ** Move label to own line
            */
            if (current_column != 0 && labels_own_line != 0 && (flags & DONT_RELOCATE_LABEL) == 0) {
                fputc ('\n', output);
                current_column = 0;
            }
            if (flags & LEVEL_OUT) {
                if (current_level > 0)
                    current_level--;
            }

            // Indent by level times tab width
            request += current_level * nesting_space;

            // Unindent by one level
            if (flags & LEVEL_MINUS)
                if (request > nesting_space) request -= nesting_space;

            // Insert space into output dependent on indent
            request_space (output, &current_column, request, 0);
            if (p2-p1) fwrite (p1, sizeof (char), p2 - p1, output);

            current_column += p2 - p1;

            p1 = p2;
            p1 = skipWhiteSpace(p1);
            //while (*p1 && isspace (*p1))
            //    p1++;

            if (*p1 && *p1 != ';') {    /* Operand */
                request = start_operand;
                request += current_level * nesting_space;
                request_space (output, &current_column, request, 1);
                p2 = p1;
                while (*p2 && *p2 != ';') {
                    if (*p2 == '"') {
                        p2++;
                        while (*p2 && *p2 != '"')
                            p2++;
                        p2++;
                    } else if (*p2 == '\'') {
                        p2++;
                        while (*p2 && *p2 != '"')
                            p2++;
                        p2++;
                    } else {
                        p2++;
                    }
                }
                while (p2 > p1 && isspace (* (p2 - 1)))
                    p2--;
                fwrite (p1, sizeof (char), p2 - p1, output);
                current_column += p2 - p1;
                p1 = p2;
                while (*p1 && isspace (*p1))
                    p1++;
            }
            if (flags & LEVEL_IN) {
                current_level++;
            }
        }
        if (*p1 == ';') {   /* Comment */

            /*
            ** Try to keep comments horizontally aligned (only works
            ** if spaces were used in source file)
            */
            p2 = p1;
            while (p2 - 1 >= p && isspace (* (p2 - 1)))
                p2--;
            if (p2 == p && p1 - p == prev_comment_original_location) {
                request = prev_comment_final_location;
            } else {
                prev_comment_original_location = p1 - p;
                if (current_column == 0)
                    request = 0;
                else if (current_column < start_mnemonic)
                    request = start_mnemonic;
                else
                    request = start_comment;
                if (current_column == 0 && align_comment == 1)
                    request = start_mnemonic;
                prev_comment_final_location = request;
            }
            request_space (output, &current_column, request, 0);
            p2 = p1;
            while (*p2)
                p2++;
            while (p2 > p1 && isspace (* (p2 - 1)))
                p2--;
            fwrite (p1, sizeof (char), p2 - p1, output);
            fputc ('\n', output);
            current_column += p2 - p1;
            while (*p++) ;
            continue;
        }
        fputc ('\n', output);
        while (*p++) ;
    }
    fclose (output);
    free (data);
    exit (0);
}
