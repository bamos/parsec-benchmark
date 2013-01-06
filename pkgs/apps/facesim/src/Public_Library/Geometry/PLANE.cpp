//#####################################################################
// Copyright 2002, 2003, Robert Bridson, Ronald Fedkiw, Eran Guendelman.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class PLANE
//#####################################################################
#include "PLANE.h"
#include "../Utilities/DEBUG_UTILITIES.h"
using namespace PhysBAM;
//#####################################################################
// Function Intersection
//#####################################################################
// A version to make BOX_3D::Intersection have to do fewer redundant dot products
template<class T> bool PLANE<T>::
Intersection (RAY_3D<T>& ray, const T thickness_over_2, const T distance, const T rate_of_approach) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Intersection
//#####################################################################
template<class T> bool PLANE<T>::
Intersection (RAY_3D<T>& ray, const T thickness_over_2) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Intersection
//#####################################################################
template<class T> bool PLANE<T>::
Intersection (const BOX_3D<T>& box, const T thickness_over_2) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Lazy_Intersection
//#####################################################################
template<class T> bool PLANE<T>::
Lazy_Intersection (RAY_3D<T>& ray) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Segment_Plane_Intersection
//#####################################################################
template<class T> bool PLANE<T>::
Segment_Plane_Intersection (const VECTOR_3D<T>& endpoint1, const VECTOR_3D<T>& endpoint2, T& interpolation_fraction) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Rectangle_Intersection
//#####################################################################
template<class T> bool PLANE<T>::
Rectangle_Intersection (RAY_3D<T>& ray, const PLANE<T>& bounding_plane_1, const PLANE<T>& bounding_plane_2, const PLANE<T>& bounding_plane_3, const PLANE<T>& bounding_plane_4,
			const T thickness_over_2) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Inside
//#####################################################################
// inside is the half space behind the normal
template<class T> bool PLANE<T>::
Inside (const VECTOR_3D<T>& location, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Lazy_Inside
//#####################################################################
// inside is the half space behind the normal
template<class T> bool PLANE<T>::
Lazy_Inside (const VECTOR_3D<T>& location) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Outside
//#####################################################################
template<class T> bool PLANE<T>::
Outside (const VECTOR_3D<T>& location, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Lazy_Outside
//#####################################################################
template<class T> bool PLANE<T>::
Lazy_Outside (const VECTOR_3D<T>& location) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Boundary
//#####################################################################
template<class T> bool PLANE<T>::
Boundary (const VECTOR_3D<T>& location, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Surface
//#####################################################################
// closest point on the surface
template<class T> VECTOR_3D<T> PLANE<T>::
Surface (const VECTOR_3D<T>& location) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################

template class PLANE<float>;
template class PLANE<double>;
