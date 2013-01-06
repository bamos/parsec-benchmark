//-----------------------------------------------------------------------------
//    ____ _               
//   | ___| |   ___  _  _  
//   | ___| |  / __)\ \/ / 
//   | |  | |_| ___) |  |
//   |_|  \___|\___)/_/\_\ Image Library 
//
//	  2006, Intel Corporation, licensed under Apache 2.0 
//
// file :		FlexDraw.h 
// author :		Scott Ettinger - scott.m.ettinger@intel.com
// description: Primitive drawing functions.
//
// date:		
// modified : 
//-----------------------------------------------------------------------------

#ifndef FLEXDRAW_H
#define FLEXDRAW_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

// Bresenham Line Algorithm implementation
template<class T, int C> 
void FlexLine(FlexImage<T,C> &Image, int x1, int y1, int x2, int y2, T *color) {
	int		x, y, dx, dy;
	int		incx, incy, balance;

	if(x2 > Image.Width() || y2 > Image.Height())
		return;
	if (x2 >= x1)
	{	dx = x2 - x1;
		incx = 1;
	}
	else
	{	dx = x1 - x2;
		incx = -1;
	}
	if (y2 >= y1)
	{	dy = y2 - y1;
		incy = 1;
	}
	else
	{	dy = y1 - y2;
		incy = -1;
	}
	x = x1;
	y = y1;
	if (dx >= dy)
	{	dy <<= 1;
		balance = dy - dx;
		dx <<= 1;
		while (x != x2)
		{	Image.SetPixel(x, y, color);
			if (balance >= 0)
			{	y += incy;
				balance -= dx;
			}
			balance += dy;
			x += incx;
		} Image.SetPixel(x, y, color);
	}
	else
	{	dx <<= 1;
		balance = dx - dy;
		dy <<= 1;
		while (y != y2)
		{	Image.SetPixel(x, y, color);
			if (balance >= 0)
			{	x += incx;
				balance -= dy;
			}
			balance += dx;
			y += incy;
		} Image.SetPixel(x, y, color);
	}
}

#endif

