//#####################################################################
// Copyright 2002, Ronald Fedkiw, Sergey Koltakov.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Function min
//#####################################################################
#ifndef __min__
#define __min__

// need to undefine the MS Windows version of min
#ifdef _WIN32
#include<windows.h>
#endif
#ifdef min
#undef min
#endif

namespace PhysBAM
{

template<class T>
inline T min (const T a, const T b)
{
	return a < b ? a : b;
}

template<class T>
inline T min (const T a, const T b, const T c)
{
	return min (a, min (b, c));
}

template<class T>
inline T min (const T a, const T b, const T c, const T d)
{
	return min (a, min (b, c, d));
}

template<class T>
inline T min (const T a, const T b, const T c, const T d, const T e)
{
	return min (a, min (b, c, d, e));
}

template<class T>
inline T min (const T a, const T b, const T c, const T d, const T e, const T f)
{
	return min (a, min (b, c, d, e, f));
}

template<class T>
inline T min (const T a, const T b, const T c, const T d, const T e, const T f, const T g)
{
	return min (a, min (b, c, d, e, f, g));
}

template<class T>
inline T min (const T a, const T b, const T c, const T d, const T e, const T f, const T g, const T h)
{
	return min (a, min (b, c, d, e, f, g, h));
}

}
#endif































































