#include "sf65.h"

sf65CMDErrCode_t sf65_OptLocalLabelCh ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
    CMDOptions -> locallabelch = cmdarg -> currentCh;
    return SF65_CMDERR_NOERR;
}

sf65CMDErrCode_t sf65_OptVerbosity ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
    ++CMDOptions -> verbosity;
    return SF65_CMDERR_NOERR;
}

sf65CMDErrCode_t sf65_OptStyle ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
    CMDOptions -> style = cmdarg -> numArg;
    return SF65_CMDERR_NOERR;
}

sf65CMDErrCode_t sf65_OptNestingLevel ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
    CMDOptions -> nesting_space = cmdarg -> numArg;

    return SF65_CMDERR_NOERR;
}

sf65CMDErrCode_t sf65_OptMnemonic ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
    if ( cmdarg -> currentCh == 'l' ) {
        CMDOptions -> mnemonics_case = 1;
    } else if ( cmdarg -> currentCh == 'u' ) {
        CMDOptions -> mnemonics_case = 2;
    } else {
        CMDOptions -> start_mnemonic = cmdarg -> numArg;
    }
    return SF65_CMDERR_NOERR;
}

sf65CMDErrCode_t sf65_OptDirective ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
    if ( cmdarg -> currentCh == 'l' ) {
        CMDOptions -> directives_case = 1;
    } else if ( cmdarg -> currentCh == 'u' ) {
        CMDOptions -> directives_case = 2;
    } else {
        CMDOptions -> start_directive = cmdarg -> numArg;
    }

    return SF65_CMDERR_NOERR;
}

sf65CMDErrCode_t sf65_OptOperand ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
    CMDOptions -> start_operand = cmdarg -> numArg;

    return SF65_CMDERR_NOERR;
}

sf65CMDErrCode_t sf65_OptComment ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
    CMDOptions -> start_comment = cmdarg -> numArg;

    return SF65_CMDERR_NOERR;
}

sf65CMDErrCode_t sf65_OptProcessor ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
    CMDOptions -> processor = cmdarg -> numArg;
    return SF65_CMDERR_NOERR;
}

sf65CMDErrCode_t sf65_OptHelp ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
    showCMDOptionsHelp();
    exit ( 1 );
    return SF65_CMDERR_NOERR;
}

sf65CMDErrCode_t sf65_OptTabs ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
    CMDOptions -> tabs = cmdarg -> numArg;

    return SF65_CMDERR_NOERR;
}

sf65CMDErrCode_t sf65_OptAlign ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
    CMDOptions -> align_comment = cmdarg -> numArg;

    return SF65_CMDERR_NOERR;
}

sf65CMDErrCode_t sf65_OptScopePadding ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
    CMDOptions -> pad_directives = cmdarg -> numArg;
    return SF65_CMDERR_NOERR;
}

sf65CMDErrCode_t sf65_OptLabelPlacement ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
    if ( strlen ( cmdarg -> currentPtr ) > 0 ) {
        CMDOptions -> oversized_labels_own_line = cmdarg -> numArg;

        if ( CMDOptions -> oversized_labels_own_line == 2 ) {
            CMDOptions -> labels_own_line = 1;
        }
    } else {
        CMDOptions -> labels_own_line = 1;
    }
    return SF65_CMDERR_NOERR;
}
