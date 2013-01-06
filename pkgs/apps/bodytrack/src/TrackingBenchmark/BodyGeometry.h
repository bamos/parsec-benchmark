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
//  author : Scott Ettinger - scott.m.ettinger@intel.com
//			 Jean-Yves Bouguet - jean-yves.bouguet@intel.com 
//  description : Body geometry description.
//  modified : 
//--------------------------------------------------------------


#ifndef BODYGEOMETRY_H
#define BODYGEOMETRY_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include <vector>
#include <string>

#include "BodyPose.h"
#include "DMatrix.h"
#include "SmallVectors.h"

#define N_PARTS 10
#define N_LENGTHS 18

//Preset rotation matrices
const float XRot90[12]  = {1.00f,  0.00f, 0.00f,  0.00f, 0.00f, 0.00f,  1.00f,  0.00f, 0.00f, -1.00f, 0.00f,  0.00f};
const float XRot180[12] = {1.00f,  0.00f, 0.00f,  0.00f, 0.00f, -1.00f, 0.00f,  0.00f, 0.00f, 0.00f,  -1.00f, 0.00f};
const float XRot270[12] = {1.00f,  0.00f, 0.00f,  0.00f, 0.00f, 0.00f,  -1.00f, 0.00f, 0.00f, 1.00f,  0.00f,  0.00f};
const float YRot90[12]  = {0.00f,  0.00f, -1.00f, 0.00f, 0.00f, 1.00f,  0.00f,  0.00f, 1.00f, 0.00f,  0.00f,  0.00f};
const float YRot180[12] = {-1.00f, 0.00f, 0.00f,  0.00f, 0.00f, 1.00f,  0.00f,  0.00f, 0.00f, 0.00f,  -1.00f, 0.00f};
const float ZRot180[12] = {-1.00f, 0.00f, 0.00f,  0.00f, 0.00f, -1.00f, 0.00f,  0.00f, 0.00f, 0.00f,  1.00f,  0.00f};

//Body geometry parameters:
class BodyParameters{
public:
	float limbs[N_PARTS][4];									//Radius of each body part
	float lengths[N_LENGTHS];									//Length of each body parts								
	
	BodyParameters(){};
	~BodyParameters(){};
	
	//Load the body shape parameters from a file
	bool InitBodyParameters(const std::string fname);						
};

//primitive cylinder object:
class KTCylinder{
public:
	float bottom, top, length;									//bottom,top radius and length of the cylinder
	DMatrix<float> pose;										//3D pose of the cylinder
	
	KTCylinder(){};
	~KTCylinder(){};
	
	//Construct a cylinder given top and bottom radii, and length
	KTCylinder(float b, float t, float l) {top = t; bottom = b; length = l; };
	
	//Set cylinder values given top and bottom radii, and length
	void Set(float b, float t, float l) {top = t; bottom = b; length = l; };
};

//primitive body geometry object:
class BodyGeometry{
private:
	std::vector<KTCylinder> mCylinders;							//Each body part is modeled as a conic cylinder
	BodyParameters mParams;										//Body shape parameters

	std::vector<Vector3f> mCentersA, mCentersB;					//centers of spheres used to approximate the conic cylinders during intersection test
	std::vector<float> mRadiiA, mRadiiB;						//radii of spheres

	bool IntersectingCylinders(KTCylinder &cylA, KTCylinder &cylB);

public:
	BodyGeometry() { mCylinders.resize(N_PARTS); };
	~BodyGeometry(){};

	//Load Body Parameters from a file
	bool InitBodyShape(std::string fname) {return mParams.InitBodyParameters(fname); };
	
	//Get/set the ith body part
	KTCylinder &operator()(int i) {return mCylinders[i]; };	
	const KTCylinder &operator()(int i) const {return mCylinders[i]; };

	//Get body parameters
	BodyParameters &Parameters() {return mParams; };
	
	//Generate body geometry from a given pose
	void ComputeGeometry(BodyPose &angles, BodyParameters &params);	

	//returns the number of body parts
	int GetBodyPartCount() const {return (int)mCylinders.size();};		

	//returns true if no pair of body parts intersect
	bool Valid();												
};



#endif

