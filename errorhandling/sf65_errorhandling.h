#ifndef __SF65ERRHANDL_H__
#define __SF65ERRHANDL_H__

/*
 * Define a list of errors which may occur during parsing.
 * The idea is of ERROR is that some functions may indicate errornous condition but
 * if caller needs more info he has to call sf65_GetError function or such.
 */
#define SF65_ERRLIST ER(NOERR), ER(ERROR), ER(NULLPTR), ER(INVALIDARGERR)

#define ER(x) SF65_##x
typedef enum {
    SF65_ERRLIST
} sf65ErrCode_t;
#undef ER

typedef struct sf65ErrorDesc_tag {
    sf65ErrCode_t errcode;
    char msg[80];
} sf65ErrorDesc_t;

/*
 * Prints a formatted string to the stderr taking a variable number of args
 */
void sf65_pError ( char *format, ... );

/*
 * Prints a formatted string to stderr but take va_list object as second arg
 */
void sf65_vpError ( char *format, va_list va );

#define SF65_SETERR_MSG(code, msg) _sf65_SetError(code, msg, __FILE__, __LINE)
#define SF65_SETERR(code) _sf65_SetError(code, "N/A", __FILE__, __LINE__)

sf65ErrCode_t _sf65_SetError ( sf65ErrCode_t code, const char* const msg, char *file, unsigned long int line );
sf65ErrCode_t sf65_GetLastError ( void );
bool sf65_ErrorOccured ( void );
FILE *sf65_OpenErrLog ( char *filename );
sf65ErrCode_t sf65_CloseErrLog ( void );

#endif
