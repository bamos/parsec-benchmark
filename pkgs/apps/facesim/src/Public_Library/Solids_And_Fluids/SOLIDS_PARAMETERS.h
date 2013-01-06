//#####################################################################
// Copyright 2004, Ron Fedkiw, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SOLIDS_PARAMETERS
//#####################################################################
#ifndef __SOLIDS_PARAMETERS__
#define __SOLIDS_PARAMETERS__

#include "../Matrices_And_Vectors/VECTOR_3D.h"
namespace PhysBAM
{

template <class T>
class SOLIDS_PARAMETERS
{
public:
	T cfl;
	T gravity;
	VECTOR_3D<T> gravity_direction;
	int cg_iterations;
	T cg_tolerance;
	bool semi_implicit, asynchronous;
	bool use_constant_mass;
	bool perform_collision_body_collisions;
	T collision_tolerance;
	bool collide_with_interior;
	bool enforce_tangential_collision_velocity;
	bool fracture;
	bool write_static_variables_every_frame;
	bool synchronize_multiple_objects;
	int preroll_frames;

	SOLIDS_PARAMETERS()
		: cfl ( (T).9), gravity ( (T) 9.8), gravity_direction (0, -1, 0),
		  cg_iterations (200), cg_tolerance ( (T) 1e-3), semi_implicit (false), asynchronous (false), use_constant_mass (false),
		  perform_collision_body_collisions (true), collision_tolerance ( (T) 1e-6), collide_with_interior (false), enforce_tangential_collision_velocity (false),
		  fracture (false), write_static_variables_every_frame (false), synchronize_multiple_objects (false), preroll_frames (0)
	{}

	virtual ~SOLIDS_PARAMETERS()
	{}

//#####################################################################
};
}
#endif
