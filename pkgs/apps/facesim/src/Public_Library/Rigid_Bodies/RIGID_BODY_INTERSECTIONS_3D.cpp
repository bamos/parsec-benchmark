//#####################################################################
// Copyright 2002, 2003, Ronald Fedkiw, Eran Guendelman.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "RIGID_BODY_INTERSECTIONS_3D.h"
#include "../Geometry/LEVELSET_IMPLICIT_SURFACE.h"
#include "../Utilities/DEBUG_UTILITIES.h"
using namespace PhysBAM;
//#####################################################################
// Function Intersection_Check
//#####################################################################
template<class T> bool RIGID_BODY_INTERSECTIONS_3D<T>::
Intersection_Check (const int index_1, const int index_2, int& particle_body, int& levelset_body)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Bounding_Volumes_Intersect
//#####################################################################
template<class T> bool RIGID_BODY_INTERSECTIONS_3D<T>::
Bounding_Volumes_Intersect (const int index_1, const int index_2) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Find_Any_Intersection
//#####################################################################
template<class T> bool RIGID_BODY_INTERSECTIONS_3D<T>::
Find_Any_Intersection (const int index_1, const int index_2, int& particle_body, int& levelset_body)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Append_All_Intersections
//#####################################################################
template<class T> void RIGID_BODY_INTERSECTIONS_3D<T>::
Append_All_Intersections (const int index_1, const int index_2, LIST_ARRAY<VECTOR_3D<T> >& particle_location, LIST_ARRAY<int>& particle_index, LIST_ARRAY<int>& particle_body, LIST_ARRAY<int>& levelset_body, const T contour_value)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Particles_In_Levelset
//#####################################################################
template<class T> void RIGID_BODY_INTERSECTIONS_3D<T>::
Particles_In_Levelset (const int particle_body_index, const int levelset_body_index, LIST_ARRAY<VECTOR_3D<T> >& particle_location, LIST_ARRAY<int>& particle_index, LIST_ARRAY<int>& particle_body,
		       LIST_ARRAY<int>& levelset_body, const T contour_value, const bool exit_early)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Get_Interfering_Triangles
//#####################################################################
template<class T> void RIGID_BODY_INTERSECTIONS_3D<T>::
Get_Interfering_Triangles (const RIGID_BODY_3D<T>& body1, const RIGID_BODY_3D<T>& body2, LIST_ARRAY<int>& triangle_list)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Intersections_Using_Hierarchy
//#####################################################################
template<class T> void RIGID_BODY_INTERSECTIONS_3D<T>::
Intersections_Using_Hierarchy (const int particle_body_index, const int levelset_body_index, LIST_ARRAY<int>& triangle_list, LIST_ARRAY<VECTOR_3D<T> >& particle_location, LIST_ARRAY<int>& particle_index, LIST_ARRAY<int>& particle_body,
			       LIST_ARRAY<int>& levelset_body, const T contour_value, const bool exit_early)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Intersections_Using_Hierarchy_And_Edges
//#####################################################################
// body2 doesn't require a triangulated surface, but if it has one it will be used for edge-face intersections
template<class T> void RIGID_BODY_INTERSECTIONS_3D<T>::
Intersections_Using_Hierarchy_And_Edges (const int particle_body_index, const int levelset_body_index, LIST_ARRAY<int>& triangle_list1, LIST_ARRAY<int>& triangle_list2, LIST_ARRAY<VECTOR_3D<T> >& particle_location,
		LIST_ARRAY<int>& particle_index, LIST_ARRAY<int>& particle_body, LIST_ARRAY<int>& levelset_body, const T contour_value, const bool exit_early)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
template class RIGID_BODY_INTERSECTIONS_3D<double>;
template class RIGID_BODY_INTERSECTIONS_3D<float>;
