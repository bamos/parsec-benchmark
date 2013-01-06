//--------------------------------------------------------------
//      ____                        _      _
//     / ___|____ _   _ ____   ____| |__  | |
//    | |   / ___| | | |  _  \/ ___|  _  \| |
//    | |___| |  | |_| | | | | |___| | | ||_|
//     \____|_|  \_____|_| |_|\____|_| |_|(_) Media benchmarks
//                   
//	 © 2006, Intel Corporation, licensed under Apache 2.0 
//
//  file : RandomGenerator.h
//  author : Scott Ettinger - scott.m.ettinger@intel.com
//  description : Random number generator object. 
// 				  The uniform distribution generator is based 
//				  on the ran2() function from Numerical
//				  Recipes in C.
//  modified : 
//---------------------------------------------------------------

#ifndef RANDOMGENERATOR_H
#define RANDOMGENERATOR_H

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include <math.h>

#define NTAB  32	//size of shuffle table

//Random number generator class. Provides uniform and normally distributed random numbers.
class RandomGenerator{
protected:
	long mIdum, mIdum2, mIy, mIv[NTAB], mIset;		//generator values
	double mGset;
public:
	RandomGenerator();
	~RandomGenerator() {};

	///Start random number generator with a new seed
	void Seed(long s) {mIdum = (s > 0) ? -s : s; Rand(); };
	///Return a uniformly distributed random value between 0 and 1
	double Rand();
	///Return a normally distributed value mean = 0, sigma = 1
	double RandN();
	///Return an exponentially distributed value of unit mean
	double RandExp();
};

#endif
