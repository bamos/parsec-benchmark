//-----------------------------------------------------------------------------
//    ____ _               
//   | ___| |   ___  _  _  
//   | ___| |  / __)\ \/ / 
//   | |  | |_| ___) |  |
//   |_|  \___|\___)/_/\_\ Image Library  
//		
//	  2006, Intel Corporation, licensed under Apache 2.0 
//
// file :		FlexDefs.h 
// author :		Scott Ettinger - scott.m.ettinger@intel.com
// description: Flex Image threshold functions
//				
// modified : 
//-----------------------------------------------------------------------------

#ifndef FLEXTHRESHOLD_H
#define FLEXTHRESHOLD_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif


//Threshold and set each pixel to either ltVal or gtVal.  Templated for all images
template<class T, int C>
void FlexThreshold(FlexImage<T,C> &im, T threshold, T ltVal, T gtVal)
{	for(int y = 0; y < im.Height(); y++)
	{	T *p = &im(0,y);	
		for(int x = 0;  x < im.Width(); x++)
			for(int c = 0; c < C; c++)
			{	if(*p < threshold)
					*p = ltVal;
				else
					*p = gtVal;
				p++;
			}
	}
}

#endif

