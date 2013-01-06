//-----------------------------------------------------------------------------
//    ____ _               
//   | ___| |   ___  _  _  
//   | ___| |  / __)\ \/ / 
//   | |  | |_| ___) |  |
//   |_|  \___|\___)/_/\_\ Image Library 
//		
//	  2006, Intel Corporation, licensed under Apache 2.0 
//
// file :		FlexOperators.h 
// author :		Scott Ettinger - scott.m.ettinger@intel.com
// description: FlexImage definition.  Templated image object.  Typed 
//				functions are called through specializations.
//
// modified : 
//-----------------------------------------------------------------------------

#ifndef FLEXOPS_H
#define FLEXOPS_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include "FlexImage.h"

//NOTE : only use images of equal size or unexpected results may occur!  
//Additional error checking may be added to this code to provide warnings or 
//	abort in the case of unequal image size. 

//NOTE : all constants must be type cast to the proper type when using operators.
//		 this can be fixed , but may risk portability between
//		 compilers.

/////////////////  binary image operators //////////////////////


//Addition of two images
template<class T, int C>
FlexImage<T,C> operator +(const FlexImage<T,C> &srcR, const FlexImage<T,C> &srcL)
{	FlexImage<T,C> tmp(srcR.Width(), srcR.Height());
	int w = srcR.Width() * C;								//this constant is calculated first to allow 
	for(int y = 0; y < srcR.Height(); y++)					//compilers to vectorize
	{	T *p = &tmp(0,y), *pr = &srcR(0,y), *pl = &srcL(0,y);
		for(int x = 0; x < w; x++)
			*(p++) = *(pr++) + *(pl++);
	}
	return(tmp);
}

//Subtraction of two images
template<class T, int C>
FlexImage<T,C> operator -(const FlexImage<T,C> &srcR, const FlexImage<T,C> &srcL)
{	FlexImage<T,C> tmp(srcR.Width(), srcR.Height());
	int w = srcR.Width() * C;
	for(int y = 0; y < srcR.Height(); y++)
	{	T *p = &tmp(0,y), *pr = &srcR(0,y), *pl = &srcL(0,y);
		for(int x = 0; x < w; x++)
			*(p++) = *(pr++) - *(pl++);
	}
	return(tmp);
}

//Multiplication of two images
template<class T, int C>
FlexImage<T,C> operator *(const FlexImage<T,C> &srcR, const FlexImage<T,C> &srcL)
{	FlexImage<T,C> tmp(srcR.Width(), srcR.Height());
	int w = srcR.Width() * C;	
	for(int y = 0; y < srcR.Height(); y++)
	{	T *p = &tmp(0,y), *pr = &srcR(0,y), *pl = &srcL(0,y);
		for(int x = 0; x < w; x++)
			*(p++) = *(pr++) * *(pl++);
	}
	return(tmp);
}

//Division of two images
template<class T, int C>
FlexImage<T,C> operator /(const FlexImage<T,C> &srcR, const FlexImage<T,C> &srcL)
{	FlexImage<T,C> tmp(srcR.Width(), srcR.Height());
	int w = srcR.Width() * C;
	for(int y = 0; y < srcR.Height(); y++)
	{	T *p = &tmp(0,y), *pr = &srcR(0,y), *pl = &srcL(0,y);
		for(int x = 0; x < w; x++)
			*(p++) = *(pr++) / *(pl++);
	}
	return(tmp);
}

/////////////////  image assignment operators //////////////////////

//In place Addition of two images
template<class T, int C>
void operator +=(FlexImage<T,C> &dst, const FlexImage<T,C> &src)
{	int w = src.Width() * C;
	for(int y = 0; y < src.Height(); y++)
	{	T *p = &dst(0,y,0), *ps = &src(0,y,0);
		for(int x = 0; x < w; x++)
			p[x] = p[x] + ps[x];
	}
}

//In place Subtraction of two images
template<class T, int C>
void operator -=(FlexImage<T,C> &dst, const FlexImage<T,C> &src)
{	int w = src.Width() * C;
	for(int y = 0; y < src.Height(); y++)
	{	T *p = &dst(0,y), *ps = &src(0,y);
		for(int x = 0; x < w; x++)
			p[x] = p[x] - ps[x];
	}
}

//In place Multiplication of two images
template<class T, int C>
void operator *=(FlexImage<T,C> &dst, const FlexImage<T,C> &src)
{	int w = src.Width() * C;
	for(int y = 0; y < src.Height(); y++)
	{	T *p = &dst(0,y), *ps = &src(0,y);
		for(int x = 0; x < w; x++)
			p[x] = p[x] * ps[x];
	}
}

//In place Division of two images
template<class T, int C>
void operator /=(FlexImage<T,C> &dst, const FlexImage<T,C> &src)
{	int w = src.Width() * C;
	for(int y = 0; y < src.Height(); y++)
	{	T *p = &dst(0,y), *ps = &src(0,y);
		for(int x = 0; x < w; x++)
			p[x] = p[x] / ps[x];
	}
}

////////////////// Binary image and constant operators ////////////////

//Addition of a constant
template<class T, int C>
FlexImage<T,C> operator +(const FlexImage<T,C> &src, T c)
{	FlexImage<T,C> tmp(src.Width(), src.Height());
	int w = src.Width() * C;
	for(int y = 0; y < src.Height(); y++)
	{	T *p = &tmp(0,y), *ps = &src(0,y);
		for(int x = 0; x < w; x++)
			*(p++) = *(ps++) + c;
	}
	return(tmp);
}

//Subtraction of a constant
template<class T, int C>
FlexImage<T,C> operator -(const FlexImage<T,C> &src, T c)
{	FlexImage<T,C> tmp(src.Width(), src.Height());
	int w = src.Width() * C;
	for(int y = 0; y < src.Height(); y++)
	{	T *p = &tmp(0,y), *ps = &src(0,y);
		for(int x = 0; x < w; x++)
			*(p++) = *(ps++) - c;
	}
	return(tmp);
}

//Multiplication by a constant
template<class T, int C>
FlexImage<T,C> operator *(const FlexImage<T,C> &src, T c)
{	int w = src.Width() * C;
	FlexImage<T,C> tmp(src.Width(), src.Height());
	for(int y = 0; y < src.Height(); y++)
	{	T *p = &tmp(0,y), *ps = &src(0,y);
		for(int x = 0; x < w; x++)
			*(p++) = *(ps++) * c;
	}
	return(tmp);
}

//Division by a constant
template<class T, int C>
FlexImage<T,C> operator /(const FlexImage<T,C> &src, T c)
{	int w = src.Width() * C;
	FlexImage<T,C> tmp(src.Width(), src.Height());
	for(int y = 0; y < src.Height(); y++)
	{	T *p = &tmp(0,y), *ps = &src(0,y);
		for(int x = 0; x < w; x++)
			*(p++) = *(ps++) / c;
	}
	return(tmp);
}

//Addition of an image to a constant
template<class T, int C>
FlexImage<T,C> operator +(T c, const FlexImage<T,C> &src)
{	return src + c;
}

//Subtraction of an image from a constant
template<class T, int C>
FlexImage<T,C> operator -(T c, const FlexImage<T,C> &src)
{	FlexImage<T,C> tmp(src.Width(), src.Height());
	int w = src.Width() * C;
	for(int y = 0; y < src.Height(); y++)
	{	T *p = &tmp(0,y), *ps = &src(0,y);
		for(int x = 0; x < w; x++)
			*(p++) = c - *(ps++);
	}
	return(tmp);
}

//Multiplication of a constant by an image
template<class T, int C>
FlexImage<T,C> operator *(T c, const FlexImage<T,C> &src)
{	return src * c;
}

//Division of a constant by an image
template<class T, int C>
FlexImage<T,C> operator /(T c, const FlexImage<T,C> &src)
{	FlexImage<T,C> tmp(src.Width(), src.Height());
	int w = src.Width() * C;
	for(int y = 0; y < src.Height(); y++)
	{	T *p = &tmp(0,y), *ps = &src(0,y);
		for(int x = 0; x < w; x++)
			*(p++) = c / *(ps++);
	}
	return(tmp);
}

/////////////// Multi-channel  constant assignment operators ////////////////

//In Place Addition of a constant
template<class T, int C> 
void operator +=(FlexImage<T,C> &dst, T c)
{	int w = dst.Width() * C;
	for(int y = 0; y < dst.Height(); y++)
	{	T *p = &dst(0,y);
		for(int x = 0; x < w; x++)
			p[x] = p[x] + c;
	}
}

//In Place Subtraction of a constant
template<class T, int C> 
void operator -=(FlexImage<T,C> &dst, T c)
{	int w = dst.Width() * C;
	for(int y = 0; y < dst.Height(); y++)
	{	T *p = &dst(0,y);
		for(int x = 0; x < w; x++)
			p[x] = p[x] - c;
	}
}

//In Place Multiplication by a constant
template<class T, int C> 
void operator *=(FlexImage<T,C> &dst, T c)
{	int w = dst.Width() * C;
	for(int y = 0; y < dst.Height(); y++)
	{	T *p = &dst(0,y);
		for(int x = 0; x < w; x++)
			p[x] = p[x] * c;
	}
}

//In Place Division by a constant
template<class T, int C> 
void operator /=(FlexImage<T,C> &dst, T c)
{	int w = dst.Width() * C;
	for(int y = 0; y < dst.Height(); y++)
	{	T *p = &dst(0,y);
		for(int x = 0; x < w; x++)
			p[x] = p[x] / c;
	}
}

#endif

