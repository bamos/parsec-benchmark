/* AUTORIGHTS
Copyright (C) 2007 Princeton University
      
This file is part of Ferret Toolkit.

Ferret Toolkit is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cass.h>


typedef unsigned char pixel_t[3];

void inv (pixel_t pixel)
{
	pixel[0] = 255;// - pixel[0];
	pixel[1] = 255;// - pixel[1];
	pixel[2] = 255;// - pixel[2];
}

void add_edge(unsigned char *rgb, int nx, int ny, unsigned char *rmap)
{
  int iy,ix,l1,l2;
  int mapsize;
  pixel_t *pixels = (pixel_t *)rgb;

  mapsize = ny*nx;

  l1 = 0;
  for (iy=0;iy<ny;iy++)
  {
    for (ix=0;ix<nx-1;ix++)
    {
      l2 = l1+1;
      if (rmap[l1]!=rmap[l2])
      {
	      inv(pixels[l1]);
	      inv(pixels[l2]);
      }
      l1++;
    }
    l1++;
  }
  l1 = 0;
  for (iy=0;iy<ny-1;iy++)
  {
    for (ix=0;ix<nx;ix++)
    {
      l2 = l1+nx;
      if (rmap[l1]!=rmap[l2])
      {
	      inv(pixels[l1]);
	      inv(pixels[l2]);
      }
      l1++;
    }
  }


}

