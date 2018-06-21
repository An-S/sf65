#include "sf65.h"

char sf65_CMDOpt_ReadNextCh ( sf65CMDArg_t *arg ) {
    NOT_NULL ( arg, 0 ) {
        arg -> currentCh = *arg -> currentPtr;
        ++arg -> currentPtr;
        return arg -> currentCh;
    }
}

sf65CMDErrCode_t sf65_CMDOpt_GetNextArg ( sf65CMDArg_t *arg ) {
    NOT_NULL ( arg, SF65_CMDERR_NULLPTR ) {
        // Can get next argument only, if still arguments left
        // argc is 1, when no arguments are given and argv[0] points to the filename of the executable
        // So, this offset of one argument has to be subtracted from argc to get the limit
        if ( arg -> argIdx < arg -> argc - 1 ) {
            arg -> currentPtr = arg -> argv[++arg->argIdx];
            arg -> currentCh = *arg -> currentPtr;
            arg -> hasOpt = arg -> currentCh == '-';
        }
    }

}



sf65CMDSwitchPresence_t stripSwitchCharacter ( char **arg ) {
    if ( **arg == '-' ) {
        ++*arg;
        return sf65_CMDSwitchPresent;
    }
    return sf65_CMDSwitchNotPresent;
}

// Gets entry from argv array with index argc. Changes case if switch character is present
char *getOpt ( int argc, char ** argv ) {
    //protect from accidently given NULL ptr
    NOT_NULL ( argv, NULL ) {
        char *arg = argv[argc];

        // Assert, that arg is not NULL ptr (must point to argv entry)
        assert ( arg );

        // -> Remark: User can pass empty string by using "". Therefore the
        // assertion >>assert ( *arg );<< must be replaced by runtime checks

        //change only to lowercase if switch character is present
        //This way, filename parameters are protected from case switching
        if ( *arg == '-' ) {
            // Missing Option after switch character
            if ( ! * ( arg + 1 ) ) {
                return arg;//NULL;
            }
            sf65_ChangeCase ( arg, strchr ( arg, '\0' ), SF65_LOWERC );
        }
        return arg;//NULL; // No arg specified after '-'
    }
}
