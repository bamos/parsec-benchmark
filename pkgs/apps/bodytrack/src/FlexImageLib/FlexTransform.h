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
// description: Flex Image geometric transformation functions
//				
// modified : 
//-----------------------------------------------------------------------------

#ifndef FLEXTRANSFORM_H
#define FLEXTRANSFORM_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include "FlexFilter.h"


//downsamples image by factor of two with simple anti-aliasing
template<class T>
void FlexDownSample2(FlexImage<T,1> &src, FlexImage<T,1> &dst, bool reallocate = true);


//------------------ Implementation -------------------------------------------

//Separable 3x3 gaussian filter
template<class T>
inline void GaussianBlur3x3(FlexImage<T,1> &src, FlexImage<T,1> &dst)
{
	float k[] = {0.25f, 0.5f, 0.25f};
	FlexImage<T,1> tmp;
	FlexFilterRowV(src, tmp, k, 3);
	FlexFilterColumnV(tmp, dst, k, 3);
}

//downsamples image by factor of two with simple anti-aliasing
template<class T>
void FlexDownSample2(FlexImage<T,1> &src, FlexImage<T,1> &dst, bool reallocate)
{
	if(reallocate)
		dst.Reallocate(src.Width() / 2, src.Height() / 2);
	FlexImage<T,1> tmp;
	GaussianBlur3x3(src, tmp);
	for(int y = 0; y < src.Height() / 2; y++)
	{	T *ps = &src(0,y * 2), *pd = &dst(0,y);
		for(int x = 0; x < src.Width() / 2; x++)
		{	*(pd++) = *ps;
			ps += 2;
		}
	}
}

#endif

