//-------------------------------------------------------------
//      ____                        _      _
//     / ___|____ _   _ ____   ____| |__  | |
//    | |   / ___| | | |  _  \/ ___|  _  \| |
//    | |___| |  | |_| | | | | |___| | | ||_|
//     \____|_|  \_____|_| |_|\____|_| |_|(_) Media benchmarks
//                           
//	 © 2006, Intel Corporation, licensed under Apache 2.0 
//
//  file : ImageMeasurements.h
//  author :	Scott Ettinger	- scott.m.ettinger@intel.com
//				Jean-Yves Bouguet - jean-yves.bouguet@intel.com
//  description : Image Measurements (silhouette and edges)
//  modified : 
//--------------------------------------------------------------


#ifndef IMAGEMEASUREMENTS_H
#define IMAGEMEASUREMENTS_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include <FlexLib.h>
#include "BinaryImage.h"
#include "ImageProjection.h"

#define FlexImage8u FlexImage<Im8u,1>
#define FlexImage32f FlexImage<Im32f,1>

#include <vector>

//Default granularity of sampling of the projected cylinder edges:
#define STEP_DEFAULT 5.00f

//Default horizontal and vertical granularities of sampling of the inside of the projected cylinder:
#define H_STEP_DEFAULT 10.00f
#define V_STEP_DEFAULT 10.00f


class ImageMeasurements
{
private:
	std::vector<Point> mSamples;											//pixel samples (for each body part, edge and inside)

	float mStep;															//Sampling resolution of the edges in pixels (default: STEP_DEFAULT)
	float mHstep,mVstep;													//Horizontal and vertical sampling resolutions of inside the limbs (defaults: H_STEP_DEFAULT,V_STEP_DEFAULT)
	
	//compute error at the edges of a projected cylinder (body part)
	void EdgeError(const ProjectedCylinder &ProjCyl, const FlexImage8u &EdgeMap, float &error, int &samplePoints);	

	//compute error inside of a projected cylinder (body part)
	void InsideError(const ProjectedCylinder &ProjCyl, const BinaryImage &FGmap, int &error, int &samplePoints);

public:
	ImageMeasurements(){SetSamplingResolutions(STEP_DEFAULT, H_STEP_DEFAULT, V_STEP_DEFAULT); };
	~ImageMeasurements(){};

	//Set sampling densities
	void SetSamplingResolutions(float edge, float inside_h, float inside_v){mStep=edge; mHstep=inside_h; mVstep=inside_v;};
	
	//Edge error of a complete body on all camera images
	float ImageErrorEdge(std::vector<FlexImage8u> &ImageMaps, MultiCameraProjectedBody &ProjBodies);

	//Silhouette error of a complete body on all camera images
	float ImageErrorInside(std::vector<BinaryImage> &ImageMaps, MultiCameraProjectedBody &ProjBodies);
};

#endif
