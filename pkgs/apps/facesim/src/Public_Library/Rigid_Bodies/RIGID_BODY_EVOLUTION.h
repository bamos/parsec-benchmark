//#####################################################################
// Copyright 2004, Ron Fedkiw, Eran Guendelman.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class RIGID_BODY_EVOLUTION
//#####################################################################
#ifndef __RIGID_BODY_EVOLUTION__
#define __RIGID_BODY_EVOLUTION__

#include "../Math_Tools/constants.h"
namespace PhysBAM
{

template<class T>
class RIGID_BODY_EVOLUTION
{
protected:
	T cfl_number;
	T max_rotation_per_time_step, max_linear_movement_fraction_per_time_step;
	T minimum_dt, maximum_dt;
	T artificial_maximum_speed;
	bool use_contact_graph, use_shock_propagation, use_push_out;
	bool clamp_velocities;
	T max_linear_velocity, max_angular_velocity; // magnitudes of vectors

public:
	RIGID_BODY_EVOLUTION()
	{
		Set_CFL_Number();
		Set_Max_Rotation_Per_Time_Step();
		Set_Max_Linear_Movement_Fraction_Per_Time_Step();
		Set_Minimum_And_Maximum_Time_Step();
		Set_Artificial_Maximum_Speed();
		Use_Contact_Graph();
		Use_Shock_Propagation();
		Use_Push_Out();
		Set_Clamp_Velocities (false);
		Set_Max_Linear_Velocity();
		Set_Max_Angular_Velocity();
	}

	void Set_CFL_Number (const T cfl_number_input = .5)
	{
		cfl_number = cfl_number_input;
	}

	void Set_Max_Rotation_Per_Time_Step (const T max_rotation_per_time_step_input = .1 * pi)
	{
		max_rotation_per_time_step = max_rotation_per_time_step_input;
	}

	void Set_Max_Linear_Movement_Fraction_Per_Time_Step (const T max_linear_movement_fraction_per_time_step_input = .1)
	{
		max_linear_movement_fraction_per_time_step = max_linear_movement_fraction_per_time_step_input;
	}

	void Set_Minimum_And_Maximum_Time_Step (const T minimum_dt_input = 0, const T maximum_dt_input = 1. / 24)
	{
		minimum_dt = minimum_dt_input;
		maximum_dt = maximum_dt_input;
	}

	void Set_Artificial_Maximum_Speed (const T artificial_maximum_speed_input = 0)
	{
		artificial_maximum_speed = artificial_maximum_speed_input;
	}

	void Use_Contact_Graph (const bool use_contact_graph_input = true)
	{
		use_contact_graph = use_contact_graph_input;
	}

	void Use_Shock_Propagation (const bool use_shock_propagation_input = true)
	{
		use_shock_propagation = use_shock_propagation_input;
	}

	void Use_Push_Out (const bool use_push_out_input = true)
	{
		use_push_out = use_push_out_input;
	}

	void Set_Clamp_Velocities (const bool clamp_velocities_input = true)
	{
		clamp_velocities = clamp_velocities_input;
	}

	void Set_Max_Linear_Velocity (const T max_linear_velocity_input = 0)
	{
		max_linear_velocity = max_linear_velocity_input;
	}

	void Set_Max_Angular_Velocity (const T max_angular_velocity_input = 0)
	{
		max_angular_velocity = max_angular_velocity_input;
	}

//#####################################################################
};
}
#endif
