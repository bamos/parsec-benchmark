//#####################################################################
// Copyright 2003-2004, Ronald Fedkiw, Eran Guendelman, Rachel Weinstein.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class RIGID_BODY_EVOLUTION_3D
//#####################################################################
#ifndef __RIGID_BODY_EVOLUTION_3D__
#define __RIGID_BODY_EVOLUTION_3D__

#include "RIGID_BODY_EVOLUTION.h"
#include "RIGID_BODY_3D.h"
#include "RIGID_BODY_COLLISIONS_3D.h"
namespace PhysBAM
{

template<class T>
class RIGID_BODY_EVOLUTION_3D: public RIGID_BODY_EVOLUTION<T>
{
private:
	using RIGID_BODY_EVOLUTION<T>::cfl_number;
	using RIGID_BODY_EVOLUTION<T>::max_rotation_per_time_step;
	using RIGID_BODY_EVOLUTION<T>::max_linear_movement_fraction_per_time_step;
	using RIGID_BODY_EVOLUTION<T>::minimum_dt;
	using RIGID_BODY_EVOLUTION<T>::maximum_dt;
	using RIGID_BODY_EVOLUTION<T>::artificial_maximum_speed;
	using RIGID_BODY_EVOLUTION<T>::use_contact_graph;
	using RIGID_BODY_EVOLUTION<T>::use_shock_propagation;
	using RIGID_BODY_EVOLUTION<T>::use_push_out;
	using RIGID_BODY_EVOLUTION<T>::clamp_velocities;
	using RIGID_BODY_EVOLUTION<T>::max_linear_velocity;
	using RIGID_BODY_EVOLUTION<T>::max_angular_velocity;

public:
	LIST_ARRAY<RIGID_BODY_3D<T>*>& rigid_bodies;
	RIGID_BODY_COLLISIONS_3D<T>& collisions;

	RIGID_BODY_EVOLUTION_3D (LIST_ARRAY<RIGID_BODY_3D<T>*>& rigid_bodies_input, RIGID_BODY_COLLISIONS_3D<T>& collisions_input)
		: rigid_bodies (rigid_bodies_input), collisions (collisions_input)
	{}

//#####################################################################
	void Advance_One_Time_Step (const T dt, const T time);
	void Update_Positions (const T dt, const T time);
	void Update_Velocities (const T dt, const T time);
	void Add_Elastic_Collisions (const T dt, const T time);
	void Apply_Contact_Forces (const T dt, const T time);
	T CFL (const bool verbose = false);
	void Clamp_Velocities();
//#####################################################################
};
}
#endif
