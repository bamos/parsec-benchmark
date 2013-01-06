//#####################################################################
// Copyright 2004, Ron Fedkiw, Geoffrey Irving, Frank Losasso.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class DEFORMABLE_OBJECT_COLLISIONS_3D
//#####################################################################
#ifndef __DEFORMABLE_OBJECT_COLLISIONS_3D__
#define __DEFORMABLE_OBJECT_COLLISIONS_3D__

#include "DEFORMABLE_OBJECT_COLLISIONS.h"
#include "../Geometry/SEGMENTED_CURVE_3D.h"
#include "../Geometry/TRIANGULATED_SURFACE.h"
#include "../Collisions_And_Interactions/COLLISION_BODY_3D.h"
namespace PhysBAM
{

template<class T> class COLLISION_BODY_LIST_3D;
template<class T> class EMBEDDED_TRIANGULATED_SURFACE;
template<class T> class EMBEDDED_TETRAHEDRALIZED_VOLUME;
template<class T> class TRIANGLES_OF_MATERIAL_3D;
template<class T> class EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE;

template<class T>
class DEFORMABLE_OBJECT_COLLISIONS_3D: public DEFORMABLE_OBJECT_COLLISIONS<T, VECTOR_3D<T> >, public COLLISION_BODY_3D<T>
{
public:
	using COLLISION_BODY_3D<T>::body_type;
	using DEFORMABLE_OBJECT_COLLISIONS<T, VECTOR_3D<T> >::collision_particles;
	using DEFORMABLE_OBJECT_COLLISIONS<T, VECTOR_3D<T> >::saved_states;
	using DEFORMABLE_OBJECT_COLLISIONS<T, VECTOR_3D<T> >::check_collision;
	using DEFORMABLE_OBJECT_COLLISIONS<T, VECTOR_3D<T> >::enforce_collision;
	using DEFORMABLE_OBJECT_COLLISIONS<T, VECTOR_3D<T> >::enforce_tangential_collision_velocity;
	using DEFORMABLE_OBJECT_COLLISIONS<T, VECTOR_3D<T> >::total_collision_velocity;
	using DEFORMABLE_OBJECT_COLLISIONS<T, VECTOR_3D<T> >::collision_normal;
	using DEFORMABLE_OBJECT_COLLISIONS<T, VECTOR_3D<T> >::normal_collision_velocity;
	using DEFORMABLE_OBJECT_COLLISIONS<T, VECTOR_3D<T> >::collision_tolerance;
	using DEFORMABLE_OBJECT_COLLISIONS<T, VECTOR_3D<T> >::roughness;
	using DEFORMABLE_OBJECT_COLLISIONS<T, VECTOR_3D<T> >::X_self_collision_free;
	using DEFORMABLE_OBJECT_COLLISIONS<T, VECTOR_3D<T> >::V_self_collision_free;
	using DEFORMABLE_OBJECT_COLLISIONS<T, VECTOR_3D<T> >::skip_collision_body;
	using DEFORMABLE_OBJECT_COLLISIONS<T, VECTOR_3D<T> >::Initialize_Object_Collisions;
	using DEFORMABLE_OBJECT_COLLISIONS<T, VECTOR_3D<T> >::embedded_particles_for_thin_shells;
	using DEFORMABLE_OBJECT_COLLISIONS<T, VECTOR_3D<T> >::saved_embedded_particles_for_thin_shells;
	SEGMENTED_CURVE_3D<T>* segmented_curve;
	TRIANGULATED_SURFACE<T>* triangulated_surface;
	COLLISION_BODY_LIST_3D<T>* collision_body_list;
	int collision_body_list_id;
	static DEFORMABLE_OBJECT_COLLISIONS_3D<T> *m_pointer;
	static void Adjust_Nodes_For_Collision_Body_Collisions_Helper_I (long thread_id, void* id);
	static T m_dt;

	DEFORMABLE_OBJECT_COLLISIONS_3D()
		: segmented_curve (0), triangulated_surface (0), collision_body_list (0), collision_body_list_id (0)
	{
		body_type = COLLISION_BODY_3D<T>::DEFORMABLE_BODY_TYPE;
	}

	virtual ~DEFORMABLE_OBJECT_COLLISIONS_3D()
	{}

	void Initialize_Segmented_Curve (SEGMENTED_CURVE_3D<T>& segmented_curve_input)
	{
		segmented_curve = &segmented_curve_input;
	}

	void Initialize_Triangulated_Surface (TRIANGULATED_SURFACE<T>& triangulated_surface_input)
	{
		triangulated_surface = &triangulated_surface_input;
	}

	void Set_Collision_Body_List (COLLISION_BODY_LIST_3D<T>& collision_body_list_input)
	{
		collision_body_list = &collision_body_list_input;
		skip_collision_body.Resize_Array (collision_body_list->collision_bodies.m);
	}

	VECTOR_3D<T> Pointwise_Object_Velocity (const int triangle_index, const VECTOR_3D<T>& location) const
	{
		assert (triangulated_surface && triangulated_surface->particles.store_velocity);
		int i, j, k;
		triangulated_surface->triangle_mesh.triangles.Get (triangle_index, i, j, k);
		VECTOR_3D<T> weights = TRIANGLE_3D<T>::Clamped_Barycentric_Coordinates (location, triangulated_surface->particles.X (i), triangulated_surface->particles.X (j), triangulated_surface->particles.X (k));
		return weights.x * triangulated_surface->particles.V (i) + weights.y * triangulated_surface->particles.V (j) + weights.z * triangulated_surface->particles.V (k);
	}

	VECTOR_3D<T> Pointwise_Object_Pseudo_Velocity (const int triangle_index, const VECTOR_3D<T>& location, const int state1, const int state2) const
	{
		assert (triangulated_surface && triangulated_surface->particles.store_velocity);
		int i, j, k;
		triangulated_surface->triangle_mesh.triangles.Get (triangle_index, i, j, k);
		VECTOR_3D<T> weights = TRIANGLE_3D<T>::Clamped_Barycentric_Coordinates (location, triangulated_surface->particles.X (i), triangulated_surface->particles.X (j), triangulated_surface->particles.X (k)), dXi, dXj, dXk;

		if (embedded_particles_for_thin_shells)
		{
			dXi = saved_embedded_particles_for_thin_shells (state2)->X (i) - saved_embedded_particles_for_thin_shells (state1)->X (i);
			dXj = saved_embedded_particles_for_thin_shells (state2)->X (j) - saved_embedded_particles_for_thin_shells (state1)->X (j);
			dXk = saved_embedded_particles_for_thin_shells (state2)->X (k) - saved_embedded_particles_for_thin_shells (state1)->X (k);
		}
		else
		{
			dXi = saved_states (state2).x->X (i) - saved_states (state1).x->X (i);
			dXj = saved_states (state2).x->X (j) - saved_states (state1).x->X (j);
			dXk = saved_states (state2).x->X (k) - saved_states (state1).x->X (k);
		}

		return (weights.x * dXi + weights.y * dXj + weights.z * dXk) / (saved_states (state2).y - saved_states (state1).y);
	}

	bool Triangulated_Surface_Intersection (RAY_3D<T>& ray) const
	{
		assert (triangulated_surface);
		return triangulated_surface->Intersection (ray, roughness);
	}

	bool Triangulated_Surface_Closest_Non_Intersecting_Point (RAY_3D<T>& ray) const
	{
		assert (triangulated_surface);
		return triangulated_surface->Closest_Non_Intersecting_Point (ray, roughness);
	}

	bool Triangulated_Surface_Inside_Any_Triangle (const VECTOR_3D<T>& location, int& triangle_id) const
	{
		assert (triangulated_surface);
		return triangulated_surface->Inside_Any_Triangle (location, triangle_id, roughness);
	}

	VECTOR_3D<T> Triangulated_Surface_Surface (const VECTOR_3D<T>& location, const T max_distance, int* triangle_id = 0, T* distance = 0) const
	{
		assert (triangulated_surface);
		return triangulated_surface->Surface (location, max_distance, roughness, triangle_id, distance);
	}

	VECTOR_3D<T> Triangulated_Surface_World_Space_Point_From_Barycentric_Coordinates (const int triangle_id, const VECTOR_3D<T>& weights) const
	{
		assert (triangulated_surface);
		int i, j, k;
		triangulated_surface->triangle_mesh.triangles.Get (triangle_id, i, j, k);
		return TRIANGLE_3D<T>::Point_From_Barycentric_Coordinates (weights, triangulated_surface->particles.X (i), triangulated_surface->particles.X (j), triangulated_surface->particles.X (k));
	}

	TRIANGLE_3D<T> Triangulated_Surface_World_Space_Triangle (const int triangle_id, const bool use_saved_state = false) const
	{
		if (use_saved_state)
		{
			assert (saved_states (1).x);

			if (embedded_particles_for_thin_shells) return Triangulated_Surface_World_Space_Triangle (triangle_id, *saved_embedded_particles_for_thin_shells (1));
			else return Triangulated_Surface_World_Space_Triangle (triangle_id, *saved_states (1).x);
		}
		else
		{
			assert (triangulated_surface);
			int i, j, k;
			triangulated_surface->triangle_mesh.triangles.Get (triangle_id, i, j, k);
			return TRIANGLE_3D<T> (triangulated_surface->particles.X (i), triangulated_surface->particles.X (j), triangulated_surface->particles.X (k));
		}
	}

	TRIANGLE_3D<T> Triangulated_Surface_World_Space_Triangle (const int triangle_id, const SOLIDS_PARTICLE<T, VECTOR_3D<T> >& state) const
	{
		assert (triangulated_surface);
		int i, j, k;
		triangulated_surface->triangle_mesh.triangles.Get (triangle_id, i, j, k);
		return TRIANGLE_3D<T> (state.X (i), state.X (j), state.X (k));
	}

	void Save_Self_Collision_Free_State() // assumes mass does not change
	{
		if (segmented_curve)
		{
			int n = segmented_curve->particles.number;
			X_self_collision_free.Resize_Array (n, false, false);
			ARRAY<VECTOR_3D<T> >::copy_up_to (segmented_curve->particles.X.array, X_self_collision_free, n);
			V_self_collision_free.Resize_Array (n, false, false);
			ARRAY<VECTOR_3D<T> >::copy_up_to (segmented_curve->particles.V.array, V_self_collision_free, n);
		}
		else if (triangulated_surface)
		{
			int n = triangulated_surface->particles.number;
			X_self_collision_free.Resize_Array (n, false, false);
			ARRAY<VECTOR_3D<T> >::copy_up_to (triangulated_surface->particles.X.array, X_self_collision_free, n);
			V_self_collision_free.Resize_Array (n, false, false);
			ARRAY<VECTOR_3D<T> >::copy_up_to (triangulated_surface->particles.V.array, V_self_collision_free, n);
		}
	}

	void Restore_Self_Collision_Free_State()
	{
		if (segmented_curve)
		{
			int n = segmented_curve->particles.number;
			ARRAY<VECTOR_3D<T> >::copy_up_to (X_self_collision_free, segmented_curve->particles.X.array, n);
			ARRAY<VECTOR_3D<T> >::copy_up_to (V_self_collision_free, segmented_curve->particles.V.array, n);
		}
		else if (triangulated_surface)
		{
			int n = triangulated_surface->particles.number;
			ARRAY<VECTOR_3D<T> >::copy_up_to (X_self_collision_free, triangulated_surface->particles.X.array, n);
			ARRAY<VECTOR_3D<T> >::copy_up_to (V_self_collision_free, triangulated_surface->particles.V.array, n);
		}
	}

	void Save_State (const int state_index, const T time = 0)
	{
		DEFORMABLE_OBJECT_COLLISIONS<T, VECTOR_3D<T> >::Save_State (state_index, time);
	}

	void Restore_State (const int state_index)
	{
		DEFORMABLE_OBJECT_COLLISIONS<T, VECTOR_3D<T> >::Restore_State (state_index);
	}

	void Delete_State (const int state_index)
	{
		DEFORMABLE_OBJECT_COLLISIONS<T, VECTOR_3D<T> >::Delete_State (state_index);
	}

	template<class RW> void Read_State (std::istream& input_stream, const int state_index)
	{
		DEFORMABLE_OBJECT_COLLISIONS<T, VECTOR_3D<T> >::template Read_State<RW> (input_stream, state_index);
	}

	template<class RW> void Write_State (std::ostream& output_stream, const int state_index)
	{
		DEFORMABLE_OBJECT_COLLISIONS<T, VECTOR_3D<T> >::template Write_State<RW> (output_stream, state_index);
	}

	T Thin_Shell_Density() const
	{
		return triangulated_surface->density;
	}

//#####################################################################
	int Adjust_Nodes_For_Collision_Body_Collisions (const T dt);
	int Adjust_Nodes_For_Collision_Body_Collisions (EMBEDDED_TRIANGULATED_SURFACE<T>& embedded_triangulated_surface, TRIANGLES_OF_MATERIAL_3D<T>& triangles_of_material, const T dt);
	int Adjust_Nodes_For_Collision_Body_Collisions (EMBEDDED_TETRAHEDRALIZED_VOLUME<T>& embedded_tetrahedralized_volume,
			EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>& embedded_tetrahedralized_volume_boundary_surface, const T dt);
	int Adjust_Mesh_For_Embedded_Self_Collision (EMBEDDED_TRIANGULATED_SURFACE<T>& embedded_triangulated_surface, TRIANGLES_OF_MATERIAL_3D<T>& triangles_of_material);
	int Adjust_Mesh_For_Embedded_Self_Collision (EMBEDDED_TETRAHEDRALIZED_VOLUME<T>& embedded_tetrahedralized_volume,
			EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>& embedded_tetrahedralized_volume_boundary_surface);
	bool Earliest_Triangle_Crossover (const VECTOR_3D<T>& start_X, const VECTOR_3D<T>& end_X, const T dt, T& hit_time, VECTOR_3D<T>& normal, VECTOR_3D<T>& weights, int& triangle_id) const;
	bool Latest_Triangle_Crossover (const VECTOR_3D<T>& start_X, const VECTOR_3D<T>& end_X, const T dt, T& hit_time, VECTOR_3D<T>& normal, VECTOR_3D<T>& weights, int& triangle_id,
					typename TRIANGLE_3D<T>::POINT_TRIANGLE_COLLISION_TYPE& returned_collision_type) const;
	bool Any_Triangle_Crossover (const VECTOR_3D<T>& start_X, const VECTOR_3D<T>& end_X, const T dt) const;
	void Compute_Occupied_Cells (const GRID_3D<T>& grid, ARRAYS_3D<bool>& occupied, const bool with_body_motion, const T extra_thickness, const T body_thickness_factor,
				     const bool reset_occupied_to_false = true) const;
	void Get_Triangle_Bounding_Boxes (LIST_ARRAY<BOX_3D<T> >& bounding_boxes, const bool with_body_motion, const T extra_thickness, const T body_thickness_factor) const;
	void Update_Intersection_Acceleration_Structures (const bool use_swept_triangle_hierarchy, const int state1 = 0, const int state2 = 0);
//#####################################################################
};
}
#endif
