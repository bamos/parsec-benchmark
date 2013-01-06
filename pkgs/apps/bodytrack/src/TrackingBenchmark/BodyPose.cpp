//-------------------------------------------------------------
//      ____                        _      _
//     / ___|____ _   _ ____   ____| |__  | |
//    | |   / ___| | | |  _  \/ ___|  _  \| |
//    | |___| |  | |_| | | | | |___| | | ||_|
//     \____|_|  \_____|_| |_|\____|_| |_|(_) Media benchmarks
//                 
//	 © 2006, Intel Corporation, licensed under Apache 2.0 
//
//  file : BodyPose.cpp
//  author : Jean-Yves Bouguet - jean-yves.bouguet@intel.com
//			 Scott Ettinger - scott.m.ettinger@intel.com
//  description : Body pose description.
//  modified : 
//--------------------------------------------------------------

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include <fstream>
#include <iostream>
#include "BodyPose.h"
#include <vector>

using namespace std;

//set all pose values
void BodyPose::Set(float *angle_values,int n)
{	mAngles.resize(n);
	for(int i=0;i<n;i++)
		mAngles[i] = angle_values[i];
}

// returns true if all body pose angles are between min and max values
bool BodyPose::Valid(PoseParams &params)
{
	return true;	//disabled - can be enabled if valid pose extent parameters are present

	if ((mAngles[0]< params.PoseMin(0))||(mAngles[0]> params.PoseMax(0)))
		return false;
	if ((mAngles[1]< params.PoseMin(1))||(mAngles[1]> params.PoseMax(1)))
		return false;
	for(int i=5;i<(int) mAngles.size();i++)
		if ((mAngles[i]< params.PoseMin(i))||(mAngles[i]> params.PoseMax(i)))
			return false;

	return true;
}

//load pose from file
bool BodyPose::Initialize(string fname)
{
	mAngles.resize(N_ANGLES);
	ifstream f(fname.c_str());
	if(!f.is_open())
	{	cout << "Unable to open Initial Pose file : " << fname << endl;
		return false;
	}
	for(int i=0; i<N_ANGLES; i++)
		f >> mAngles[i];
	return true;
}

//load pose parameters from file
bool PoseParams::Initialize(string fname)
{
	stdAngle.resize(N_ANGLES);
	minAngles.resize(N_ANGLES);
	maxAngles.resize(N_ANGLES);

	ifstream f(fname.c_str());
	if(!f.is_open())
	{	cout << "Unable to open Pose Parameter file : " << fname << endl;
		return false;
	}
	for(int i=0; i<N_ANGLES; i++)
		f >> stdAngle[i];
	for(int i=0; i<N_ANGLES; i++)
		f >> minAngles[i];
	for(int i=0; i<N_ANGLES; i++)
		f >> maxAngles[i];
	return true;
}
