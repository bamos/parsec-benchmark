//#####################################################################
// Copyright 2004-2005, Igor Neverov, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class LANDMARK_PROXIMITY_OPTIMIZATION
//#####################################################################
#ifndef __LANDMARK_PROXIMITY_OPTIMIZATION__
#define __LANDMARK_PROXIMITY_OPTIMIZATION__

#include "../../Public_Library/Matrices_And_Vectors/MATRIX_MXN.h"
#include "FACE_OPTIMIZATION.h"
#include "FACE_LANDMARK_OPTIMIZATION_GOAL.h"

namespace PhysBAM
{

template <class T>
class LANDMARK_PROXIMITY_OPTIMIZATION: public FACE_OPTIMIZATION<T>
{
public:
	using FACE_OPTIMIZATION<T>::control_parameters;
	using FACE_OPTIMIZATION<T>::deformable_object;
	using FACE_OPTIMIZATION<T>::jacobian;
	using FACE_OPTIMIZATION<T>::Update_Jacobian;
	using FACE_OPTIMIZATION<T>::Update_Collision_Body_Positions_And_Velocities;

	FACE_LANDMARK_OPTIMIZATION_GOAL<T>& landmark_optimization_goal;
	T linesearch_cg_tolerance, linesearch_newton_tolerance;
	int linesearch_cg_iterations, linesearch_newton_iterations, linesearch_depth;
	T penalty_ramping_factor, penalty_ramping_step;
	ARRAY<T> proximity_weights;
	bool use_golden_section_search;

	LANDMARK_PROXIMITY_OPTIMIZATION (DEFORMABLE_OBJECT_3D<T> &deformable_object_input, FACE_CONTROL_PARAMETERS<T>& control_parameters_input,
					 EXTERNAL_FORCES_AND_VELOCITIES<T, VECTOR_3D<T> >&default_external_forces_and_velocities_input, FACE_LANDMARK_OPTIMIZATION_GOAL<T>& landmark_optimization_goal_input)
		: FACE_OPTIMIZATION<T> (deformable_object_input, control_parameters_input, default_external_forces_and_velocities_input), landmark_optimization_goal (landmark_optimization_goal_input),
		  linesearch_cg_tolerance ( (T) 1e-3), linesearch_newton_tolerance ( (T) 1e-2), linesearch_cg_iterations (100), linesearch_newton_iterations (20), linesearch_depth (6),
		  proximity_weights (0), penalty_ramping_factor ( (T) 1e6), penalty_ramping_step ( (T) 1.01), use_golden_section_search (true)
	{}

	void Write_Optimization_Goal_Data (const std::string& output_prefix, const int frame_input, const int step_input = 0) const
	{
		std::string f = STRING_UTILITIES::string_sprintf (".%d", frame_input), s = (step_input > 0) ? STRING_UTILITIES::string_sprintf (".step_%d", step_input) : "";
		assert (landmark_optimization_goal.target_landmarks.m == landmark_optimization_goal.embedded_landmarks.m);
		ARRAY<VECTOR_3D<T> > embedded_landmark_positions (landmark_optimization_goal.embedded_landmarks.m);

		for (int i = 1; i <= landmark_optimization_goal.embedded_landmarks.m; i++)
			embedded_landmark_positions (i) = landmark_optimization_goal.embedded_landmarks (i).Evaluate (deformable_object.particles.X.array);

		FILE_UTILITIES::Write_To_File<T> (output_prefix + "embedded_markers" + f + s, embedded_landmark_positions);
	}

	void Default() const
	{
		std::cout << "THIS LANDMARK_PROXIMITY_OPTIMIZATION FUNCTION IS NOT DEFINED!" << std::endl;
	}

//#####################################################################
	bool Optimization_Step();
	void Perform_Linesearch_Sampling();
	void Perform_Golden_Section_Search();
	T Simulate_And_Optimize_Kinematics (ARRAY<VECTOR_3D<T> >& sample_positions, VECTOR_ND<T>& sample_controls, const ARRAY<int>& active_subset,
					    const ARRAY<int>& active_kinematic_subset, const VECTOR_ND<T>& original_active_kinematic_controls, const T sample_fraction);
	T Optimization_Functional() const;
	VECTOR_ND<T> Landmark_To_Target_Distances() const;
	T Optimize_Kinematic_Parameters (const ARRAY<VECTOR_3D<T> >& sample_positions, VECTOR_ND<T>& sample_controls);
	void Simulate_Linesearch_Sequence (ARRAY<ARRAY<VECTOR_3D<T> > >& linesearch_positions, ARRAY<VECTOR_ND<T> >& linesearch_controls, ARRAY<T>& linesearch_fractions,
					   const VECTOR_ND<T>& original_controls, const VECTOR_ND<T>& final_controls, const T lower_fraction, const T upper_fraction, const int levels);
	void Compute_Linearized_Proximity_Error (MATRIX_NXN<T>& optimization_jacobian, VECTOR_ND<T>& optimization_residual, T& optimization_min_error);
	VECTOR_ND<T> Compute_Gauss_Newton_Update (T* proximity_error_after_optimization = 0, bool verbose = true);
//#####################################################################
};
}
#endif
