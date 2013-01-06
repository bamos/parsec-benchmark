//-----------------------------------------------------------------------------
//    ____ _               
//   | ___| |   ___  _  _  
//   | ___| |  / __)\ \/ / 
//   | |  | |_| ___) |  |
//   |_|  \___|\___)/_/\_\ Image Library  Intel Corporation
//		
//	  2006, Intel Corporation, licensed under Apache 2.0 
//
// file :		FlexImageStore.h 
// author :		Scott Ettinger - scott.m.ettinger@intel.com
// description: FlexImageStore definition.  Templated image storage object.  
//				All storage for images is allocated in a FlexImageStore object. 
//				FlexImageStore objects use a reference counter to track how
//				many FlexImage objects are associated with it.  When the 
//				count is zero, it is destroyed by the last FlexImage object to use it.
//
//				The smart pointers must be protected by a semaphore lock 
//				(currently commented out) to be accessed by multiple threads.
//
//				
// modified : 
//-----------------------------------------------------------------------------

#ifndef FLEXIMAGESTORE_H
#define FLEXIMAGESTORE_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include "FlexDefs.h"

template <class T, int C>
class FlexImageStore { 

protected:
	
	int mCount;
	//CSemaphore mLock;
	T *mData;				//Pixel Data
	FISize mSize;			//Image dimensions
	int mStepBytes;			//Stride in bytes per line (step y)

	void Allocate();

public:

	///Destructor
	~FlexImageStore();
	///Default Constructor
	FlexImageStore();
	///Constructor specifying dimensions
	FlexImageStore(int width, int height);

	///IPP memory allocation 
	inline T *FlexMalloc(int widthPixels, int heightPixels, int &pStepBytes);

	///Reallocate to new size - returns true if successful
	bool SetSize(int width, int height);

	//Decrement Reference counter - returns true if zero references remain
	bool DecReferences();
	///Increment Reference counter
	void IncReferences();

	///Get function for data pointer
	inline T *Data() { return(mData); };
	///Get function for image size
	inline FISize Size() { return(mSize); };
	///Get function for step size (bytes per line)
	inline int StepBytes() { return(mStepBytes); };
	
	///Returns false if data is successfully allocated, true if error occurs
	bool Err() { return(mData == NULL);} ;

	//Sets data pointer directly - Dangerous!! Debug purposes only...
	void SetData(T *p) {mData = p;};

};

//Implementation

template <class T, int C>
FlexImageStore<T,C>::FlexImageStore()
{
	mCount = 1;
	mSize.height = 0;
	mSize.width = 0;
	mData = NULL;
}

template <class T, int C>
FlexImageStore<T,C>::FlexImageStore(int width, int height)
{	//construct to given size
	mCount = 1;
	mData = NULL;
	SetSize(width, height);
}

template <class T, int C>
bool FlexImageStore<T,C>::SetSize(int width, int height)
{	//reallocate to new size
	mSize.width = width;
	mSize.height = height;
	mData = NULL;
	Allocate();
	return( Err() );
}

template <class T, int C>
FlexImageStore<T,C>::~FlexImageStore()
{
	if(mData != NULL)
		free(mData);
}

template <class T, int C>
void FlexImageStore<T,C>::Allocate()
{	//free any existing data
	if(mData != NULL)
		free(mData);
	//Allocate space for pixel data	
	mData = FlexMalloc(mSize.width, mSize.height, mStepBytes);
}

//update reference counts
template <class T, int C>
bool FlexImageStore<T,C>::DecReferences()
{
	//CSingleLock l(mLock, true);
	return( (--mCount) == 0);
}
template <class T, int C>
void FlexImageStore<T,C>::IncReferences()
{
	//CSingleLock l(mLock, true);
	mCount++;
}

template<class T, int C>
T *FlexImageStore<T,C>::FlexMalloc(int widthPixels, int heightPixels, int &StepBytes)
{
	StepBytes = widthPixels * C * sizeof(T);			//calculate bytes per image line
	StepBytes = int((StepBytes - 1) / 4) * 4 + 4;		//enforce multiples of 4
	return (T *)malloc(size_t(StepBytes * heightPixels));
}

#endif

