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

/********
*
*  tiff_rgba_io.h
*
*  Description:
*    Describes the functions defined in tiff_gl_io.c.
*
*
*********/

#ifndef _TIFF_RGBA_IO_
#define _TIFF_RGBA_IO_

#ifdef __cplusplus
extern "C" {
#endif

/******
*
*  tiff_save_image
*
*  Arguments:
*    char *filename - name of TIFF image target file
*    int *pixels - 32-bit r,g,b,a image
*    int width - image width
*    int height - image height
*
*  Description:
*    Stores the pixels into the given file name as a TIFF file.
*
******/

int tiff_save_rgba(char *, int *, int, int);



/******
*
*  tiff_save_gl_canvas
*
*  Arguments:
*    char *filename - name of TIFF image target file
*    int **pixels - 32-bit r,g,b,a image
*    int *width - image width
*    int *height - image height
*
*  Description:
*    Loads the pixels from the named TIFF file.
*
******/

int tiff_load_rgba(char *, int **, int*, int*);


#ifdef __cplusplus
}
#endif

#endif
