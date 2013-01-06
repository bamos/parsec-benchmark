//#####################################################################
// Copyright 2002-2004, Robert Bridson, Ronald Fedkiw, Eran Guendelman, Geoffrey Irving, Neil Molino, Andrew Selle, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class TRIANGLE_3D
//#####################################################################
#include "TRIANGLE_3D.h"
#include "../Utilities/DEBUG_UTILITIES.h"
using namespace PhysBAM;
//#####################################################################
// Function Change_Size
//#####################################################################
// Enlarges the triangle by pushing out the triangle edges by distance 'delta' orthogonally to the edges.
// This keeps the incenter fixed.
template<class T> void TRIANGLE_3D<T>::
Change_Size (const T delta)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Intersection
//#####################################################################
template<class T> bool TRIANGLE_3D<T>::
Intersection (RAY_3D<T>& ray, const T thickness_over_2) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Lazy_Intersection
//#####################################################################
template<class T> bool TRIANGLE_3D<T>::
Lazy_Intersection (RAY_3D<T>& ray) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Closest_Non_Intersecting_Point
//#####################################################################
template<class T> bool TRIANGLE_3D<T>::
Closest_Non_Intersecting_Point (RAY_3D<T>& ray, const T thickness_over_2) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Point_Inside_Triangle
//#####################################################################
template<class T> bool TRIANGLE_3D<T>::
Point_Inside_Triangle (const VECTOR_3D<T>& point, const T thickness_over_2) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Planar_Point_Inside_Triangle
//#####################################################################
template<class T> bool TRIANGLE_3D<T>::
Planar_Point_Inside_Triangle (const VECTOR_3D<T>& point, const T thickness_over_2) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Lazy_Planar_Point_Inside_Triangle
//#####################################################################
template<class T> bool TRIANGLE_3D<T>::
Lazy_Planar_Point_Inside_Triangle (const VECTOR_3D<T>& point) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Minimum_Edge_Length
//#####################################################################
template<class T> T TRIANGLE_3D<T>::
Minimum_Edge_Length() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Maximum_Edge_Length
//#####################################################################
template<class T> T TRIANGLE_3D<T>::
Maximum_Edge_Length() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Bounding_Box
//#####################################################################
template<class T> BOX_3D<T> TRIANGLE_3D<T>::
Bounding_Box() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Region
//#####################################################################
// returns the region one is near in priority of 1=vertex, 2=edge, 3=face based on distance, region_id differentiates which point or edge
template<class T> int TRIANGLE_3D<T>::
Region (const VECTOR_3D<T>& location, int& region_id, const T distance) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Closest_Point
//#####################################################################
template<class T> VECTOR_3D<T> TRIANGLE_3D<T>::
Closest_Point (const VECTOR_3D<T>& location, VECTOR_3D<T>& weights) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Distance_To_Triangle
//#####################################################################
template<class T> T TRIANGLE_3D<T>::
Distance_To_Triangle (const VECTOR_3D<T>& location) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Minimum_Angle
//#####################################################################
template<class T> T TRIANGLE_3D<T>::
Minimum_Angle() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Maximum_Angle
//#####################################################################
template<class T> T TRIANGLE_3D<T>::
Maximum_Angle() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Signed_Solid_Angle
//#####################################################################
// positive for normals that point away from the center - not reliable if center is too close to the triangle face
template<class T> T TRIANGLE_3D<T>::
Signed_Solid_Angle (const VECTOR_3D<T>& center) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Segment_Triangle_Intersection
//#####################################################################
template<class T> bool TRIANGLE_3D<T>::
Segment_Triangle_Intersection (const SEGMENT_3D<T>& segment, const T thickness_over_2) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Segment_Triangle_Intersection
//#####################################################################
template<class T> bool TRIANGLE_3D<T>::
Segment_Triangle_Intersection (const SEGMENT_3D<T>& segment, T& a, VECTOR_3D<T>& weights, const T thickness_over_2) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Point_Triangle_Interaction
//#####################################################################
// outputs unsigned distance
template<class T> bool TRIANGLE_3D<T>::
Point_Triangle_Interaction (const VECTOR_3D<T>& x, const T interaction_distance, T& distance) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Point_Triangle_Interaction_Data
//#####################################################################
// assumes an unsigned distance is sent into the function
template<class T> void TRIANGLE_3D<T>::
Point_Triangle_Interaction_Data (const VECTOR_3D<T>& x, T& distance, VECTOR_3D<T>& interaction_normal, VECTOR_3D<T>& weights) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Point_Triangle_Interaction
//#####################################################################
template<class T> bool TRIANGLE_3D<T>::
Point_Triangle_Interaction (const VECTOR_3D<T>& x, const VECTOR_3D<T>& v, const VECTOR_3D<T>& v1, const VECTOR_3D<T>& v2, const VECTOR_3D<T>& v3, const T interaction_distance, T& distance,
			    VECTOR_3D<T>& interaction_normal, VECTOR_3D<T>& weights, T& relative_speed, const bool exit_early) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Point_Triangle_Interaction_One_Sided
//#####################################################################
template<class T> bool TRIANGLE_3D<T>::
Point_Triangle_Interaction_One_Sided (const VECTOR_3D<T>& x, const VECTOR_3D<T>& v, const VECTOR_3D<T>& v1, const VECTOR_3D<T>& v2, const VECTOR_3D<T>& v3, const T interaction_distance,
				      T& distance, VECTOR_3D<T>& interaction_normal, VECTOR_3D<T>& weights, T& relative_speed, const bool exit_early) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Robust_Point_Triangle_Collision
//#####################################################################
template<class T> typename TRIANGLE_3D<T>::POINT_TRIANGLE_COLLISION_TYPE TRIANGLE_3D<T>::
Robust_Point_Triangle_Collision (const TRIANGLE_3D<T>& initial_triangle, const TRIANGLE_3D<T>& final_triangle, const VECTOR_3D<T>& x, const VECTOR_3D<T>& final_x, const T dt, const T collision_thickness,
				 T& collision_time, VECTOR_3D<T>& normal, VECTOR_3D<T>& weights, T& relative_speed)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Point_Triangle_Collision
//#####################################################################
template<class T> bool TRIANGLE_3D<T>::
Point_Triangle_Collision (const VECTOR_3D<T>& x, const VECTOR_3D<T>& v, const VECTOR_3D<T>& v1, const VECTOR_3D<T>& v2, const VECTOR_3D<T>& v3, const T dt, const T collision_thickness,
			  T& collision_time, VECTOR_3D<T>& normal, VECTOR_3D<T>& weights, T& relative_speed, const bool exit_early) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################

template class TRIANGLE_3D<float>;
template class TRIANGLE_3D<double>;
