//#####################################################################
// Copyright 2003, 2004, Zhaosheng Bao, Ronald Fedkiw, Eran Guendelman, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SEGMENT_2D
//#####################################################################
#include "../Geometry/ORIENTED_BOX_2D.h"
#include "SEGMENT_2D.h"
#include "../Utilities/DEBUG_UTILITIES.h"
using namespace PhysBAM;
//#####################################################################
// Segment_Line_Intersection
//#####################################################################
template<class T> bool SEGMENT_2D<T>::
Segment_Line_Intersection (const VECTOR_2D<T>& point_on_line, const VECTOR_2D<T>& normal_of_line, T &interpolation_fraction) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Closest_Point_On_Segment
//#####################################################################
template<class T> VECTOR_2D<T> SEGMENT_2D<T>::
Closest_Point_On_Segment (const VECTOR_2D<T>& point) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Distance_From_Point_To_Segment
//#####################################################################
template<class T> T SEGMENT_2D<T>::
Distance_From_Point_To_Segment (const VECTOR_2D<T>& point) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Closest_Point_On_Line
//#####################################################################
template<class T> VECTOR_2D<T> SEGMENT_2D<T>::
Closest_Point_On_Line (const VECTOR_2D<T>& point) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Distance_From_Point_To_Line
//#####################################################################
template<class T> T SEGMENT_2D<T>::
Distance_From_Point_To_Line (const VECTOR_2D<T>& point) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Shortest_Vector_Between_Segments
//#####################################################################
template<class T> VECTOR_2D<T> SEGMENT_2D<T>::
Shortest_Vector_Between_Segments (const SEGMENT_2D<T>& segment, T& a, T& b) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Segment_Segment_Interaction
//#####################################################################
template<class T> int SEGMENT_2D<T>::
Segment_Segment_Interaction (const SEGMENT_2D<T>& segment, const VECTOR_2D<T>& v1, const VECTOR_2D<T>& v2, const VECTOR_2D<T>& v3, const VECTOR_2D<T>& v4, const T interaction_distance,
			     T& distance, VECTOR_2D<T>& normal, T& a, T& b, T& relative_speed, const T small_number) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Thickened_Box
//#####################################################################
template<class T> ORIENTED_BOX_2D<T> SEGMENT_2D<T>::
Thickened_Oriented_Box (const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Intersection
//#####################################################################
template<class T> bool SEGMENT_2D<T>::
Intersection (RAY_2D<T>& ray, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Intersection_X_Segment
//#####################################################################
template<class T> bool SEGMENT_2D<T>::
Intersection_X_Segment (RAY_2D<T>& ray, const T x1, const T x2, const T y, const T thickness_over_two)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Intersection
//#####################################################################
template<class T> bool SEGMENT_2D<T>::
Intersection_Y_Segment (RAY_2D<T>& ray, const T x, const T y1, const T y2, const T thickness_over_two)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Fuzzy_Intersection
//#####################################################################
template<class T> bool SEGMENT_2D<T>::
Fuzzy_Intersection (RAY_2D<T>& ray, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Inside
//#####################################################################
template<class T> bool SEGMENT_2D<T>::
Inside (const VECTOR_2D<T>& point, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Closest_Non_Intersecting_Point
//#####################################################################
template<class T> bool SEGMENT_2D<T>::
Closest_Non_Intersecting_Point (RAY_2D<T>& ray, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Robust_Point_Segment_Collision
//#####################################################################
template<class T> typename SEGMENT_2D<T>::POINT_SEGMENT_COLLISION_TYPE SEGMENT_2D<T>::
Robust_Point_Segment_Collision (const SEGMENT_2D<T>& initial_segment, const SEGMENT_2D<T>& final_segment, const VECTOR_2D<T> &x, const VECTOR_2D<T> &final_x, const T dt, const T collision_thickness, T& collision_time, T& collision_alpha)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Segment_Point_Collision
//#####################################################################
template<class T> bool SEGMENT_2D<T>::
Segment_Point_Collision (const VECTOR_2D<T>& x, const VECTOR_2D<T>& v, const VECTOR_2D<T>& v1, const VECTOR_2D<T>& v2, const T dt, const T collision_thickness, T& collision_time, T& collision_alpha) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
template class SEGMENT_2D<float>;
template class SEGMENT_2D<double>;
