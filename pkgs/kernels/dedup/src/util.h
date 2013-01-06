#ifndef _UTIL_H_
#define _UTIL_H_

#include "dedupdef.h"

/* File I/O with error checking */
int xread(int sd, void *buf, size_t len);
int xwrite(int sd, const void *buf, size_t len);

/* Process file header */
int read_header(int fd, byte *compress_type);
int write_header(int fd, byte compress_type);

#endif //_UTIL_H_

