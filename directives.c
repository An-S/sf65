#include "sf65.h"

/*
** DASM directives
*/
directives_t directives_dasm[] = {
    {"=",         DONT_RELOCATE_LABEL},
    {"a16",  0},
    {"a8",  0},
    {"addr",  0},
    {"align",  0},
    {"asciiz",  0},
    {"assert",  0},
    {"autoimport",  0},
    {"bankbyte",  0},
    {"bankbytes",  0},
    {"blank",  0},
    {"bss",  0},
    {"byt",  0},
    {"byte",  0},
    {"case",  0},
    {"charmap",  0},
    {"code",  0},
    {"concat",  0},
    {"condes",  0},
    {"const",  0},
    {"constructor",  0},
    {"cpu",  0},
    {"data",  0},
    {"dbyt",  0},
    {"debuginfo",  0},
    {"def",  0},
    {"define",  0},
    {"defined",  0},
    {"delmac",  0},
    {"delmacro",  0},
    {"definedmacro",  0},
    {"destructor",  0},
    {"dword",  0},
    {"else",  LEVEL_MINUS},
    {"elseif",  LEVEL_OUT},
    {"end",  0},
    {"endenum",  0},
    {"endif",  LEVEL_OUT},
    {"endmac",  LEVEL_OUT},
    {"endmacro",  LEVEL_OUT},
    {"endproc",  LEVEL_OUT},
    {"endrep",  LEVEL_OUT},
    {"endrepeat",  LEVEL_OUT},
    {"endscope",  LEVEL_OUT},
    {"endstruct",  LEVEL_OUT},
    {"enum",  LEVEL_IN},
    {"error",  0},
    {"exitmac",  0},
    {"exitmacro",  0},
    {"export",  0},
    {"exportzp",  0},
    {"faraddr",  0},
    {"fatal",  0},
    {"feature",  0},
    {"fileopt",  0},
    {"fopt",  0},
    {"forceimport",  0},
    {"global",  0},
    {"globalzp",  0},
    {"hibyte",  0},
    {"hibytes",  0},
    {"hiword",  0},
    {"i16",  0},
    {"i8",  0},
    {"ident",  0},
    {"if",  LEVEL_IN},
    {"ifblank",  LEVEL_IN},
    {"ifconst",  LEVEL_IN},
    {"ifdef",  LEVEL_IN},
    {"ifnblank",  LEVEL_IN},
    {"ifndef",  LEVEL_IN},
    {"ifnref",  LEVEL_IN},
    {"ifp02",  LEVEL_IN},
    {"ifp816",  LEVEL_IN},
    {"ifpc02",  LEVEL_IN},
    {"ifpsc02",  LEVEL_IN},
    {"ifref",  LEVEL_IN},
    {"import",  0},
    {"importzp",  0},
    {"incbin",  0},
    {"include",  0},
    {"interruptor",  0},
    {"left",  0},
    {"linecont",  0},
    {"list",  0},
    {"listbytes",  0},
    {"lobyte",  0},
    {"lobytes",  0},
    {"local",  0},
    {"localchar",  0},
    {"loword",  0},
    {"mac",  LEVEL_IN},
    {"macpack",  0},
    {"macro",  LEVEL_IN},
    {"match",  0},
    {"mid",  0},
    {"org",  0},
    {"out",  0},
    {"p02",  0},
    {"p816",  0},
    {"pagelen",  0},
    {"pagelength",  0},
    {"paramcount",  0},
    {"pc02",  0},
    {"popseg",  0},
    {"proc",  LEVEL_IN},
    {"psc02",  0},
    {"pushseg",  0},
    {"ref",  0},
    {"referenced",  0},
    {"reloc",  0},
    {"repeat",  LEVEL_IN},
    {"res",  0},
    {"right",  0},
    {"rodata",  0},
    {"scope",  LEVEL_IN},
    {"segment",  0},
    {"setcpu",  0},
    {"sizeof",  0},
    {"smart",  0},
    {"sprintf",  0},
    {"strat",  0},
    {"string",  0},
    {"strlen",  0},
    {"struct",  LEVEL_IN},
    {"sunplus",  0},
    {"tag",  0},
    {"tcount",  0},
    {"time",  0},
    {"undef",  0},
    {"undefine",  0},
    {"union",  0},
    {"version",  0},
    {"warning",  0},
    {"word",  0},
    {"xmatch",  0},
    {"zeropage",  0},

    {NULL,       0}
};