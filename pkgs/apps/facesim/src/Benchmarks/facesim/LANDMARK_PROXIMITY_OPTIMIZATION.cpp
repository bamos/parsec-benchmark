//#####################################################################
// Copyright 2004-2005, Igor Neverov, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class LANDMARK_PROXIMITY_OPTIMIZATION
//#####################################################################
#include "LANDMARK_PROXIMITY_OPTIMIZATION.h"
#include "../../Public_Library/Interpolation/LINEAR_INTERPOLATION.h"
#include <iomanip>
using namespace PhysBAM;
template class LANDMARK_PROXIMITY_OPTIMIZATION<float>;
template class LANDMARK_PROXIMITY_OPTIMIZATION<double>;
//#####################################################################
// Function Optimization_Step
//#####################################################################
template<class T> bool LANDMARK_PROXIMITY_OPTIMIZATION<T>::
Optimization_Step()
{
	if (use_golden_section_search) Perform_Golden_Section_Search();
	else Perform_Linesearch_Sampling();

	return true;
}
//#####################################################################
// Function Perform_Linesearch_Sampling
//#####################################################################
template<class T> void LANDMARK_PROXIMITY_OPTIMIZATION<T>::
Perform_Linesearch_Sampling()
{
	VECTOR_ND<T> original_controls;
	ARRAY<int> active_subset (control_parameters.Active_Subset());
	control_parameters.Get (original_controls, active_subset);
	VECTOR_ND<T> controls_update = Compute_Gauss_Newton_Update();
	VECTOR_ND<T> final_controls = original_controls + controls_update;
	ARRAY<ARRAY<VECTOR_3D<T> > > linesearch_positions;
	ARRAY<VECTOR_ND<T> > linesearch_controls;
	ARRAY<T> linesearch_fractions;
	std::cout << "Gauss Newton Update before linesearch" << std::endl;

	for (int i = 1; i <= active_subset.m; i++) std::cout << "#" << std::setw (2) << active_subset (i) << " : Old value " << std::setw (13) << original_controls (i)
				<< "   New value " << std::setw (13) << final_controls (i) << "   Difference " << std::setw (13) << controls_update (i) << std::endl;

	control_parameters.Set (final_controls, active_subset);
	control_parameters.Print_Diagnostics();
	control_parameters.Set (original_controls, active_subset);
	Simulate_Linesearch_Sequence (linesearch_positions, linesearch_controls, linesearch_fractions, original_controls, final_controls, 0, (T) 1, linesearch_depth);
	ARRAY<T> linesearch_cost_values (linesearch_positions.m);

	for (int i = 1; i <= linesearch_positions.m; i++)
	{
		std::cout << "Optimizing kinematics for linesearch value : " << linesearch_fractions (i) << std::endl;
		linesearch_cost_values (i) = Optimize_Kinematic_Parameters (linesearch_positions (i), linesearch_controls (i));
		std::cout << "Cost function value : " << linesearch_cost_values (i) << "; Q=" << Optimization_Functional() << std::endl;
	}

	int optimal_sample = ARRAY<T>::argmin (linesearch_cost_values);
	std::cout << "Optimal update fraction = " << linesearch_fractions (optimal_sample) << std::endl;
	deformable_object.particles.X.array = linesearch_positions (optimal_sample);
	control_parameters.Set (linesearch_controls (optimal_sample), control_parameters.Active_Subset());
}
//#####################################################################
// Function Perform_Golden_Section_Search
//#####################################################################
template<class T> void LANDMARK_PROXIMITY_OPTIMIZATION<T>::
Perform_Golden_Section_Search()
{
	const T golden_section_inverse = (sqrt ( (T) 5) - 1) / 2;
	VECTOR_ND<T> original_active_controls, original_active_kinematic_controls;
	ARRAY<int> active_subset (control_parameters.Active_Subset()), active_kinematic_subset (control_parameters.Active_Kinematic_Subset());
	control_parameters.Get (original_active_controls, active_subset);
	control_parameters.Get (original_active_kinematic_controls, active_kinematic_subset);
	VECTOR_ND<T> controls_update = Compute_Gauss_Newton_Update();
	ARRAY<ARRAY<VECTOR_3D<T> > > linesearch_positions (4);
	ARRAY<VECTOR_ND<T> > linesearch_controls (4);
	ARRAY<T> linesearch_fractions (4);
	ARRAY<T> linesearch_cost_values (4);
	linesearch_fractions (1) = 0;
	linesearch_fractions (2) = (T) 1 - golden_section_inverse;
	linesearch_fractions (3) = golden_section_inverse;
	linesearch_fractions (4) = (T) 1;
	linesearch_controls (1) = original_active_controls;
	linesearch_controls (4) = original_active_controls + controls_update;
	linesearch_positions (1) = linesearch_positions (2) = linesearch_positions (3) = linesearch_positions (4) = deformable_object.particles.X.array;
	linesearch_cost_values (1) = Simulate_And_Optimize_Kinematics (linesearch_positions (1), linesearch_controls (1), active_subset, active_kinematic_subset, original_active_kinematic_controls, 0);
	linesearch_cost_values (4) = Simulate_And_Optimize_Kinematics (linesearch_positions (4), linesearch_controls (4), active_subset, active_kinematic_subset, original_active_kinematic_controls, (T) 1);

	if (linesearch_depth)
	{
		linesearch_controls (2) = LINEAR_INTERPOLATION<T, VECTOR_ND<T> >::Linear (linesearch_controls (1), linesearch_controls (4), (T) 1 - golden_section_inverse);
		linesearch_controls (3) = LINEAR_INTERPOLATION<T, VECTOR_ND<T> >::Linear (linesearch_controls (1), linesearch_controls (4), golden_section_inverse);
		ARRAY<VECTOR_3D<T> >::copy (golden_section_inverse, linesearch_positions (1), (T) 1 - golden_section_inverse, linesearch_positions (4), linesearch_positions (2));
		linesearch_cost_values (2) = Simulate_And_Optimize_Kinematics (linesearch_positions (2), linesearch_controls (2), active_subset, active_kinematic_subset, original_active_kinematic_controls, (T) 1 - golden_section_inverse);
		ARRAY<VECTOR_3D<T> >::copy (golden_section_inverse, linesearch_positions (2), (T) 1 - golden_section_inverse, linesearch_positions (4), linesearch_positions (3));
		linesearch_cost_values (3) = Simulate_And_Optimize_Kinematics (linesearch_positions (3), linesearch_controls (3), active_subset, active_kinematic_subset, original_active_kinematic_controls, golden_section_inverse);
	}
	else linesearch_cost_values (2) = linesearch_cost_values (3) = FLT_MAX;

	for (int current_depth = 2; current_depth <= linesearch_depth; current_depth++)
	{
		for (int i = 1; i <= 4; i++) std::cout << "Proximity error for linesearch parameter " << linesearch_fractions (i) << " : " << linesearch_cost_values (i) << std::endl;

		if (linesearch_cost_values (2) > linesearch_cost_values (3))
		{
			linesearch_positions (1) = linesearch_positions (2);
			linesearch_controls (1) = linesearch_controls (2);
			linesearch_fractions (1) = linesearch_fractions (2);
			linesearch_cost_values (1) = linesearch_cost_values (2);
			linesearch_positions (2) = linesearch_positions (3);
			linesearch_controls (2) = linesearch_controls (3);
			linesearch_fractions (2) = linesearch_fractions (3);
			linesearch_cost_values (2) = linesearch_cost_values (3);
			ARRAY<VECTOR_3D<T> >::copy (golden_section_inverse, linesearch_positions (2), (T) 1 - golden_section_inverse, linesearch_positions (4), linesearch_positions (3));
			linesearch_controls (3) = LINEAR_INTERPOLATION<T, VECTOR_ND<T> >::Linear (linesearch_controls (2), linesearch_controls (4), (T) 1 - golden_section_inverse);
			linesearch_fractions (3) = LINEAR_INTERPOLATION<T, T>::Linear (linesearch_fractions (2), linesearch_fractions (4), (T) 1 - golden_section_inverse);
			linesearch_cost_values (3) = Simulate_And_Optimize_Kinematics (linesearch_positions (3), linesearch_controls (3), active_subset, active_kinematic_subset, original_active_kinematic_controls, linesearch_fractions (3));
		}
		else
		{
			linesearch_positions (4) = linesearch_positions (3);
			linesearch_controls (4) = linesearch_controls (3);
			linesearch_fractions (4) = linesearch_fractions (3);
			linesearch_cost_values (4) = linesearch_cost_values (3);
			linesearch_positions (3) = linesearch_positions (2);
			linesearch_controls (3) = linesearch_controls (2);
			linesearch_fractions (3) = linesearch_fractions (2);
			linesearch_cost_values (3) = linesearch_cost_values (2);
			ARRAY<VECTOR_3D<T> >::copy ( (T) 1 - golden_section_inverse, linesearch_positions (1), golden_section_inverse, linesearch_positions (3), linesearch_positions (2));
			linesearch_controls (2) = LINEAR_INTERPOLATION<T, VECTOR_ND<T> >::Linear (linesearch_controls (1), linesearch_controls (3), golden_section_inverse);
			linesearch_fractions (2) = LINEAR_INTERPOLATION<T, T>::Linear (linesearch_fractions (1), linesearch_fractions (3), golden_section_inverse);
			linesearch_cost_values (2) = Simulate_And_Optimize_Kinematics (linesearch_positions (2), linesearch_controls (2), active_subset, active_kinematic_subset, original_active_kinematic_controls, linesearch_fractions (2));
		}
	}

	int optimal_sample = ARRAY<T>::argmin (linesearch_cost_values);
	std::cout << "Optimal update fraction = " << linesearch_fractions (optimal_sample) << std::endl;
	deformable_object.particles.X.array = linesearch_positions (optimal_sample);
	control_parameters.Set (linesearch_controls (optimal_sample), control_parameters.Active_Subset());
}
//#####################################################################
// Function Simulate_And_Optimize_Kinematics
//#####################################################################
template<class T> T LANDMARK_PROXIMITY_OPTIMIZATION<T>::
Simulate_And_Optimize_Kinematics (ARRAY<VECTOR_3D<T> >& sample_positions, VECTOR_ND<T>& sample_controls, const ARRAY<int>& active_subset, const ARRAY<int>& active_kinematic_subset,
				  const VECTOR_ND<T>& original_active_kinematic_controls, const T sample_fraction)
{
	control_parameters.Set (sample_controls, active_subset);
	control_parameters.Set (original_active_kinematic_controls, active_kinematic_subset);
	control_parameters.Get (sample_controls, active_subset);
	std::cout << "Solving for linesearch fraction " << sample_fraction << std::endl;
	control_parameters.Print_Diagnostics();
	deformable_object.particles.X.array = sample_positions;
	Update_Collision_Body_Positions_And_Velocities (0);
	deformable_object.Advance_One_Time_Step_Quasistatic (0, 0, linesearch_cg_tolerance, linesearch_cg_iterations, linesearch_newton_tolerance, linesearch_newton_iterations, true, true);
	sample_positions = deformable_object.particles.X.array;
	std::cout << "Optimizing kinimatics for linesearch fraction " << sample_fraction << std::endl;
	T proximity_error = Optimize_Kinematic_Parameters (sample_positions, sample_controls);
	std::cout << "Proximity error for linesearch fraction " << sample_fraction << " : " << proximity_error << std::endl;
	return proximity_error;
}
//#####################################################################
// Function Optimization_Functional
//#####################################################################
template<class T> T LANDMARK_PROXIMITY_OPTIMIZATION<T>::
Optimization_Functional() const
{
	assert (landmark_optimization_goal.target_landmarks.m == landmark_optimization_goal.embedded_landmarks.m);
	T Q = 0;

	for (int i = 1; i <= landmark_optimization_goal.embedded_landmarks.m; i++)
		Q += (landmark_optimization_goal.embedded_landmarks (i).Evaluate (deformable_object.particles.X.array) - landmark_optimization_goal.target_landmarks (i)).Magnitude_Squared();

	return Q;
}
//#####################################################################
// Function Landmark_To_Target_Distances
//#####################################################################
template<class T> VECTOR_ND<T> LANDMARK_PROXIMITY_OPTIMIZATION<T>::
Landmark_To_Target_Distances() const
{
	assert (landmark_optimization_goal.target_landmarks.m == landmark_optimization_goal.embedded_landmarks.m);
	VECTOR_ND<T> distances (landmark_optimization_goal.embedded_landmarks.m);

	for (int i = 1; i <= landmark_optimization_goal.embedded_landmarks.m; i++)
		distances (i) = (landmark_optimization_goal.embedded_landmarks (i).Evaluate (deformable_object.particles.X.array) - landmark_optimization_goal.target_landmarks (i)).Magnitude();

	return distances;
}
//#####################################################################
// Function Optimize_Kinematic_Parameters
//#####################################################################
template<class T> T LANDMARK_PROXIMITY_OPTIMIZATION<T>::
Optimize_Kinematic_Parameters (const ARRAY<VECTOR_3D<T> >& sample_positions, VECTOR_ND<T>& sample_controls)
{
	ARRAY<int> active_kinematic_subset (control_parameters.Active_Kinematic_Subset()), active_nonkinematic_subset (control_parameters.Active_Nonkinematic_Subset());
	ARRAY<int> active_subset (control_parameters.Active_Subset());
	VECTOR_ND<T> active_controls_save;
	control_parameters.Get (active_controls_save, active_subset);
	control_parameters.Set (sample_controls, active_subset);
	VECTOR_ND<T> active_kinematic_controls;
	control_parameters.Get (active_kinematic_controls, active_kinematic_subset);
	control_parameters.Set_Active (false, active_nonkinematic_subset);
	deformable_object.particles.X.array = sample_positions;
	Update_Jacobian (false, false);
	T proximity_error;
	active_kinematic_controls += Compute_Gauss_Newton_Update (&proximity_error, false);
	control_parameters.Set (active_kinematic_controls, active_kinematic_subset);
	control_parameters.Get (sample_controls, active_subset);
	control_parameters.Set (active_controls_save, active_subset);
	control_parameters.Set_Active (true, active_nonkinematic_subset);
	return proximity_error;
}
//#####################################################################
// Function Simulate_Linesearch_Sequence
//#####################################################################
template<class T> void LANDMARK_PROXIMITY_OPTIMIZATION<T>::
Simulate_Linesearch_Sequence (ARRAY<ARRAY<VECTOR_3D<T> > >& linesearch_positions, ARRAY<VECTOR_ND<T> >& linesearch_controls, ARRAY<T>& linesearch_fractions,
			      const VECTOR_ND<T>& original_controls, const VECTOR_ND<T>& final_controls, const T lower_fraction, const T upper_fraction, const int levels)
{
	int number_of_samples = (1 << (levels - 1)) + 1;
	linesearch_positions.Resize_Array (number_of_samples);
	linesearch_controls.Resize_Array (number_of_samples);
	linesearch_fractions.Resize_Array (number_of_samples);

	for (int i = 1; i <= number_of_samples; i++) linesearch_fractions (i) = LINEAR_INTERPOLATION<T, T>::Linear ( (T) 1, (T) number_of_samples, lower_fraction, upper_fraction, (T) i);

	for (int i = 1; i <= number_of_samples; i++) linesearch_positions (i).Resize_Array (deformable_object.particles.number);

	for (int i = 1; i <= number_of_samples; i++) linesearch_controls (i) = LINEAR_INTERPOLATION<T, VECTOR_ND<T> >::Linear (original_controls, final_controls, linesearch_fractions (i));

	ARRAY<int> active_subset (control_parameters.Active_Subset()), active_kinematic_subset (control_parameters.Active_Kinematic_Subset());
	VECTOR_ND<T> active_kinematic_controls_save;
	control_parameters.Get (active_kinematic_controls_save, active_kinematic_subset);
	linesearch_positions (1) = linesearch_positions (number_of_samples) = deformable_object.particles.X.array;

	for (int l = 0; l <= levels; l++)
	{
		int streak = (l > 0) ? (1 << (levels - l)) : 0;

		for (int sample = streak + 1; sample <= number_of_samples; sample += 2 * streak)
		{
			std::cout << "Solving for linesearch parameter = " << linesearch_fractions (sample) << std::endl;
			deformable_object.particles.X.array = linesearch_positions (sample);
			control_parameters.Set (linesearch_controls (sample), active_subset);
			control_parameters.Set (active_kinematic_controls_save, active_kinematic_subset);
			Update_Collision_Body_Positions_And_Velocities (0);
			deformable_object.Advance_One_Time_Step_Quasistatic (0, 0, linesearch_cg_tolerance, linesearch_cg_iterations, linesearch_newton_tolerance, linesearch_newton_iterations, true, true);
			control_parameters.Get (linesearch_controls (sample), active_subset);
			linesearch_positions (sample) = deformable_object.particles.X.array;

			if (l == 0) break;

			if (l < levels) ARRAY<VECTOR_3D<T> >::copy ( (T).5, linesearch_positions (sample), (T).5, linesearch_positions (sample - streak), linesearch_positions (sample - streak / 2));

			if (1 < l && l < levels) ARRAY<VECTOR_3D<T> >::copy ( (T).5, linesearch_positions (sample), (T).5, linesearch_positions (sample + streak), linesearch_positions (sample + streak / 2));
		}
	}
}
//#####################################################################
// Function Compute_Gauss_Newton_Update
//#####################################################################
template<class T> VECTOR_ND<T> LANDMARK_PROXIMITY_OPTIMIZATION<T>::
Compute_Gauss_Newton_Update (T* proximity_error_after_optimization, bool verbose)
{
	MATRIX_NXN<T> optimization_jacobian, penalty_hessian;
	VECTOR_ND<T> optimization_residual, penalty_gradient;
	T optimization_min_error;

	Compute_Linearized_Proximity_Error (optimization_jacobian, optimization_residual, optimization_min_error);
	VECTOR_ND<T> original_active_controls, controls_update (optimization_jacobian.n);
	control_parameters.Get (original_active_controls, control_parameters.Active_Subset());
	int penalty_iterations = (int) (log (penalty_ramping_factor) / log (penalty_ramping_step)) + 1;

	for (int iteration = 1; iteration < penalty_iterations; iteration++)
	{
		T proximity_error = (optimization_jacobian * controls_update - optimization_residual).Magnitude_Squared() + optimization_min_error;
		T penalty = control_parameters.Penalty(), penalty_weight = pow (penalty_ramping_step, (T) (iteration - penalty_iterations));

		if (verbose) std::cout << "Iteration " << iteration << " : Proximity error = " << proximity_error << " Penalty weight = " << penalty_weight << " Unweighted penalty = " << penalty
					       << " Weighted penalty = " << penalty_weight*penalty << " Total functional = " << proximity_error + penalty_weight*penalty << std::endl;

		penalty_gradient = penalty_weight * control_parameters.Penalty_Gradient();
		penalty_hessian = penalty_weight * control_parameters.Penalty_Hessian();
		MATRIX_NXN<T> augmented_hessian (optimization_jacobian.Normal_Equations_Matrix() + (T).5 * penalty_hessian);
		VECTOR_ND<T> augmented_gradient (optimization_jacobian.Transpose_Times (optimization_residual - optimization_jacobian * controls_update) - (T).5 * penalty_gradient);
		controls_update += augmented_hessian.PLU_Solve (augmented_gradient);
		control_parameters.Set (original_active_controls + controls_update, control_parameters.Active_Subset());
	}

	control_parameters.Set (original_active_controls, control_parameters.Active_Subset());

	if (proximity_error_after_optimization) *proximity_error_after_optimization = (optimization_jacobian * controls_update - optimization_residual).Magnitude_Squared() + optimization_min_error;

	return controls_update;
}
//#####################################################################
// Function Compute_Linearized_Proximity_Error
//#####################################################################
template<class T> void LANDMARK_PROXIMITY_OPTIMIZATION<T>::
Compute_Linearized_Proximity_Error (MATRIX_NXN<T>& optimization_jacobian, VECTOR_ND<T>& optimization_residual, T& optimization_min_error)
{
	int L = landmark_optimization_goal.embedded_landmarks.m;
	ARRAY<int> active_control_subset (control_parameters.Active_Subset());
	MATRIX_MXN<T> landmark_jacobian (3 * L, control_parameters.Active_Size());
	VECTOR_ND<T> landmark_residual (3 * L);

	for (int i = 1; i <= control_parameters.Active_Size(); i++) for (int l = 1; l <= L; l++)
		{
			VECTOR_3D<T> jacobian_element = landmark_optimization_goal.embedded_landmarks (l).Evaluate (jacobian (active_control_subset (i)));

			for (int k = 1; k <= 3; k++) landmark_jacobian (3 * l + k - 3, i) = jacobian_element[k];
		}

	for (int l = 1; l <= L; l++)
	{
		VECTOR_3D<T> position_element = landmark_optimization_goal.embedded_landmarks (l).Evaluate (deformable_object.particles.X.array), target_element = landmark_optimization_goal.target_landmarks (l);

		for (int k = 1; k <= 3; k++) landmark_residual (3 * l + k - 3) = target_element[k] - position_element[k];
	}

	if (proximity_weights.m)
	{
		assert (proximity_weights.m == L);

		for (int i = 1; i <= control_parameters.Active_Size(); i++) for (int l = 1; l <= L; l++) for (int k = 1; k <= 3; k++) landmark_jacobian (3 * l + k - 3, i) *= proximity_weights (l);

		for (int l = 1; l <= L; l++) for (int k = 1; k <= 3; k++) landmark_residual (3 * l + k - 3) *= proximity_weights (l);
	}

	landmark_jacobian.Householder_QR_Factorization();
	optimization_jacobian = * (landmark_jacobian.R);
	landmark_residual = MATRIX_MXN<T>::Householder_Transform (landmark_residual, *landmark_jacobian.V);
	optimization_residual = VECTOR_ND<T> (landmark_jacobian.n);

	for (int i = 1; i <= landmark_jacobian.n; i++)
	{
		optimization_residual (i) = landmark_residual (i);
		landmark_residual (i) = 0;
	}

	optimization_min_error = landmark_residual.Magnitude_Squared();
}
//#####################################################################
