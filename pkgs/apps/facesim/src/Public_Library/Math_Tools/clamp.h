//#####################################################################
// Copyright 2002-2004, Ronald Fedkiw, Geoffrey Irving, Igor Neverov, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Function clamp
//#####################################################################
#ifndef __clamp__
#define __clamp__
#include "../Matrices_And_Vectors/VECTOR_1D.h"
#include "../Matrices_And_Vectors/VECTOR_2D.h"
#include "../Matrices_And_Vectors/VECTOR_3D.h"
#include "../Matrices_And_Vectors/SYMMETRIC_MATRIX_2X2.h"
#include "../Matrices_And_Vectors/SYMMETRIC_MATRIX_3X3.h"

namespace PhysBAM
{

template<class T>
inline T clamp (const T x, const T xmin, const T xmax)
{
	if (x <= xmin) return xmin;
	else if (x >= xmax) return xmax;
	else return x;
}

template<class T>
inline T clamp_min (const T x, const T xmin)
{
	if (x <= xmin) return xmin;
	else return x;
}

template<class T>
inline T clamp_max (const T x, const T xmax)
{
	if (x >= xmax) return xmax;
	else return x;
}

template<class T>
inline VECTOR_1D<T> clamp (const VECTOR_1D<T> x, const VECTOR_1D<T> xmin, const VECTOR_1D<T> xmax)
{
	return VECTOR_1D<T> (clamp (x.x, xmin.x, xmax.x));
}

template<class T>
inline VECTOR_1D<T> clamp_min (const VECTOR_1D<T> x, const VECTOR_1D<T> xmin)
{
	return VECTOR_1D<T> (clamp_min (x.x, xmin.x));
}

template<class T>
inline VECTOR_1D<T> clamp_max (const VECTOR_1D<T> x, const VECTOR_1D<T> xmax)
{
	return VECTOR_1D<T> (clamp_max (x.x, xmax.x));
}

template<class T>
inline VECTOR_2D<T> clamp (const VECTOR_2D<T> x, const VECTOR_2D<T> xmin, const VECTOR_2D<T> xmax)
{
	return VECTOR_2D<T> (clamp (x.x, xmin.x, xmax.x), clamp (x.y, xmin.y, xmax.y));
}

template<class T>
inline VECTOR_2D<T> clamp_min (const VECTOR_2D<T> x, const VECTOR_2D<T> xmin)
{
	return VECTOR_2D<T> (clamp_min (x.x, xmin.x), clamp_min (x.y, xmin.y));
}

template<class T>
inline VECTOR_2D<T> clamp_max (const VECTOR_2D<T> x, const VECTOR_2D<T> xmax)
{
	return VECTOR_2D<T> (clamp_max (x.x, xmax.x), clamp_max (x.y, xmax.y));
}

template<class T>
inline VECTOR_3D<T> clamp (const VECTOR_3D<T> x, const VECTOR_3D<T> xmin, const VECTOR_3D<T> xmax)
{
	return VECTOR_3D<T> (clamp (x.x, xmin.x, xmax.x), clamp (x.y, xmin.y, xmax.y), clamp (x.z, xmin.z, xmax.z));
}

template<class T>
inline VECTOR_3D<T> clamp_min (const VECTOR_3D<T> x, const VECTOR_3D<T> xmin)
{
	return VECTOR_3D<T> (clamp_min (x.x, xmin.x), clamp_min (x.y, xmin.y), clamp_min (x.z, xmin.z));
}

template<class T>
inline VECTOR_3D<T> clamp_max (const VECTOR_3D<T> x, const VECTOR_3D<T> xmax)
{
	return VECTOR_3D<T> (clamp_max (x.x, xmax.x), clamp_max (x.y, xmax.y), clamp_max (x.z, xmax.z));
}

template<class T>
inline SYMMETRIC_MATRIX_2X2<T> clamp (const SYMMETRIC_MATRIX_2X2<T> x, const SYMMETRIC_MATRIX_2X2<T> xmin, const SYMMETRIC_MATRIX_2X2<T> xmax)
{
	return SYMMETRIC_MATRIX_2X2<T> (clamp (x.x11, xmin.x11, xmax.x11), clamp (x.x21, xmin.x21, xmax.x21), clamp (x.x22, xmin.x22, xmax.x22));
}

template<class T>
inline SYMMETRIC_MATRIX_2X2<T> clamp_min (const SYMMETRIC_MATRIX_2X2<T> x, const SYMMETRIC_MATRIX_2X2<T> xmin)
{
	return SYMMETRIC_MATRIX_2X2<T> (clamp_min (x.x11, xmin.x11), clamp_min (x.x21, xmin.x21), clamp_min (x.x22, xmin.x22));
}

template<class T>
inline SYMMETRIC_MATRIX_2X2<T> clamp_max (const SYMMETRIC_MATRIX_2X2<T> x, const SYMMETRIC_MATRIX_2X2<T> xmax)
{
	return SYMMETRIC_MATRIX_2X2<T> (clamp_max (x.x11, xmax.x11), clamp_max (x.x21, xmax.x21), clamp_max (x.x22, xmax.x22));
}

template<class T>
inline SYMMETRIC_MATRIX_3X3<T> clamp (const SYMMETRIC_MATRIX_3X3<T> x, const SYMMETRIC_MATRIX_3X3<T> xmin, const SYMMETRIC_MATRIX_3X3<T> xmax)
{
	return SYMMETRIC_MATRIX_3X3<T> (clamp (x.x11, xmin.x11, xmax.x11), clamp (x.x21, xmin.x21, xmax.x21), clamp (x.x31, xmin.x31, xmax.x31), clamp (x.x22, xmin.x22, xmax.x22), clamp (x.x32, xmin.x32, xmax.x32), clamp (x.x33, xmin.x33, xmax.x33));
}

template<class T>
inline SYMMETRIC_MATRIX_3X3<T> clamp_min (const SYMMETRIC_MATRIX_3X3<T> x, const SYMMETRIC_MATRIX_3X3<T> xmin)
{
	return SYMMETRIC_MATRIX_3X3<T> (clamp_min (x.x11, xmin.x11), clamp_min (x.x21, xmin.x21), clamp_min (x.x31, xmin.x31), clamp_min (x.x22, xmin.x22), clamp_min (x.x32, xmin.x32), clamp_min (x.x33, xmin.x33));
}

template<class T>
inline SYMMETRIC_MATRIX_3X3<T> clamp_max (const SYMMETRIC_MATRIX_3X3<T> x, const SYMMETRIC_MATRIX_3X3<T> xmax)
{
	return SYMMETRIC_MATRIX_3X3<T> (clamp_max (x.x11, xmax.x11), clamp_max (x.x21, xmax.x21), clamp_max (x.x31, xmax.x31), clamp_max (x.x22, xmax.x22), clamp_max (x.x32, xmax.x32), clamp_max (x.x33, xmax.x33));
}

}
#endif
