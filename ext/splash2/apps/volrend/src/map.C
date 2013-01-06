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

/******************************************************************************
*                                                                             *
*    map.c:   Loads 3D density map.                                           *
*                                                                             *
******************************************************************************/

#include "incl.h"

/* The following declarations show the layout of the .den file.              */
/* If changed, the version number must be incremented and code               */
/* written to handle loading of both old and current versions.               */

				/* Version for new .den files:               */
#define	MAP_CUR_VERSION	1	/*   Initial release                         */
uint16_t map_version;		/* Version of this .den file                 */

uint16_t orig_min[NM],		/* Dimensions of original data file          */
         orig_max[NM],		/*   (CT:  from <file>.header file           */
         orig_len[NM];		/*    ED:  from <file>.mi file)              */

uint16_t extr_min[NM],		/* Portion of file extracted for this map    */
         extr_max[NM],		/*   (mins and maxes will be subset of       */
         extr_len[NM];		/*    orig and lengths will be <= orig)      */

uint16_t map_min[NM],		/* Dimensions of this map                    */
         map_max[NM],		/*   (mins will be 0 in this program and     */
         map_len[NM];		/*    lens may be != extr if warps > 0)      */

uint16_t map_warps;		/* Number of warps since extraction          */
				/*   (0 = none)                              */

uint32_t map_length;		/* Total number of densities in map          */
				/*   (= product of lens)                     */
DENSITY *map_address;		/* Pointer to map                            */

/* End of layout of .den file.                                               */

EXTERN_ENV


Load_Map(filename)
     char filename[];
{
  char local_filename[FILENAME_STRING_SIZE];
  int fd;

  strcpy(local_filename,filename);
  strcat(local_filename,".den");
  fd = Open_File(local_filename);
  
  Read_Shorts(fd,&map_version, (long)sizeof(map_version));
  if (map_version != MAP_CUR_VERSION) 
    Error("    Can't load version %d file\n",map_version);
  
  Read_Shorts(fd,orig_min,(long)sizeof(orig_min));
  Read_Shorts(fd,orig_max,(long)sizeof(orig_max));
  Read_Shorts(fd,orig_len,(long)sizeof(orig_len));
  
  Read_Shorts(fd,extr_min,(long)sizeof(extr_min));
  Read_Shorts(fd,extr_max,(long)sizeof(extr_max));
  Read_Shorts(fd,extr_len,(long)sizeof(extr_len));
  
  Read_Shorts(fd,map_min,(long)sizeof(map_min));
  Read_Shorts(fd,map_max,(long)sizeof(map_max));
  Read_Shorts(fd,map_len,(long)sizeof(map_len));
  
  Read_Shorts(fd,&map_warps,(long)sizeof(map_warps));
  Read_Longs(fd,&map_length,(long)sizeof(map_length));
  
  Allocate_Map(&map_address,map_length);
  
  printf("    Loading map from .den file...\n");
  Read_Bytes(fd,map_address,(long)(map_length*sizeof(DENSITY)));
  Close_File(fd);
}


Allocate_Map(address, length)
     DENSITY **address;
     long length;
{
  int i;
  unsigned int p,numbytes;

  printf("    Allocating density map of %ld bytes...\n",
	 length*sizeof(DENSITY));

  *address = (DENSITY *)NU_MALLOC(length*sizeof(DENSITY),0);

  if (*address == NULL)
    Error("    No space available for map.\n");
  else
    for (i=0; i<length; i++) *(*address+i) = 0;

}


Deallocate_Map(address)
DENSITY **address;
{
  printf("    Deallocating density map...\n");

/*  G_FREE(*address);  */

  *address = NULL;
}














