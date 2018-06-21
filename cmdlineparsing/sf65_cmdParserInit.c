#include "sf65_cmdline.h"

sf65CMDErrCode_t sf65_CMDOpt_InitParser ( sf65CMDArg_t *arg, int argc, char **argv ) {
    CHECK_NULL ( arg, SF65_CMDERR_NULLPTR ); CHECK_NULL ( argv, SF65_CMDERR_NULLPTR ) {

        // At least filename of executable is passed, so argc is min == 1
        assert ( argc > 0 );

        arg -> argv = argv;
        // Retrieve argument only, if command line arguments were given
        // (beside the filename of the executable)
        arg -> currentPtr = argv[0];
        arg -> argIdx = 0;
        return SF65_CMDERR_NOERR;
    }
}

void sf65_SetDefaultCMDOptions ( sf65Options_t *CMDOptions ) {
    /*WITH ( CMDOptions ) {
        WLET ( style, 0 );
    }*/

    CMDOptions -> style = 0;
    CMDOptions -> verbosity = 0;
    CMDOptions -> processor = 1;
    CMDOptions -> start_mnemonic = 8;
    CMDOptions -> start_operand = 16;
    CMDOptions -> start_comment = 32;
    CMDOptions -> start_directive = 0;//7;
    CMDOptions -> tabs = 0;
    CMDOptions -> align_comment = 1;
    CMDOptions -> nesting_space = 4;
    CMDOptions -> labels_own_line = 0;
    CMDOptions -> oversized_labels_own_line = 1;
    CMDOptions -> mnemonics_case = 0;
    CMDOptions -> directives_case = 0;
    CMDOptions -> pad_directives = 1;

    CMDOptions -> locallabelch = '@';

    // Default to stdin
    CMDOptions -> infilename = "-";
    // Default to stdout
    CMDOptions -> outfilename = "-";
}
