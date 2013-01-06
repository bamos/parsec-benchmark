//#####################################################################
// Copyright 2004-2005, Igor Neverov, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class FACE_OPTIMIZATION
//#####################################################################
#ifndef __FACE_OPTIMIZATION__
#define __FACE_OPTIMIZATION__

#include "../../Public_Library/Deformable_Objects/DEFORMABLE_OBJECT_3D.h"
#include "../../Public_Library/Forces_And_Torques/SOLIDS_FORCES.h"
#include "../../Public_Library/Solids_And_Fluids/SOLIDS_EVOLUTION_CALLBACKS.h"
#include "FACE_CONTROL_PARAMETERS.h"

namespace PhysBAM
{

template <class T>
class FACE_OPTIMIZATION: public EXTERNAL_FORCES_AND_VELOCITIES<T, VECTOR_3D<T> >, public SOLIDS_EVOLUTION_CALLBACKS<T, VECTOR_3D<T> >
{
public:
	DEFORMABLE_OBJECT_3D<T>& deformable_object;
	EXTERNAL_FORCES_AND_VELOCITIES<T, VECTOR_3D<T> >& default_external_forces_and_velocities;
	SOLIDS_EVOLUTION_CALLBACKS<T, VECTOR_3D<T> >* solids_evolution_callbacks;
	FACE_CONTROL_PARAMETERS<T>& control_parameters;
	ARRAY<ARRAY<VECTOR_3D<T> > > jacobian;
	ARRAY<VECTOR_3D<T> > control_force_jacobian;
	int step;
	T jacobian_cg_tolerance;
	int jacobian_cg_iterations, jacobian_iterations, optimization_iterations;

	FACE_OPTIMIZATION (DEFORMABLE_OBJECT_3D<T> &deformable_object_input, FACE_CONTROL_PARAMETERS<T>& control_parameters_input,
			   EXTERNAL_FORCES_AND_VELOCITIES<T, VECTOR_3D<T> >& default_external_forces_and_velocities_input)
		: deformable_object (deformable_object_input), default_external_forces_and_velocities (default_external_forces_and_velocities_input), solids_evolution_callbacks (0),
		  control_parameters (control_parameters_input), step (0), jacobian_cg_tolerance ( (T) 1e-2), jacobian_iterations (5), jacobian_cg_iterations (100), optimization_iterations (1)
	{}

	virtual ~FACE_OPTIMIZATION()
	{}

	template <class RW>
	void Write_Optimization_Data (const std::string& output_prefix, const int frame_input, const int step_input = 0) const
	{
		std::string f = STRING_UTILITIES::string_sprintf (".%d", frame_input), s = (step_input > 0) ? STRING_UTILITIES::string_sprintf (".step_%d", step_input) : "";
		FILE_UTILITIES::Write_To_File<RW> (output_prefix + "optimization_positions" + f + s, deformable_object.particles.X.array);
		FILE_UTILITIES::Write_To_File<RW> (output_prefix + "optimization_jacobian" + f + s, jacobian);
		FILE_UTILITIES::Write_To_File<RW> (output_prefix + "optimization_controls" + f + s, control_parameters);
		Write_Optimization_Goal_Data (output_prefix, frame_input, step_input);
	}

	template <class RW>
	void Read_Optimization_Data (const std::string& output_prefix, const int frame_input, const int step_input = 0)
	{
		std::string f = STRING_UTILITIES::string_sprintf (".%d", frame_input), s = (step_input > 0) ? STRING_UTILITIES::string_sprintf (".step_%d", step_input) : "";
		FILE_UTILITIES::Read_From_File<RW> (output_prefix + "optimization_positions" + f + s, deformable_object.particles.X.array);
		FILE_UTILITIES::Read_From_File<RW> (output_prefix + "optimization_jacobian" + f + s, jacobian);
		FILE_UTILITIES::Read_From_File<RW> (output_prefix + "optimization_controls" + f + s, control_parameters);
		control_parameters.Save_Controls();
	}

	template <class RW>
	void Write_Optimization_Controls_And_Positions (const std::string& output_prefix, const int frame_input, const int step_input = 0) const
	{
		std::string f = STRING_UTILITIES::string_sprintf (".%d", frame_input), s = (step_input > 0) ? STRING_UTILITIES::string_sprintf (".step_%d", step_input) : "";
		FILE_UTILITIES::Write_To_File<RW> (output_prefix + "optimization_positions" + f + s, deformable_object.particles.X.array);
		FILE_UTILITIES::Write_To_File<RW> (output_prefix + "optimization_controls" + f + s, control_parameters);
		Write_Optimization_Goal_Data (output_prefix, frame_input, step_input);
	}

	template <class RW>
	void Read_Optimization_Controls_And_Positions (const std::string& output_prefix, const int frame_input, const int step_input = 0)
	{
		std::string f = STRING_UTILITIES::string_sprintf (".%d", frame_input), s = (step_input > 0) ? STRING_UTILITIES::string_sprintf (".step_%d", step_input) : "";
		FILE_UTILITIES::Read_From_File<RW> (output_prefix + "optimization_positions" + f + s, deformable_object.particles.X.array);
		FILE_UTILITIES::Read_From_File<RW> (output_prefix + "optimization_controls" + f + s, control_parameters);
	}

	template <class RW>
	void Write_Jacobian (const std::string& output_prefix, const int frame_input, const int step_input = 0) const
	{
		std::string f = STRING_UTILITIES::string_sprintf (".%d", frame_input), s = (step_input > 0) ? STRING_UTILITIES::string_sprintf (".step_%d", step_input) : "";
		FILE_UTILITIES::Write_To_File<RW> (output_prefix + "optimization_jacobian" + f + s, jacobian);
	}

	template <class RW>
	void Read_Jacobian (const std::string& output_prefix, const int frame_input, const int step_input = 0)
	{
		std::string f = STRING_UTILITIES::string_sprintf (".%d", frame_input), s = (step_input > 0) ? STRING_UTILITIES::string_sprintf (".step_%d", step_input) : "";
		FILE_UTILITIES::Read_From_File<RW> (output_prefix + "optimization_jacobian" + f + s, jacobian);
	}

	void Default() const
	{
		std::cout << "THIS FACE_OPTIMIZATION FUNCTION IS NOT DEFINED!" << std::endl;
	}

//#####################################################################
	virtual bool Optimization_Step()
	{
		Default();
		exit (1);
	}
	virtual T Optimization_Functional() const
	{
		Default();
		exit (1);
	}
	virtual void Write_Optimization_Goal_Data (const std::string& output_prefix, const int frame_input, const int step_input = 0) const
	{
		Default();
		exit (1);
	}
	void Update_Jacobian (bool resize_jacobians_of_inactive_controls_to_zero = true, bool verbose = true);
	void Add_External_Forces (ARRAY<VECTOR_3D<T> >& F, const T time, const int id_number = 1);
	void Set_External_Positions (ARRAY<VECTOR_3D<T> >& X, const T time, const int id_number);
	void Zero_Out_Enslaved_Position_Nodes (ARRAY<VECTOR_3D<T> >& X, const T time, const int id_number);
	void Update_Collision_Body_Positions_And_Velocities (const T time);
	//#####################################################################
};
}
#endif
