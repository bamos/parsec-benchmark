//-----------------------------------------------------------------------------G
//    ____ _               
//   | ___| |   ___  _  _  
//   | ___| |  / __)\ \/ / 
//   | |  | |_| ___) |  |
//   |_|  \___|\___)/_/\_\ Image Library 
//		
//	  2006, Intel Corporation, licensed under Apache 2.0 
//
// file :		FlexDataExchange.h 
// author :		Scott Ettinger - scott.m.ettinger@intel.com
// description: Definition of Conversion functions between image types.
//				Only a subset of functions are implemented for benchmarks.
//				
// modified : 
//-----------------------------------------------------------------------------

#ifndef FLEXDATAEXCHANGE_H
#define FLEXDATAEXCHANGE_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include "FlexDefs.h"

//min() will be undefined at end of header to avoid conflicts
#ifndef min
#  define min(x, y)       (((x) < (y)) ? (x) : (y))
#endif

template<class T, int C> 
class FlexImage;

//////////////////////////////////////////////////////////////////////////////////////////////////
//									Copy Functions
//
//  Names: FlexCopy, FlexCopyM, FlexCopyCM, FlexCopyCMC1, FlexCopyC1CM, FlexCopyP3
//
///////////////////////////////////////////////////////////////////////////////////////////////////

///copy all pixels of all color channels
template<class T, int C>
FIStatus FlexCopy(const FlexImage<T,C> &src, FlexImage<T,C> &dst, bool allocate = true);

///copy a single channel image to a channel of a multi-channel image
template<class T, int C>
FIStatus FlexCopyC1CM(FlexImage<T,1> &src, FlexImage<T,C> &dst, int dstChannel);

/*
///copy a single channel of a multi-channel image to a channel of another multi-channel image
template<class T, int C>
FIStatus FlexCopyCM(FlexImage<T,C> &src, FlexImage<T,C> &dst, int srcChannel, int dstChannel);

///copy a single channel of a multi-channel image to a single channel image
template<class T, int C>
FIStatus FlexCopyCMC1(FlexImage<T,C> &src, FlexImage<T,1> &dst, int srcChannel);

//copy masked pixels of all color channels
template<class T, int C>
FIStatus FlexCopyM(FlexImage<T,C> &src, FlexImage<T,C> &dst, FlexImage<Ipp8u,1> &msk);

///split 3 channel image into separate plane images
template<class T>
FIStatus FlexCopyP3(FlexImage<T,3> &src, FlexImage<T,1> &p1, FlexImage<T,1> &p2, FlexImage<T,1> &p3);

///split 4 channel image into separate plane images
template<class T>
FIStatus FlexCopyP4(FlexImage<T,4> &src, FlexImage<T,1> &p1, FlexImage<T,1> &p2, FlexImage<T,1> &p3, FlexImage<T,1> &p4);
*/

/*
///copy all pixels of all color channels with alpha
template<class T>
FIStatus FlexCopyA(const FlexImage<T,4> &src, FlexImage<T,4> &dst);
*/

//////////////////////////////////////////////////////////////////////////////////////////////////
//									Convert Functions
//
//  Names: FlexConvert, FlexConvertA
//
///////////////////////////////////////////////////////////////////////////////////////////////////

///Image conversion - Converts between images but uses size of src 
template<class T1, class T2, int C>
FIStatus FlexConvert(const FlexImage<T1,C> &src, FlexImage<T2,C> &dst, bool allocate = true);

//Allows for templated functions that convert from same type to same type
template<class T, int C>
FIStatus FlexConvert(const FlexImage<T,C> &src, FlexImage<T,C> &dst, bool allocate = true)
{	return( FlexCopy(src, dst, allocate) ); };

/*
///Same as FlexConvert, but uses Alpha channel (i.e. calls ippConvertAC4 instead of ippConverC4
///src and dst must contain 4 channels or returns ippStsNumChannelsErr
template<class T1, class T2, int C>
FIStatus FlexConvertA(FlexImage<T1,C> &src, FlexImage<T2,C> &dst, IppRoundMode roundMode = ippRndNear);
*/

//////////////////////////////////////////////////////////////////////////////////////////////////
//									Set Functions
//
//  Names: FlexSet, FlexSetA, FlexSetM, ipipSetAM, FlexSetC
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------Set functions--------------------------

///Image set single channel to a value
template<class T>
FIStatus FlexSet(FlexImage<T,1> &img, T value);

///Image set multi-channels to a value
template<class T, int C>
FIStatus FlexSet(FlexImage<T,C> &img, T value[]);

/*
///Image set skipping alpha channel - 4 channel images only
template<class T, int C>
FIStatus FlexSetA(FlexImage<T,C> &img, T value[]);

///Image set masked values 1 channel
template<class T>
FIStatus FlexSetM(FlexImage<T,1> &img, T value, FlexImage<Ipp8u,1> &msk);

///Image set masked values multi-channel
template<class T, int C>
FIStatus FlexSetM(FlexImage<T,C> &img, T value[], FlexImage<Ipp8u,1> &msk);

///Image set masked values 4 channel alpha
template<class T, int C>
FIStatus FlexSetAM(FlexImage<T,C> &img, T *value, FlexImage<Ipp8u,1> &msk);

///Image set values on individual channel - 3 and 4 channel images only
template<class T, int C>
FIStatus FlexSetC(FlexImage<T,C> &img, T value, int channel);
*/

//////////////////////////////////////////////////////////////////////////////////////////////////
//									SwapChannels Functions
//
//  Names: SwapChannels, SwapChannelsA, SwapChannelsI
//
///////////////////////////////////////////////////////////////////////////////////////////////////

///Swap Channels - order values must be from {0, 1, 2}
template<class T, int C>
FIStatus FlexSwapChannels(FlexImage<T,C> &src, FlexImage<T,C> &dst, int dstOrder[3]);

//:Swap channels immediate
template<class T, int C>
FIStatus FlexSwapChannelsI(FlexImage<T,3> &src, int dstOrder[3]);

/*
template<class T>
FIStatus FlexSwapChannelsA(FlexImage<T,4> &src, FlexImage<T,4> &dst, int dstOrder[3]);
*/




//---------------------------------------- Implementation ----------------------------------------



//////////////////////////////////////////////////////////////////////////////////////////////////
//									Copy Functions
//
//  Names: FlexCopy, FlexCopyM, FlexCopyCM, FlexCopyCMC1, FlexCopyC1CM, FlexCopyP3
//
///////////////////////////////////////////////////////////////////////////////////////////////////

template<class T, int C>
FIStatus FlexCopy(const FlexImage<T,C> &src, FlexImage<T,C> &dst, bool allocate)
{
	if(allocate)											//allocate destination if desired
		dst.Reallocate(src.Size());
	int w = min(src.Width(), dst.Width());				//determine number of pixels to copy (intersection of src and dst areas)
	int h = min(src.Height(), dst.Height());
	int n = w * sizeof(T) * C;								//get number of bytes to copy per line
	T *pSrc = (T *)src.Data(), *pDst = (T *)dst.Data();
	for(int i = 0; i < h; i++)								//copy the data line by line
	{	memcpy(pDst, pSrc, n);
		pSrc = (T *)((char *)pSrc + src.StepBytes());
		pDst = (T *)((char *)pDst + dst.StepBytes());
	}
	return 0;
}

template<class T, int C>
FIStatus FlexCopyC1CM(FlexImage<T,1> &src, FlexImage<T,C> &dst, int dstChannel)
{
	if(C < dstChannel + 1)									//exit if invalid channel selected
		return -1;
	int w = min(src.Width(), dst.Width());				//determine number of pixels to copy (intersection of src and dst areas)
	int h = min(src.Height(), dst.Height());
	T *pSrc = (T *)src.Data(), *pDst = (T *)dst.Data() + dstChannel;
	for(int j = 0; j < h; j++)
	{	T *ps = pSrc, *pd = pDst;
		for(int i = 0; i < w; i++)
		{	*pd = *(ps++);
			pd += C;
		}
		pSrc = (T *)((char *)pSrc + src.StepBytes());
		pDst = (T *)((char *)pDst + dst.StepBytes());
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
//									Convert Functions
//
//  Names: FlexConvert, FlexConvertA
//
///////////////////////////////////////////////////////////////////////////////////////////////////

///Image conversion - Converts between image types
template<class T1, class T2, int C>
FIStatus FlexConvert(const FlexImage<T1,C> &src, FlexImage<T2,C> &dst, bool allocate)
{	
	if(allocate)											//allocate destination if desired
		dst.Reallocate(src.Size());
	int w = min(src.Width(), dst.Width());				//determine number of pixels to copy (intersection of src and dst areas)
	int h = min(src.Height(), dst.Height());
	T1 *pSrc = (T1 *)src.Data();
	T2 *pDst = dst.Data();
	for(int j = 0; j < h; j++)								//copy the data line by line
	{	T1 *ps = pSrc;
		T2 *pd = pDst;
		for(int i = 0; i < w * C; i++)
			*pd++ = T2(*ps++);
		pSrc = (T1 *)((char *)pSrc + src.StepBytes());
		pDst = (T2 *)((char *)pDst + dst.StepBytes());
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//									Set Functions
//
//  Names: FlexSet, FlexSetA, FlexSetM, ipipSetAM, FlexSetC
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------Set functions--------------------------

///Set all channels to given value
template<class T, int C>
FIStatus FlexSet(FlexImage<T,1> &img, T value)
{	int w = img.Width() * C;
	T *pLine = img.Data();					//get pointer to the data
	for(int j = 0; j < img.Height(); j++)
	{	T *p = pLine;
		for(int i = 0; i < w; i++)			//set each pixel to value
			*(p++) = value;
		pLine += img.StepBytes();
	}
	return 0;
}

///set multi-channels to given values
template<class T, int C>
FIStatus FlexSet(FlexImage<T,C> &img, T value[])
{
	if(img.Height() < 1)
		return -1;
	T *p = img.Data();						//get pointer to data
	int w = img.Width() * C;
	for(int i = 0; i < w; i++)				//generate first line of image
		*(p++) = value[i % C];

	T *ps = img.Data();
	char *pd = (char *)&img(0,1);
	int n = w * sizeof(T);
	for(int i = 1; i < img.Height(); i++)	//copy first line to rest of image
	{	memcpy(pd, ps, n);
		pd += img.StepBytes();
	}
	return 0;
}

//undefine min() to avoid conflicts with system headers
#undef min

#endif

