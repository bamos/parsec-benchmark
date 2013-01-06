//-------------------------------------------------------------
//      ____                        _      _
//     / ___|____ _   _ ____   ____| |__  | |
//    | |   / ___| | | |  _  \/ ___|  _  \| |
//    | |___| |  | |_| | | | | |___| | | ||_|
//     \____|_|  \_____|_| |_|\____|_| |_|(_) Media benchmarks
//                  
//	  2006, Intel Corporation, licensed under Apache 2.0 
//
//  file : AsyncIO.cpp
//  author : Scott Ettinger - scott.m.ettinger@intel.com
//  description : Asynchronous image loading object. Loads all 
//				  images and converts foreground maps to binary.
//				  implements a synchronized deque for blocking.
//				  
//  modified : 
//--------------------------------------------------------------


#include "system.h"
#include "FlexIO.h"
#include "AsyncIO.h"
#include <iomanip>
#include <sstream>

//templated conversion to string with field width
template<class T>
inline std::string str(T n, int width = 0, char pad = '0')
{	std::stringstream ss;
	ss << std::setw(width) << std::setfill(pad) << n;
	return ss.str();
}

//load a given set of image and foreground files
void AsyncImageLoader::LoadSet(std::vector<std::string> &FGfiles, BinaryImageSet &FGimages, std::vector<std::string> &ImageFiles, ImageSet &images)
{
	FlexImage<Im8u,1> im;
	for(unsigned int i = 0; i < FGfiles.size(); i++)
	{	if(!FlexLoadBMP(FGfiles[i].c_str(), im))									//Load foreground maps and raw images
		{	std::cout << "Unable to load image: " << FGfiles[i].c_str() << std::endl;
			mFailed = true;
		}	
		FGimages[i].ConvertToBinary(im);											//binarize foreground maps to 0 and 1
		if(!FlexLoadBMP(ImageFiles[i].c_str(), images[i]))
		{	std::cout << "Unable to load image: " << ImageFiles[i].c_str() << std::endl;
			mFailed = true;
		}
	}	
}

//thread entry function - continuously loads images (producer)
void AsyncImageLoader::Run()
{
	ImageSet images(mNumCameras);
	BinaryImageSet FGImages(mNumCameras);
	std::vector<std::string> FGfiles(mNumCameras), ImageFiles(mNumCameras);
	while(mCurrentFrame < mNumFrames && !mFailed)
	{
		for(unsigned int i = 0; i < mNumCameras; i++)								//generate filenames
		{	FGfiles[i] = mPath + "FG" + str(i + 1) + DIR_SEPARATOR + "image" + str(mCurrentFrame, 4) + ".bmp";
			ImageFiles[i] = mPath + "CAM" + str(i + 1) + DIR_SEPARATOR + "image" + str(mCurrentFrame, 4) + ".bmp";
		}

		LoadSet(FGfiles, FGImages, ImageFiles, images);								//load the data and convert FG images to binary
		
		mDataLock.Lock();															//push images to buffer
		mImageBuffer.push_back(images);
		mFGBuffer.push_back(FGImages);
		mDataLock.Unlock();
		
		mCondEmpty.NotifyOne();														//notify waiting threads of data
		if(mImageBuffer.size() >= mBufferSize)										//if buffer is full
		{	mLock1.Lock();
			mCondFull.Wait();														//wait on full condition (wait until a request removes a set of images)
			mLock1.Unlock();
		}
		mCurrentFrame++;
	}
}

//data request function (consumer)
bool AsyncImageLoader::GetNextImageSet(ImageSet &images, BinaryImageSet &FGMaps)
{
	if(mFailed)																		
		return false;

	while(mImageBuffer.size() == 0)													//if buffer is empty, wait until images are loaded
	{	mLock2.Lock();
		mCondEmpty.Wait();
		mLock2.Unlock();
	}

	mDataLock.Lock();																//get images from buffer
	images = mImageBuffer[0];   mImageBuffer.pop_front();
	FGMaps = mFGBuffer[0];      mFGBuffer.pop_front();
	mDataLock.Unlock();

	mCondFull.NotifyOne();															//notify loading thread that buffer is not full
	return true;
}

