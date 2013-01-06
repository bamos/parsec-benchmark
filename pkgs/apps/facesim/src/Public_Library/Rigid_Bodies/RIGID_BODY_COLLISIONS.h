//#####################################################################
// Copyright 2004, Eran Guendelman, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class RIGID_BODY_COLLISIONS
//#####################################################################
#ifndef __RIGID_BODY_COLLISIONS__
#define __RIGID_BODY_COLLISIONS__

#include "RIGID_BODY_SKIP_COLLISION_CHECK.h"
#include "RIGID_BODY_COLLISION_MANAGER.h"
#include "../Arrays/LIST_ARRAY.h"
#include "../Arrays/LIST_ARRAYS.h"
namespace PhysBAM
{

template<class T>
class RIGID_BODY_COLLISIONS
{
public:
	bool verbose;
	RIGID_BODY_SKIP_COLLISION_CHECK skip_collision_check;
	RIGID_BODY_COLLISION_MANAGER collision_manager;
protected:
	int collision_iterations;
	ARRAYS_2D<bool> collision_matrix; // indexed by rigid body id; collision_matrix(i,j)==true means body i affects body j
	int contact_iterations, contact_level_iterations, contact_pair_iterations;
	bool epsilon_scaling, epsilon_scaling_for_level;
	int shock_propagation_iterations, shock_propagation_level_iterations, shock_propagation_pair_iterations;
	int push_out_iterations, push_out_level_iterations, push_out_pair_iterations;
	bool use_freezing_with_push_out, use_gradual_push_out;
	T desired_separation_distance;
	bool rolling_friction;
	LIST_ARRAY<LIST_ARRAYS<int> > precomputed_contact_pairs_for_level;

public:
	RIGID_BODY_COLLISIONS()
		: verbose (false)
	{
		Set_Collision_Iterations();
		Set_Contact_Iterations();
		Set_Contact_Level_Iterations();
		Set_Contact_Pair_Iterations();
		Use_Epsilon_Scaling();
		Use_Epsilon_Scaling_For_Level();
		Set_Shock_Propagation_Iterations();
		Set_Shock_Propagation_Level_Iterations();
		Set_Shock_Propagation_Pair_Iterations();
		Set_Push_Out_Iterations();
		Set_Push_Out_Level_Iterations();
		Set_Push_Out_Pair_Iterations();
		Use_Freezing_With_Push_Out();
		Use_Gradual_Push_Out();
		Set_Desired_Separation_Distance();
		Use_Rolling_Friction (false);
	}

	void Set_Collision_Iterations (const int collision_iterations_input = 5)
	{
		collision_iterations = collision_iterations_input;
	}

	void Set_Contact_Iterations (const int contact_iterations_input = 10)
	{
		contact_iterations = contact_iterations_input;
	}

	void Set_Contact_Level_Iterations (const int contact_level_iterations_input = 2)
	{
		contact_level_iterations = contact_level_iterations_input;
	}

	void Set_Contact_Pair_Iterations (const int contact_pair_iterations_input = 20)
	{
		contact_pair_iterations = contact_pair_iterations_input;
	}

	void Use_Epsilon_Scaling (const bool epsilon_scaling_input = true)
	{
		epsilon_scaling = epsilon_scaling_input;
	}

	void Use_Epsilon_Scaling_For_Level (const bool epsilon_scaling_for_level_input = true)
	{
		epsilon_scaling_for_level = epsilon_scaling_for_level_input;
	}

	void Set_Shock_Propagation_Iterations (const int shock_propagation_iterations_input = 1)
	{
		shock_propagation_iterations = shock_propagation_iterations_input;
	}

	void Set_Shock_Propagation_Level_Iterations (const int shock_propagation_level_iterations_input = 2)
	{
		shock_propagation_level_iterations = shock_propagation_level_iterations_input;
	}

	void Set_Shock_Propagation_Pair_Iterations (const int shock_propagation_pair_iterations_input = 10)
	{
		shock_propagation_pair_iterations = shock_propagation_pair_iterations_input;
	}

	void Set_Push_Out_Iterations (const int push_out_iterations_input = 1)
	{
		push_out_iterations = push_out_iterations_input;
	}

	void Set_Push_Out_Level_Iterations (const int push_out_level_iterations_input = 2)
	{
		push_out_level_iterations = push_out_level_iterations_input;
	}

	void Set_Push_Out_Pair_Iterations (const int push_out_pair_iterations_input = 5)
	{
		push_out_pair_iterations = push_out_pair_iterations_input;
	}

	void Use_Gradual_Push_Out (const bool use_gradual_push_out_input = true)
	{
		use_gradual_push_out = use_gradual_push_out_input;
	}

	void Use_Freezing_With_Push_Out (const bool use_freezing_with_push_out_input = true)
	{
		use_freezing_with_push_out = use_freezing_with_push_out_input;
	}

	void Set_Desired_Separation_Distance (const T desired_separation_distance_input = 0)
	{
		desired_separation_distance = desired_separation_distance_input;
	}

	void Use_Rolling_Friction (const bool rolling_friction_input = true)
	{
		rolling_friction = rolling_friction_input;
	}

//#####################################################################
};
}
#endif
