//-------------------------------------------------------------
//      ____                        _      _
//     / ___|____ _   _ ____   ____| |__  | |
//    | |   / ___| | | |  _  \/ ___|  _  \| |
//    | |___| |  | |_| | | | | |___| | | ||_|
//     \____|_|  \_____|_| |_|\____|_| |_|(_) Media benchmarks
//                      
//	 © 2006, Intel Corporation, licensed under Apache 2.0 
//
//  file : 
//  author : Jean-Yves Bouguet - jean-yves.bouguet@intel.com
//			 Scott Ettinger - scott.m.ettinger@intel.com
//  description : Image Projection description.
//  modified : 
//--------------------------------------------------------------


#ifndef IMAGEPROJECTION_H
#define IMAGEPROJECTION_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include "CameraModel.h"
#include "BodyGeometry.h"

#include <vector>

// Project a single 3D point onto a single camera
void ProjectPoints(Vector3f &pt_3D, Point &pt_2D, Camera &camera);

// Image projection of a cylinder onto a single camera
class ProjectedCylinder{
public:
	//std::vector<Point> mPts;										//Projected points of the cylinder (4)
	Point mPts[4];
	ProjectedCylinder(){};
	~ProjectedCylinder(){};
	void ImageProjection(const KTCylinder &cyl,Camera &camera);		//Image projection of the 3D cylinder
};

// Image projection of a body (multiple cylinders) onto a single camera
class ProjectedBody{
private:
	std::vector<ProjectedCylinder> mProjCyls;						//Each projected body part
public:
	ProjectedBody() {};
	~ProjectedBody(){};
	ProjectedCylinder &operator()(int i) {return mProjCyls[i]; };	//Get or set the ith body part
	void ImageProjection(const BodyGeometry &body, Camera &camera);	//image projection of the entire body
	int Size(){return (int)mProjCyls.size(); };
};

// Image projection of a body onto multiple cameras
class MultiCameraProjectedBody{
private:
	std::vector<ProjectedBody> mProjBodies;							//Each projected body
public:
	MultiCameraProjectedBody() {};
	~MultiCameraProjectedBody(){};
	ProjectedBody &operator()(int i){return mProjBodies[i];};				//Get/set the ith body part
	void ImageProjection(const BodyGeometry &body, MultiCamera &cameras);	//image projection of the entire body
	int Size(){return (int)mProjBodies.size(); };
};

#endif
