//#####################################################################
// Copyright 2005, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class STORYTELLING_EXAMPLE
//#####################################################################
#ifndef __STORYTELLING_EXAMPLE__
#define __STORYTELLING_EXAMPLE__

#include "../FACE_EXAMPLE.h"
#include "../../../Public_Library/Constitutive_Models/DIAGONALIZED_FACE_3D.h"
#include "../../../Public_Library/Collisions_And_Interactions/COLLISION_PENALTY_FORCES.h"

namespace PhysBAM
{

template <class T, class RW>
class STORYTELLING_EXAMPLE: public FACE_EXAMPLE<T, RW>
{
public:
	using SOLIDS_FLUIDS_EXAMPLE<T>::output_directory;
	using SOLIDS_FLUIDS_EXAMPLE<T>::data_directory;
	using FACE_EXAMPLE<T, RW>::model_directory;
	using FACE_EXAMPLE<T, RW>::input_directory;
	using SOLIDS_FLUIDS_EXAMPLE<T>::frame_rate;
	using SOLIDS_FLUIDS_EXAMPLE<T>::restart;
	using SOLIDS_FLUIDS_EXAMPLE<T>::restart_frame;
	using SOLIDS_FLUIDS_EXAMPLE<T>::last_frame;
	using SOLIDS_FLUIDS_EXAMPLE_3D<T, RW>::solids_parameters;
	using FACE_EXAMPLE<T, RW>::newton_tolerance;
	using FACE_EXAMPLE<T, RW>::newton_iterations;
	using FACE_EXAMPLE<T, RW>::use_rigid_body_collision;
	using FACE_EXAMPLE<T, RW>::use_self_collision;
	using FACE_EXAMPLE<T, RW>::use_partially_converged_result;
	using FACE_EXAMPLE<T, RW>::number_of_muscles;
	using FACE_EXAMPLE<T, RW>::attached_nodes;
	using FACE_EXAMPLE<T, RW>::jaw_attachment_index;
	using FACE_EXAMPLE<T, RW>::control_parameters;
	using SOLIDS_FLUIDS_EXAMPLE<T>::verbose;
	std::string control_directory;
	T control_frame_rate;

	FACE_CONTROL_PARAMETERS<T> input_control_parameters;
	ACTIVATION_CONTROL_SET<T> *input_activation_controls;
	ATTACHMENT_FRAME_CONTROL_SET<T> *input_attachment_frame_controls;

	STORYTELLING_EXAMPLE()
	{
		output_directory = "Storytelling/output";
		control_directory = data_directory + "/Face_Data/Motion_Data/Storytelling_Controls";

		// Simulation source
		model_directory = data_directory + "/Face_Data/Eftychis_840k";
		input_directory = model_directory + "/Front_370k";

		// Control settings
		control_frame_rate = frame_rate;

		// Restart and initialization settings
		restart = false;
		restart_frame = 0;
		last_frame = 1;

		// Simulation settings
		solids_parameters.cg_tolerance = (T) 2e-3;
		solids_parameters.cg_iterations = 200;
		//newton_tolerance=(T)5e-2;
		//newton_iterations=10;
		newton_tolerance = (T) 500;
		newton_iterations = 1;

		// Collision settings
		use_rigid_body_collision = true;
		use_self_collision = false;
		solids_parameters.perform_collision_body_collisions = use_rigid_body_collision || use_self_collision;

		// Solver settings
		use_partially_converged_result = true;

		// Debugging settings
		solids_parameters.deformable_body_parameters.print_residuals = true;
		verbose = true;

	}

	~STORYTELLING_EXAMPLE()
	{}

//#####################################################################
// Function Initialize_Bodies
//#####################################################################
	void Initialize_Bodies()
	{
		FACE_EXAMPLE<T, RW>::Initialize_Bodies();

		DEFORMABLE_OBJECT_3D<T>& deformable_object = solids_parameters.deformable_body_parameters.list (1);
		input_activation_controls = new ACTIVATION_CONTROL_SET<T>;

		for (int i = 1; i <= number_of_muscles; i++) input_activation_controls->Add_Activation();

		input_control_parameters.list.Append_Element (input_activation_controls);
		input_attachment_frame_controls = new ATTACHMENT_FRAME_CONTROL_SET<T> (deformable_object.particles.X.array, attached_nodes, jaw_attachment_index);
		input_attachment_frame_controls->template Initialize_Jaw_Joint_From_File<RW> (input_directory + "/jaw_joint_parameters");
		input_control_parameters.list.Append_Element (input_attachment_frame_controls);

	}
//#####################################################################
// Function Set_External_Controls
//#####################################################################
	void Set_External_Controls (const T time)
	{
		T fractional_control_frame = time * control_frame_rate;
		int integer_control_frame = (int) fractional_control_frame;
		T interpolation_fraction = fractional_control_frame - (T) integer_control_frame;
		std::string last_control_filename = control_directory + "/storytelling_controls." + STRING_UTILITIES::string_sprintf ("%d", integer_control_frame);
		std::string next_control_filename = control_directory + "/storytelling_controls." + STRING_UTILITIES::string_sprintf ("%d", integer_control_frame + 1);
		FILE_UTILITIES::Read_From_File<RW> (last_control_filename, input_control_parameters);
		input_control_parameters.Save_Controls();
		FILE_UTILITIES::Read_From_File<RW> (next_control_filename, input_control_parameters);
		input_control_parameters.Interpolate (interpolation_fraction);
		VECTOR_ND<T> input_controls;
		input_control_parameters.Get (input_controls);
		control_parameters.Set (input_controls);
	}
//#####################################################################
};
}
#endif
