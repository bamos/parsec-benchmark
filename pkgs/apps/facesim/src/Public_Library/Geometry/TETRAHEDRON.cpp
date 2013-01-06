//#####################################################################
// Copyright 2002, 2003, Robert Bridson, Ronald Fedkiw, Eran Guendelman, Geoffrey Irving, Neil Molino, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license
// contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class TETRAHEDRON
//#####################################################################
#include "TETRAHEDRON.h"
#include "../Utilities/DEBUG_UTILITIES.h"
using namespace PhysBAM;
//#####################################################################
// Function Intersection
//#####################################################################
template<class T> bool TETRAHEDRON<T>::
Intersection (RAY_3D<T>& ray, const T thickness) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Normal
//#####################################################################
template<class T> VECTOR_3D<T> TETRAHEDRON<T>::
Normal (const VECTOR_3D<T>& location, const int aggregate) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Inside
//#####################################################################
template<class T> bool TETRAHEDRON<T>::
Inside (const VECTOR_3D<T>& location, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Outside
//#####################################################################
template<class T> bool TETRAHEDRON<T>::
Outside (const VECTOR_3D<T>& location, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Boundary
//#####################################################################
template<class T> bool TETRAHEDRON<T>::
Boundary (const VECTOR_3D<T>& location, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Surface
//#####################################################################
template<class T> VECTOR_3D<T> TETRAHEDRON<T>::
Surface (const VECTOR_3D<T>& location) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Volume
//#####################################################################
template<class T> T TETRAHEDRON<T>::
Volume() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Signed_Volume
//#####################################################################
template<class T> T TETRAHEDRON<T>::
Signed_Volume() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Minimum_Angle
//#####################################################################
template<class T> T TETRAHEDRON<T>::
Minimum_Angle() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Maximum_Angle
//#####################################################################
template<class T> T TETRAHEDRON<T>::
Maximum_Angle() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Minimum_Edge_Length
//#####################################################################
template<class T> T TETRAHEDRON<T>::
Minimum_Edge_Length() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Maximum_Edge_Length
//#####################################################################
template<class T> T TETRAHEDRON<T>::
Maximum_Edge_Length() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Minimum_Altitude
//#####################################################################
template<class T> T TETRAHEDRON<T>::
Minimum_Altitude() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Aspect_Ratio
//#####################################################################
template<class T> T TETRAHEDRON<T>::
Aspect_Ratio() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Minimum_Dihedral_Angle
//#####################################################################
template<class T> T TETRAHEDRON<T>::
Minimum_Dihedral_Angle() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Maximum_Dihedral_Angle
//#####################################################################
template<class T> T TETRAHEDRON<T>::
Maximum_Dihedral_Angle() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Signed_Reciprocal_Aspect_Ratio
//#####################################################################
template<class T> T TETRAHEDRON<T>::
Signed_Reciprocal_Aspect_Ratio (const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2, const VECTOR_3D<T>& x3, const VECTOR_3D<T>& x4)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
template class TETRAHEDRON<float>;
template class TETRAHEDRON<double>;
