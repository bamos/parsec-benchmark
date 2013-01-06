//-------------------------------------------------------------
//      ____                        _      _
//     / ___|____ _   _ ____   ____| |__  | |
//    | |   / ___| | | |  _  \/ ___|  _  \| |
//    | |___| |  | |_| | | | | |___| | | ||_|
//     \____|_|  \_____|_| |_|\____|_| |_|(_) Media benchmarks
//            
//	 © 2006, Intel Corporation, licensed under Apache 2.0 
//
//  file : CovarianceMatrix.cpp
//  author : Jean-Yves Bouguet - jean-yves.bouguet@intel.com
//  description : Computes the set of standard deviation matrices for all annealing layers
//  modified : 
//--------------------------------------------------------------

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include "CovarianceMatrix.h"

#include <math.h>

//Computes the set of standard deviation matrices for a given number of annealing layers and a tracking model
void GenerateStDevMatrices(int layers,PoseParams &params, std::vector<std::vector<float> > &stdDevs)
{
	stdDevs.resize(layers);

	int N_pose_angles = (int) params.stdVec().size();

	// Set the fine level standard deviation matrix:
	stdDevs[0].resize(N_pose_angles);
	for(int i=0;i<N_pose_angles;i++)
		stdDevs[0][i] = params.PoseStd(i);

	if(layers<2) return; // Single layer scenario (special case)

	float alpha_sqrt = (float) sqrt((double) ALPHA_M);

	//Set the last layer:
	stdDevs[layers-1].resize(N_pose_angles);
	for(int i=0;i<N_pose_angles;i++)
		stdDevs[layers-1][i] = alpha_sqrt * stdDevs[0][i];

	if(layers<3) return; // Two layers scenario

	// Set all the other matrices:
	for(int j=layers-2;j>=1;j--)
	{	stdDevs[j].resize(N_pose_angles);
		for(int i=0;i<N_pose_angles;i++)
			stdDevs[j][i] = alpha_sqrt * stdDevs[j+1][i];
	}
}

//Computes the set of standard deviation matrices for a given number of annealing layers (using pose parameters loaded from a file)
void GenerateStDevMatrices(int layers, const char* fname, std::vector<std::vector<float> > &stdDevs)
{
	// Get the pose parameters for the standard deviations
	PoseParams params;
	params.Initialize(fname);

	GenerateStDevMatrices(layers,params,stdDevs);
}
