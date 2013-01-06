//#####################################################################
// Copyright 2004-2005, Igor Neverov, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class ACTIVATION_CONTROL_SET
//#####################################################################
#ifndef __ACTIVATION_CONTROL_SET__
#define __ACTIVATION_CONTROL_SET__

#include "FACE_CONTROL_SET.h"
#include "../../Public_Library/Forces_And_Torques/DIAGONALIZED_FINITE_VOLUME_3D.h"
#include "../../Public_Library/Interpolation/LINEAR_INTERPOLATION.h"

namespace PhysBAM
{

template <class T>
class ACTIVATION_CONTROL_SET: public FACE_CONTROL_SET<T>
{
public:
	LIST_ARRAY<T> activations, activations_save;
	LIST_ARRAY<bool> activation_active;
	mutable int single_activation_used_for_force_derivative;
	DIAGONALIZED_FINITE_VOLUME_3D<T>* muscle_force;
	T activation_cutoff, max_optimization_step_length, activation_penalty_coefficient;

	ACTIVATION_CONTROL_SET()
		: single_activation_used_for_force_derivative (0), muscle_force (0), activation_cutoff ( (T) 1), activation_penalty_coefficient ( (T) 10), max_optimization_step_length ( (T).4)
	{
		Save_Controls();
	}

	int Size() const
	{
		return activations.m;
	}

	int Add_Activation (const bool active = true)
	{
		assert (activations.m == activation_active.m);
		activations.Append_Element (0), activation_active.Append_Element (active);
		Save_Controls();
		return activations.m;
	}

	T operator() (const int control_id) const
	{
		return activations (control_id);
	}

	T& operator() (const int control_id)
	{
		return activations (control_id);
	}

	bool Control_Active (const int control_id) const
	{
		return activation_active (control_id);
	}

	bool& Control_Active (const int control_id)
	{
		return activation_active (control_id);
	}

	bool Positions_Determined_Kinematically (const int control_id) const
	{
		return false;
	}

	void Force_Derivative (ARRAY<VECTOR_3D<T> >& dFdl, const int control_id) const
	{
		assert (muscle_force);
		assert (Control_Active (control_id));
		single_activation_used_for_force_derivative = control_id;
		ARRAY<VECTOR_3D<T> >::copy (VECTOR_3D<T>(), dFdl);
		muscle_force->Add_Velocity_Independent_Forces (dFdl);
		single_activation_used_for_force_derivative = 0;
	}

	static T Piecewise_Quadratic_Penalty (const T a, const T a_min, const T a_max)
	{
		if (a < a_min) return sqr (a - a_min);

		if (a <= a_max) return 0;

		return sqr (a - a_max);
	}
	static T Piecewise_Quadratic_Penalty_Prime (const T a, const T a_min, const T a_max)
	{
		if (a < a_min) return 2 * (a - a_min);

		if (a <= a_max) return 0;

		return 2 * (a - a_max);
	}
	static T Piecewise_Quadratic_Penalty_Double_Prime (const T a, const T a_min, const T a_max)
	{
		if (a < a_min) return 2;

		if (a <= a_max) return 0;

		return 2;
	}

	T Penalty() const
	{
		assert (activations.m == activations_save.m);
		T penalty = 0;

		for (int i = 1; i <= activations.m; i++) penalty += Piecewise_Quadratic_Penalty (activations (i), clamp<T> (activations_save (i) - max_optimization_step_length, 0, activation_cutoff),
					clamp<T> (activations_save (i) + max_optimization_step_length, 0, activation_cutoff));

		return activation_penalty_coefficient * penalty;
	}

	T Penalty_Gradient (const int control_id) const
	{
		assert (1 <= control_id && control_id <= Size());
		return activation_penalty_coefficient * Piecewise_Quadratic_Penalty_Prime (activations (control_id), clamp<T> (activations_save (control_id) - max_optimization_step_length, 0, activation_cutoff),
				clamp<T> (activations_save (control_id) + max_optimization_step_length, 0, activation_cutoff));
	}

	T Penalty_Hessian (const int control_id1, const int control_id2) const
	{
		assert (1 <= control_id1 && control_id1 <= Size() && 1 <= control_id2 && control_id2 <= Size());

		if (control_id1 != control_id2) return 0;

		return activation_penalty_coefficient * Piecewise_Quadratic_Penalty_Double_Prime (activations (control_id1), clamp<T> (activations_save (control_id1) - max_optimization_step_length, 0, activation_cutoff),
				clamp<T> (activations_save (control_id1) + max_optimization_step_length, 0, activation_cutoff));
	}

	void Save_Controls()
	{
		activations_save = activations;
	}

	void Project_Parameters_To_Allowable_Range (const bool active_controls_only)
	{
		for (int i = 1; i <= activations.m; i++) if (!active_controls_only || activation_active (i))
				activations (i) = clamp<T> (activations (i), max (activations_save (i) - max_optimization_step_length, (T) 0), min (activations_save (i) + max_optimization_step_length, activation_cutoff));
	}

	void Interpolate (const T interpolation_fraction)
	{
		for (int i = 1; i <= activations.m; i++) activations (i) = LINEAR_INTERPOLATION<T, T>::Linear (activations_save (i), activations (i), interpolation_fraction);
	}

	void Print_Diagnostics (std::ostream& output = std::cout) const
	{
		for (int i = 1; i <= activations.m; i++)
		{
			output << "Activation #" << i << " [";

			if (activation_active (i)) output << "active] : ";
			else output << "inactive] : ";

			output << activations (i) << std::endl;
		}

		output << "Penalty at current activation levels : " << Penalty() << std::endl;
	}

//#####################################################################
	void Set_Attachment_Positions (ARRAY<VECTOR_3D<T> >&X) const {}
	void Kinematically_Update_Positions (ARRAY<VECTOR_3D<T> >&X) const {}
	void Kinematically_Update_Jacobian (ARRAY<VECTOR_3D<T> >&dX) const {}
//#####################################################################
};
}
#endif
