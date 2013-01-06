//#####################################################################
// Copyright 2002-2004, Robert Bridson, Ronald Fedkiw, Sergey Koltakov, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SEGMENT_3D
//#####################################################################
#include "SEGMENT_3D.h"
#include "../Utilities/DEBUG_UTILITIES.h"
using namespace PhysBAM;
//#####################################################################
// Function Closest_Point_On_Segment
//#####################################################################
template<class T> VECTOR_3D<T> SEGMENT_3D<T>::
Closest_Point_On_Segment (const VECTOR_3D<T>& point) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Distance_From_Point_To_Segment
//#####################################################################
template<class T> T SEGMENT_3D<T>::
Distance_From_Point_To_Segment (const VECTOR_3D<T>& point) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Closest_Point_On_Line
//#####################################################################
template<class T> VECTOR_3D<T> SEGMENT_3D<T>::
Closest_Point_On_Line (const VECTOR_3D<T>& point) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Distance_From_Point_To_Line
//#####################################################################
template<class T> T SEGMENT_3D<T>::
Distance_From_Point_To_Line (const VECTOR_3D<T>& point) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Shortest_Vector_Between_Segments
//#####################################################################
// vector points from input segment to this segment; not accurate as the segments become parallel
template<class T> VECTOR_3D<T> SEGMENT_3D<T>::
Shortest_Vector_Between_Segments (const SEGMENT_3D<T>& segment, VECTOR_2D<T>& weights) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Segment_Segment_Interaction
//#####################################################################
// returns the distance, normal and weights
template<class T> bool SEGMENT_3D<T>::
Segment_Segment_Interaction (const SEGMENT_3D<T>& segment, const T interaction_distance, T& distance, VECTOR_3D<T>& normal, VECTOR_2D<T>& weights) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Segment_Segment_Interaction_Data
//#####################################################################
// input the distance, normal and weights
template<class T> void SEGMENT_3D<T>::
Segment_Segment_Interaction_Data (const SEGMENT_3D<T>& segment, const VECTOR_3D<T>& v1, const VECTOR_3D<T>& v2, const VECTOR_3D<T>& v3, const VECTOR_3D<T>& v4, T& distance,
				  VECTOR_3D<T>& normal, const VECTOR_2D<T>& weights, const T small_number, const bool verbose) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Segment_Segment_Interaction
//#####################################################################
template<class T> bool SEGMENT_3D<T>::
Segment_Segment_Interaction (const SEGMENT_3D<T>& segment, const VECTOR_3D<T>& v1, const VECTOR_3D<T>& v2, const VECTOR_3D<T>& v3, const VECTOR_3D<T>& v4, const T interaction_distance,
			     T& distance, VECTOR_3D<T>& normal, VECTOR_2D<T>& weights, T& relative_speed, const T small_number, const bool exit_early, const bool verbose) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Segment_Segment_Collision
//#####################################################################
template<class T> bool SEGMENT_3D<T>::
Segment_Segment_Collision (const SEGMENT_3D<T>& segment, const VECTOR_3D<T>& v1, const VECTOR_3D<T>& v2, const VECTOR_3D<T>& v3, const VECTOR_3D<T>& v4, const T dt,
			   const T collision_thickness, T& collision_time, VECTOR_3D<T>& normal, VECTOR_2D<T>& weights, T& relative_speed, const T small_number, const bool exit_early) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Barycentric_Coordinates
//#####################################################################
template<class T> VECTOR_2D<T> SEGMENT_3D<T>::
Barycentric_Coordinates (const VECTOR_3D<T>& location, const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
template class SEGMENT_3D<float>;
template class SEGMENT_3D<double>;
