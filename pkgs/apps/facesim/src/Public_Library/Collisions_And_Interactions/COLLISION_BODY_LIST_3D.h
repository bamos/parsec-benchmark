//#####################################################################
// Copyright 2004, Ron Fedkiw, Frank Losasso, Andrew Selle
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class COLLISION_BODY_LIST_3D
//#####################################################################
#ifndef __COLLISION_BODY_LIST_3D__
#define __COLLISION_BODY_LIST_3D__

#include "../Arrays/ARRAY.h"
#include "COLLISION_BODY_3D.h"
#include "../Rigid_Bodies/RIGID_BODY_LIST_3D.h"
#include "../Deformable_Objects/DEFORMABLE_OBJECT_LIST_3D.h"
namespace PhysBAM
{

template<class T>
class COLLISION_BODY_LIST_3D
{
public:
	ARRAY<COLLISION_BODY_3D<T>*> collision_bodies;

	COLLISION_BODY_LIST_3D()
	{}

	void Add_Body (COLLISION_BODY_3D<T>* collision_body)
	{
		collision_bodies.Append_Element (collision_body);
	}

	void Add_Bodies (RIGID_BODY_LIST_3D<T>& rigid_body_list)
	{
		for (int i = 1; i <= rigid_body_list.rigid_bodies.m; i++) collision_bodies.Append_Element (rigid_body_list.rigid_bodies (i));
	}

	void Add_Bodies (DEFORMABLE_OBJECT_LIST_3D<T>& deformable_object_list)
	{
		for (int i = 1; i <= deformable_object_list.deformable_objects.m; i++) collision_bodies.Append_Element (&deformable_object_list.deformable_objects (i)->collisions);
	}

	COLLISION_BODY_3D<T>& operator() (const int i) const
	{
		return *collision_bodies (i);
	}

	void Save_State (const int state_index, const T time = 0)
	{
		for (int i = 1; i <= collision_bodies.m; i++) collision_bodies (i)->Save_State (state_index, time);
	}

	void Restore_State (const int state_index)
	{
		for (int i = 1; i <= collision_bodies.m; i++) collision_bodies (i)->Restore_State (state_index);
	}

	void Delete_State (const int state_index)
	{
		for (int i = 1; i <= collision_bodies.m; i++) collision_bodies (i)->Delete_State (state_index);
	}

	template<class RW> void Read_State (const int state_index, const std::string& prefix, const std::string& suffix)
	{
		for (int i = 1; i <= collision_bodies.m; i++)
		{
			std::istream* input_stream = FILE_UTILITIES::Safe_Open_Input (STRING_UTILITIES::string_sprintf ("%s_%d%s", prefix.c_str(), i, suffix.c_str()));

			if (collision_bodies (i)->body_type == COLLISION_BODY_3D<T>::RIGID_BODY_TYPE) // we're forced to do special cases because templated Read_State can't be virtual
				( (RIGID_BODY_3D<T>*) collision_bodies (i))->template Read_State<RW> (*input_stream, state_index);
			else if (collision_bodies (i)->body_type == COLLISION_BODY_3D<T>::DEFORMABLE_BODY_TYPE)
				( (DEFORMABLE_OBJECT_COLLISIONS_3D<T>*) collision_bodies (i))->template Read_State<RW> (*input_stream, state_index);

			delete input_stream;
		}
	}

	template<class RW> void Write_State (const int state_index, const std::string& prefix, const std::string& suffix) const
	{
		for (int i = 1; i <= collision_bodies.m; i++)
		{
			std::ostream* output_stream = FILE_UTILITIES::Safe_Open_Output (STRING_UTILITIES::string_sprintf ("%s_%d%s", prefix.c_str(), i, suffix.c_str()));

			if (collision_bodies (i)->body_type == COLLISION_BODY_3D<T>::RIGID_BODY_TYPE) // we're forced to do special cases because templated Write_State can't be virtual
				( (RIGID_BODY_3D<T>*) collision_bodies (i))->template Write_State<RW> (*output_stream, state_index);
			else if (collision_bodies (i)->body_type == COLLISION_BODY_3D<T>::DEFORMABLE_BODY_TYPE)
				( (DEFORMABLE_OBJECT_COLLISIONS_3D<T>*) collision_bodies (i))->template Write_State<RW> (*output_stream, state_index);

			delete output_stream;
		}
	}

//#####################################################################
	bool Triangulated_Surface_Intersection_With_Any_Body (RAY_3D<T>& ray, int& body_id) const;
	bool Triangulated_Surface_Closest_Non_Intersecting_Point_Of_Any_Body (RAY_3D<T>& ray, int& body_id) const;
	bool Triangulated_Surface_Inside_Any_Triangle_Of_Any_Body (const VECTOR_3D<T>& location, int& body_id, int& triangle_id) const;
	bool Earliest_Triangle_Crossover (const VECTOR_3D<T>& start_X, const VECTOR_3D<T>& end_X, const T dt, T& hit_time, VECTOR_3D<T>& normal, VECTOR_3D<T>& weights, int& body_id,
					  int& triangle_id) const;
	bool Latest_Triangle_Crossover (const VECTOR_3D<T>& start_X, const VECTOR_3D<T>& end_X, const T dt, T& hit_time, VECTOR_3D<T>& normal, VECTOR_3D<T>& weights, int& body_id, int& triangle_id,
					typename TRIANGLE_3D<T>::POINT_TRIANGLE_COLLISION_TYPE& returned_collision_type) const;
	bool Any_Triangle_Crossover (const VECTOR_3D<T>& start_X, const VECTOR_3D<T>& end_X, const T dt, int& triangle_id) const;
	VECTOR_3D<T> Closest_Surface_Point (const VECTOR_3D<T>& location, const T max_distance, T& distance, int& body_id, int& triangle_id) const;
	bool Intersection_Between_Points (const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2, int& body_id, int& triangle_id, VECTOR_3D<T>& intersection_point) const;
	void Compute_Occupied_Cells (const GRID_3D<T>& grid, ARRAYS_3D<bool>& occupied, const bool with_body_motion, const T extra_thickness, const T body_thickness_factor) const;
	void Compute_Occupied_Cells (const OCTREE_GRID<T>& grid, ARRAY<bool>& occupied, bool with_body_motion, T extra_thickness, T body_thickness_factor) const;
	void Update_Intersection_Acceleration_Structures (const bool use_swept_triangle_hierarchy, const int state1 = 0, const int state2 = 0);
	bool Get_Body_Penetration (const VECTOR_3D<T>& start_X, const VECTOR_3D<T>& end_X, const T contour_value, const T dt, int& body_id, int& triangle_id,
				   T& start_phi, T& end_phi, VECTOR_3D<T>& end_body_normal, VECTOR_3D<T>& body_velocity) const;
	bool Push_Out_Point (VECTOR_3D<T>& X, const T collision_distance, const bool check_particle_crossover, bool& particle_crossover) const;
//#####################################################################
};
}
#endif
