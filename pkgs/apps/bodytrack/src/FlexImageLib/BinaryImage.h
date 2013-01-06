//-----------------------------------------------------------------------------
//    ____ _               
//   | ___| |   ___  _  _  
//   | ___| |  / __)\ \/ / 
//   | |  | |_| ___) |  |
//   |_|  \___|\___)/_/\_\ Image Library  
//		
//	  2006, Intel Corporation, licensed under Apache 2.0 
//
// file :		BinaryImage.h 
// author :		Scott Ettinger - scott.m.ettinger@intel.com
// description: BinaryImage definition.  Simple Binary image storage object
// modified : 
//-----------------------------------------------------------------------------

#ifndef BINARYIMAGE_H
#define BINARYIMAGE_H

#include <vector>
#include <iostream>
#include "FlexImage.h"


//Binary Image class.  Stores each pixel as 1 bit.  SetPixel is NOT thread safe  
//as setting independent pixels can access the same byte.
class BinaryImage{

protected : 
	//std::vector<unsigned char> mData;
	FlexImageStore<Im8u,1> *mStore;
	unsigned char *mData;
	int mWidth, mHeight;

public:

	//constructors
	BinaryImage() : mStore(NULL) {};
	BinaryImage(int w, int h) : mStore(NULL) {Reallocate(w,h); };

	//copy constructor
	BinaryImage(const BinaryImage &src)
	{	mStore = NULL;
		*this = src;
	}

	int Width()	const	{return mWidth; };
	int Height() const	{return mHeight; };

	FlexImageStore<Im8u,1> *ImageStore() const {return mStore; };

	//allocate space for a given image size
	void Reallocate(int w, int h)
	{	mWidth = w; mHeight = h;
		Allocate();
	}

	//set all pixels to zero
	void Clear() { memset(mData, 0, mWidth * mHeight / 8 + 1); };

	//pixel access
	int operator()(int x, int y) const		
	{	int p = mWidth * y + x;
		int i = p / 8;
		return (mData[i] >> (p % 8)) & 1;
	}

	//set a given pixel to 1
	void SetPixel(int x, int y)				
	{	int p = mWidth * y + x;
		int i = p / 8;
		mData[i] = mData[i] | (unsigned char)(1 << (p % 8)); 
	}

	//set a given pixel to 0
	void ClearPixel(int x, int y)				
	{	int p = mWidth * y + x;
		int i = p / 8;
		mData[i] = mData[i] & ((unsigned char)255 ^ (unsigned char)(1 << (p % 8))); 
	}

	//convert from an 8 bit grayscale image to a binary image
	//all non-zero pixels will be a binary 1
	void ConvertToBinary(const FlexImage<Im8u,1> &src)
	{
		Reallocate(src.Width(), src.Height());
		Clear();
		int b = 0, i = 0;
		for(int y = 0; y < src.Height(); y++)
		{	Im8u *psrc = &src(0,y);
			for(int x = 0; x < src.Width(); x++)
			{	unsigned char d = *(psrc++) != 0 ? 1 : 0;
				mData[i] = mData[i] | (d << b);
				if(++b > 7)
				{	b = 0;
					i++;
				}
			}
		}
	}

	//remove my reference to the current image data
	void DecReference()
	{	if(mStore != NULL)														//if no references remain, destroy FlexImageStore object
			if( mStore->DecReferences() )
				delete mStore;
		mStore = NULL;
	}

	//allocate new storage for image data
	void Allocate()
	{	DecReference();															//free old FlexImageStore if I am the only reference
		mStore = new FlexImageStore<Im8u,1>(mWidth * mHeight / 8 + 1, 1);		//Allocate a new FlexImageStore to own pixel data and copy parameters
		mData = mStore->Data();
		if( mStore->Err() )
			std::cout << "Unable to allocate binary image." << std::endl;
	}

	//shallow copy - uses pointer and reference counting
	void operator=(const BinaryImage &img)
	{	mWidth = img.Width();
		mHeight = img.Height();
		if(mStore != img.ImageStore())
		{	DecReference();														//copy pointer to FlexImageStore object and increment its reference count
			mStore = (FlexImageStore<Im8u,1> *)img.ImageStore();
			mStore->IncReferences();
			mData = mStore->Data();
		}
	}

	//show image in text form
	void Print() const
	{	for(int y = 0; y < mHeight; y++)
		{	std::cout << std::endl;
			for(int x = 0; x < mWidth; x++)
				std::cout << operator()(x, y) << " ";
		}
	}

	//destructor - frees image data if no other images reference it
	~BinaryImage()
	{	DecReference();
	}

};

#endif

