//-----------------------------------------------------------------------------
//    ____ _               
//   | ___| |   ___  _  _  
//   | ___| |  / __)\ \/ / 
//   | |  | |_| ___) |  |
//   |_|  \___|\___)/_/\_\ Image Library  
//		
//	  2006, Intel Corporation, licensed under Apache 2.0 
//
// file :		FlexImage.h 
// author :		Scott Ettinger - scott.m.ettinger@intel.com
// description: FlexImage definition.  Templated image object.  Typed 
//				functions are called through specializations.	
// modified : 
//-----------------------------------------------------------------------------

#ifndef FLEXIMAGE_H
#define FLEXIMAGE_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include <string>
#include "FlexDefs.h"
#include "FlexDataExchange.h"
#include "FlexImageStore.h"


template <class T, int C>
class FlexImage { 

protected:

	FlexImageStore<T,C> *mStore;	//Pointer to FlexImageStore object owning pixel data
	T *mData;						//Pointer to image data
	int mBpp;						//Size in bytes of pixel   (step x)
	int mStepBytes;					//Stride in bytes per line (step y)
	FISize mSize;					//Image dimensions
	FlexImageStatus mStatus;		//Error status for image object

protected:

	///Allocate image store object according to width, height, and channel data members
	void Allocate();
	///Decrement reference and destroy image store if necessary
	void DecReference();

public:

	//-------------------------- Constructors --------------------------------
	///Default Constructor
	FlexImage();
	///Constructor specifying width, height - allocates a new FlexImageStore
	FlexImage(int width, int height);
	///Constructor specifying FISize - allocates a new FlexImageStore
	FlexImage(FISize size);
	///Constructor specifying width, height, and initial value - allocates a new FlexImageStore
	FlexImage(int width, int height, T InitVal);
	///Constructor specifying FISize and initial value - allocates a new FlexImageStore
	FlexImage(FISize size, T InitVal);
	///Copy Constructor
	FlexImage(const FlexImage<T,C> &src);
	///Creates a sub-image from an FlexImage referencing the same memory
	FlexImage(const FlexImage<T,C> &src, int x, int y, int width, int height);
	///Sub image constructor specifying an IppRect
	FlexImage(const FlexImage<T,C> &src, const FIRect &r);

	///Destructor - frees image store if object has last reference to it
	~FlexImage();

	//-------------------------- Member Functions -----------------------------

	///Reallocate to a given size - creates new image store
	bool Reallocate(int width, int height);
	bool Reallocate(FISize size);

	///Get function for pixel data pointer
	inline T *Data()					{ return(mData); };
	inline const T *Data()		const	{ return(mData); };

	//various get functions
	inline int StepBytes()		const	{ return(mStepBytes); };
	inline int BytesPerPixel()	const	{ return(mBpp); };
	inline FISize Size()		const	{ return(mSize); };
	inline int Width()			const	{ return(mSize.width); };
	inline int Height()			const	{ return(mSize.height); };
	inline bool Empty()			const	{ return(mSize.width == 0 && mSize.height == 0); };

	///Convenience function for image rect ( 0,0,w,h ) 
	inline FIRect ImgRect() const;

	///Equals operator - sets image to refer to same memory as source image
	void operator=(const FlexImage<T,C> &src);

	///Create Sub Image of a given image - points to same memory 
	void CreateSubImage(const FlexImage<T,C> &img, int x, int y, int width, int height);
	void CreateSubImage(const FlexImage<T,C> &img, const FIRect &r);

	///Convert image to another data type
	template<class T2>
	FIStatus ConvertFrom(const FlexImage<T2,C> &src, bool allocate = true);
	template<class T2>
	FIStatus ConvertTo(FlexImage<T2,C> &dst, bool allocate = true) const;

	///Deep copy of a given image 
	FIStatus CopyTo(FlexImage<T,C> &dst, bool allocate = true) const;
	FIStatus CopyFrom(const FlexImage<T,C> &src, bool allocate = true);
	
	///Pixel access through () operator for given plane
	inline T &operator ()(int x, int y, int c) {return( *(T *)((char *)mData + mStepBytes * y + mBpp * x + c*sizeof(T) )); };
	inline T &operator ()(int x, int y, int c) const {return( *(T *)((char *)mData + mStepBytes * y + mBpp * x + c*sizeof(T) )); };
	///Pixel access through () operator into plane 0
	inline T &operator ()(int x, int y) { return (operator()(x, y, 0)); };
	inline T &operator ()(int x, int y) const { return (operator()(x, y, 0)); };

	///() operator returns a sub image with 4 parameters
	FlexImage<T,C> operator ()(int x, int y, int width, int height);

	///Fill ROI of image specifying a value for each channel
	FIStatus Set(T value[C+1]);
	///Fill ROI of image specifying a single value for all channels
	FIStatus Set(T value)
	{	T colors[C+1];
		for(int i=0; i < C; i++)
			colors[i] = value;
		return(Set(colors));
	};

	//set a pixel to a color given as an array sized to the number of channels
	void SetPixel(int x, int y, T *color);

	///reallocate to given dimensions if not already equal.  If equal, do nothing. Returns true if resized.
	bool ReallocateNE(int w, int h); 

	///reallocate to given dimensions if not already of equal or greater size in both dimensions
	bool ReallocateGE(int w, int h);

	///determine if another image is of equal size
	bool EqualSize(const FlexImage<T,C> &img);

	///Return Error/Warning status
	FlexImageStatus Status() { return mStatus; };

	///Print contents of image to standard out
	void Print();



	// --------------------------------------------------------------------------------------
	//Advanced functions - most users will never need these

	///Get function for Image store object
	inline FlexImageStore<T,C> *ImageStore() { return(mStore); };
	inline const FlexImageStore<T,C> *ImageStore() const { return(mStore); };

	///Create Sub Image from a given image store
	void CreateSubImage(FlexImageStore<T,C> &store, int x, int y, int width, int height);
	void CreateSubImage(FlexImageStore<T,C> &store, FIRect &r);

	///Creates a sub-image from an FlexImagestore 
	FlexImage(FlexImageStore<T,C> &store, int x, int y, int width, int height);

};



//------------------------Implementation required to be in header -------------------

template<class T, int C>
template<class T2>
FIStatus FlexImage<T,C>::ConvertFrom(const FlexImage<T2,C> &src, bool allocate)
{	return( FlexConvert<T2,T,C>(src, *this, allocate) );
}

template<class T, int C>
template<class T2>
FIStatus FlexImage<T,C>::ConvertTo(FlexImage<T2,C> &dst, bool allocate) const
{	return( FlexConvert<T,T2,C>(*this, dst, allocate) );
}

#endif

