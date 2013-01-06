//-------------------------------------------------------------
//      ____                        _      _
//     / ___|____ _   _ ____   ____| |__  | |
//    | |   / ___| | | |  _  \/ ___|  _  \| |
//    | |___| |  | |_| | | | | |___| | | ||_|
//     \____|_|  \_____|_| |_|\____|_| |_|(_) Media benchmarks
//                          
//	 © 2006, Intel Corporation, licensed under Apache 2.0 
//
//  file : BodyPose.h
//  author : Jean-Yves Bouguet - jean-yves.bouguet@intel.com
//			 Scott Ettinger - scott.m.ettinger@intel.com
//  description : Body pose description.
//  modified : 
//--------------------------------------------------------------


#ifndef BODYPOSE_H
#define BODYPOSE_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include <string>
#include <vector>


#define N_ANGLES 31

//Pose parameters
class PoseParams{
private:
	std::vector<float> stdAngle;							//Standard deviations on the pose angles
	std::vector<float> minAngles;							//minimum values of the pose angles
	std::vector<float> maxAngles;							//maximum values of the pose angles
public:
	PoseParams(){};
	~PoseParams(){};
	bool Initialize(std::string fname);						//Initialize the pose parameters from a file
	float &PoseStd(int i){return stdAngle[i];};				//ith entry of the standard deviation vector
	float &PoseMin(int i){return minAngles[i];};			//ith entry of the minimum vector
	float &PoseMax(int i){return maxAngles[i];};			//ith entry of the maximum vector
	std::vector<float> &stdVec(){return stdAngle;};			//the standard deviation vector
	std::vector<float> &minVec(){return minAngles;};		//the minimum vector
	std::vector<float> &maxVec(){return maxAngles;};		//the maximum vector
};

//Body pose
class BodyPose{
private:
	PoseParams mParams;										//Body Pose parameters
	std::vector<float> mAngles;								//Angles describing the body pose
public:
	BodyPose(){};
	~BodyPose(){};
	bool Initialize(std::string fname);						//Initialize the body pose from a file
	bool InitParams(std::string fname)
	{	return mParams.Initialize(fname);};					//Initialize the pose parameters from a file
	float &operator()(int i){return mAngles[i];};			//Get or set the ith body pose angle entry
	std::vector<float> Pose() {return mAngles;};			//Get or set the entire set of angles
	PoseParams &Params() {return mParams; };				//Get the pose parameters
	void Set(float *angle_values,int n);					//Set the set of angles from a float* and the size
	void Set(const std::vector<float> &v) {mAngles = v;};
	int Size(){return (int) mAngles.size();};				//returns the number of body parts
	PoseParams	&PoseParameters(){return mParams;};
	bool Valid(PoseParams &params);							// returns true only if all body pose angles are between min and max values
};

#endif
