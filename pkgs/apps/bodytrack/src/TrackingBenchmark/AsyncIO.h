//-------------------------------------------------------------
//      ____                        _      _
//     / ___|____ _   _ ____   ____| |__  | |
//    | |   / ___| | | |  _  \/ ___|  _  \| |
//    | |___| |  | |_| | | | | |___| | | ||_|
//     \____|_|  \_____|_| |_|\____|_| |_|(_) Media benchmarks
//                  
//	  2006, Intel Corporation, licensed under Apache 2.0 
//
//  file : AsyncIO.h
//  author : Scott Ettinger - scott.m.ettinger@intel.com
//  description : Asynchronous image loading object. Loads all 
//				  images and converts foreground maps to binary.
//				  
//  modified : 
//--------------------------------------------------------------

#ifndef ASYNCIO_H
#define ASYNCIO_H

#include <deque>
#include "threads/Thread.h"
#include "threads/Mutex.h"
#include "threads/Condition.h"
#include "threads/Barrier.h"
#include "FlexImage.h"
#include "BinaryImage.h"

typedef std::vector<FlexImage<Im8u,1> > ImageSet;
typedef std::vector<BinaryImage> BinaryImageSet;

//Asynchronous Image loading object
class AsyncImageLoader : public threads::Runnable {

protected:
	std::deque<ImageSet > mImageBuffer;			//image buffer
	std::deque<BinaryImageSet > mFGBuffer;		//foreground image buffer
	unsigned int mNumCameras;					//number of cameras (images) per frame
	unsigned int mBufferSize;					//max number of images in the buffer
	std::string mPath;							//dataset path

	threads::Mutex mDataLock;					//synchronization objects
	threads::Mutex mLock1, mLock2;			
	threads::Condition mCondFull;
	threads::Condition mCondEmpty;

	bool mFailed;								//image load failed flag
	unsigned int mCurrentFrame;					//current frame to be loaded
	unsigned int mNumFrames;					//total number of frames

	//load a given set of image and foreground files
	void LoadSet(std::vector<std::string> &FGFiles, BinaryImageSet &FGimages, std::vector<std::string> &ImageFiles, ImageSet &images);

public:

	AsyncImageLoader() : mCondFull(mLock1), mCondEmpty(mLock2), mCurrentFrame(0), mNumCameras(0), mBufferSize(16), mFailed(false) 
	{	mImageBuffer.resize(0); 
	};

	~AsyncImageLoader() {};

	//thread code
	void Run();

	//sets / gets
	void SetNumCameras(unsigned int n) { mNumCameras = n; };
	void SetBufferSize(unsigned int n) { mBufferSize = n; };
	void SetNumFrames(unsigned int n)  { mNumFrames = n; };
	void SetPath(std::string &path)    { mPath = path; };
	
	//get next set of images from 
	bool GetNextImageSet(ImageSet &images, BinaryImageSet &FGMaps);

};


#endif

