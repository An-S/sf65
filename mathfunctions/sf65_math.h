#ifndef __SF65MATH_H__
#define __SF65MATH_H__

/*
 * Returns the value aligned to the next multiple of align or
 * keep value if it is already a multiple of align
 */
int sf65_align ( int val, int align );

/*
 * Returns -1,0,1 dependent on sign of argument
 */
int sgn ( int x );

/* Tests, if a pointer is in range between a start pointer and and end pointer
 * defined by the size of the range
 */
bool inRange ( const char *p, const char *first, int size );

/*
 * Takes a pointer to a char array and attempts to convert
 * the string to a corresponding numeric representation.
 * If conversion fails, -1 is returned.
 */
int sf65_ConvertStrToInt ( char* arg );

#endif
