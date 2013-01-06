//-----------------------------------------------------------------------------
//    ____ _               
//   | ___| |   ___  _  _  
//   | ___| |  / __)\ \/ / 
//   | |  | |_| ___) |  |
//   |_|  \___|\___)/_/\_\ Image Library  
//
//	  2007, Intel Corporation, licensed under Apache 2.0 
//
// file :		FlexColor.h 
// author :		Scott Ettinger - scott.m.ettinger@intel.com
// description: Color conversion functions
//				
// modified : 
//-----------------------------------------------------------------------------

#ifndef FLEXCOLOR_H
#define FLEXCOLOR_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include "FlexImage.h"
#include <algorithm>

//////////////////////////////////////////////////////////////////////////////////////////////////
//									RGBToGray Functions
//
//  Names:	ippiRGBToGray, ippiRGBToGrayA
//
///////////////////////////////////////////////////////////////////////////////////////////////////

template<class T>
FIStatus FlexRGBToGray(FlexImage<T,3> &src, FlexImage<T,1> &dst, bool allocate = true);

template<class T>
FIStatus FlexGrayToRGB(FlexImage<T,1> &src, FlexImage<T,3> &dst, bool allocate = true);



//----------------------------------- Implementation -----------------------------------------------

template<class T>
FIStatus FlexRGBToGray(FlexImage<T,3> &src, FlexImage<T,1> &dst, bool allocate)
{	
	if(allocate)
		dst.Reallocate(src.Size());
	int w = std::min(src.Width(), dst.Width());					//determine number of pixels to copy (intersection of src and dst areas)
	int h = std::min(src.Height(), dst.Height());
	T *pSrc = (T *)src.Data(), *pDst = (T *)dst.Data();
	for(int j = 0; j < h; j++)
	{	T *ps = pSrc, *pd = pDst;
		for(int i = 0; i < w; i++)
		{	*(pd++) = (T)(ps[0] * 0.299f + ps[1] * .587f + ps[2] * .114f);
			ps += 3;
		}
		pSrc = (T *)((char *)pSrc + src.StepBytes());
		pDst = (T *)((char *)pDst + dst.StepBytes());
	}
	return 0;
}

template<class T>
FIStatus FlexGrayToRGB(FlexImage<T,1> &src, FlexImage<T,3> &dst, bool allocate)
{
	if(allocate)
		dst.Reallocate(src.Size());
	FlexCopyC1CM(src, dst, 0);						//duplicate to all 3 image planes
	FlexCopyC1CM(src, dst, 1);
	FlexCopyC1CM(src, dst, 2);
	return 0;
}

// ------------------------------- Functions not included for benchmarks ---------------------------

/*template<class T>
IppStatus ippiRGBToGrayA(FlexImage<T,4> &src, FlexImage<T,1> &dst);
*/

/*
//////////////////////////////////////////////////////////////////////////////////////////////////
//									RGBToYCbCr Functions
//
//  Names:	ippiRGBToYCbCr, ippiRGBToYCbCrA
//
//  Template Specializations:
//			ippiRGBToYCbCr		:	8u_C3
//			ippiRGBToYCbCrA		:	8u_C4
//			
///////////////////////////////////////////////////////////////////////////////////////////////////

IppStatus ippiRGBToYCbCr(FlexImage<Ipp8u,3> &src, FlexImage<Ipp8u,3> &dst);
IppStatus ippiRGBToYCbCrA(FlexImage<Ipp8u,4> &src, FlexImage<Ipp8u,4> &dst);

//////////////////////////////////////////////////////////////////////////////////////////////////
//									YCbCrToRGB Functions
//
//  Names:	ippiYCbCrToRGB, ippiYCbCrToRGBA
//
//  Template Specializations:
//			ippiYCbCrToRGB		:	8u_C3
//			ippiYCbCrToRGBA		:	8u_C4
//			
///////////////////////////////////////////////////////////////////////////////////////////////////

IppStatus ippiYCbCrToRGB(FlexImage<Ipp8u,3> &src, FlexImage<Ipp8u,3> &dst);
IppStatus ippiYCbCrToRGBA(FlexImage<Ipp8u,4> &src, FlexImage<Ipp8u,4> &dst);

template<class T>
IppStatus ippiColorTwist(FlexImage<T, 3> &src, FlexImage<T,3> &dst, const Ipp32f twist[3][4]);
template<class T>
IppStatus ippiColorTwistI(FlexImage<T, 3> &src, const Ipp32f twist[3][4]);
template<class T>
IppStatus ippiColorTwistA(FlexImage<T, 4> &src, FlexImage<T,4> &dst, const Ipp32f twist[3][4]);
template<class T>
IppStatus ippiColorTwistAI(FlexImage<T, 4> &src, const Ipp32f twist[3][4]);

*/


#endif

