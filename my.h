#include <stdint.h>

#define FALSE	0
#define TRUE	(!FALSE)
#ifndef NULL
#define NULL ((void *)0)
#endif
#define putch(a)  putc((a),stdout)

#ifdef _MSC_VER 
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif
