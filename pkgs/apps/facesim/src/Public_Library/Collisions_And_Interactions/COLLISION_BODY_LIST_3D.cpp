//#####################################################################
// Copyright 2004, Ron Fedkiw, Frank Losasso, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "COLLISION_BODY_LIST_3D.h"
using namespace PhysBAM;
//#####################################################################
// Function Triangulated_Surface_Intersection_With_Any_Body
//#####################################################################
template<class T> bool COLLISION_BODY_LIST_3D<T>::
Triangulated_Surface_Intersection_With_Any_Body (RAY_3D<T>& ray, int& body_id) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Triangulated_Surface_Closest_Non_Intersecting_Point_Of_Any_Body
//#####################################################################
template<class T> bool COLLISION_BODY_LIST_3D<T>::
Triangulated_Surface_Closest_Non_Intersecting_Point_Of_Any_Body (RAY_3D<T>& ray, int& body_id) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Triangulated_Surface_Inside_Any_Triangle_Of_Any_Body
//#####################################################################
template<class T> bool COLLISION_BODY_LIST_3D<T>::
Triangulated_Surface_Inside_Any_Triangle_Of_Any_Body (const VECTOR_3D<T>& location, int& body_id, int& triangle_id) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Earliest_Triangle_Crossover
//#####################################################################
template<class T> bool COLLISION_BODY_LIST_3D<T>::
Earliest_Triangle_Crossover (const VECTOR_3D<T>& start_X, const VECTOR_3D<T>& end_X, const T dt, T& hit_time, VECTOR_3D<T>& normal, VECTOR_3D<T>& weights, int& body_id, int& triangle_id) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Latest_Triangle_Crossover
//#####################################################################
template<class T> bool COLLISION_BODY_LIST_3D<T>::
Latest_Triangle_Crossover (const VECTOR_3D<T>& start_X, const VECTOR_3D<T>& end_X, const T dt, T& hit_time, VECTOR_3D<T>& normal, VECTOR_3D<T>& weights, int& body_id, int& triangle_id,
			   typename TRIANGLE_3D<T>::POINT_TRIANGLE_COLLISION_TYPE& returned_collision_type) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Any_Triangle_Crossover
//#####################################################################
template<class T> bool COLLISION_BODY_LIST_3D<T>::
Any_Triangle_Crossover (const VECTOR_3D<T>& start_X, const VECTOR_3D<T>& end_X, const T dt, int& triangle_id) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Closest_Surface_Point
//#####################################################################
template<class T> VECTOR_3D<T>  COLLISION_BODY_LIST_3D<T>::
Closest_Surface_Point (const VECTOR_3D<T>& location, const T max_distance, T& distance, int& body_id, int& triangle_id) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Intersection_Between_Points
//#####################################################################
template<class T> bool COLLISION_BODY_LIST_3D<T>::
Intersection_Between_Points (const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2, int& body_id, int& triangle_id, VECTOR_3D<T>& intersection_point) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Compute_Occupied_Cells
//#####################################################################
template<class T> void COLLISION_BODY_LIST_3D<T>::
Compute_Occupied_Cells (const GRID_3D<T>& grid, ARRAYS_3D<bool>& occupied, const bool with_body_motion, const T extra_thickness, const T body_thickness_factor) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Compute_Occupied_Cells
//#####################################################################
template<class T> void COLLISION_BODY_LIST_3D<T>::
Compute_Occupied_Cells (const OCTREE_GRID<T>& grid, ARRAY<bool>& occupied, bool with_body_motion, T extra_thickness, T body_thickness_factor) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Update_Intersection_Acceleration_Structures
//#####################################################################
template<class T> void COLLISION_BODY_LIST_3D<T>::
Update_Intersection_Acceleration_Structures (const bool use_swept_triangle_hierarchy, const int state1, const int state2)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Get_Body_Penetration
//#####################################################################
// normal is flipped to ensure that start_phi is positive
template<class T> bool COLLISION_BODY_LIST_3D<T>::
Get_Body_Penetration (const VECTOR_3D<T>& start_X, const VECTOR_3D<T>& end_X, const T contour_value, const T dt, int& body_id, int& triangle_id,
		      T& start_phi, T& end_phi, VECTOR_3D<T>& end_body_normal, VECTOR_3D<T>& body_velocity) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Push_Out_Point
//#####################################################################
template<class T> bool COLLISION_BODY_LIST_3D<T>::
Push_Out_Point (VECTOR_3D<T>& X, const T collision_distance, const bool check_particle_crossover, bool& particle_crossover) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################


template class COLLISION_BODY_LIST_3D<float>;
template class COLLISION_BODY_LIST_3D<double>;
