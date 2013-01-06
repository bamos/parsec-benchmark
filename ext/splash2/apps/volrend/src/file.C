/*************************************************************************/
/*                                                                       */
/*  Copyright (c) 1994 Stanford University                               */
/*                                                                       */
/*  All rights reserved.                                                 */
/*                                                                       */
/*  Permission is given to use, copy, and modify this software for any   */
/*  non-commercial purpose as long as this copyright notice is not       */
/*  removed.  All other uses, including redistribution in whole or in    */
/*  part, are forbidden without prior written permission.                */
/*                                                                       */
/*  This software is provided with absolutely no warranty and no         */
/*  support.                                                             */
/*                                                                       */
/*************************************************************************/

/********** storing/loading of large arrays to/from files **********/

#include "incl.h"

#define	PMODE	0644		/* RW for owner, R for group, R for others */
#define	RWMODE	0		/* Read-only                               */

EXTERN_ENV

Create_File(filename)
     char filename[];
{
  int fd;
  if ((fd = creat(filename,PMODE)) == -1) {
    Error("    Can't create %s\n",filename);
  }
  return(fd);
}


Open_File(filename)
     char filename[];
{
  int fd;
  if ((fd = open(filename,RWMODE)) == -1) {
    Error("    Can't open %s\n",filename);
  }
  return(fd);
}


Write_Bytes(fd,array,length)
     int fd;
     unsigned char array[];
     long length;
{
  long n_written;
  long more_written;
  n_written = write(fd,array,MIN(length,32766));
  if (n_written != -1) {
    while (n_written < length) {
      more_written = write(fd,&array[n_written],
			   MIN(length-n_written,32766));
      if (more_written == -1) break;
      n_written += more_written;
    }
  }
  if (n_written != length) {
    Close_File(fd);
    Error("    Write failed on file %d\n",fd);
  }
}


Write_Shorts(fd,array,length)
     int fd;
     unsigned char array[];
     long length;
{
  int i;
  unsigned char byte;
  long n_written;
  long more_written;
#ifdef FLIP
  for (i=0; i<length; i+=2) {
    byte = array[i];
    array[i] = array[i+1];
    array[i+1] = byte;
  }
#endif
  n_written = write(fd,array,MIN(length,32766));
  if (n_written != -1) {
    while (n_written < length) {
      more_written = write(fd,&array[n_written],
			   MIN(length-n_written,32766));
      if (more_written == -1) break;
      n_written += more_written;
    }
  }
  if (n_written != length) {
    Close_File(fd);
    Error("    Write failed on file %d\n",fd);
  }
#ifdef FLIP
  for (i=0; i<length; i+=2) {
    byte = array[i];
    array[i] = array[i+1];
    array[i+1] = byte;
  }
#endif
}


Write_Longs(fd,array,length)
     int fd;
     unsigned char array[];
     long length;
{
  int i;
  unsigned char byte;
  long n_written;
  long more_written;
#ifdef FLIP
  for (i=0; i<length; i+=4) {
    byte = array[i];
    array[i] = array[i+3];
    array[i+3] = byte;
    byte = array[i+1];
    array[i+1] = array[i+2];
    array[i+2] = byte;
  }
#endif
  n_written = write(fd,array,MIN(length,32766));
  if (n_written != -1) {
    while (n_written < length) {
      more_written = write(fd,&array[n_written],
			   MIN(length-n_written,32766));
      if (more_written == -1) break;
      n_written += more_written;
    }
  }
  if (n_written != length) {
    Close_File(fd);
    Error("    Write failed on file %d\n",fd);
  }
#ifdef FLIP
  for (i=0; i<length; i+=4) {
    byte = array[i];
    array[i] = array[i+3];
    array[i+3] = byte;
    byte = array[i+1];
    array[i+1] = array[i+2];
    array[i+2] = byte;
  }
#endif
}


Read_Bytes(fd,array,length)
     int fd;
     unsigned char array[];
     long length;
{
  long n_read;
  long more_read;
  n_read = read(fd,array,MIN(length,32766));
  if (n_read != -1 && n_read != 0) {
    while (n_read < length) {
      more_read = read(fd,&array[n_read],
		       MIN(length-n_read,32766));
      if (more_read == -1 || more_read == 0) break;
      n_read += more_read;
    }
  }
  if (n_read != length) {
    Close_File(fd);
    Error("    Read failed on file %d\n",fd);
  }
}


Read_Shorts(fd,array,length)
     int fd;
     unsigned char array[];
     long length;
{
  int i;
  unsigned char byte;
  long n_read;
  long more_read;
  n_read = read(fd,array,MIN(length,32766));
  if (n_read != -1 && n_read != 0) {
    while (n_read < length) {
      more_read = read(fd,&array[n_read],
		       MIN(length-n_read,32766));
      if (more_read == -1 || more_read == 0) break;
      n_read += more_read;
    }
  }
  if (n_read != length) {
    Close_File(fd);
    Error("    Read failed on file %d\n",fd);
  }
#ifdef FLIP
  for (i=0; i<length; i+=2) {
    byte = array[i];
    array[i] = array[i+1];
    array[i+1] = byte;
  }
#endif
}


Read_Longs(fd,array,length)
     int fd;
     unsigned char array[];
     long length;
{
  int i;
  unsigned char byte;
  long n_read;
  long more_read;
  n_read = read(fd,array,MIN(length,32766));
  if (n_read != -1 && n_read != 0) {
    while (n_read < length) {
      more_read = read(fd,&array[n_read],
		       MIN(length-n_read,32766));
      if (more_read == -1 || more_read == 0) break;
      n_read += more_read;
    }
  }
  if (n_read != length) {
    Close_File(fd);
    Error("    Read failed on file %d\n",fd);
  }
#ifdef FLIP
  for (i=0; i<length; i+=4) {
    byte = array[i];
    array[i] = array[i+3];
    array[i+3] = byte;
    byte = array[i+1];
    array[i+1] = array[i+2];
    array[i+2] = byte;
  }
#endif
}


Close_File(fd)
     int fd;
{
  if (close(fd) == -1) {
    Error("    Can't close file %d\n",fd);
  }
}
