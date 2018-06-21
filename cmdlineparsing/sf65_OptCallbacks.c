#include "cmdlineparsing/sf65_cmdline.h"

sf65CMDErrCode_t sf65_OptVerbosity ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
    ++CMDOptions -> verbosity;
    return SF65_CMDERR_NOERR;
}

sf65CMDErrCode_t sf65_OptStyle ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
    CMDOptions -> style = cmdarg -> numArg;
    return SF65_CMDERR_NOERR;
}

sf65CMDErrCode_t sf65_OptNestingLevel ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
    return SF65_CMDERR_NOERR;
}

sf65CMDErrCode_t sf65_OptMnemonic ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
    /*if ( *currentOptPtr == 'l' ) {
        CMDOptions -> mnemonics_case = 1;
    } else if ( *currentOptPtr == 'u' ) {
        CMDOptions -> mnemonics_case = 2;
    } else {
        CMDOptions -> start_mnemonic = cmdcmdarg -> numArg;
    }*/
    return SF65_CMDERR_NOERR;
}

sf65CMDErrCode_t sf65_OptDirective ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
    return SF65_CMDERR_NOERR;
}

sf65CMDErrCode_t sf65_OptComment ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
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
    return SF65_CMDERR_NOERR;
}

sf65CMDErrCode_t sf65_OptAlign ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
    return SF65_CMDERR_NOERR;
}

sf65CMDErrCode_t sf65_OptScopePadding ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
    CMDOptions -> pad_directives = cmdarg -> numArg;
    return SF65_CMDERR_NOERR;
}

sf65CMDErrCode_t sf65_OptLabelPlacement ( sf65Options_t *CMDOptions, sf65CMDArg_t *cmdarg ) {
    return SF65_CMDERR_NOERR;
}
