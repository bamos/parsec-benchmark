//#####################################################################
// Copyright 2002-2004 Doug Enright, Ronald Fedkiw, Frederic Gibou, Neil Molino.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class LEVELSET
//#####################################################################
#ifndef __LEVELSET__
#define __LEVELSET__

#include "../Arrays/ARRAYS_1D.h"
#include "../Interpolation/LINEAR_INTERPOLATION.h"
#include "../Math_Tools/constants.h"
#include "../Geometry/POLYGON.h"
#include "../Utilities/DEBUG_UTILITIES.h"
namespace PhysBAM
{

template<class T, class TV>
class LEVELSET
{
public:
	T small_number;
	INTERPOLATION<T, T> *interpolation, *curvature_interpolation, *secondary_interpolation;
	INTERPOLATION<T, TV>* normal_interpolation;
protected:
	T max_time_step;
	int reinitialization_runge_kutta_order;
	T reinitialization_cfl;
	int reinitialization_spatial_order;
	bool curvature_motion;
	T sigma;
	bool refine_fmm_initialization_with_iterative_solver;
	int fmm_initialization_iterations;
	T fmm_initialization_iterative_tolerance;
	T fmm_initialization_iterative_drift_fraction;
protected:
	LINEAR_INTERPOLATION<T, T> interpolation_default, curvature_interpolation_default;
	INTERPOLATION<T, T>* collision_aware_interpolation_plus_default, *collision_aware_interpolation_minus_default;
	LINEAR_INTERPOLATION<T, TV> normal_interpolation_default;

	LEVELSET()
		: collision_aware_interpolation_plus_default (0), collision_aware_interpolation_minus_default (0)
	{
		Set_Small_Number();
		Set_Max_Time_Step();
		Set_Reinitialization_Runge_Kutta_Order();
		Set_Reinitialization_CFL();
		Use_WENO_For_Reinitialization();
		curvature_motion = false; // default is no curvature motion
		Initialize_FMM_Initialization_Iterative_Solver();
		interpolation = &interpolation_default;
		normal_interpolation = &normal_interpolation_default;
		curvature_interpolation = &curvature_interpolation_default;
	}

	~LEVELSET()
	{
		delete collision_aware_interpolation_plus_default;
		delete collision_aware_interpolation_minus_default;
	}

public:
	void Set_Small_Number (const T small_number_input = 1e-8)
	{
		small_number = small_number_input;
	}

	void Set_Max_Time_Step (const T max_time_step_input = 1e8)
	{
		max_time_step = max_time_step_input;
	}

	void Set_Reinitialization_Runge_Kutta_Order (const int order = 3)
	{
		assert (order >= 1 && order <= 3);
		reinitialization_runge_kutta_order = order;
	}

	void Set_Reinitialization_CFL (const T cfl = .5)
	{
		reinitialization_cfl = cfl;
		assert (cfl <= 1);
	}

	void Use_WENO_For_Reinitialization() // 5th order
	{
		reinitialization_spatial_order = 5;
	}

	void Use_ENO_For_Reinitialization (const int order = 3)
	{
		assert (order >= 1 && order <= 3);
		reinitialization_spatial_order = order;
	}

	void Set_Curvature_Motion (const T sigma_input = 1) // times by the grid spacing
	{
		curvature_motion = true;
		sigma = sigma_input;
		assert (sigma >= 0);
	}

	void Initialize_FMM_Initialization_Iterative_Solver (const bool refine_fmm_initialization_with_iterative_solver_input = true, const int fmm_initialization_iterations_input = 10,
			const T fmm_initialization_iterative_tolerance_input = 1e-2, const T fmm_initialization_iterative_drift_fraction_input = .1)
	{
		refine_fmm_initialization_with_iterative_solver = refine_fmm_initialization_with_iterative_solver_input;
		fmm_initialization_iterations = fmm_initialization_iterations_input;
		fmm_initialization_iterative_tolerance = fmm_initialization_iterative_tolerance_input;
		fmm_initialization_iterative_drift_fraction = fmm_initialization_iterative_drift_fraction_input;
	}

	void Set_Custom_Interpolation (INTERPOLATION<T, T>& interpolation_input)
	{
		interpolation = &interpolation_input;
	}

	void Set_Custom_Secondary_Interpolation (INTERPOLATION<T, T>& interpolation_input)
	{
		secondary_interpolation = &interpolation_input;
	}

	void Set_Custom_Normal_Interpolation (INTERPOLATION<T, TV>& normal_interpolation_input)
	{
		normal_interpolation = &normal_interpolation_input;
	}

	void Set_Custom_Curvature_Interpolation (INTERPOLATION<T, T>& curvature_interpolation_input)
	{
		curvature_interpolation = &curvature_interpolation_input;
	}

	void Set_Collision_Aware_Interpolation (const int sign)
	{
		assert (sign == 1 || sign == -1);
		assert (collision_aware_interpolation_plus_default && collision_aware_interpolation_minus_default);

		if (sign == 1) Set_Custom_Interpolation (*collision_aware_interpolation_plus_default);
		else if (sign == -1) Set_Custom_Interpolation (*collision_aware_interpolation_minus_default);
	}

	static T Sign (const T phi)
	{
		if (phi <= 0) return -1;
		else return 1;
	}

	static int Interface (const T phi_1, const T phi_2)
	{
		if ( (phi_1 > 0 && phi_2 <= 0) || (phi_1 <= 0 && phi_2 > 0)) return 1;
		else return 0;
	}

	static int Thin_Shells_Interface (const T phi_1, const T phi_2)
	{
		if (phi_1 == FLT_MAX || phi_2 == FLT_MAX) return 0;
		else return Interface (phi_1, phi_2);
	}

	static T Heaviside (const T phi, const T half_width = 0)
	{
		if (phi <= -half_width) return 0;
		else if (phi >= half_width) return 1;
		else
		{
			T phi_over_half_width = phi / half_width;
			return (T).5 * (1 + phi_over_half_width + sin ( (T) pi * phi_over_half_width) / (T) pi);
		}
	}

	static T Heaviside (const T phi, const T value_minus, const T value_plus, const T half_width = 0)
	{
		return value_minus + (value_plus - value_minus) * Heaviside (phi, half_width);
	}

	static T Delta (const T phi, const T half_width)
	{
		if (phi <= -half_width || phi >= half_width) return 0;
		else return (T) (1 + cos (pi * phi / half_width)) / (2 * half_width);
	}

	// finds the interface as an average of the neighbors
	static T Average (const T phi_left, const T value_left, const T phi_right, const T value_right)
	{
		T left = fabs (phi_left), right = fabs (phi_right);
		return (left * value_right + right * value_left) / (left + right);
	}

	// finds the interface value, while while sorting out the sign of phi
	static T Convex_Average (const T phi_1, const T phi_2, const T value_minus, const T value_plus)
	{
		if (phi_1 <= 0) return Average (phi_1, value_minus, phi_2, value_plus);
		else return Average (phi_2, value_minus, phi_1, value_plus);
	}

	static T Theta (const T phi_left, const T phi_right)
	{
		return phi_left / (phi_left - phi_right);
	}

	static T Theta_Quadratic (const T phi_left_left, const T phi_left, const T phi_right, const T dx)
	{
		NOT_IMPLEMENTED();
	}

	static T Theta_Cubic (const T phi_left_left, const T phi_left, const T phi_right, const T phi_right_right, const T dx)
	{
		NOT_IMPLEMENTED();
	}

protected:

//#####################################################################
public:
	T Negative_Cell_Fraction (const T phi_left, const T phi_right) const;
	T Positive_Cell_Fraction (const T phi_left, const T phi_right) const;
	T Negative_Cell_Fraction (const T phi_lower_left, const T phi_lower_right, const T phi_upper_left, const T phi_upper_right, const T aspect_ratio = 1) const;
	T Positive_Cell_Fraction (const T phi_lower_left, const T phi_lower_right, const T phi_upper_left, const T phi_upper_right, const T aspect_ratio = 1) const;
protected:
	void HJ_WENO (const int m, const T dx, const ARRAYS_1D<T>& phi, ARRAYS_1D<T>& phix_minus, ARRAYS_1D<T>& phix_plus) const;
	void HJ_ENO (const int order, const int m, const T dx, const ARRAYS_1D<T>& phi, ARRAYS_1D<T>& phix_minus, ARRAYS_1D<T>& phix_plus) const;
//#####################################################################
};
}
#endif

