//-------------------------------------------------------------
//      ____                        _      _
//     / ___|____ _   _ ____   ____| |__  | |
//    | |   / ___| | | |  _  \/ ___|  _  \| |
//    | |___| |  | |_| | | | | |___| | | ||_|
//     \____|_|  \_____|_| |_|\____|_| |_|(_) Media benchmarks
//                  
//	  2006, Intel Corporation, licensed under Apache 2.0 
//
//  file : TrackingModelOMP.cpp
//  author : Scott Ettinger - scott.m.ettinger@intel.com
//  description : Observation model for kinematic tree body 
//				  tracking threaded with OpenMP.
//				  
//  modified : 
//--------------------------------------------------------------

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include "TrackingModelOMP.h"
#include <vector>
#include <string>
#include <omp.h>
#include "system.h"

using namespace std;


//------------------------ Threaded versions of image filters --------------------

//OMP threaded - 1D filter Row wise 1 channel any type data or kernel valid pixels only
template<class T, class T2>
bool FlexFilterRowVOMP(FlexImage<T,1> &src, FlexImage<T,1> &dst, T2 *kernel, int kernelSize, bool allocate = true)
{
	if(kernelSize % 2 == 0)									//enforce odd length kernels
		return false;
	if(allocate)
		dst.Reallocate(src.Size());
	dst.Set((T)0);
	int n = kernelSize / 2, h = src.Height();
	#pragma omp parallel for
	for(int y = 0; y < h; y++)
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

//OMP threaded - 1D filter Column wise 1 channel any type data or kernel valid pixels only
template<class T, class T2>
bool FlexFilterColumnVOMP(FlexImage<T,1> &src, FlexImage<T,1> &dst, T2 *kernel, int kernelSize, bool allocate = true)
{
	if(kernelSize % 2 == 0)									//enforce odd length kernels
		return false;
	if(allocate)
		dst.Reallocate(src.Size());
	dst.Set((T)0);
	int n = kernelSize / 2;
	int sb = src.StepBytes(), h = src.Height() - n;
	#pragma omp parallel for
	for(int y = n; y < h; y++)
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

// ----------------------------------------------------------------------------------

//Generate an edge map from the original camera image
//Separable 7x7 gaussian filter - threaded
inline void GaussianBlurOMP(FlexImage8u &src, FlexImage8u &dst)
{
	float k[] = {0.12149085090552f, 0.14203719483447f, 0.15599734045770f, 0.16094922760463f, 0.15599734045770f, 0.14203719483447f, 0.12149085090552f};
	FlexImage8u tmp;
	FlexFilterRowVOMP(src, tmp, k, 7);											//separable gaussian convolution using kernel k
	FlexFilterColumnVOMP(tmp, dst, k, 7);
}

//Calculate gradient magnitude and threshold to binarize - threaded
inline FlexImage8u GradientMagThresholdOMP(FlexImage8u &src, float threshold)
{
	FlexImage8u r(src.Size());
	ZeroBorder(r);
	#pragma omp parallel for
	for(int y = 1; y < src.Height() - 1; y++)																					//for each pixel
	{	Im8u *p = &src(1,y), *ph = &src(1,y - 1), *pl = &src(1,y + 1), *pr = &r(1,y);
		for(int x = 1; x < src.Width() - 1; x++)
		{	float xg = -0.125f * ph[-1] + 0.125f * ph[1] - 0.250f * p[-1] + 0.250f * p[1] - 0.125f * pl[-1] + 0.125f * pl[1];	//calc x and y gradients
			float yg = -0.125f * ph[-1] - 0.250f * ph[0] - 0.125f * ph[1] + 0.125f * pl[-1] + 0.250f * pl[0] + 0.125f * pl[1];
			float mag = xg * xg + yg * yg;																						//calc magnitude and threshold
			*pr = (mag < threshold) ? 0 : 255;
			p++; ph++; pl++; pr++;
		}
	}
	return r;
}

//Generate an edge map from the original camera image
void TrackingModelOMP::CreateEdgeMap(FlexImage8u &src, FlexImage8u &dst)
{
	FlexImage8u gr = GradientMagThresholdOMP(src, 16.0f);						//calc gradient magnitude and threshold
	GaussianBlurOMP(gr, dst);													//Blur to create distance error map
}

//templated conversion to string with field width
template<class T>
inline string str(T n, int width = 0, char pad = '0')
{	stringstream ss;
	ss << setw(width) << setfill(pad) << n;
	return ss.str();
}

//load and process all images for new observation at a given time(frame)
//Overloaded from base class for future threading to overlap disk I/O with 
//generating the edge maps
bool TrackingModelOMP::GetObservation(float timeval)
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
		CreateEdgeMap(im, mEdgeMaps[i]);										//Create edge maps
	}
	return true;
}