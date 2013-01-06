#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdio.h>
#include <stdlib.h>

#define TRACE(fmt, msg...) {                                               \
       fprintf(stderr, "[%s] " fmt, __FUNCTION__, ##msg);                  \
       }                               \

#define EXIT_TRACE(fmt, msg...) {                                          \
       TRACE(fmt, ##msg);                                                  \
       exit(-1);                                                           \
}

#ifndef HERE
#define HERE TRACE("file %s, line %d, func %s\n",  __FILE__, __LINE__, __FUNCTION__)
#endif

#endif //_DEBUG_H_

