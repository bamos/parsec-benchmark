//-------------------------------------------------------------
//      ____                        _      _
//     / ___|____ _   _ ____   ____| |__  | |
//    | |   / ___| | | |  _  \/ ___|  _  \| |
//    | |___| |  | |_| | | | | |___| | | ||_|
//     \____|_|  \_____|_| |_|\____|_| |_|(_) Media benchmarks
//                           
//	 © 2006, Intel Corporation, licensed under Apache 2.0 
//
//  file : CovarianceMatrix.h
//  author : Jean-Yves Bouguet - jean-yves.bouguet@intel.com
//  description : Computes the set of standard deviation matrices.
//  modified : 
//--------------------------------------------------------------


#ifndef COVARIANCEMATRIX_H
#define COVARIANCEMATRIX_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include <vector>
#include "BodyPose.h"

// Particle survival rate
#define ALPHA_M .5

//Computes the set of standard deviation matrices for a given number of annealing layers and set of pose parameters
void GenerateStDevMatrices(int layers,PoseParams &params, std::vector<std::vector<float> > &stdDevs);

//Computes the set of standard deviation matrices for a given number of annealing layers (using pose parameters loaded from a file)
void GenerateStDevMatrices(int layers, const char* fname, std::vector<std::vector<float> > &stdDevs);

#endif
