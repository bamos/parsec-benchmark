//#####################################################################
// Copyright 2004-2005, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class FACE_OPTIMIZATION
//#####################################################################
#include "FACE_OPTIMIZATION.h"
using namespace PhysBAM;
//#####################################################################
// Function Advance_To_Target_Time
//#####################################################################
template<class T> void FACE_OPTIMIZATION<T>::
Update_Jacobian (bool resize_jacobians_of_inactive_controls_to_zero, bool verbose)
{
	deformable_object.Set_External_Forces_And_Velocities (*this);
	jacobian.Resize_Array (control_parameters.Size());

	if (control_parameters.Active_Nonkinematic_Size())
	{
		deformable_object.Enforce_Definiteness (false);
		deformable_object.Update_Position_Based_State();
		deformable_object.Update_Collision_Penalty_Forces_And_Derivatives();
	}

	for (int i = 1; i <= control_parameters.Size(); i++)
	{
		if (verbose) std::cout << "Updating jacobian for control parameter " << i << std::endl;

		if (!control_parameters.Active (i))
		{
			if (resize_jacobians_of_inactive_controls_to_zero) jacobian (i).Resize_Array (0);
		}
		else if (control_parameters.Active_Kinematic (i))
		{
			jacobian (i).Resize_Array (deformable_object.particles.number);
			control_parameters.Position_Derivative (jacobian (i), i);
		}
		else
		{
			control_force_jacobian.Resize_Array (deformable_object.particles.number);
			control_parameters.Force_Derivative (control_force_jacobian, i);
			jacobian (i).Resize_Array (deformable_object.particles.number);
			control_parameters.Kinematically_Update_Jacobian (jacobian (i));
			deformable_object.One_Newton_Step_Toward_Steady_State (jacobian_cg_tolerance, jacobian_cg_iterations, 0, jacobian (i), true, 0, false);
		}
	}

	deformable_object.Set_External_Forces_And_Velocities (default_external_forces_and_velocities);
}
//#####################################################################
// Funciton Add_External_Forces
//#####################################################################
template<class T> void FACE_OPTIMIZATION<T>::
Add_External_Forces (ARRAY<VECTOR_3D<T> >& F, const T time, const int id_number)
{
	F += control_force_jacobian;
}
//#####################################################################
// Set_External_Positions
//#####################################################################
template<class T> void FACE_OPTIMIZATION<T>::
Set_External_Positions (ARRAY<VECTOR_3D<T> >& X, const T time, const int id_number)
{
	default_external_forces_and_velocities.Set_External_Positions (X, time, id_number);
}
//#####################################################################
// Zero_Out_Enslaved_Position_Nodes
//#####################################################################
template<class T> void FACE_OPTIMIZATION<T>::
Zero_Out_Enslaved_Position_Nodes (ARRAY<VECTOR_3D<T> >& X, const T time, const int id_number)
{
	default_external_forces_and_velocities.Zero_Out_Enslaved_Position_Nodes (X, time, id_number);
}
//#####################################################################
// Function Update_Collision_Body_Positions_And_Velocities
//#####################################################################
template<class T> void FACE_OPTIMIZATION<T>::
Update_Collision_Body_Positions_And_Velocities (const T time)
{
	if (solids_evolution_callbacks) solids_evolution_callbacks->Update_Collision_Body_Positions_And_Velocities (time);
}
//#####################################################################

template class FACE_OPTIMIZATION<float>;
template class FACE_OPTIMIZATION<double>;

