#ifndef __SF65_MACROS_H__
#define __SF65_MACROS_H__

#include <stdlib.h>

#define NOT_NULL(arg, errorCode) if(!(arg)){assert( (arg) != NULL );SF65_SETERR(SF65_NULLPTR);return errorCode;} else
#define _SF65_STR(x) #x
#define SF65_MKSTR(x) _SF65_STR(x)

#endif
