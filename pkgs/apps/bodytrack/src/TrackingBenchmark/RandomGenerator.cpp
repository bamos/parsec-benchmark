//--------------------------------------------------------------
//      ____                        _      _
//     / ___|____ _   _ ____   ____| |__  | |
//    | |   / ___| | | |  _  \/ ___|  _  \| |
//    | |___| |  | |_| | | | | |___| | | ||_|
//     \____|_|  \_____|_| |_|\____|_| |_|(_) Media benchmarks
//                   
//	 © 2006, Intel Corporation, licensed under Apache 2.0 
//
//  file : RandomGenerator.cpp
//  author : Scott Ettinger - scott.m.ettinger@intel.com
//  description : Random number generator object. 
// 				  The uniform distribution generator is based 
//				  on the ran2() function 
//  modified : 
//---------------------------------------------------------------

#if defined(HAVE_CONFIG_H)
# include "config.h"
#endif

#include "RandomGenerator.h"

//Random generator numerical constants
#define IM1   2147483563
#define IM2   2147483399
#define AM    (1.0/IM1)
#define IMM1  (IM1-1)
#define IA1   40014
#define IA2   40692
#define IQ1   53668
#define IQ2   52774
#define IR1   12211
#define IR2   3791
#define NDIV  (1+IMM1/NTAB)
#define EPS   1.2e-7
#define RNMX  (1.0 - EPS)

RandomGenerator::RandomGenerator()
{	mIy = 0; mIdum2 = 123456789; mIset = 0; 
	Seed(1);
}

//Generate pseudo-random values from 0 to 1.  This algorithm 
//is known to be portable and to have very little serial correlation.
double RandomGenerator::Rand()
{	if (mIdum <= 0)										// initialize if negative 
	{	mIdum = mIdum2 = (-mIdum < 1) ? 1 : -mIdum;		// guarantee positive and non-zero 
		for(int i = NTAB + 7; i >= 0; i--) 				// load shuffle table after 8 warm-ups
		{	long k = mIdum / IQ1;
			mIdum = IA1 * (mIdum - k * IQ1) - k * IR1;	
			if (mIdum < 0) mIdum += IM1;
			if (i < NTAB) mIv[i] = mIdum;
		}
		mIy = mIv[0];
	}
	long k = mIdum / IQ1;								//otherwise update to new state
	mIdum = IA1 * (mIdum - k * IQ1) - k * IR1;
	if (mIdum < 0) mIdum += IM1;
	k = mIdum2 / IQ2;
	mIdum2 = IA2 * (mIdum2 - k * IQ2) - k * IR2;
	if (mIdum2 < 0)	mIdum2 += IM2;
	int j = mIy / NDIV;
	mIy = mIv[j] - mIdum2;
	mIv[j] = mIdum;
	if(mIy < 1) mIy += IMM1;
	double temp;
	if ((temp = AM * mIy) > RNMX)
		return RNMX;									
	else
		return temp;
}

//Generate random numbers from a Normal distribution N(0,1)
double RandomGenerator::RandN()							
{   double fac, rsq, v1, v2;
    if (mIset == 0) 
	{   do 
		{	v1 = 2.0 * Rand() - 1.0;					//use fundamental transformation law of probabilities
			v2 = 2.0 * Rand() - 1.0;
			rsq = v1 * v1 + v2 * v2;
        } while (rsq >= 1.0 || rsq == 0.0);
        fac = sqrt(-2.0 * log(rsq)/rsq);
        mGset = v1 * fac;
        mIset = 1;
        return ((v2 * fac));
    } 
	else 
	{   mIset=0;
        return mGset;
    }
}

//Generate Exponentially distributed random numbers
double RandomGenerator::RandExp()
{
	double d = Rand();
	while(d == 0)
		d = Rand();
	return -log(d);
}
