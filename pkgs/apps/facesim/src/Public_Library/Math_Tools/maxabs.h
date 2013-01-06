//#####################################################################
// Copyright 2002-2003, Ronald Fedkiw, Eran Guendelman.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Function maxabs
//#####################################################################
//
// finds the maximum absolute value
//
//#####################################################################
#ifndef __maxabs__
#define __maxabs__

#include<math.h>
#include "max.h"
namespace PhysBAM
{

// a should already be non-negative
template<class T>
inline T maxabs_incremental (const T a, const T b)
{
	return max (a, (T) fabs (b));
}

template<class T>
inline T maxabs (const T a, const T b)
{
	return max ( (T) fabs (a), (T) fabs (b));
}

inline double maxabs (const double a, const int b)
{
	return max (fabs (a), fabs ( (double) b));
}

inline double maxabs (const int a, const double b)
{
	return max (fabs ( (double) a), fabs (b));
}

template<class T>
inline T maxabs (const T a, const T b, const T c)
{
	return maxabs_incremental (maxabs (a, b), c);
}

template<class T>
inline T maxabs (const T a, const T b, const T c, const T d)
{
	return maxabs_incremental (maxabs (a, b, c), d);
}

template<class T>
inline T maxabs (const T a, const T b, const T c, const T d, const T e)
{
	return maxabs_incremental (maxabs (a, b, c, d), e);
}

template<class T>
inline T maxabs (const T a, const T b, const T c, const T d, const T e, const T f)
{
	return maxabs_incremental (maxabs (a, b, c, d, e), f);
}

}
#endif

