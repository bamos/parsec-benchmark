//#####################################################################
// Copyright 2002, Ronald Fedkiw.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Function maxmag
//#####################################################################
//
// finds the maximum value in magnitude and returns it with the sign
//
//#####################################################################
#ifndef __maxmag__
#define __maxmag__

namespace PhysBAM
{

template<class T>
inline T maxmag (const T a, const T b)
{
	return fabs (a) > fabs (b) ? a : b;
}

inline double maxmag (const double a, const int b)
{
	return fabs (a) > fabs ( (double) b) ? a : b;
}

inline double maxmag (const int a, const double b)
{
	return fabs ( (double) a) > fabs (b) ? a : b;
}

template<class T>
inline T maxmag (const T a, const T b, const T c)
{
	return maxmag (a, maxmag (b, c));
}

template<class T>
inline T maxmag (const T a, const T b, const T c, const T d)
{
	return maxmag (a, maxmag (b, c, d));
}

template<class T>
inline T maxmag (const T a, const T b, const T c, const T d, const T e)
{
	return maxmag (a, maxmag (b, c, d, e));
}

template<class T>
inline T maxmag (const T a, const T b, const T c, const T d, const T e, const T f)
{
	return maxmag (a, maxmag (b, c, d, e, f));
}

}
#endif

