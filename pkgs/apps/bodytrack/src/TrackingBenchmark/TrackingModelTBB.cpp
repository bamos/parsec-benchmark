//-------------------------------------------------------------
//      ____                        _      _
//     / ___|____ _   _ ____   ____| |__  | |
//    | |   / ___| | | |  _  \/ ___|  _  \| |
//    | |___| |  | |_| | | | | |___| | | ||_|
//     \____|_|  \_____|_| |_|\____|_| |_|(_) Media benchmarks
//                  
//	  2006, Intel Corporation, licensed under Apache 2.0 
//
//  file : TrackingModelTBB.cpp
//  author :      Scott Ettinger - scott.m.ettinger@intel.com
//  description : Observation model for kinematic tree body 
//				  tracking threaded with TBB
//				  
//  modified : 
//--------------------------------------------------------------

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include "TrackingModelTBB.h"
#include <vector>
#include <string>
#include "system.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "tbb/blocked_range2d.h"

using namespace std;
using namespace tbb;

#define GRAIN_SIZE 8

//object used with TBB parallel for to do the gradient magnitude and thresholding operation
template<class T>
class DoGradientMagThresholdTBB{

  const FlexImage<T,1> *p_tmp, *p_src; 
  float threshold; 
  
public:
	void operator()(const blocked_range<int>& range) const
	{
		const FlexImage<T,1> &tmp = *p_tmp;
		const FlexImage<T,1> &src = *p_src; 
       
		for(int y = range.begin(); y < range.end(); y++)
		{
			Im8u *p = &src(1,y), *ph = &src(1,y - 1), *pl = &src(1,y + 1), *pr = &tmp(1,y);
			for(int x = 1; x < src.Width() - 1; x++)
			{	
				float xg = -0.125f * ph[-1] + 0.125f * ph[1] - 0.250f * p[-1] + 0.250f * p[1] - 0.125f * pl[-1] + 0.125f * pl[1];	//calc x and y gradients
				float yg = -0.125f * ph[-1] - 0.250f * ph[0] - 0.125f * ph[1] + 0.125f * pl[-1] + 0.250f * pl[0] + 0.125f * pl[1];
				float mag = xg * xg + yg * yg;																					//calc magnitude and threshold
				*pr = (mag < threshold) ? 0 : 255;
				 p++; ph++; pl++; pr++;
			}
		}
	}

	DoGradientMagThresholdTBB(FlexImage<T,1> *_source, FlexImage<T,1> *_tmp, float _threshold) : 
		p_tmp(_tmp), p_src(_source), threshold(_threshold) {}

};

//object used with TBB parallel for to do row filtering
template<class T, class T2> 
  class DoFlexFilterRowVTBB{
    
    FlexImage<T, 1> *p_src, *p_dst; 
    T2 *kernel;
    int pn;   
 
  public:
    
    void operator() ( const blocked_range<int>& r) const{
 
      int n = pn;
      FlexImage<T, 1> &src = *p_src; 
      FlexImage<T,1> &dst = *p_dst; 
      int source_width = src.Width();

      for(int y = r.begin(); y < r.end(); y++)
	{
	  T *psrc = &src(n, y);
	  T *pdst = &dst(n, y); 
	  for(int x = n; x < source_width - n; x++)
	    {
	      int k = 0; 
	      T2 acc = 0; 
	      for(int i = -n; i <= n; i++)
		acc += (T2)(psrc[i] * kernel[k++]);
	      
	      *pdst = (T)acc;
	      pdst++;
	      psrc++;
	    }
	}
    }

    DoFlexFilterRowVTBB(FlexImage<T,1> *_source, FlexImage<T,1> *_dest, T2 *_kernel, int _n) :
	p_src(_source), p_dst(_dest), kernel(_kernel), pn(_n) {}
};

//object used with TBB parallel for to do column filtering
template<class T, class T2> 
  class DoFlexFilterColumnVTBB{
    
    FlexImage<T, 1> *p_src, *p_dst; 
    T2 *kernel;
    int pn;

  public:
    
    void operator() ( const blocked_range<int>& r) const
	{
		FlexImage<T,1> &src = *p_src; 
		FlexImage<T,1> &dst = *p_dst;
		int source_width = src.Width(); 
		int sb = src.StepBytes();
		int n = pn;

		for(int y = r.begin(); y < r.end(); y++)
		{
		  T *psrc = &src(0, y);
		  T *pdst = &dst(0, y); 
		  for(int x = 0; x < source_width; x++)
		  {
			int k = 0; 
			T2 acc = 0; 
			for(int i = -n; i <= n; i++)
				acc += (T2)(*(T *)((char *)psrc + sb * i) * kernel[k++]);
		      
			*pdst = (T)acc;
			pdst++;
			psrc++;
		   }
		}
    }

    DoFlexFilterColumnVTBB(FlexImage<T,1> *_source, FlexImage<T,1> *_dest, T2 *_kernel, int _n) : 
		p_src(_source), p_dst(_dest), kernel(_kernel), pn(_n) {}
																				
};

//TBB threaded - 1D filter Row wise 1 channel any type data or kernel valid pixels only
template<class T, class T2>
bool FlexFilterRowVTBB(FlexImage<T,1> &src, FlexImage<T,1> &dst, T2 *kernel, int kernelSize, bool allocate = true)
{
	if(kernelSize % 2 == 0)									//enforce odd length kernels
		return false;
	if(allocate)
		dst.Reallocate(src.Size());
	dst.Set((T)0);
	int n = kernelSize / 2, h = src.Height();
	
	//TBB parallel_for
	parallel_for(blocked_range<int>(0, h, GRAIN_SIZE), DoFlexFilterRowVTBB<T,T2>(&src, &dst, kernel, n));
	
	return true;
}

//TBB threaded - 1D filter Column wise 1 channel any type data or kernel valid pixels only
template<class T, class T2>
bool FlexFilterColumnVTBB(FlexImage<T,1> &src, FlexImage<T,1> &dst, T2 *kernel, int kernelSize, bool allocate = true)
{
	if(kernelSize % 2 == 0)									//enforce odd length kernels
		return false;
	if(allocate)
		dst.Reallocate(src.Size());
	dst.Set((T)0);
	int n = kernelSize / 2;
	int h = src.Height() - n;

	//TBB parallel_for
	parallel_for(blocked_range<int>(n, h, GRAIN_SIZE), DoFlexFilterColumnVTBB<T,T2>(&src, &dst, kernel, n));

	return true;
}

inline void GaussianBlurTBB(FlexImage8u *src, FlexImage8u *dst)
{
    FlexImage8u tmp(src->Size());
  	float k[] = {0.12149085090552f, 0.14203719483447f, 0.15599734045770f, 0.16094922760463f, 0.15599734045770f, 0.14203719483447f, 0.12149085090552f};
	FlexFilterRowVTBB(*src, tmp, k, 7);
	FlexFilterColumnVTBB(tmp, *dst, k, 7);
}

inline FlexImage8u GradientMagThresholdTBB(FlexImage8u &src, float threshold)
{
	FlexImage8u r(src.Size());
	ZeroBorder(r);

	//TBB parallel_for
	parallel_for(blocked_range<int>(1, src.Height() - 1), DoGradientMagThresholdTBB<Im8u>(&src, &r, threshold), auto_partitioner());
	return r; 
}
  

//Generate an edge map from the original camera image
void TrackingModelTBB::CreateEdgeMap(FlexImage8u &src, FlexImage8u &dst)
{
  FlexImage8u gr = GradientMagThresholdTBB(src, 16.0f);		//calc gradient magnitude and threshold
  GaussianBlurTBB(&gr, &dst);								//Blur to create distance error map
}

//Generate an edge map from the original camera image
void ComputeEdgeMapsTBB(FlexImage8u &src, FlexImage8u &dst)
{
  FlexImage8u gr = GradientMagThresholdTBB(src, 16.0f);		//calc gradient magnitude and threshold
  GaussianBlurTBB(&gr, &dst);								//Blur to create distance error map
}


//TBB block class to process images in parallel
class DoProcessImages	{

	ImageSet *mEdgeMaps;
	vector<string> *mFGfiles, *mImageFiles;
	BinaryImageSet *mFGmaps;

public:

	DoProcessImages(vector<string> *FGfiles, vector<string> *ImageFiles, ImageSet *edgeMaps, BinaryImageSet *FGMaps)
		: mEdgeMaps(edgeMaps), mFGfiles(FGfiles), mImageFiles(ImageFiles), mFGmaps(FGMaps) {};

	void operator() (const blocked_range<int> &r) const
	{
		FlexImage8u im;
		for(int i = r.begin(); i < r.end(); i++)
		{	if(!FlexLoadBMP((*mFGfiles)[i].c_str(), im))							//Load foreground maps and raw images
			{	cout << "Unable to load image: " << (*mFGfiles)[i].c_str() << endl;
				return;
			}	
			(*mFGmaps)[i].ConvertToBinary(im);									//binarize foreground maps to 0 and 1
			if(!FlexLoadBMP((*mImageFiles)[i].c_str(), im))
			{	cout << "Unable to load image: " << (*mImageFiles)[i].c_str() << endl;
				return;
			}
			ComputeEdgeMapsTBB(im, (*mEdgeMaps)[i]);									//Create edge maps
		}
	}
};

//templated conversion to string with field width
template<class T>
inline string str(T n, int width = 0, char pad = '0')
{	stringstream ss;
	ss << setw(width) << setfill(pad) << n;
	return ss.str();
}

//load and process all images for new observation at a given time(frame)
bool TrackingModelTBB::GetObservation(float timeval)
{
	int frame = (int)timeval;													//generate image filenames
	int n = mCameras.GetCameraCount();
	vector<string> FGfiles(n), ImageFiles(n);
	for(int i = 0; i < n; i++)													
	{	FGfiles[i] = mPath + "FG" + str(i + 1) + DIR_SEPARATOR + "image" + str(frame, 4) + ".bmp";
		ImageFiles[i] = mPath + "CAM" + str(i + 1) + DIR_SEPARATOR + "image" + str(frame, 4) + ".bmp";
	}
	FlexImage8u im;
	for(int i = 0; i < (int)FGfiles.size(); i++)
	{	if(!FlexLoadBMP(FGfiles[i].c_str(), im))								//Load foreground maps and raw images
		{	cout << "Unable to load image: " << FGfiles[i].c_str() << endl;
			return false;
		}	
		mFGMaps[i].ConvertToBinary(im);											//binarize foreground maps to 0 and 1
		if(!FlexLoadBMP(ImageFiles[i].c_str(), im))
		{	cout << "Unable to load image: " << ImageFiles[i].c_str() << endl;
			return false;
		}
		ComputeEdgeMapsTBB(im, mEdgeMaps[i]);									//Create edge maps
	}
	return true;
}

//TBB pipeline stage function
void *TrackingModelTBB::operator ()(void *inToken)
{
	int n = mCameras.GetCameraCount();

	ImageSetToken *token = new ImageSetToken;
	token->edgeMaps.resize(n);
	token->FGmaps.resize(n);

	if(mCurFrame >= mNumFrames)
		return NULL;

	std::cout << "Processing frame : " << mCurFrame << std::endl;

	vector<string> FGfiles(n), ImageFiles(n);
	for(int i = 0; i < n; i++)													
	{	FGfiles[i] = mPath + "FG" + str(i + 1) + DIR_SEPARATOR + "image" + str(mCurFrame, 4) + ".bmp";
		ImageFiles[i] = mPath + "CAM" + str(i + 1) + DIR_SEPARATOR + "image" + str(mCurFrame, 4) + ".bmp";
	}

	//TBB parallel_for
	parallel_for(blocked_range<int>(0, n), DoProcessImages(&FGfiles, &ImageFiles, &(token->edgeMaps), &(token->FGmaps)), auto_partitioner());

	mCurFrame++;
	return (void *)token;														//pass to next stage (TBB uses void * !)
}

