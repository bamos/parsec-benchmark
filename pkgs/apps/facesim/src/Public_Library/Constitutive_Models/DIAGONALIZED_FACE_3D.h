//#####################################################################
// Copyright 2004, Eftychios Sifakis
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class DIAGONALIZED_FACE_3D
//#####################################################################
#ifndef __DIAGONALIZED_FACE_3D__
#define __DIAGONALIZED_FACE_3D__

#include "DIAGONALIZED_CONSTITUTIVE_MODEL_3D.h"
namespace PhysBAM
{

template<class T>
class DIAGONALIZED_FACE_3D: public DIAGONALIZED_CONSTITUTIVE_MODEL_3D<T>
{
public:
	using DIAGONALIZED_CONSTITUTIVE_MODEL_3D<T>::anisotropic;
	using DIAGONALIZED_CONSTITUTIVE_MODEL_3D<T>::enforce_definiteness;
	using DIAGONALIZED_CONSTITUTIVE_MODEL_3D<T>::lambda;
	using DIAGONALIZED_CONSTITUTIVE_MODEL_3D<T>::mu;
	using DIAGONALIZED_CONSTITUTIVE_MODEL_3D<T>::alpha;
	using DIAGONALIZED_CONSTITUTIVE_MODEL_3D<T>::beta;

	T constant_mu_10, constant_mu_01, constant_kappa;
	ARRAY<T> *tet_mu_10, *tet_mu_01, *tet_kappa;
	T fiber_p1, fiber_p2, fiber_p3, fiber_p4, fiber_cutoff;
	LIST_ARRAY<T> fiber_max_stress;
	T failure_threshold;
	LIST_ARRAY<T>& muscle_activations;
	LIST_ARRAY<LIST_ARRAY<int> > tet_muscles;
	LIST_ARRAY<LIST_ARRAY<VECTOR_3D<T> > > tet_fibers;
	LIST_ARRAY<LIST_ARRAY<T> > tet_densities;
	LIST_ARRAY<LIST_ARRAY<T> > tension, tension_derivative, active_tension_unit_activation;
	int* single_activation_used_for_force_derivative;

	DIAGONALIZED_FACE_3D (const T mu_10_input, const T mu_01_input, const T kappa_input, const T Rayleigh_coefficient, LIST_ARRAY<T>& muscle_activations_input,
			      int* single_activation_used_for_force_derivative_input, const LIST_ARRAY<T>* peak_isometric_stress, const T failure_threshold_input)
		: constant_mu_10 (mu_10_input), constant_mu_01 (mu_01_input), constant_kappa (kappa_input), tet_mu_10 (0), tet_mu_01 (0), tet_kappa (0), failure_threshold (failure_threshold_input),
		  muscle_activations (muscle_activations_input), single_activation_used_for_force_derivative (single_activation_used_for_force_derivative_input)
	{
		anisotropic = true;
		mu = 2 * (constant_mu_10 + constant_mu_01);
		lambda = (T) one_third * (8 * constant_mu_01 - 4 * constant_mu_10) + constant_kappa;
		alpha = Rayleigh_coefficient * lambda;
		beta = Rayleigh_coefficient * mu;
		fiber_p1 = (T).05;
		fiber_p2 = (T) 6.6;
		fiber_cutoff = (T) 1.4;
		T cutoff_scaled = fiber_p2 * (fiber_cutoff - 1);

		if (peak_isometric_stress)
		{
			assert (peak_isometric_stress->m == muscle_activations.m);
			fiber_max_stress = *peak_isometric_stress;
		}
		else
		{
			fiber_max_stress.Resize_Array (muscle_activations.m);
			LIST_ARRAY<T>::copy ( (T) 3e5, fiber_max_stress);
		}

		fiber_p3 = fiber_p1 * fiber_p2 * (exp (cutoff_scaled) - 1);
		fiber_p4 = fiber_p1 * (exp (cutoff_scaled) * (1 - fiber_p2 * fiber_cutoff) + fiber_p2 - 1);
	}

	void Initialize_Inhomogeneous_Material_Properties (const int number_of_tetrahedrons)
	{
		tet_mu_10 = new ARRAY<T> (number_of_tetrahedrons);
		ARRAY<T>::copy (constant_mu_10, *tet_mu_10);
		tet_mu_01 = new ARRAY<T> (number_of_tetrahedrons);
		ARRAY<T>::copy (constant_mu_01, *tet_mu_01);
		tet_kappa = new ARRAY<T> (number_of_tetrahedrons);
		ARRAY<T>::copy (constant_kappa, *tet_kappa);
	}

	void Set_Material_Properties_Of_Subset (const LIST_ARRAY<int>& tetrahedron_list, const T mu_10, const T mu_01, const T kappa)
	{
		assert (tet_mu_10 && tet_mu_01 && tet_kappa);

		for (int i = 1; i <= tetrahedron_list.m; i++)
		{
			(*tet_mu_10) (tetrahedron_list (i)) = mu_10;
			(*tet_mu_01) (tetrahedron_list (i)) = mu_01;
			(*tet_kappa) (tetrahedron_list (i)) = kappa;
		}
	}

	void Initialize_Fiber_Data (const STRAIN_MEASURE_3D<T>& strain_measure, const LIST_ARRAY<LIST_ARRAY<int> >& muscle_tets, const LIST_ARRAY<LIST_ARRAY<VECTOR_3D<T> > >& muscle_fibers,
				    const LIST_ARRAY<LIST_ARRAY<T> >& muscle_densities)
	{
		int n = strain_measure.tetrahedron_mesh.tetrahedrons.m;
		tet_muscles.Resize_Array (n);
		tet_fibers.Resize_Array (n);
		tet_densities.Resize_Array (n);
		tension.Resize_Array (n);
		tension_derivative.Resize_Array (n);
		active_tension_unit_activation.Resize_Array (n);

		for (int m = 1; m <= muscle_tets.m; m++) for (int t = 1; t <= muscle_tets (m).m; t++)
			{
				tet_muscles (muscle_tets (m) (t)).Append_Element (m);
				tet_densities (muscle_tets (m) (t)).Append_Element (muscle_densities (m) (t));
				tet_fibers (muscle_tets (m) (t)).Append_Element (strain_measure.F (muscle_tets (m) (t)).Transpose_Times (muscle_fibers (m) (t)));
				tension (muscle_tets (m) (t)).Append_Element (0);
				active_tension_unit_activation (muscle_tets (m) (t)).Append_Element (0);
				tension_derivative (muscle_tets (m) (t)).Append_Element (0);
			}
	}

	T Tension (const int tet_index, const int tet_muscle_index, const T stretch) const
	{
		T strain = stretch - 1, strain_abs = fabs (strain), activation = muscle_activations (tet_muscles (tet_index) (tet_muscle_index)), density = tet_densities (tet_index) (tet_muscle_index);
		T active_tension = 0, passive_tension = 0, scale = (T) 25 / (T) 6;

		if (stretch > fiber_cutoff) passive_tension = fiber_p3 * stretch + fiber_p4;
		else if (stretch > 1) passive_tension = fiber_p1 * (exp (fiber_p2 * strain) - fiber_p2 * strain - 1);

		if (strain_abs < .4) active_tension = activation * density * (1 - scale * sqr (strain));
		else if (strain_abs < .6) active_tension = 2 * scale * activation * density * sqr (strain_abs - (T).6);

		return fiber_max_stress (tet_muscles (tet_index) (tet_muscle_index)) * (active_tension + passive_tension);
	}

	T Active_Tension_Unit_Activation (const int tet_index, const int tet_muscle_index, const T stretch) const
	{
		T strain = stretch - 1, strain_abs = fabs (strain), density = tet_densities (tet_index) (tet_muscle_index), active_tension = 0, scale = (T) 25 / (T) 6;

		if (strain_abs < .4) active_tension = density * (1 - scale * sqr (strain));
		else if (strain_abs < .6) active_tension = 2 * scale * density * sqr (strain_abs - (T).6);

		return fiber_max_stress (tet_muscles (tet_index) (tet_muscle_index)) * active_tension;
	}

	T Tension_Derivative (const int tet_index, const int tet_muscle_index, const T stretch) const
	{
		T strain = stretch - 1, strain_abs = fabs (strain), activation = muscle_activations (tet_muscles (tet_index) (tet_muscle_index)), density = tet_densities (tet_index) (tet_muscle_index);
		T active_tension_derivative = 0, passive_tension_derivative = 0, scale = (T) 25 / (T) 6;

		if (stretch > fiber_cutoff) passive_tension_derivative = fiber_p3;
		else if (stretch > 1) passive_tension_derivative = fiber_p1 * fiber_p2 * (exp (fiber_p2 * strain) - 1);

		if (strain_abs < .4) active_tension_derivative = -2 * scale * activation * density * strain;
		else if (strain_abs < .6) active_tension_derivative = 4 * scale * activation * density * (strain - sign (strain) * (T).6);

		return fiber_max_stress (tet_muscles (tet_index) (tet_muscle_index)) * (active_tension_derivative + passive_tension_derivative);
	}

	MATRIX_3X3<T> P_From_Strain (const DIAGONAL_MATRIX_3X3<T>& F, const MATRIX_3X3<T>& V, const T scale, const int tetrahedron_index) const
	{
		T mu_10 = (tet_mu_10) ? (*tet_mu_10) (tetrahedron_index) : constant_mu_10, mu_01 = (tet_mu_01) ? (*tet_mu_01) (tetrahedron_index) : constant_mu_01, kappa = (tet_kappa) ? (*tet_kappa) (tetrahedron_index) : constant_kappa;

		if (single_activation_used_for_force_derivative && (*single_activation_used_for_force_derivative)) return P_From_Strain_Unit_Activation (F, V, scale, tetrahedron_index, *single_activation_used_for_force_derivative);

		DIAGONAL_MATRIX_3X3<T> F_threshold = F.Max (failure_threshold), C = F_threshold * F_threshold, F_cube = C * F_threshold, F_inverse = F_threshold.Inverse(), isotropic_part;
		MATRIX_3X3<T> anisotropic_part;
		T I_C = C.Trace(), II_C = (C * C).Trace(), J = F_threshold.Determinant(), Jcc = pow (J, - (T) two_thirds);
		isotropic_part = (2 * Jcc * (mu_10 + Jcc * mu_01 * I_C)) * F_threshold - (2 * Jcc * Jcc * mu_01) * F_cube + ( (kappa * (T) log (J) - (T) two_thirds * Jcc * (mu_10 * I_C + Jcc * mu_01 * (I_C * I_C - II_C)))) * F_inverse;

		for (int m = 1; m <= tet_muscles (tetrahedron_index).m; m++)
		{
			VECTOR_3D<T> fiber = V.Transpose_Times (tet_fibers (tetrahedron_index) (m)), F_fiber = F_threshold * fiber;
			anisotropic_part += (tension (tetrahedron_index) (m) / F_fiber.Magnitude()) * MATRIX_3X3<T>::Outer_Product (F_fiber, fiber);
		}

		return scale * (anisotropic_part + isotropic_part);
	}

	MATRIX_3X3<T> P_From_Strain_Unit_Activation (const DIAGONAL_MATRIX_3X3<T>& F, const MATRIX_3X3<T>& V, const T scale, const int tetrahedron_index, const int muscle_id) const
	{
		assert (1 <= muscle_id && muscle_id <= muscle_activations.m);
		DIAGONAL_MATRIX_3X3<T> F_threshold = F.Max (failure_threshold);
		int tet_muscle_index = tet_muscles (tetrahedron_index).Find (muscle_id);

		if (!tet_muscle_index) return MATRIX_3X3<T>();

		VECTOR_3D<T> fiber = V.Transpose_Times (tet_fibers (tetrahedron_index) (tet_muscle_index)), F_fiber = F_threshold * fiber;
		return scale * (active_tension_unit_activation (tetrahedron_index) (tet_muscle_index) / F_fiber.Magnitude()) * MATRIX_3X3<T>::Outer_Product (F_fiber, fiber);
	}

	MATRIX_3X3<T> P_From_Strain_Rate (const DIAGONAL_MATRIX_3X3<T>& F, const MATRIX_3X3<T>& F_dot, const T scale) const
	{
		SYMMETRIC_MATRIX_3X3<T> strain_rate = F_dot.Symmetric_Part();
		return 2 * scale * beta * strain_rate + scale * alpha * strain_rate.Trace();
	}

	void Isotropic_Stress_Derivative (const DIAGONAL_MATRIX_3X3<T>& F, DIAGONALIZED_ISOTROPIC_STRESS_DERIVATIVE_3D<T>& dP_dF, const int tetrahedron_index = 0) const
	{
		T mu_10 = (tet_mu_10) ? (*tet_mu_10) (tetrahedron_index) : constant_mu_10, mu_01 = (tet_mu_01) ? (*tet_mu_01) (tetrahedron_index) : constant_mu_01, kappa = (tet_kappa) ? (*tet_kappa) (tetrahedron_index) : constant_kappa;
		DIAGONAL_MATRIX_3X3<T> F_threshold = F.Max (failure_threshold), C = F_threshold * F_threshold, F_cube = C * F_threshold, F_inverse = F_threshold.Inverse();
		T I_C = C.Trace(), II_C = (C * C).Trace(), J = F_threshold.Determinant(), Jcc = pow (J, - (T) two_thirds);
		SYMMETRIC_MATRIX_3X3<T> alpha;
		alpha.x11 = 2 * Jcc * (mu_10 + Jcc * mu_01 * (I_C - C.x11 - C.x11));
		alpha.x21 = 2 * Jcc * (mu_10 + Jcc * mu_01 * (I_C - C.x22 - C.x11));
		alpha.x31 = 2 * Jcc * (mu_10 + Jcc * mu_01 * (I_C - C.x33 - C.x11));
		alpha.x22 = 2 * Jcc * (mu_10 + Jcc * mu_01 * (I_C - C.x22 - C.x22));
		alpha.x32 = 2 * Jcc * (mu_10 + Jcc * mu_01 * (I_C - C.x33 - C.x22));
		alpha.x33 = 2 * Jcc * (mu_10 + Jcc * mu_01 * (I_C - C.x33 - C.x33));
		SYMMETRIC_MATRIX_3X3<T> beta;
		beta.x11 = (Jcc * (T) two_thirds * (mu_10 * I_C + Jcc * mu_01 * (I_C * I_C - II_C)) - kappa * log (J)) * F_inverse.x11 * F_inverse.x11 - 2 * Jcc * Jcc * mu_01 * F_threshold.x11 * F_threshold.x11;
		beta.x21 = (Jcc * (T) two_thirds * (mu_10 * I_C + Jcc * mu_01 * (I_C * I_C - II_C)) - kappa * log (J)) * F_inverse.x22 * F_inverse.x11 - 2 * Jcc * Jcc * mu_01 * F_threshold.x22 * F_threshold.x11;
		beta.x31 = (Jcc * (T) two_thirds * (mu_10 * I_C + Jcc * mu_01 * (I_C * I_C - II_C)) - kappa * log (J)) * F_inverse.x33 * F_inverse.x11 - 2 * Jcc * Jcc * mu_01 * F_threshold.x33 * F_threshold.x11;
		beta.x22 = (Jcc * (T) two_thirds * (mu_10 * I_C + Jcc * mu_01 * (I_C * I_C - II_C)) - kappa * log (J)) * F_inverse.x22 * F_inverse.x22 - 2 * Jcc * Jcc * mu_01 * F_threshold.x22 * F_threshold.x22;
		beta.x32 = (Jcc * (T) two_thirds * (mu_10 * I_C + Jcc * mu_01 * (I_C * I_C - II_C)) - kappa * log (J)) * F_inverse.x33 * F_inverse.x22 - 2 * Jcc * Jcc * mu_01 * F_threshold.x33 * F_threshold.x22;
		beta.x33 = (Jcc * (T) two_thirds * (mu_10 * I_C + Jcc * mu_01 * (I_C * I_C - II_C)) - kappa * log (J)) * F_inverse.x33 * F_inverse.x33 - 2 * Jcc * Jcc * mu_01 * F_threshold.x33 * F_threshold.x33;
		SYMMETRIC_MATRIX_3X3<T> eta;
		eta.x11 = 4 * Jcc * Jcc * mu_01;
		eta.x31 = -Jcc * (T) one_third * (4 * mu_10 + 8 * Jcc * mu_01 * I_C);
		eta.x32 = 8 * (T) one_third * Jcc * Jcc * mu_01;
		eta.x33 = Jcc * (T) one_ninth * (4 * mu_10 * I_C + Jcc * 8 * mu_01 * (I_C * I_C - II_C)) + kappa;
		MATRIX_3X3<T> F_base (F_threshold.x11, F_threshold.x22, F_threshold.x33, F_cube.x11, F_cube.x22, F_cube.x33, F_inverse.x11, F_inverse.x22, F_inverse.x33);
		SYMMETRIC_MATRIX_3X3<T> gamma = SYMMETRIC_MATRIX_3X3<T>::Conjugate (F_base, eta);
		dP_dF.x1111 = alpha.x11 + beta.x11 + gamma.x11;
		dP_dF.x2222 = alpha.x22 + beta.x22 + gamma.x22;
		dP_dF.x3333 = alpha.x33 + beta.x33 + gamma.x33;
		dP_dF.x2211 = gamma.x21;
		dP_dF.x3311 = gamma.x31;
		dP_dF.x3322 = gamma.x32;
		dP_dF.x2121 = alpha.x21;
		dP_dF.x3131 = alpha.x31;
		dP_dF.x3232 = alpha.x32;
		dP_dF.x2112 = beta.x21;
		dP_dF.x3113 = beta.x31;
		dP_dF.x3223 = beta.x31;

		if (enforce_definiteness) dP_dF.Fix_Indefinite_Blocks();
	}

	MATRIX_3X3<T> dP_From_dF (const MATRIX_3X3<T>& dF, const DIAGONAL_MATRIX_3X3<T>& F, const MATRIX_3X3<T>& V,
				  const DIAGONALIZED_ISOTROPIC_STRESS_DERIVATIVE_3D<T>& dP_dF, const T scale, const int tetrahedron_index) const
	{
		MATRIX_3X3<T> dP = DIAGONALIZED_CONSTITUTIVE_MODEL_3D<T>::dP_From_dF (dF, dP_dF, scale, tetrahedron_index);
		DIAGONAL_MATRIX_3X3<T> F_threshold = F.Max (failure_threshold);

		for (int m = 1; m <= tet_muscles (tetrahedron_index).m; m++)
		{
			VECTOR_3D<T> fiber = V.Transpose_Times (tet_fibers (tetrahedron_index) (m)), F_fiber = F_threshold * fiber, dF_fiber = dF * fiber;
			T stretch_squared = F_fiber.Magnitude_Squared(), stretch = sqrt (stretch_squared);
			T c1 = tension (tetrahedron_index) (m) / stretch, c2 = (tension_derivative (tetrahedron_index) (m) - c1) / stretch_squared;
			VECTOR_3D<T> dPw = c1 * dF_fiber + c2 * VECTOR_3D<T>::Dot_Product (dF_fiber, F_fiber) * F_fiber;
			dP += scale * MATRIX_3X3<T>::Outer_Product (dPw, fiber);
		}

		return dP;
	}

	void Update_State_Dependent_Auxiliary_Variables (const DIAGONAL_MATRIX_3X3<T>& F, const MATRIX_3X3<T>& V, const int tetrahedron_index)
	{
		DIAGONAL_MATRIX_3X3<T> F_threshold = F.Max (failure_threshold);

		for (int m = 1; m <= tet_muscles (tetrahedron_index).m; m++)
		{
			VECTOR_3D<T> fiber = V.Transpose_Times (tet_fibers (tetrahedron_index) (m)), F_fiber = F_threshold * fiber;
			T stretch = F_fiber.Magnitude();
			tension (tetrahedron_index) (m) = Tension (tetrahedron_index, m, stretch);
			active_tension_unit_activation (tetrahedron_index) (m) = Active_Tension_Unit_Activation (tetrahedron_index, m, stretch);

			if (enforce_definiteness) tension_derivative (tetrahedron_index) (m) = max ( (T) 0, Tension_Derivative (tetrahedron_index, m, stretch));
			else tension_derivative (tetrahedron_index) (m) = Tension_Derivative (tetrahedron_index, m, stretch);
		}
	}

//#####################################################################
};
}
#endif
