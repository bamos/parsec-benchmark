//-----------------------------------------------------------------------------
//    ____ _               
//   | ___| |   ___  _  _  
//   | ___| |  / __)\ \/ / 
//   | |  | |_| ___) |  |
//   |_|  \___|\___)/_/\_\ Image Library  
//					
//	  2006, Intel Corporation, licensed under Apache 2.0 
//
// file :		FlexFilter.h
// author :		Scott Ettinger - scott.m.ettinger@intel.com
// description: FlexImage filter functions
//				
// modified : 
//-----------------------------------------------------------------------------

#ifndef FLEXFILTER_H
#define FLEXFILTER_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include "FlexImage.h"

//1 channel filters are implemented separately from multi-channel for speed

//1D filter Row wise 1 channel any type data or kernel valid pixels only
template<class T, class T2>
bool FlexFilterRowV(FlexImage<T,1> &src, FlexImage<T,1> &dst, T2 *kernel, int kernelSize, bool allocate = true);

//1D filter Column wise 1 channel any type data or kernel valid pixels only
template<class T, class T2>
bool FlexFilterColumnV(FlexImage<T,1> &src, FlexImage<T,1> &dst, T2 *kernel, int kernelSize, bool allocate = true);

//2D Filter any type data or kernel valid pixels only
template<class T, class T2>
bool FlexFilter2DV(FlexImage<T,1> &src, FlexImage<T,1> &dst, T2 *kernel, int kWidth, int kHeight, bool allocate = true);

//Zero a 1 pixel border of the image
template<class T> 
inline void ZeroBorder(FlexImage<T,1> &im);

//----------------------------- Implementation --------------------------------

//1D filter Row wise 1 channel any type data or kernel valid pixels only
template<class T, class T2>
bool FlexFilterRowV(FlexImage<T,1> &src, FlexImage<T,1> &dst, T2 *kernel, int kernelSize, bool allocate)
{
	if(kernelSize % 2 == 0)									//enforce odd length kernels
		return false;
	if(allocate)
		dst.Reallocate(src.Size());
	dst.Set((T)0);
	int n = kernelSize / 2;
	for(int y = 0; y < src.Height(); y++)
	{	T *psrc = &src(n, y), *pdst = &dst(n, y);
		for(int x = n; x < src.Width() - n; x++)
		{	int k = 0;
			T2 acc = 0;
			for(int i = -n; i <= n; i++) 
				acc += (T2)(psrc[i] * kernel[k++]);
			*pdst = (T)acc;
			pdst++;
			psrc++;
		}
	}
	return true;
}

//1D filter Column wise 1 channel any type data or kernel valid pixels only
template<class T, class T2>
bool FlexFilterColumnV(FlexImage<T,1> &src, FlexImage<T,1> &dst, T2 *kernel, int kernelSize, bool allocate)
{
	if(kernelSize % 2 == 0)									//enforce odd length kernels
		return false;
	if(allocate)
		dst.Reallocate(src.Size());
	dst.Set((T)0);
	int n = kernelSize / 2;
	int sb = src.StepBytes();
	for(int y = n; y < src.Height() - n; y++)
	{	T *psrc = &src(0, y), *pdst = &dst(0, y);
		for(int x = 0; x < src.Width(); x++)
		{	int k = 0;
			T2 acc = 0;
			for(int i = -n; i <= n; i++) 
				acc += (T2)(*(T *)((char *)psrc + sb * i) * kernel[k++]);
			*pdst = (T)acc;
			pdst++;
			psrc++;
		}
	}
	return true;
}

//2D Filter any type data or kernel valid pixels only
template<class T, class T2>
bool FlexFilter2DV(FlexImage<T,1> &src, FlexImage<T,1> &dst, T2 *kernel, int kWidth, int kHeight, bool allocate)
{
	if(kWidth % 2 == 0 || kHeight % 2 == 0)
		return false;
	if(allocate)
		dst.Reallocate(src.Size());
	dst.Set((T)0);
	int nw = kWidth / 2, nh = kHeight / 2;
	int sb = src.StepBytes();
	for(int y = nh; y < src.Height() - nh; y++)
	{	T *psrc = &src(nw, y - nh), *pdst = &dst(nw, y);
		for(int x = nw; x < src.Width() - nw; x++)
		{	int k = 0;
			T *p = psrc++;
			for(int j = -nh; j <= nh; j++)
			{	for(int i = -nw; i <= nw; i++) 
					*pdst += p[i] * kernel[k++];
				p = (T *)((char *)p + sb);
			}	
			pdst++;
		}
	}
	return true;
}

//Zero a 1 pixel border of the image
template<class T> 
inline void ZeroBorder(FlexImage<T,1> &im)
{
	T *p1 = &im(0,0), *p2 = &im(0,im.Height() - 1);
	for(int i = 0; i < im.Width(); i++)
	{	*(p1++) = 0;
		*(p2++) = 0;
	}
	p1 = &im(0,0);	p2 = &im(im.Width() - 1, 0);
	for(int i = 0; i < im.Height(); i++)
	{	*p1 = 0; *p2 = 0;
		p1 = (T *)((Im8u *)p1 + im.StepBytes());
		p2 = (T *)((Im8u *)p2 + im.StepBytes());
	}
}

#endif

