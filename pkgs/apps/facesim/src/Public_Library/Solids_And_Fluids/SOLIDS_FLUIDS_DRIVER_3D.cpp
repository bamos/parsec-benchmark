//#####################################################################
// Copyright 2004, Ron Fedkiw, Eran Guendelman, Geoffrey Irving, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SOLIDS_FLUIDS_DRIVER_3D
//#####################################################################
#include "SOLIDS_FLUIDS_DRIVER_3D.h"
#include "../Rigid_Bodies/RIGID_BODY_EVOLUTION_3D.h"
#include "../Utilities/LOG.h"
using namespace PhysBAM;
//#####################################################################
// Function Initialize
//#####################################################################
template<class T, class RW> void SOLIDS_FLUIDS_DRIVER_3D<T, RW>::
Initialize()
{
	SOLIDS_FLUIDS_DRIVER<T>::Initialize();
	example.solids_parameters.rigid_body_parameters.Set_Rigid_Body_Parameters_Callbacks (example);
	solids_evolution.Set_Solids_Evolution_Callbacks (example);
	example.Initialize_Bodies();
	solids_evolution.time = time;
	example.solids_parameters.deformable_body_parameters.list.Set_External_Forces_And_Velocities (example);
	example.solids_parameters.rigid_body_parameters.list.Set_External_Forces_And_Velocities (example);
	solids_evolution.Initialize_Deformable_Objects (example.frame_rate, example.restart, example.verbose_dt);

	if (example.restart)
	{
		example.Read_Output_Files_Solids (example.restart_frame);
		solids_evolution.time = time = example.initial_time + example.restart_frame / example.frame_rate;
	}

	if (example.solids_parameters.fracture) example.Initialize_Fracture();

	solids_evolution.Initialize_Rigid_Bodies (example.frame_rate);

	// preroll
	if (!example.restart && example.solids_parameters.preroll_frames) Preroll_Solids (example.first_frame - example.solids_parameters.preroll_frames);
}
//#####################################################################
// Function Preroll_Solids
//#####################################################################
template<class T, class RW> void SOLIDS_FLUIDS_DRIVER_3D<T, RW>::
Preroll_Solids (const int preroll_frame)
{
	int target_frame = current_frame;
	solids_evolution.time = time = Time_At_Frame (preroll_frame);

	for (current_frame = preroll_frame; current_frame < target_frame; current_frame++)
	{
		if (example.write_output_files) Write_Output_Files (current_frame);

		LOG::Push_Scope ("FRAME", "Preroll frame %d", current_frame + 1);
		solids_evolution.Advance_To_Target_Time (Time_At_Frame (current_frame + 1), example.verbose_dt);
		time = Time_At_Frame (current_frame + 1);

		if (example.verbose) std::cout << "TIME = " << time << std::endl;

		LOG::Pop_Scope();

	}
}
//#####################################################################
// Function Advance_To_Target_Time
//#####################################################################
template<class T, class RW> void SOLIDS_FLUIDS_DRIVER_3D<T, RW>::
Advance_To_Target_Time (const T target_time)
{
	solids_evolution.Advance_To_Target_Time (target_time, example.verbose_dt);
	time = target_time;
}
//#####################################################################
// Function Postprocess_Frame
//#####################################################################
template<class T, class RW> void SOLIDS_FLUIDS_DRIVER_3D<T, RW>::
Postprocess_Frame (const int frame)
{
	SOLIDS_FLUIDS_DRIVER<T>::Postprocess_Frame (frame);

	if (example.solids_parameters.rigid_body_parameters.print_interpenetration_statistics) solids_evolution.rigid_body_evolution->collisions.Print_Interpenetration_Statistics();
}
//#####################################################################

template class SOLIDS_FLUIDS_DRIVER_3D<float, float>;
template class SOLIDS_FLUIDS_DRIVER_3D<double, double>;
template class SOLIDS_FLUIDS_DRIVER_3D<double, float>;
