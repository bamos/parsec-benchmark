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
*    voxel.c:  Reformat maps into voxel map for better cache behavior?        *
*               Not used in SPLASH version since not much perf. impact.       *
*                                                                             *
******************************************************************************/

#include "incl.h"

VOXEL *vox_address;
short vox_len[NM];
long vox_length;
int vox_xlen,vox_xylen;

EXTERN_ENV

Voxel()
{
  int i;
  NORMAL *local_norm_address;
  OPACITY *local_opc_address;
  VOXEL *local_vox_address;

  for (i=0; i<NM; i++)
    vox_len[i] = opc_len[i];
  vox_length = norm_length + opc_length; /* opc_length = norm_length */
  Allocate_Voxel(&vox_address,vox_length);
  local_norm_address = norm_address;
  local_opc_address = opc_address;
  local_vox_address = vox_address;
  for (i=0; i<opc_length; i++) {
    *local_vox_address++ = *local_norm_address++;
    *local_vox_address++ = *local_opc_address++;
  }
  vox_xlen = vox_len[X] * 2 + 3;
  vox_xylen = vox_len[X] * vox_len[Y] * 2 + 3;
  Deallocate_Normal(norm_address);
  Deallocate_Opacity(opc_address);
}


Allocate_Voxel(address, length)
     VOXEL **address;
     long length;
{
  unsigned int i,j,size,type_per_page,count,block;
  unsigned int p,numbytes;

  printf("    Allocating voxel map of %ld bytes...\n",
	 length*sizeof(VOXEL));

  *address = (VOXEL *)NU_MALLOC(length*sizeof(VOXEL),0);

  if (*address == NULL)
    Error("    No space available for map.\n");

/*  POSSIBLE ENHANCEMENT:  Here's where one might distribute the 
    voxel map among physical memories if one wanted to.  If this 
    routine were used at all, that is.
*/

  for (i=0; i<length; i++) *(*address+i) = 0;

}


