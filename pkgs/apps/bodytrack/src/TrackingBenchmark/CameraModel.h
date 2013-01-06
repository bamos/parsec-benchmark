//-------------------------------------------------------------
//      ____                        _      _
//     / ___|____ _   _ ____   ____| |__  | |
//    | |   / ___| | | |  _  \/ ___|  _  \| |
//    | |___| |  | |_| | | | | |___| | | ||_|
//     \____|_|  \_____|_| |_|\____|_| |_|(_) Media benchmarks
//               
//	 © 2006, Intel Corporation, licensed under Apache 2.0 
//
//  file : CameraModel.h
//  author : Jean-Yves Bouguet - jean-yves.bouguet@intel.com
//			 Scott Ettinger - scott.m.ettinger@intel.com
//  description : Camera Model description (single and multiple).
//  modified : 
//--------------------------------------------------------------


#ifndef CAMERAMODEL_H
#define CAMERAMODEL_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include <vector>
#include "SmallVectors.h"
#include "DMatrix.h"

#define CAMERA_COUNT	4

class Point{
public:
	float x,y;
	Point(){};
	Point(float vx, float vy){x = vx; y=vy;};
	~Point(){};
	inline void Set(float vx, float vy){x = vx; y=vy;};
};

//Single camera model:
//For more information, check http://www.vision.caltech.edu/bouguetj/calib_doc
class Camera{
public:
	Point fc;												//Focal length
	Point cc;												//Principal point
	float alpha_c;											//skew coefficient
	float kc[5];											//Distortion parameters 
	float Rc_ext[3][3];										//Rotation matrix (extrinsics)
	Vector3f omc_ext;										//Rotation vector (extrinsics)
    Vector3f Tc_ext;										//Translation vector (extrinsics)
	DMatrix<float> mc_ext;									//Full extrinsics (=[Rc_ext Tc_ext])
    Vector3<float> eye;										//Position of the camera in the world reference frame
	Camera(){};
	~Camera(){};
	bool LoadParams(const char *fname);						//Load the camera parameters from a file
};


//Multiple Cameras:
class MultiCamera{
private:
	std::vector<Camera> mCameras;							//Multiple cameras
public:
	MultiCamera() {};
	~MultiCamera(){};
	void SetCameras(int n) {mCameras.resize(n); };			//set number of cameras
	Camera &operator()(int i){return mCameras[i];};			//Get the ith camera
	std::vector<Camera> &operator()(){return mCameras;};	//Get all the cameras
	int GetCameraCount(){return (int) mCameras.size();};	//returns the number cameras
};


#endif
