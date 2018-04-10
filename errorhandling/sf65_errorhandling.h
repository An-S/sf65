#ifndef __SF65ERRHANDL_H__
#define __SF65ERRHANDL_H__

/*
 * Prints a formatted string to the stderr taking a variable number of args
 */
void sf65_pError ( char *format, ... );

/*
 * Prints a formatted string to stderr but take va_list object as second arg
 */
void sf65_vpError ( char *format, va_list va );

#endif
