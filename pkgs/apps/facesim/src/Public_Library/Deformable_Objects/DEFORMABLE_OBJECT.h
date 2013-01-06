//#####################################################################
// Copyright 2003-2006, Zhaosheng Bao, Ronald Fedkiw, Geoffrey Irving, Igor Neverov, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class DEFORMABLE_OBJECT
//#####################################################################
#ifndef __DEFORMABLE_OBJECT__
#define __DEFORMABLE_OBJECT__

#include "../Arrays/LIST_ARRAY.h"
#include "../Particles/SOLIDS_PARTICLE.h"
#include "../Forces_And_Torques/EXTERNAL_FORCES_AND_VELOCITIES.h"
#define AGGREGATE_CG_OPERATIONS
namespace PhysBAM
{

template<class T, class TV> class SOLIDS_FORCES;

template<class T, class TV>
class DEFORMABLE_OBJECT
{
public:
	SOLIDS_PARTICLE<T, TV> particles;
	LIST_ARRAY<SOLIDS_FORCES<T, TV>*> solids_forces;
	EXTERNAL_FORCES_AND_VELOCITIES<T, TV>* external_forces_and_velocities;
	ARRAY<TV> V_save; // save velocity in time stepping
	T cfl_number;
	bool implicit_damping;
	int id_number;
	bool print_diagnostics;
	bool print_residuals;
	bool simulate;
private:
	EXTERNAL_FORCES_AND_VELOCITIES<T, TV> external_forces_and_velocities_default;
public:
	mutable ARRAY<TV> F_full, R_full, S_full, V_start_full, V_end_full, dX_full;

	DEFORMABLE_OBJECT()
		: external_forces_and_velocities (&external_forces_and_velocities_default), id_number (1), simulate (true)
	{
		Print_Diagnostics (false);
		Print_Residuals (false);
		Set_CFL_Number();
		Set_Implicit_Damping();
	}

	virtual ~DEFORMABLE_OBJECT()
	{}

	void Print_Diagnostics (const bool print_diagnostics_input = true)
	{
		print_diagnostics = print_diagnostics_input;
	}

	void Print_Residuals (const bool print_residuals_input = true)
	{
		print_residuals = print_residuals_input;
	}

	void Set_External_Forces_And_Velocities (EXTERNAL_FORCES_AND_VELOCITIES<T, TV>& external_forces_and_velocities_input, const int id_number_input = 1)
	{
		external_forces_and_velocities = &external_forces_and_velocities_input;
		id_number = id_number_input;
	}

	void Set_CFL_Number (const T cfl_number_input = .5)
	{
		cfl_number = cfl_number_input;
	}

	void Set_Implicit_Damping (const bool implicit_damping_input = true)
	{
		implicit_damping = implicit_damping_input;
	}

	void Add_All_Forces (ARRAY<TV>& F, const bool damping_only = false)
	{
		if (!damping_only) Add_Velocity_Independent_Forces (F);

		Add_Velocity_Dependent_Forces (F);
	}

	void Restore_Velocity()
	{
		ARRAY<TV>::exchange_arrays (V_save, particles.V.array);
	}

//#####################################################################
	void Update_Position_Based_State();
	void Delete_Position_Based_State();
	void Save_Velocity();
	void Euler_Step_Position (const T dt);
	void Euler_Step_Velocity (const T dt, const T time);
	void Euler_Step_Position_And_Velocity (const T dt, const T time);
	void Predictor_Corrector_Step_Velocity (const T dt, const T time, const int corrector_steps = 1);
	void Predictor_Corrector_Integrate_Velocity (const T start_time, const T end_time, const int corrector_steps = 1);
	void Backward_Euler_Step_Velocity_With_Fallback (const T dt, const T time, const T convergence_tolerance = 1e-6, const int max_iterations = 50, const bool verbose = false);
#ifdef AGGREGATE_CG_OPERATIONS
	static void Backward_Euler_Step_Velocity_CG_Helper_I (long thread_id, void* helper_raw);
	static void Backward_Euler_Step_Velocity_CG_Helper_II (long thread_id, void* helper_raw);
#endif
	bool Backward_Euler_Step_Velocity (const T dt, const T time, const T convergence_tolerance = 1e-6, const int max_iterations = 50, const bool use_forward_euler_initial_guess = false, int* iterations_used = 0,
					   const bool damping_only = false);
#ifdef AGGREGATE_CG_OPERATIONS
	static void One_Newton_Step_Toward_Steady_State_CG_Helper_I (long thread_id, void* helper_raw);
	static void One_Newton_Step_Toward_Steady_State_CG_Helper_II (long thread_id, void* helper_raw);
	static void One_Newton_Step_Toward_Steady_State_CG_Helper_III (long thread_id, void* helper_raw);
#endif
	bool One_Newton_Step_Toward_Steady_State (const T convergence_tolerance, const int max_iterations, const T time, ARRAY<TV>& dX, const bool balance_external_forces_only = false,
			int* iterations_used = 0, const bool update_positions_and_state = true);
	void Add_Velocity_Independent_Forces (ARRAY<TV>& F) const;
	void Add_Velocity_Dependent_Forces (ARRAY<TV>& F) const;
	void Add_Velocity_Dependent_Forces (ARRAY<TV>& F, const int partition_id) const;
	void Force_Differential (const ARRAY<TV>& dX, ARRAY<TV>& dF) const;
	void Force_Differential (const ARRAY<TV>& dX, ARRAY<TV>& dF, const int partition_id) const;
	void Enforce_Definiteness (const bool enforce_definiteness_input = true);
	T CFL (const bool verbose = false);
	T CFL_Elastic_And_Damping();
	T CFL_Elastic();
	T CFL_Damping();
	T CFL_Strain_Rate();
//#####################################################################
};
}
#endif
