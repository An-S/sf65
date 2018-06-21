#include "sf65.h"

void showCMDOptionsHelp ( void ) {
    fprintf ( stderr, "\n" );
    fprintf ( stderr, "SF65 by Monte Carlos / Cascade, Mar 2018. Based on:\n\n" );
    fprintf ( stderr, "Pretty6502 " VERSION " by Oscar Toledo G. http://nanochess.org/\n" );
    fprintf ( stderr, "For more information about the changes since forking, see release notes.\n" );
    fprintf ( stderr, "\n" );
    fprintf ( stderr, "Usage:\n" );
    fprintf ( stderr, "    [./]sf65 [args] [input.asm] [output.asm]\n" );
    fprintf ( stderr, "\n" );
    fprintf ( stderr, "DON'T USE SAME OUTPUT FILE AS INPUT, though it's possible,\n" );
    fprintf ( stderr, "you can DAMAGE YOUR SOURCE if this program has bugs.\n" );
    fprintf ( stderr, "\n" );
    fprintf ( stderr, "You can specify '-' for input and/or output file\n\n" );

    fprintf ( stderr, "Arguments:\n" );
    fprintf ( stderr, "    -s0       Code in four columns (default)\n" );
    fprintf ( stderr, "              label: mnemonic operand comment\n" );
    fprintf ( stderr, "    -s1       Code in three columns\n" );
    fprintf ( stderr, "              label: mnemonic+operand comment\n" );
    /*fprintf ( stderr, "    -p0       Processor unknown\n" );
    fprintf ( stderr, "    -p1       Processor 6502 + DASM syntax (default)\n" );*/
    fprintf ( stderr, "    -m8       Start of mnemonic column (default)\n" );
    fprintf ( stderr, "    -o16      Start of operand column (default)\n" );
    fprintf ( stderr, "    -c32      Start of comment column (default)\n" );
    fprintf ( stderr, "    -t8       Use tabs of size 8 to reach column\n" );
    fprintf ( stderr, "    -t0       Use spaces to align (default)\n" );
    fprintf ( stderr, "    -a0       Align comments to nearest column\n" );
    fprintf ( stderr, "    -a1       Comments at line start are aligned\n" );
    fprintf ( stderr, "              to mnemonic (default)\n" );
    fprintf ( stderr, "    -n4       Nesting spacing (can be any number\n" );
    fprintf ( stderr, "              of spaces or multiple of tab size)\n" );
    fprintf ( stderr, "    -l0       Do not put labels in own line\n" );
    fprintf ( stderr, "    -l[1]     Put oversized labels in own line\n" );
    fprintf ( stderr, "    -l2       Put all labels in own line\n" );
    fprintf ( stderr, "    -dl       Change directives to lowercase\n" );
    fprintf ( stderr, "    -du       Change directives to uppercase\n" );
    fprintf ( stderr, "    -ml       Change mnemonics to lowercase\n" );
    fprintf ( stderr, "    -mu       Change mnemonics to uppercase\n" );
    fprintf ( stderr, "    -e0|1     Pad sectioning directives with empty lines\n" );
    fprintf ( stderr, "\n" );
    fprintf ( stderr, "Assumes all your labels are at start of line and there is space\n" );
    fprintf ( stderr, "before mnemonic.\n" );
    fprintf ( stderr, "\n" );
    fprintf ( stderr, "Accepts any assembler file where ; means comment\n" );
    fprintf ( stderr, "[label] mnemonic [operand] ; comment\n" );
    sf65_fputnl ( stderr );
    sf65_fputnl ( stderr );
}
