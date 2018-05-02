#ifndef __SF65_H__
#define __SF65_H__

#define VERSION "v0.2"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>

#define NOT_NULL(arg, errorCode) if(!(arg)){assert( (arg) != NULL );return errorCode;} else
#define _SF65_STR(x) #x
#define SF65_MKSTR(x) _SF65_STR(x)

#include "sf65_types.h"
#include "stringfunctions/sf65_stringfunctions.h"
#include "errorhandling/sf65_errorhandling.h"
#include "mathfunctions/sf65_math.h"
#include "cmdlineparsing/sf65_cmdline.h"
#include "fileio/fileio.h"
#include "outputformatting/sf65_outputformatting.h"
#include "expressionparsing/sf65_expressionparsing.h"
#include "expressioneval/sf65_expression_eval.h"

char echoChar ( char ch );




#endif
