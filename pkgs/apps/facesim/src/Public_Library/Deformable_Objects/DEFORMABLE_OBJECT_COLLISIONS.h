//#####################################################################
// Copyright 2004, Ron Fedkiw, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class DEFORMABLE_OBJECT_COLLISIONS
//#####################################################################
#ifndef __DEFORMABLE_OBJECT_COLLISIONS__
#define __DEFORMABLE_OBJECT_COLLISIONS__

#include "../Data_Structures/PAIR.h"
#include "../Particles/SOLIDS_PARTICLE.h"
namespace PhysBAM
{

template<class T, class TV>
class DEFORMABLE_OBJECT_COLLISIONS
{
public:
	SOLIDS_PARTICLE<T, TV>* collision_particles;
	SOLIDS_PARTICLE<T, TV>* embedded_particles_for_thin_shells;
	LIST_ARRAY<PAIR<SOLIDS_PARTICLE<T, TV>*, T> > saved_states;
	LIST_ARRAY<SOLIDS_PARTICLE<T, TV>* > saved_embedded_particles_for_thin_shells;
	ARRAY<bool> check_collision;
	ARRAY<bool> enforce_collision;
	bool enforce_tangential_collision_velocity;
	ARRAY<TV> total_collision_velocity;
	ARRAY<TV> collision_normal;
	ARRAY<T> normal_collision_velocity;
	T collision_tolerance;
	T roughness;
	bool perform_self_collision;
	ARRAY<TV> X_self_collision_free, V_self_collision_free;
	ARRAY<bool> skip_collision_body;

	DEFORMABLE_OBJECT_COLLISIONS()
		: collision_particles (0), embedded_particles_for_thin_shells (0), enforce_tangential_collision_velocity (false), collision_tolerance ( (T) 1e-6), roughness ( (T) 1e-8)
	{
		Perform_Self_Collision();
	}

	virtual ~DEFORMABLE_OBJECT_COLLISIONS()
	{
		for (int i = 1; i <= saved_states.m; i++) delete saved_states (i).x;
	}

	void Perform_Self_Collision (const bool perform_self_collision_input = true)
	{
		perform_self_collision = perform_self_collision_input;
	}

	void Initialize_Object_Collisions (SOLIDS_PARTICLE<T, TV>& collision_particles_input, const bool enforce_tangential_collision_velocity_input)
	{
		collision_particles = &collision_particles_input;
		enforce_tangential_collision_velocity = enforce_tangential_collision_velocity_input;
		Reset_Object_Collisions(); // in case collisions already exist
		check_collision.Resize_Array (collision_particles->number, false, false);
		ARRAY<bool>::copy (true, check_collision);
		enforce_collision.Resize_Array (collision_particles->number);

		if (enforce_tangential_collision_velocity) total_collision_velocity.Resize_Array (collision_particles->number);
		else
		{
			collision_normal.Resize_Array (collision_particles->number);
			normal_collision_velocity.Resize_Array (collision_particles->number);
		}
	}

	void Set_Embedded_Particles_For_Thin_Shells (SOLIDS_PARTICLE<T, TV>& embedded_particles)
	{
		embedded_particles_for_thin_shells = &embedded_particles;
	}

	void Set_Collision_Velocities (ARRAY<TV>& V) // for external forces and velocities
	{
		if (enforce_tangential_collision_velocity)
		{
			for (int p = 1; p <= check_collision.m; p++) if (enforce_collision (p)) V (p) = total_collision_velocity (p);
		}
		else for (int p = 1; p <= check_collision.m; p++) if (enforce_collision (p)) V (p) += (normal_collision_velocity (p) - TV::Dot_Product (V (p), collision_normal (p))) * collision_normal (p);
	}

	void Zero_Out_Collision_Velocities (ARRAY<TV>& V) // for external forces and velocities
	{
		if (enforce_tangential_collision_velocity)
		{
			for (int p = 1; p <= check_collision.m; p++) if (enforce_collision (p)) V (p) = TV();
		}
		else for (int p = 1; p <= check_collision.m; p++) if (enforce_collision (p)) V (p) -= TV::Dot_Product (V (p), collision_normal (p)) * collision_normal (p);
	}

	void Reset_Object_Collisions()
	{
		ARRAY<bool>::copy (false, enforce_collision);
	}

	virtual void Save_State (PAIR<SOLIDS_PARTICLE<T, TV>*, T>& state, const T time = 0) const
	{
		state.x->Initialize_Particles (*collision_particles);
		state.y = time;
	}

	virtual void Restore_State (const PAIR<SOLIDS_PARTICLE<T, TV>*, T>& state)
	{
		collision_particles->Initialize_Particles (*state.x);
	}

	void Save_State (const int state_index, const T time = 0)
	{
		if (saved_states.m < state_index) saved_states.Resize_Array (state_index);

		delete saved_states (state_index).x;
		saved_states (state_index).x = new SOLIDS_PARTICLE<T, TV>;

		if (embedded_particles_for_thin_shells)
		{
			if (saved_embedded_particles_for_thin_shells.m < state_index) saved_embedded_particles_for_thin_shells.Resize_Array (state_index);

			delete saved_embedded_particles_for_thin_shells (state_index);
			saved_embedded_particles_for_thin_shells (state_index) = new SOLIDS_PARTICLE<T, TV>;
		}

		Save_State (saved_states (state_index), time);

		if (embedded_particles_for_thin_shells) saved_embedded_particles_for_thin_shells (state_index)->Initialize_Particles (*embedded_particles_for_thin_shells);
	}

	void Restore_State (const int state_index)
	{
		assert (saved_states (state_index).x);
		Restore_State (saved_states (state_index));

		if (embedded_particles_for_thin_shells) embedded_particles_for_thin_shells->Initialize_Particles (*saved_embedded_particles_for_thin_shells (state_index));
	}

	void Delete_State (const int state_index)
	{
		delete saved_states (state_index).x;
		saved_states (state_index).x = 0;

		if (embedded_particles_for_thin_shells)
		{
			delete saved_embedded_particles_for_thin_shells (state_index);
			saved_embedded_particles_for_thin_shells (state_index) = 0;
		}
	}

	template <class RW> void Read_State (std::istream& input_stream, const int state_index)
	{
		if (saved_states.m < state_index) saved_states.Resize_Array (state_index);

		delete saved_states (state_index).x;
		saved_states (state_index).x = new SOLIDS_PARTICLE<T, TV>;
		char version;
		bool have_embedded;
		Read_Binary<RW> (input_stream, version);
		assert (version == 1);
		Read_Binary<RW> (input_stream, saved_states (state_index).y, *saved_states (state_index).x, have_embedded);

		if (embedded_particles_for_thin_shells)
		{
			assert (have_embedded);

			if (saved_embedded_particles_for_thin_shells.m < state_index) saved_embedded_particles_for_thin_shells.Resize_Array (state_index);

			delete saved_embedded_particles_for_thin_shells (state_index);
			saved_embedded_particles_for_thin_shells (state_index) = new SOLIDS_PARTICLE<T, TV>;
			Read_Binary<RW> (input_stream, *saved_embedded_particles_for_thin_shells (state_index));
		}
		else
		{
			assert (!have_embedded);
		}
	}

	template <class RW> void Write_State (std::ostream& output_stream, const int state_index) const
	{
		assert (saved_states (state_index).x);
		char version = 1;
		bool have_embedded = embedded_particles_for_thin_shells != 0;
		Write_Binary<RW> (output_stream, version, saved_states (state_index).y, *saved_states (state_index).x, have_embedded);

		if (embedded_particles_for_thin_shells)
		{
			assert (saved_embedded_particles_for_thin_shells (state_index));
			Write_Binary<RW> (output_stream, *saved_embedded_particles_for_thin_shells (state_index));
		}
	}

//#####################################################################
};
}
#endif
