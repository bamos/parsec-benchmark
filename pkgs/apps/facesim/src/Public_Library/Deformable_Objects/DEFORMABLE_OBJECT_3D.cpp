//#####################################################################
// Copyright 2004-2006, Zhaosheng Bao, Ron Fedkiw, Geoffrey Irving, Sergey Koltakov, Andrew Selle, Eftychios Sifakis, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class DEFORMABLE_OBJECT_3D
//#####################################################################
#include "DEFORMABLE_OBJECT_3D.h"
#include "../Forces_And_Torques/BODY_FORCES_3D.h"
#include "../Forces_And_Torques/DIAGONALIZED_FINITE_VOLUME_3D.h"
#include "../Constitutive_Models/CONSTITUTIVE_MODEL_3D.h"
#include "../Constitutive_Models/DIAGONALIZED_FACE_3D.h"
#include "../Constitutive_Models/STRAIN_MEASURE_3D.h"
#include "../Collisions_And_Interactions/COLLISION_PENALTY_FORCES.h"
#include "../Utilities/LOG.h"
#include "../Arrays/ARRAY_PARALLEL_OPERATIONS.h"
using namespace PhysBAM;
extern bool PHYSBAM_THREADED_RUN;


//#####################################################################
// Function Delete_Forces
//#####################################################################
template<class T> void DEFORMABLE_OBJECT_3D<T>::
Delete_Forces()
{
	solids_forces.Clean_Up_Memory();
	body_forces.Delete_Pointers_And_Clean_Memory();
	diagonalized_finite_volume_3d.Delete_Pointers_And_Clean_Memory();
	strain_measure_3d.Delete_Pointers_And_Clean_Memory();
	constitutive_model_3d.Delete_Pointers_And_Clean_Memory();
	diagonalized_constitutive_model_3d.Delete_Pointers_And_Clean_Memory();
}
//#####################################################################
// Function Update_Collision_Penalty_Forces_And_Derivatives
//#####################################################################
template<class T> void DEFORMABLE_OBJECT_3D<T>::
Update_Collision_Penalty_Forces_And_Derivatives()
{
	LOG::Time ("UCPF", "UCPF");

	for (int i = 1; i <= collision_penalty_forces.m; i++) collision_penalty_forces (i)->Update_Forces_And_Derivatives();

	LOG::Stop_Time();
}
//#####################################################################
// Function Advance_One_Time_Step
//#####################################################################
template<class T> void DEFORMABLE_OBJECT_3D<T>::
Advance_One_Time_Step (const T time, const T dt, const T cg_tolerance, const int cg_iterations, const bool perform_collision_body_collisions, const bool verbose)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Advance_One_Time_Step_Semi_Implicit
//#####################################################################
template<class T> void DEFORMABLE_OBJECT_3D<T>::
Advance_One_Time_Step_Semi_Implicit (const T time, const T dt, const bool perform_collision_body_collisions, const bool verbose)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Advance_One_Time_Step_Using_Quasistatics
//#####################################################################
template<class T> void DEFORMABLE_OBJECT_3D<T>::
Advance_One_Time_Step_Quasistatic (const T time, const T dt, const T cg_tolerance, const int cg_iterations, const T newton_tolerance, const int newton_iterations,
				   const bool use_partially_converged_result, const bool verbose)
{
	LOG::Push_Scope ("AOTSQ", "AOTSQ");
	// prepare for force computation
	LOG::Time ("AOTSQ - Initialize");
	Enforce_Definiteness (true);
	external_forces_and_velocities->Update_Time_Varying_Material_Properties (time + dt, id_number);

	// Iterate to steady state
	double supnorm = 0;
	int iteration;
	dX_full.Resize_Array (particles.number, false, false), R_full.Resize_Array (particles.number, false, false);
	LOG::Stop_Time();
	LOG::Push_Scope ("AOTSQ - NR loop", "AOTSQ - NR loop");

	for (iteration = 0; iteration < newton_iterations; iteration++)
	{
		LOG::Time ("AOTSQ - NR loop - Initialize");

#ifndef NEW_SERIAL_IMPLEMENTATIOM

		if (PHYSBAM_THREADED_RUN)
		{
#endif
			ARRAY_PARALLEL_OPERATIONS<VECTOR_3D<T>, T, VECTOR_3D<T> >::Clear_Parallel (dX_full, *particles.particle_ranges);
			ARRAY_PARALLEL_OPERATIONS<VECTOR_3D<T>, T, VECTOR_3D<T> >::Clear_Parallel (R_full, *particles.particle_ranges);

#ifndef NEW_SERIAL_IMPLEMENTATIOM
		}
		else
		{
			ARRAY<VECTOR_3D<T> >::copy (VECTOR_3D<T>(), dX_full);
			ARRAY<VECTOR_3D<T> >::copy (VECTOR_3D<T>(), R_full);
		}

#endif

		LOG::Stop_Time();
		Update_Collision_Penalty_Forces_And_Derivatives();

		if (One_Newton_Step_Toward_Steady_State (cg_tolerance, cg_iterations, time + dt, dX_full, false, 0, iteration == 0) || use_partially_converged_result)
			if (PHYSBAM_THREADED_RUN) ARRAY_PARALLEL_OPERATIONS<VECTOR_3D<T>, T, VECTOR_3D<T> >::Add_Scaled_Array_Parallel (dX_full, (T) 1, particles.X.array, *particles.particle_ranges);

#ifndef NEW_SERIAL_IMPLEMENTATIOM
			else particles.X.array += dX_full;

#else
			else ARRAY_PARALLEL_OPERATIONS<VECTOR_3D<T>, T, VECTOR_3D<T> >::Add_Scaled_Array_Parallel (dX_full, (T) 1, particles.X.array, *particles.particle_ranges);

#endif

		Update_Position_Based_State();
		Update_Collision_Penalty_Forces_And_Derivatives();
		Add_Velocity_Independent_Forces (R_full);
		LOG::Time ("AOTSQ - NR loop - Boundary conditions");
		external_forces_and_velocities->Add_External_Forces (R_full, time, id_number);
		external_forces_and_velocities->Zero_Out_Enslaved_Position_Nodes (R_full, time, id_number);
		LOG::Stop_Time();
		LOG::Time ("AOTSQ - NR loop - Compute residual");
		supnorm = (T) 0;

		if (PHYSBAM_THREADED_RUN) supnorm = ARRAY_PARALLEL_OPERATIONS<VECTOR_3D<T>, T, VECTOR_3D<T> >::Maximum_Magnitude_Squared_Parallel (R_full, *particles.particle_ranges);

#ifndef NEW_SERIAL_IMPLEMENTATIOM
		else for (int i = 1; i <= particles.number; i++)
			{
				double s2 = R_full (i).Magnitude_Squared();
				supnorm = max (supnorm, s2);
			}

#else
		else supnorm = ARRAY_PARALLEL_OPERATIONS<VECTOR_3D<T>, T, VECTOR_3D<T> >::Maximum_Magnitude_Squared_Parallel (R_full, *particles.particle_ranges);

#endif

		supnorm = sqrt (supnorm);

		if (print_residuals) LOG::cout << "Newton iteration residual after " << iteration + 1 << " iterations = " << supnorm << std::endl;

		if (supnorm <= newton_tolerance)
		{
			LOG::cout << "Newton converged in " << iteration + 1 << " steps for object " << id_number << std::endl;
			break;
		}

		LOG::Stop_Time();
	}

	LOG::Pop_Scope(); // from AOTSQ - NR loop

	if (iteration >= newton_iterations && print_diagnostics)
	{
		LOG::cout << "Newton iteration did not converge in " << newton_iterations << " for object " << id_number << std::endl;
		LOG::cout << "\tError = " << supnorm << ", Newton tolerance = " << newton_tolerance << std::endl;
	}

	LOG::Pop_Scope(); // from AOTSQ
}

//#####################################################################
// Function Add_Quasistatic_Neo_Hookean_Elasticity
//#####################################################################
template<class T> int DEFORMABLE_OBJECT_3D<T>::
Add_Quasistatic_Neo_Hookean_Elasticity (TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume, const T youngs_modulus, const T poissons_ratio, const bool verbose)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_Diagonalized_Linear_Finite_Volume
//#####################################################################
template<class T> int DEFORMABLE_OBJECT_3D<T>::
Add_Quasistatic_Diagonalized_Linear_Finite_Volume (TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume, const T youngs_modulus, const T poissons_ratio, const bool verbose, const bool precompute_stiffness_matrix)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_Quasistatic_Diagonalized_Neo_Hookean_Elasticity
//#####################################################################
template<class T> int DEFORMABLE_OBJECT_3D<T>::
Add_Quasistatic_Diagonalized_Neo_Hookean_Elasticity (TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume, const T youngs_modulus, const T poissons_ratio, const T failure_threshold,
		const bool verbose, const bool precompute_stiffness_matrix)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_Quasistatic_Diagonalized_Linear_Elasticity
//#####################################################################
template<class T> int DEFORMABLE_OBJECT_3D<T>::
Add_Quasistatic_Diagonalized_Linear_Elasticity (TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume, const T youngs_modulus, const T poissons_ratio, const bool verbose)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_Diagonalized_Mooney_Rivlin
//#####################################################################
template<class T> int DEFORMABLE_OBJECT_3D<T>::
Add_Diagonalized_Mooney_Rivlin (TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume, const T mu_10, const T mu_01, const T kappa, const T Rayleigh_coefficient, const T failure_threshold,
				const bool limit_time_step_by_strain_rate, const T max_strain_per_time_step, const bool use_rest_state_for_strain_rate, const bool verbose)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_Quasistatic_Diagonalized_Mooney_Rivlin
//#####################################################################
template<class T> int DEFORMABLE_OBJECT_3D<T>::
Add_Quasistatic_Diagonalized_Mooney_Rivlin (TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume, const T mu_10, const T mu_01, const T kappa, const T failure_threshold, const bool verbose,
		const bool precompute_stiffness_matrix)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_Diagonalized_Face
//#####################################################################
template<class T> int DEFORMABLE_OBJECT_3D<T>::
Add_Diagonalized_Face (TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume, const LIST_ARRAY<LIST_ARRAY<int> >& muscle_tets, const LIST_ARRAY<LIST_ARRAY<VECTOR_3D<T> > >& muscle_fibers,
		       const LIST_ARRAY<LIST_ARRAY<T> >& muscle_densities, LIST_ARRAY<T>& muscle_activations, const LIST_ARRAY<T>* peak_isometric_stress, const T mu_10, const T mu_01,
		       const T kappa, const T Rayleigh_coefficient, const T failure_threshold, const bool limit_time_step_by_strain_rate, const T max_strain_per_time_step,
		       const bool use_rest_state_for_strain_rate, const bool verbose)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_Quasistatic_Diagonalized_Face
//#####################################################################
template<class T> int DEFORMABLE_OBJECT_3D<T>::
Add_Quasistatic_Diagonalized_Face (TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume, const LIST_ARRAY<LIST_ARRAY<int> >& muscle_tets, const LIST_ARRAY<LIST_ARRAY<VECTOR_3D<T> > >& muscle_fibers,
				   const LIST_ARRAY<LIST_ARRAY<T> >& muscle_densities, LIST_ARRAY<T>& muscle_activations, int *single_activation_used_for_force_derivative, const LIST_ARRAY<T>* peak_isometric_stress,
				   DIAGONALIZED_CONSTITUTIVE_MODEL_3D<T> **face_constitutive_model, const T mu_10, const T mu_01, const T kappa, const T failure_threshold, const bool verbose)
{
	STRAIN_MEASURE_3D<T> *strain_measure = new STRAIN_MEASURE_3D<T> (tetrahedralized_volume);
	strain_measure_3d.Append_Element (strain_measure);
	DIAGONALIZED_FACE_3D<T>* constitutive_model = new DIAGONALIZED_FACE_3D<T> (mu_10, mu_01, kappa, 0, muscle_activations, single_activation_used_for_force_derivative, peak_isometric_stress, failure_threshold);
	constitutive_model->Initialize_Fiber_Data (*strain_measure, muscle_tets, muscle_fibers, muscle_densities);
	diagonalized_constitutive_model_3d.Append_Element (constitutive_model);

	if (face_constitutive_model) *face_constitutive_model = constitutive_model;

	DIAGONALIZED_FINITE_VOLUME_3D<T>* fvm = new DIAGONALIZED_FINITE_VOLUME_3D<T> (*strain_measure, *constitutive_model);
	fvm->Use_Quasistatics();
	fvm->Use_Stiffness_Matrix();
	diagonalized_finite_volume_3d.Append_Element (fvm);
	solids_forces.Append_Element (fvm);
	return solids_forces.m;
}
//#####################################################################

template class DEFORMABLE_OBJECT_3D<float>;
template class DEFORMABLE_OBJECT_3D<double>;
