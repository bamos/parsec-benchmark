//#####################################################################
// Copyright 2004-2005, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class FACE_DRIVER
//#####################################################################
#include "FACE_DRIVER.h"
using namespace PhysBAM;
template class FACE_DRIVER<float, float>;
template class FACE_DRIVER<double, double>;
template class FACE_DRIVER<double, float>;
//#####################################################################
// Function Initialize
//#####################################################################
template<class T, class RW> void FACE_DRIVER<T, RW>::
Initialize()
{
	QUASISTATICS_DRIVER<T, RW>::Initialize();

	if (example.optimization && example.restart && example.restart_step)
	{
		example.optimization->template Read_Optimization_Data<RW> (example.output_directory + "/", example.restart_frame, example.restart_step);
		example.optimization->step = example.restart_step;
	}
}
//#####################################################################
// Function Advance_To_Target_Time
//#####################################################################
template<class T, class RW> void FACE_DRIVER<T, RW>::
Advance_To_Target_Time (const T target_time)
{

	if (example.optimization) // Use optimization for controls
	{
		assert (example.optimization_goal);

		if (example.static_target_frame == -1) example.optimization_goal->Update_Target (current_frame + 1);
		else example.optimization_goal->Update_Target (example.static_target_frame);

		example.optimization_goal->Write_Goal_Data (example.output_directory + "/", current_frame + 1);

		if (example.restart && current_frame == example.restart_frame) example.optimization->step = example.restart_step;
		else example.optimization->step = 0;

		for (example.optimization->step++; example.optimization->step <= example.optimization->optimization_iterations; example.optimization->step++)
		{
			example.control_parameters.Print_Diagnostics();
			example.control_parameters.Save_Controls();
			example.optimization->Optimization_Step();
			ARRAY<VECTOR_3D<T> >& positions = example.solids_parameters.deformable_body_parameters.list.deformable_objects (1)->particles.X.array;
			example.control_parameters.Print_Diagnostics();
			example.control_parameters.Kinematically_Update_Positions (positions);
			T time_save = solids_evolution.time;
			QUASISTATICS_DRIVER<T, RW>::Advance_To_Target_Time (target_time);
			solids_evolution.time = time_save;
			std::cout << "Optimization functional : " << example.optimization->Optimization_Functional() << std::endl;

			for (int i = 1; i <= example.optimization->jacobian_iterations; i++)
			{
				example.optimization->Update_Jacobian();
				example.control_parameters.Save_Controls();
			}

			example.optimization->template Write_Optimization_Data<RW> (example.output_directory + "/", current_frame, example.optimization->step);

			if (example.write_last_step) Write_Last_Step (current_frame, example.optimization->step);
		}

		solids_evolution.time = target_time;
	}
	else   // Simulate existing control profile
	{
		ARRAY<VECTOR_3D<T> >& positions = example.solids_parameters.deformable_body_parameters.list.deformable_objects (1)->particles.X.array;
		example.control_parameters.Save_Controls();
		example.Set_External_Controls (target_time);

		if (example.verbose) example.control_parameters.Print_Diagnostics();

		example.control_parameters.Kinematically_Update_Positions (positions);
		// Start ROI
		QUASISTATICS_DRIVER<T, RW>::Advance_To_Target_Time (target_time);
		// End ROI
	}
}
//#####################################################################
