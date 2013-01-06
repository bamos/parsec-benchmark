//-------------------------------------------------------------
//      ____                        _      _
//     / ___|____ _   _ ____   ____| |__  | |
//    | |   / ___| | | |  _  \/ ___|  _  \| |
//    | |___| |  | |_| | | | | |___| | | ||_|
//     \____|_|  \_____|_| |_|\____|_| |_|(_) Media benchmarks
//
//	 © 2006, Intel Corporation, licensed under Apache 2.0 
//
//  file : AnnealingFactor.h
//  author : Jean-Yves Bouguet - jean-yves.bouguet@intel.com
//  description : function that estimates the alleanign factor needed
//				  for the annealed particle filter
//  modified : 
//--------------------------------------------------------------


#ifndef ANNEALINGFACTOR_H
#define ANNEALINGFACTOR_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include <vector>


// Desired precision on the alpha parameter:
#define ALPHA_PRECISION 0.00001f


// Minimum and maximum values of the annealing coefficient beta
#define BETA_MIN 0
#define BETA_MAX 1000

// Maximum number of iterations:
#define N_ITER_MAX 100

// Maximum number that can be exponentiated in floats:
#define LOG_MAX_FLOAT 40

// Estimates the optimal beta coefficient to achieve a particle survival rate of alpha_desired
float BetaAnnealingFactor(std::vector<float> &ets,float alpha_desired, float beta_min = BETA_MIN, float beta_max = BETA_MAX );

// Debug function that sets the vector ets to a value
void set_ets(std::vector<float> &ets);

#endif

