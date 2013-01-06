//#####################################################################
// Copyright 2002-2004, Zhaosheng Bao, Robert Bridson, Ronald Fedkiw, Eran Guendelman, Geoffrey Irving, Sergey Koltakov, Frank Losasso, Neil Molino, Andrew Selle, Rachel Weinstein.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class RIGID_BODY_3D
//#####################################################################
#ifndef __RIGID_BODY_3D__
#define __RIGID_BODY_3D__

#include <string.h>
#include <stdlib.h>
#include "../Arrays/LIST_ARRAY.h"
#include "../Grids/SEGMENT_MESH.h"
#include "../Matrices_And_Vectors/VECTOR_3D.h"
#include "../Matrices_And_Vectors/DIAGONAL_MATRIX_3X3.h"
#include "../Matrices_And_Vectors/SYMMETRIC_MATRIX_3X3.h"
#include "../Matrices_And_Vectors/MATRIX_3X3.h"
#include "../Matrices_And_Vectors/MATRIX_4X4.h"
#include "../Matrices_And_Vectors/QUATERNION.h"
#include "../Matrices_And_Vectors/FRAME.h"
#include "../Forces_And_Torques/EXTERNAL_FORCES_AND_VELOCITIES.h"
#include "../Geometry/IMPLICIT_SURFACE.h"
#include "../Geometry/TRIANGULATED_SURFACE.h"
#include "../Geometry/TETRAHEDRALIZED_VOLUME.h"
#include "../Geometry/EMBEDDED_TETRAHEDRALIZED_VOLUME.h"
#include "../Geometry/ORIENTED_BOX_3D.h"
#include "../Read_Write/FILE_UTILITIES.h"
#include "../Collisions_And_Interactions/COLLISION_BODY_3D.h"
#include "RIGID_BODY_STATE_3D.h"
#include "MASS_PROPERTIES_3D.h"
namespace PhysBAM
{
template<class T> class RIGID_BODY_FORCES_3D;
template<class T> class RIGID_BODY_IMPULSE_ACCUMULATOR_3D;

template<class T>
class RIGID_BODY_3D: public COLLISION_BODY_3D<T>
{
public:
	using COLLISION_BODY_3D<T>::body_type;
	using COLLISION_BODY_3D<T>::Set_Collision_Thickness;

	T mass; // center of mass is (0,0,0) in object space
	DIAGONAL_MATRIX_3X3<T> inertia_tensor; // diagonal inertia tensor in object space, already scaled by the mass of the object
	T surface_roughness; // small number indicating errors in the geometry
	VECTOR_3D<T> position; // center of mass in world space
	QUATERNION<T> orientation; // with respect to the center of mass
	VECTOR_3D<T> velocity; // of the center of mass
	VECTOR_3D<T> angular_momentum; // with respect to the center of mass
	VECTOR_3D<T> angular_velocity; // with respect to the center of mass - needs to be computed!
	LIST_ARRAY<RIGID_BODY_FORCES_3D<T>*> body_forces;
	EXTERNAL_FORCES_AND_VELOCITIES<T, VECTOR_3D<T> >* external_forces_and_velocities, external_forces_and_velocities_default;
	RIGID_BODY_IMPULSE_ACCUMULATOR_3D<T>* impulse_accumulator;
	TRIANGULATED_SURFACE<T>* triangulated_surface; // discrete representation of the geometry
	IMPLICIT_SURFACE<T>* implicit_surface; // implicit representation of geometry
	TETRAHEDRALIZED_VOLUME<T>* tetrahedralized_volume;
	ORIENTED_BOX_3D<T> oriented_box;
	BOX_3D<T> axis_aligned_bounding_box;
	bool bounding_box_up_to_date;
	T coefficient_of_restitution; // not saved to file
	T coefficient_of_friction; //not saved to file
	T coefficient_of_rolling_friction; // not saved to file
	std::string name; // not saved to file - for convenience and debugging.
	bool is_static; // not saved to file - indicates whether this object is static in the scene
	bool is_kinematic;
	bool is_temporarily_static;   // not saved to file
	bool add_to_spatial_partition; // not saved to file - only false for (some) static objects
	LIST_ARRAY<RIGID_BODY_STATE_3D<T>*> saved_states;
	bool thin_shell;
	bool CFL_initialized; // true if precalculation is done, false if out of date
	T bounding_box_radius; // needed for CFL calculation
	int id_number;

	RIGID_BODY_3D()
		: external_forces_and_velocities (&external_forces_and_velocities_default), impulse_accumulator (0), triangulated_surface (0), implicit_surface (0), tetrahedralized_volume (0), bounding_box_up_to_date (false),
		  is_static (false), is_kinematic (false), is_temporarily_static (false), add_to_spatial_partition (true), thin_shell (false),
		  CFL_initialized (false), id_number (0)
	{
		body_type = COLLISION_BODY_3D<T>::RIGID_BODY_TYPE;
		mass = 1;
		inertia_tensor = DIAGONAL_MATRIX_3X3<T>::Identity_Matrix();
		Set_Surface_Roughness();
		Set_Coefficient_Of_Restitution();
		Set_Coefficient_Of_Friction();
		Set_Coefficient_Of_Rolling_Friction();
		Set_Collision_Thickness();
	}

	~RIGID_BODY_3D()
	{
		for (int i = 1; i <= saved_states.m; i++) delete saved_states (i);
	}

	void Set_Name (const std::string& name_input)
	{
		name = name_input;
	}

	void Set_Id_Number (const int id_number_input)
	{
		id_number = id_number_input;
	}

	static void Print_Names (const LIST_ARRAY<RIGID_BODY_3D<T>*>& rigid_bodies, const LIST_ARRAY<int>& indices)
	{
		std::cout << "{";

		for (int i = 1; i <= indices.m; i++)
		{
			std::cout << "\"" << rigid_bodies (indices (i))->name << "\"";

			if (i < indices.m) std::cout << ", ";
		}

		std::cout << "}";
	}

	static void Print_Pairs (const LIST_ARRAY<RIGID_BODY_3D<T>*>& rigid_bodies, const LIST_ARRAYS<int>& pairs)
	{
		std::cout << "{";

		for (int i = 1; i <= pairs.m; i++)
		{
			std::cout << "(\"" <<  rigid_bodies (pairs (1, i))->name << "\", \"" << rigid_bodies (pairs (2, i))->name << "\")";

			if (i < pairs.m) std::cout << ", ";
		}

		std::cout << "}";
	}

	void Set_Mass (const T mass_input) // rescales moments of inertia
	{
		inertia_tensor *= mass_input / mass;
		mass = mass_input;
	}

	void Set_Inertia_Tensor (const DIAGONAL_MATRIX_3X3<T>& inertia_tensor_input) // already scaled by the mass of the object
	{
		inertia_tensor = inertia_tensor_input;
	}

	void Rescale (const T scaling_factor, const bool rescale_mass = true)
	{
		NOT_IMPLEMENTED();
	}

	void Set_Surface_Roughness (const T surface_roughness_input = 1e-6)
	{
		surface_roughness = surface_roughness_input;
	}

	void Set_External_Forces_And_Velocities (EXTERNAL_FORCES_AND_VELOCITIES<T, VECTOR_3D<T> >& external_forces_and_velocities_input)
	{
		external_forces_and_velocities = &external_forces_and_velocities_input;
	}

	void Set_Coefficient_Of_Restitution (const T coefficient_input = .5)
	{
		coefficient_of_restitution = coefficient_input;
	}

	static T Coefficient_Of_Restitution (const RIGID_BODY_3D<T>& body1, const RIGID_BODY_3D<T>& body2)
	{
		return min (body1.coefficient_of_restitution, body2.coefficient_of_restitution);
	}

	void Set_Coefficient_Of_Friction (const T coefficient_input = .5)
	{
		coefficient_of_friction = coefficient_input;
	}

	static T Coefficient_Of_Friction (const RIGID_BODY_3D<T>& body1, const RIGID_BODY_3D<T>& body2)
	{
		return min (body1.coefficient_of_friction, body2.coefficient_of_friction);
	}

	void Set_Coefficient_Of_Rolling_Friction (const T coefficient_input = .5)
	{
		coefficient_of_rolling_friction = coefficient_input;
	}

	static T Coefficient_Of_Rolling_Friction (const RIGID_BODY_3D<T>& body1, const RIGID_BODY_3D<T>& body2)
	{
		return min (body1.coefficient_of_rolling_friction, body2.coefficient_of_rolling_friction);
	}

	bool Has_Infinite_Inertia() const
	{
		return is_static || is_temporarily_static || is_kinematic;
	}

	bool Is_Simulated() const
	{
		return !is_static && !is_kinematic;
	}

	void Transform (const MATRIX_4X4<T>& A)
	{
		position = A * position;
		orientation *= QUATERNION<T> (A);
	}

	void Set_State (const FRAME<T>& frame)
	{
		position = frame.t;
		orientation = frame.r;
	}

	void Set_State (const RIGID_BODY_3D<T>& rigid_body, const FRAME<T>& frame = FRAME<T>())
	{
		position = rigid_body.position + frame.t;
		orientation = rigid_body.orientation * frame.r;
	}

	FRAME<T> Frame() const
	{
		return FRAME<T> (position, orientation);
	}

	RAY_3D<T> Object_Space_Ray (const RAY_3D<T>& world_space_ray) const
	{
		RAY_3D<T> transformed_ray (Object_Space_Point (world_space_ray.endpoint), Object_Space_Vector (world_space_ray.direction));
		transformed_ray.semi_infinite = world_space_ray.semi_infinite;
		transformed_ray.t_max = world_space_ray.t_max;
		transformed_ray.aggregate_id = world_space_ray.aggregate_id;
		return transformed_ray;
	}

	VECTOR_3D<T> Object_Space_Point (const VECTOR_3D<T>& world_space_point) const
	{
		return orientation.Inverse_Rotate (world_space_point - position);
	}

	VECTOR_3D<T> Object_Space_Point (const VECTOR_3D<T>& world_space_point, const RIGID_BODY_STATE_3D<T>& state) const
	{
		return state.orientation.Inverse_Rotate (world_space_point - state.position);
	}

	VECTOR_3D<T> Object_Space_Vector (const VECTOR_3D<T>& world_space_vector) const
	{
		return orientation.Inverse_Rotate (world_space_vector);
	}

	VECTOR_3D<T> Object_Space_Vector (const VECTOR_3D<T>& world_space_vector, const RIGID_BODY_STATE_3D<T>& state) const
	{
		return state.orientation.Inverse_Rotate (world_space_vector);
	}

	VECTOR_3D<T> World_Space_Point (const VECTOR_3D<T>& object_space_point) const
	{
		return orientation.Rotate (object_space_point) + position;
	}

	static VECTOR_3D<T> World_Space_Point (const VECTOR_3D<T>& object_space_point, const RIGID_BODY_STATE_3D<T>& state)
	{
		return state.orientation.Rotate (object_space_point) + state.position;
	}

	VECTOR_3D<T> World_Space_Vector (const VECTOR_3D<T>& object_space_vector) const
	{
		return orientation.Rotate (object_space_vector);
	}

	static VECTOR_3D<T> World_Space_Vector (const VECTOR_3D<T>& object_space_vector, const RIGID_BODY_STATE_3D<T>& state)
	{
		return state.orientation.Rotate (object_space_vector);
	}

	SYMMETRIC_MATRIX_3X3<T> World_Space_Inertia_Tensor() const // relative to the center of mass
	{
		MATRIX_3X3<T> object_to_world_transformation = orientation.Matrix_3X3();
		return SYMMETRIC_MATRIX_3X3<T>::Conjugate (object_to_world_transformation, inertia_tensor);
	}

	SYMMETRIC_MATRIX_3X3<T> World_Space_Inertia_Tensor_Inverse() const // relative to the center of mass
	{
		MATRIX_3X3<T> object_to_world_transformation = orientation.Matrix_3X3();
		return SYMMETRIC_MATRIX_3X3<T>::Conjugate (object_to_world_transformation, inertia_tensor.Inverse());
	}

	SYMMETRIC_MATRIX_3X3<T> World_Space_Inertia_Tensor (const VECTOR_3D<T>& reference_point) const // relative to a reference point
	{
		VECTOR_3D<T> dx = position - reference_point;
		SYMMETRIC_MATRIX_3X3<T> new_term = mass * (VECTOR_3D<T>::Dot_Product (dx, dx) - SYMMETRIC_MATRIX_3X3<T>::Outer_Product (dx));
		return World_Space_Inertia_Tensor() + new_term;
	}

	void Update_Angular_Velocity() // needs to be called to keep the angular velocity valid
	{
		angular_velocity = World_Space_Vector (inertia_tensor.Inverse_Times (Object_Space_Vector (angular_momentum)));
	}

	void Update_Angular_Velocity (RIGID_BODY_STATE_3D<T>& state) const // needs to be called to keep the angular velocity valid
	{
		state.angular_velocity = World_Space_Vector (inertia_tensor.Inverse_Times (Object_Space_Vector (state.angular_momentum, state)), state);
	}

	void Update_Angular_Momentum() // assumes a valid angular_velocity
	{
		angular_momentum = World_Space_Vector (inertia_tensor * Object_Space_Vector (angular_velocity));
	}

	void Update_Angular_Momentum (RIGID_BODY_STATE_3D<T>& state) const // assumes a valid angular_velocity
	{
		state.angular_momentum = World_Space_Vector (inertia_tensor * Object_Space_Vector (state.angular_velocity, state), state);
	}

	VECTOR_3D<T> Pointwise_Object_Velocity (const VECTOR_3D<T>& world_space_point) const
	{
		return velocity + VECTOR_3D<T>::Cross_Product (angular_velocity, world_space_point - position);
	}

	// overridden in DEFORMABLE_OBJECT_RIGID_BODY_WRAPPER_3D where particle_index is used instead of world_space_point
	virtual VECTOR_3D<T> Pointwise_Object_Velocity_At_Particle (const VECTOR_3D<T>& world_space_point, const int particle_index) const
	{
		return velocity + VECTOR_3D<T>::Cross_Product (angular_velocity, world_space_point - position);
	}

	VECTOR_3D<T> Pointwise_Object_Velocity (const int triangle_id, const VECTOR_3D<T>& world_space_point) const // extra triangle_id is not used, but for a virtual function in COLLISION BODY
	{
		return velocity + VECTOR_3D<T>::Cross_Product (angular_velocity, world_space_point - position);
	}

	VECTOR_3D<T> Pointwise_Object_Pseudo_Velocity (const int triangle_id, const VECTOR_3D<T>& world_space_point, const int state1, const int state2) const
	{
		VECTOR_3D<T> object_space_point = Object_Space_Point (world_space_point);
		return (World_Space_Point (object_space_point, *saved_states (state2)) - World_Space_Point (object_space_point, *saved_states (state1))) / (saved_states (state2)->time - saved_states (state1)->time);
	}

	static VECTOR_3D<T> Relative_Velocity (const RIGID_BODY_3D<T>& body1, const RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& world_point)
	{
		return body1.Pointwise_Object_Velocity (world_point) - body2.Pointwise_Object_Velocity (world_point);
	}

	static VECTOR_3D<T> Relative_Velocity_At_Body1_Particle (const RIGID_BODY_3D<T>& body1, const RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& world_point, const int particle_index)
	{
		return body1.Pointwise_Object_Velocity_At_Particle (world_point, particle_index) - body2.Pointwise_Object_Velocity (world_point);
	}

	static VECTOR_3D<T> Relative_Angular_Velocity (const RIGID_BODY_3D<T>& body1, const RIGID_BODY_3D<T>& body2) // make sure the angular velocities are updated before calling this!
	{
		return body1.angular_velocity - body2.angular_velocity;
	}

	T Kinetic_Energy() const
	{
		return Translational_Kinetic_Energy() + Rotational_Kinetic_Energy();
	}

	T Translational_Kinetic_Energy() const
	{
		return (T).5 * mass * velocity.Magnitude_Squared();
	}

	T Rotational_Kinetic_Energy() const
	{
		VECTOR_3D<T> object_space_angular_momentum = Object_Space_Vector (angular_momentum);
		return (T).5 * inertia_tensor.Inverse_Inner_Product (object_space_angular_momentum, object_space_angular_momentum);
	}

	SYMMETRIC_MATRIX_3X3<T> Impulse_Factor (const VECTOR_3D<T>& location) const
	{
		if (Has_Infinite_Inertia()) return SYMMETRIC_MATRIX_3X3<T>(); // return zero matrix

		MATRIX_3X3<T> moment_arm = MATRIX_3X3<T>::Cross_Product_Matrix (location - position);
		return SYMMETRIC_MATRIX_3X3<T>::Conjugate (moment_arm, World_Space_Inertia_Tensor_Inverse()) + (1 / mass);
	}

	static SYMMETRIC_MATRIX_3X3<T> Impulse_Factor (const RIGID_BODY_3D<T>& body1, const RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& location)
	{
		assert (!body1.Has_Infinite_Inertia() || !body2.Has_Infinite_Inertia());
		return body1.Impulse_Factor (location) + body2.Impulse_Factor (location);
	}

	void Initialize_Triangulated_Surface (TRIANGULATED_SURFACE<T>& triangulated_surface_input) // set up acceleration stuctures too
	{
		triangulated_surface = &triangulated_surface_input;

		if (!triangulated_surface->triangle_mesh.segment_mesh) triangulated_surface->triangle_mesh.Initialize_Segment_Mesh();

		if (!triangulated_surface->triangle_mesh.triangle_edges) triangulated_surface->triangle_mesh.Initialize_Triangle_Edges();

		if (!triangulated_surface->triangle_mesh.segment_mesh->incident_segments) triangulated_surface->triangle_mesh.segment_mesh->Initialize_Incident_Segments();

		if (!triangulated_surface->segment_lengths) triangulated_surface->Initialize_Segment_Lengths();

		if (!triangulated_surface->triangle_list) triangulated_surface->Update_Triangle_List();

		if (!triangulated_surface->bounding_box) triangulated_surface->Update_Bounding_Box();
	}

	bool Triangulated_Surface_Intersection (RAY_3D<T>& ray) const
	{
		RAY_3D<T> object_space_ray = Object_Space_Ray (ray);

		if (triangulated_surface->Intersection (object_space_ray, surface_roughness))
		{
			ray.semi_infinite = false;
			ray.t_max = object_space_ray.t_max;
			ray.aggregate_id = object_space_ray.aggregate_id;
			return true;
		}
		else return false;
	}

	bool Triangulated_Surface_Closest_Non_Intersecting_Point (RAY_3D<T>& ray) const
	{
		RAY_3D<T> object_space_ray = Object_Space_Ray (ray);

		if (triangulated_surface->Closest_Non_Intersecting_Point (object_space_ray, surface_roughness))
		{
			ray.Restore_Intersection_Information (object_space_ray);
			return true;
		}
		else return false;
	}

	VECTOR_3D<T> Triangulated_Surface_Normal (const VECTOR_3D<T>& location, const int aggregate = 0) const
	{
		return World_Space_Vector (triangulated_surface->Normal (Object_Space_Point (location), aggregate));
	}

	bool Triangulated_Surface_Inside (const VECTOR_3D<T>& location) const
	{
		return triangulated_surface->Inside (Object_Space_Point (location), surface_roughness);
	}

	bool Triangulated_Surface_Outside (const VECTOR_3D<T>& location) const
	{
		return triangulated_surface->Outside (Object_Space_Point (location), surface_roughness);
	}

	bool Triangulated_Surface_Boundary (const VECTOR_3D<T>& location) const
	{
		return triangulated_surface->Boundary (Object_Space_Point (location), surface_roughness);
	}

	bool Triangulated_Surface_Inside_Any_Triangle (const VECTOR_3D<T>& location, int& triangle_id) const
	{
		return triangulated_surface->Inside_Any_Triangle (Object_Space_Point (location), triangle_id, surface_roughness);
	}

	VECTOR_3D<T> Triangulated_Surface_Surface (const VECTOR_3D<T>& location, const T max_distance, int* triangle_id = 0, T* distance = 0) const
	{
		return World_Space_Point (triangulated_surface->Surface (Object_Space_Point (location), max_distance, surface_roughness, triangle_id, distance));
	}

	TRIANGLE_3D<T> Triangulated_Surface_World_Space_Triangle (const int triangle_id, const bool use_saved_state = false) const
	{
		if (use_saved_state)
		{
			assert (saved_states (1));
			return Triangulated_Surface_World_Space_Triangle (triangle_id, *saved_states (1));
		}
		else return TRIANGLE_3D<T> (World_Space_Point (triangulated_surface->particles.X (triangulated_surface->triangle_mesh.triangles (1, triangle_id))),
						    World_Space_Point (triangulated_surface->particles.X (triangulated_surface->triangle_mesh.triangles (2, triangle_id))),
						    World_Space_Point (triangulated_surface->particles.X (triangulated_surface->triangle_mesh.triangles (3, triangle_id))));
	}

	TRIANGLE_3D<T> Triangulated_Surface_World_Space_Triangle (const int triangle_id, const RIGID_BODY_STATE_3D<T>& state) const
	{
		return TRIANGLE_3D<T> (World_Space_Point (triangulated_surface->particles.X (triangulated_surface->triangle_mesh.triangles (1, triangle_id)), state),
				       World_Space_Point (triangulated_surface->particles.X (triangulated_surface->triangle_mesh.triangles (2, triangle_id)), state),
				       World_Space_Point (triangulated_surface->particles.X (triangulated_surface->triangle_mesh.triangles (3, triangle_id)), state));
	}

	VECTOR_3D<T> Triangulated_Surface_World_Space_Point_From_Barycentric_Coordinates (const int triangle_id, const VECTOR_3D<T>& weights) const
	{
		return TRIANGLE_3D<T>::Point_From_Barycentric_Coordinates (weights,
				World_Space_Point (triangulated_surface->particles.X (triangulated_surface->triangle_mesh.triangles (1, triangle_id))),
				World_Space_Point (triangulated_surface->particles.X (triangulated_surface->triangle_mesh.triangles (2, triangle_id))),
				World_Space_Point (triangulated_surface->particles.X (triangulated_surface->triangle_mesh.triangles (3, triangle_id))));
	}

	VECTOR_3D<T> Triangulated_Surface_World_Space_Point_From_Barycentric_Coordinates (const int triangle_id, const VECTOR_3D<T>& weights, const RIGID_BODY_STATE_3D<T>& state) const
	{
		return TRIANGLE_3D<T>::Point_From_Barycentric_Coordinates (weights,
				World_Space_Point (triangulated_surface->particles.X (triangulated_surface->triangle_mesh.triangles (1, triangle_id)), state),
				World_Space_Point (triangulated_surface->particles.X (triangulated_surface->triangle_mesh.triangles (2, triangle_id)), state),
				World_Space_Point (triangulated_surface->particles.X (triangulated_surface->triangle_mesh.triangles (3, triangle_id)), state));
	}

	void Triangulated_Surface_Translate (const VECTOR_3D<T>& translation)
	{
		for (int i = 1; i <= triangulated_surface->particles.number; i++) triangulated_surface->particles.X (i) += translation;

		triangulated_surface->Update_Triangle_List();
		triangulated_surface->Update_Bounding_Box();
	}

	void Initialize_Implicit_Surface (IMPLICIT_SURFACE<T>& implicit_surface_input)
	{
		implicit_surface = &implicit_surface_input;        // don't recompute cell min/max if already computed
		implicit_surface->Update_Box();
		implicit_surface->Compute_Cell_Minimum_And_Maximum (false);
	}

	void Initialize_Tetrahedralized_Volume (TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume_input)
	{
		tetrahedralized_volume = &tetrahedralized_volume_input;

		if (!tetrahedralized_volume->bounding_box) tetrahedralized_volume->Update_Bounding_Box();

		if (!tetrahedralized_volume->tetrahedron_list) tetrahedralized_volume->Update_Tetrahedron_List();

		if (!tetrahedralized_volume->tetrahedron_hierarchy)
		{
			tetrahedralized_volume->Initialize_Tetrahedron_Hierarchy();
			tetrahedralized_volume->tetrahedron_hierarchy->Update_Box_Radii();
		}
	}

	bool Implicit_Surface_Intersection (RAY_3D<T>& ray) const
	{
		RAY_3D<T> object_space_ray = Object_Space_Ray (ray);

		if (implicit_surface->Intersection (object_space_ray, surface_roughness))
		{
			ray.semi_infinite = false;
			ray.t_max = object_space_ray.t_max;
			ray.aggregate_id = object_space_ray.aggregate_id;
			return true;
		}
		else return false;
	}

	virtual VECTOR_3D<T> Implicit_Surface_Normal (const VECTOR_3D<T>& location, const int aggregate = -1) const
	{
		return World_Space_Vector (implicit_surface->Normal (Object_Space_Point (location), aggregate));
	}

	VECTOR_3D<T> Implicit_Surface_Extended_Normal (const VECTOR_3D<T>& location, T& phi_value, const int aggregate = -1, const int location_particle_index = 0) const
	{
		phi_value = implicit_surface->Extended_Phi (Object_Space_Point (location));
		return World_Space_Vector (implicit_surface->Extended_Normal (Object_Space_Point (location), aggregate));
	}

	VECTOR_3D<T> Implicit_Surface_Normal (const VECTOR_3D<T>& location, T& phi_value, const int aggregate = -1, const int location_particle_index = 0) const
	{
		phi_value = (*implicit_surface) (Object_Space_Point (location));
		return World_Space_Vector (implicit_surface->Normal (Object_Space_Point (location), aggregate));
	}

	VECTOR_3D<T> Implicit_Surface_Extended_Normal (const VECTOR_3D<T>& location, const int aggregate = -1) const
	{
		return World_Space_Vector (implicit_surface->Extended_Normal (Object_Space_Point (location), aggregate));
	}

	VECTOR_3D<T> Implicit_Surface_Normal_In_Object_Space (const VECTOR_3D<T>& object_space_location, const int aggregate = -1) const
	{
		return World_Space_Vector (implicit_surface->Normal (object_space_location, aggregate));
	}

	VECTOR_3D<T> Implicit_Surface_Extended_Normal_In_Object_Space (const VECTOR_3D<T>& object_space_location, const int aggregate = -1) const
	{
		return World_Space_Vector (implicit_surface->Extended_Normal (object_space_location, aggregate));
	}

	bool Implicit_Surface_Inside (const VECTOR_3D<T>& location) const
	{
		return implicit_surface->Inside (Object_Space_Point (location), surface_roughness);
	}

	bool Implicit_Surface_Inside_In_Object_Space (const VECTOR_3D<T>& object_space_location) const
	{
		return implicit_surface->Inside (object_space_location, surface_roughness);
	}

	virtual bool Implicit_Surface_Lazy_Inside (const VECTOR_3D<T>& location, T contour_value = 0) const
	{
		return implicit_surface->Lazy_Inside (Object_Space_Point (location), contour_value);
	}

	bool Implicit_Surface_Lazy_Inside_In_Object_Space (const VECTOR_3D<T>& object_space_location, T contour_value = 0) const
	{
		return implicit_surface->Lazy_Inside (object_space_location, contour_value);
	}

	virtual bool Implicit_Surface_Lazy_Inside_And_Value (const VECTOR_3D<T>& location, T& phi, T contour_value = 0) const

	{
		return implicit_surface->Lazy_Inside_And_Value (Object_Space_Point (location), phi, contour_value);
	}

	bool Implicit_Surface_Lazy_Inside_And_Value_In_Object_Space (const VECTOR_3D<T>& location, T& phi, T contour_value = 0) const
	{
		return implicit_surface->Lazy_Inside_And_Value (location, phi, contour_value);
	}

	bool Implicit_Surface_Lazy_Inside_Extended_Levelset_And_Value (const VECTOR_3D<T>& location, T& phi_value, T contour_value = 0) const
	{
		return implicit_surface->Lazy_Inside_Extended_Levelset_And_Value (Object_Space_Point (location), phi_value, contour_value);
	}

	bool Implicit_Surface_Outside (const VECTOR_3D<T>& location) const
	{
		return implicit_surface->Outside (Object_Space_Point (location), surface_roughness);
	}

	bool Implicit_Surface_Lazy_Outside (const VECTOR_3D<T>& location) const
	{
		return implicit_surface->Lazy_Outside (Object_Space_Point (location));
	}

	bool Implicit_Surface_Lazy_Outside_Extended_Levelset (const VECTOR_3D<T>& location, T contour_value = 0) const
	{
		return implicit_surface->Lazy_Outside_Extended_Levelset (Object_Space_Point (location), contour_value);
	}

	bool Implicit_Surface_Lazy_Outside_Extended_Levelset_And_Value (const VECTOR_3D<T>& location, T& phi_value, T contour_value = 0) const
	{
		return implicit_surface->Lazy_Outside_Extended_Levelset_And_Value (Object_Space_Point (location), phi_value, contour_value);
	}

	bool Implicit_Surface_Boundary (const VECTOR_3D<T>& location) const
	{
		return implicit_surface->Boundary (Object_Space_Point (location), surface_roughness);
	}

	VECTOR_3D<T> Implicit_Surface_Surface (const VECTOR_3D<T>& location, const int max_iterations = 1) const
	{
		return World_Space_Point (implicit_surface->Surface (Object_Space_Point (location), surface_roughness, max_iterations));
	}

	T Implicit_Surface_Value (const VECTOR_3D<T>& location) const
	{
		return (*implicit_surface) (Object_Space_Point (location));
	}

	T Implicit_Surface_Value_In_Object_Space (const VECTOR_3D<T>& object_space_location) const
	{
		return (*implicit_surface) (object_space_location);
	}

	void Update_Bounding_Box()
	{
		assert (triangulated_surface && triangulated_surface->bounding_box);
		bounding_box_up_to_date = true;
		oriented_box = ORIENTED_BOX_3D<T> (*triangulated_surface->bounding_box, orientation, World_Space_Point (triangulated_surface->bounding_box->Minimum_Corner()));
		axis_aligned_bounding_box = oriented_box.Axis_Aligned_Bounding_Box();
	}

	void Update_Bounding_Box_From_Implicit_Surface()
	{
		assert (implicit_surface);
		bounding_box_up_to_date = true;
		oriented_box = ORIENTED_BOX_3D<T> (implicit_surface->box, orientation, World_Space_Point (implicit_surface->box.Minimum_Corner()));
		axis_aligned_bounding_box = oriented_box.Axis_Aligned_Bounding_Box();
	}

	const ORIENTED_BOX_3D<T>& Oriented_Bounding_Box() const
	{
		assert (bounding_box_up_to_date);
		return oriented_box;
	}

	const BOX_3D<T>& Axis_Aligned_Bounding_Box() const
	{
		assert (bounding_box_up_to_date);
		return axis_aligned_bounding_box;
	}

	void Save_State (RIGID_BODY_STATE_3D<T>& state, const T time = 0) const
	{
		state.time = time;
		state.position = position;
		state.orientation = orientation;
		state.velocity = velocity;
		state.angular_momentum = angular_momentum;
		state.angular_velocity = angular_velocity;
	}

	void Restore_State (const RIGID_BODY_STATE_3D<T>& state)
	{
		position = state.position;
		orientation = state.orientation;
		velocity = state.velocity;
		angular_momentum = state.angular_momentum;
		angular_velocity = state.angular_velocity;
	}

	void Save_State (const int state_index, const T time = 0)
	{
		if (saved_states.m < state_index) saved_states.Resize_Array (state_index);

		if (!saved_states (state_index)) saved_states (state_index) = new RIGID_BODY_STATE_3D<T>;

		Save_State (*saved_states (state_index), time);
	}

	void Restore_State (const int state_index)
	{
		assert (saved_states (state_index));
		Restore_State (*saved_states (state_index));
	}

	void Delete_State (const int state_index)
	{
		delete saved_states (state_index);
		saved_states (state_index) = 0;
	}

	template<class RW> void Read_State (std::istream& input_stream, const int state_index)
	{
		if (saved_states.m < state_index) saved_states.Resize_Array (state_index);

		if (!saved_states (state_index)) saved_states (state_index) = new RIGID_BODY_STATE_3D<T>;

		saved_states (state_index)->template Read<RW> (input_stream);
	}

	template<class RW> void Write_State (std::ostream& output_stream, const int state_index)
	{
		assert (saved_states (state_index));
		saved_states (state_index)->template Write<RW> (output_stream);
	}

	T Thin_Shell_Density() const
	{
		return triangulated_surface->density;
	}

	template<class RW>
	void Read (std::istream& input_stream)
	{
		Read_Binary<RW> (input_stream, mass, inertia_tensor, surface_roughness, position, orientation, velocity, angular_momentum);
		Update_Angular_Velocity();
	}

	template<class RW>
	void Write (std::ostream& output_stream) const
	{
		Write_Binary<RW> (output_stream, mass, inertia_tensor, surface_roughness, position, orientation, velocity, angular_momentum);
	}

//#####################################################################
	void Euler_Step (const T dt, const T time = 0, const T small_number = 1e-7);
	void Euler_Step_Position (const T dt, const T time = 0, const T small_number = 1e-7);
	void Euler_Step_Velocity (const T dt, const T time = 0);
	void Add_Velocity_Independent_Forces (VECTOR_3D<T>& F, VECTOR_3D<T>& torque) const;
	void Add_Velocity_Dependent_Forces (VECTOR_3D<T>& F, VECTOR_3D<T>& torque) const; // can depend on position too
	void Initialize_CFL();
	T CFL (const T max_distance_per_time_step, const T max_rotation_per_time_step, const bool verbose = false);
	void Interpolate_Between_States (const RIGID_BODY_STATE_3D<T>& state1, const RIGID_BODY_STATE_3D<T>& state2, const T time, RIGID_BODY_STATE_3D<T>& interpolated_state);
	void Compute_Velocity_Between_States (const RIGID_BODY_STATE_3D<T>& state1, const RIGID_BODY_STATE_3D<T>& state2, RIGID_BODY_STATE_3D<T>& result_state);
	static void Apply_Impulse (RIGID_BODY_3D<T>& body1, RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& location, const VECTOR_3D<T>& impulse);
	static void Apply_Impulse_And_Angular_Impulse (RIGID_BODY_3D<T>& body1, RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& impulse, const VECTOR_3D<T>& angular_impulse);
	static void Apply_Angular_Impulse (RIGID_BODY_3D<T>& body1, RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& angular_impulse);
	static void Apply_Collision_Impulse (RIGID_BODY_3D<T>& body1, RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& location, const VECTOR_3D<T>& normal, const T coefficient_of_restitution,
					     const T coefficient_of_friction = 0, const bool clamp_friction_magnitude = true, const bool rolling_friction = false);
	static void Apply_Collision_Impulse (RIGID_BODY_3D<T>& body1, RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& location, const VECTOR_3D<T>& normal,
					     const VECTOR_3D<T>& relative_velocity, const T coefficient_of_restitution, const T coefficient_of_friction = 0, const bool clamp_friction_magnitude = true, const bool rolling_friction = false);
	static void Apply_Sticking_Impulse (RIGID_BODY_3D<T>& body1, RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& location);
	static void Apply_Angular_Sticking_Impulse (RIGID_BODY_3D<T>& body1, RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& location);
	static void Apply_Sticking_And_Angular_Sticking_Impulse (RIGID_BODY_3D<T>& body1, RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& location,
			const VECTOR_3D<T>& delta_relative_angular_velocity, const VECTOR_3D<T>& delta_relative_linear_velocity, const T epsilon_scale = 1);
	static void Apply_Rolling_Friction (RIGID_BODY_3D<T>& body1, RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& location, const VECTOR_3D<T>& normal, const T normal_impulse);
	static void Find_Impulse_And_Angular_Impulse (const RIGID_BODY_3D<T>& body1, const RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& location,
			const VECTOR_3D<T>& delta_rel_velocity_at_location, const VECTOR_3D<T>& delta_rel_angular_velocity, VECTOR_3D<T>& impulse, VECTOR_3D<T>& angular_impulse);
	static void Apply_Push (RIGID_BODY_3D<T>& body1, RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& location, const VECTOR_3D<T>& normal, const T distance);
	virtual void Adjust_Point_For_Collision (VECTOR_3D<T>& X, VECTOR_3D<T>& V, const T point_mass, const T penetration_depth, const T one_over_dt);
	virtual void Adjust_Point_For_Collision (VECTOR_3D<T>& X, const T penetration_depth);
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
