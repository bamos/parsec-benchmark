//#####################################################################
// Copyright 2004-2005, Zhaosheng Bao, Ron Fedkiw, Geoffrey Irving, Frank Losasso.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class DEFORMABLE_OBJECT_COLLISIONS_3D
//#####################################################################
#include "DEFORMABLE_OBJECT_COLLISIONS_3D.h"
#include "../Collisions_And_Interactions/COLLISION_BODY_LIST_3D.h"

using namespace PhysBAM;
template<class T> DEFORMABLE_OBJECT_COLLISIONS_3D<T>* DEFORMABLE_OBJECT_COLLISIONS_3D<T>::m_pointer = NULL;
template<class T> T  DEFORMABLE_OBJECT_COLLISIONS_3D<T>::m_dt = (T) 0;

//#####################################################################
// Function Adjust_Nodes_For_Collision_Body_Collisions_Helper_I
//#####################################################################

template<class T> void DEFORMABLE_OBJECT_COLLISIONS_3D<T>::
Adjust_Nodes_For_Collision_Body_Collisions_Helper_I (long thread_id, void* id)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Adjust_Nodes_For_Collision_Body_Collisions
//#####################################################################

template<class T> int DEFORMABLE_OBJECT_COLLISIONS_3D<T>::
Adjust_Nodes_For_Collision_Body_Collisions (const T dt)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Adjust_Nodes_For_Collision_Body_Collisions
//#####################################################################
template<class T> int DEFORMABLE_OBJECT_COLLISIONS_3D<T>::
Adjust_Nodes_For_Collision_Body_Collisions (EMBEDDED_TRIANGULATED_SURFACE<T>& embedded_triangulated_surface, TRIANGLES_OF_MATERIAL_3D<T>& triangles_of_material, const T dt)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Adjust_Nodes_For_Collision_Body_Collisions
//#####################################################################
template<class T> int DEFORMABLE_OBJECT_COLLISIONS_3D<T>::
Adjust_Nodes_For_Collision_Body_Collisions (EMBEDDED_TETRAHEDRALIZED_VOLUME<T>& embedded_tetrahedralized_volume,
		EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>& embedded_tetrahedralized_volume_boundary_surface, const T dt)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Adjust_Mesh_For_Embedded_Self_Collision
//#####################################################################
template<class T> int DEFORMABLE_OBJECT_COLLISIONS_3D<T>::
Adjust_Mesh_For_Embedded_Self_Collision (EMBEDDED_TRIANGULATED_SURFACE<T>& embedded_triangulated_surface, TRIANGLES_OF_MATERIAL_3D<T>& triangles_of_material)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Adjust_Mesh_For_Embedded_Self_Collision
//#####################################################################
template<class T> int DEFORMABLE_OBJECT_COLLISIONS_3D<T>::
Adjust_Mesh_For_Embedded_Self_Collision (EMBEDDED_TETRAHEDRALIZED_VOLUME<T>& embedded_tetrahedralized_volume,
		EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>& embedded_tetrahedralized_volume_boundary_surface)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Earliest_Triangle_Crossover
//#####################################################################
template<class T> bool DEFORMABLE_OBJECT_COLLISIONS_3D<T>::
Earliest_Triangle_Crossover (const VECTOR_3D<T>& start_X, const VECTOR_3D<T>& end_X, const T dt, T& hit_time, VECTOR_3D<T>& normal, VECTOR_3D<T>& weights, int& triangle_id) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Latest_Triangle_Crossover
//#####################################################################
template<class T> bool DEFORMABLE_OBJECT_COLLISIONS_3D<T>::
Latest_Triangle_Crossover (const VECTOR_3D<T>& start_X, const VECTOR_3D<T>& end_X, const T dt, T& hit_time, VECTOR_3D<T>& normal, VECTOR_3D<T>& weights, int& triangle_id,
			   typename TRIANGLE_3D<T>::POINT_TRIANGLE_COLLISION_TYPE& returned_collision_type) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Any_Triangle_Crossover
//#####################################################################
template<class T> bool DEFORMABLE_OBJECT_COLLISIONS_3D<T>::
Any_Triangle_Crossover (const VECTOR_3D<T>& start_X, const VECTOR_3D<T>& end_X, const T dt) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Compute_Occupied_Cells
//#####################################################################
template<class T> void DEFORMABLE_OBJECT_COLLISIONS_3D<T>::
Compute_Occupied_Cells (const GRID_3D<T>& grid, ARRAYS_3D<bool>& occupied, const bool with_body_motion, const T extra_thickness, const T body_thickness_factor,
			const bool reset_occupied_to_false) const
{
	NOT_IMPLEMENTED();
}
//#####################################################################
// Function Get_Triangle_Bounding_Boxes
//#####################################################################
template<class T> void DEFORMABLE_OBJECT_COLLISIONS_3D<T>::
Get_Triangle_Bounding_Boxes (LIST_ARRAY<BOX_3D<T> >& bounding_boxes, const bool with_body_motion, const T extra_thickness, const T body_thickness_factor) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Update_Intersection_Acceleration_Structures
//#####################################################################
template<class T> void DEFORMABLE_OBJECT_COLLISIONS_3D<T>::
Update_Intersection_Acceleration_Structures (const bool use_swept_triangle_hierarchy, const int state1, const int state2)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
template class DEFORMABLE_OBJECT_COLLISIONS_3D<float>;
template class DEFORMABLE_OBJECT_COLLISIONS_3D<double>;
