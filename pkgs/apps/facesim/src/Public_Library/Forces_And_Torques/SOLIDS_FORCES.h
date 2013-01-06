//#####################################################################
// Copyright 2002-2004, Zhaosheng Bao, Geoffrey Irving, Igor Neverov, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SOLIDS_FORCES
//#####################################################################
#ifndef __SOLIDS_FORCES__
#define __SOLIDS_FORCES__

#include <stdlib.h>
#include <float.h>
#include <assert.h>
#include <iostream>
#include "../Utilities/DEBUG_UTILITIES.h"
namespace PhysBAM
{

template<class T> class ARRAY;
template<class T, class TV> class SOLIDS_PARTICLE;

template<class T, class TV>
class SOLIDS_FORCES
{
public:
	SOLIDS_PARTICLE<T, TV>& particles;
	bool CFL_initialized; // 1 if precalculation is done, 0 if out of date
	T CFL_elastic_time_step, CFL_damping_time_step;
	bool use_rest_state_for_strain_rate;
	bool limit_time_step_by_strain_rate;
	T max_strain_per_time_step; // for limiting the timestep in the CFL calculation
	bool use_velocity_independent_forces, use_velocity_dependent_forces;
	bool use_position_based_state;

	SOLIDS_FORCES (SOLIDS_PARTICLE<T, TV>& particles_input)
		: particles (particles_input), CFL_initialized (false), CFL_elastic_time_step (FLT_MAX), CFL_damping_time_step (FLT_MAX)
	{
		Use_Rest_State_For_Strain_Rate (false);
		Limit_Time_Step_By_Strain_Rate();
		Use_Velocity_Independent_Forces();
		Use_Velocity_Dependent_Forces();
		Use_Position_Based_State();
	}

	virtual ~SOLIDS_FORCES()
	{}

	void Use_Velocity_Independent_Forces (const bool use_velocity_independent_forces_input = true)
	{
		use_velocity_independent_forces = use_velocity_independent_forces_input;
	}

	void Use_Velocity_Dependent_Forces (const bool use_velocity_dependent_forces_input = true)
	{
		use_velocity_dependent_forces = use_velocity_dependent_forces_input;
	}

	void Use_Position_Based_State (const bool use_position_based_state_input = true)
	{
		use_position_based_state = use_position_based_state_input;
	}

	virtual void Use_Rest_State_For_Strain_Rate (const bool use_rest_state_for_strain_rate_input = true)
	{
		use_rest_state_for_strain_rate = use_rest_state_for_strain_rate_input;
	}

	virtual void Limit_Time_Step_By_Strain_Rate (const bool limit_time_step_by_strain_rate_input = true, const T max_strain_per_time_step_input = .1)
	{
		limit_time_step_by_strain_rate = limit_time_step_by_strain_rate_input;
		assert (max_strain_per_time_step_input > 0);
		max_strain_per_time_step = max_strain_per_time_step_input;
	}

	T CFL (const T cfl_number = 1)
	{
		T dt_V_independent = FLT_MAX, dt_V_dependent = FLT_MAX, dt_full = FLT_MAX, dt_strain_rate = FLT_MAX;

		if (use_velocity_independent_forces) dt_V_independent = CFL_Velocity_Independent();

		if (use_velocity_dependent_forces) dt_V_dependent = CFL_Velocity_Dependent();

		T one_over_dt_full = 1 / dt_V_independent + 1 / dt_V_dependent;

		if (one_over_dt_full) dt_full = cfl_number / one_over_dt_full;

		if (limit_time_step_by_strain_rate) dt_strain_rate = CFL_Strain_Rate();

		return min (dt_full, dt_strain_rate);
	}

	virtual T CFL_Velocity_Independent()
	{
		if (!CFL_initialized) Initialize_CFL();

		return CFL_elastic_time_step;
	}

	virtual T CFL_Velocity_Dependent()
	{
		if (!CFL_initialized) Initialize_CFL();

		return CFL_damping_time_step;
	}

	void Default() const
	{
		std::cout << "THIS SOLIDS_FORCES FUNCTION IS NOT DEFINED!" << std::endl;
		assert (false);
	}

//#####################################################################
	virtual void Update_Position_Based_State() {}
	virtual void Update_Position_Based_State (const int partition_id)
	{
		NOT_IMPLEMENTED();
	}
	virtual void Delete_Position_Based_State() {}
	virtual void Add_Velocity_Independent_Forces (ARRAY<TV>& F) const
	{
		Default();
		exit (1);
	}
	virtual void Add_Velocity_Independent_Forces (ARRAY<TV>& F, const int partition_id) const
	{
		NOT_IMPLEMENTED();
	}
	virtual void Add_Velocity_Dependent_Forces (ARRAY<TV>& F) const
	{
		Default();
		exit (1);
	}
	virtual void Add_Velocity_Dependent_Forces (ARRAY<TV>& F, const int partition_id) const
	{
		NOT_IMPLEMENTED();
	}
	virtual void Add_Force_Differential (const ARRAY<TV>& dX, ARRAY<TV>& dF) const
	{
		Default();
		exit (1);
	}
	virtual void Add_Force_Differential (const ARRAY<TV>& dX, ARRAY<TV>& dF, const int partition_id) const
	{
		NOT_IMPLEMENTED();
	}
	virtual void Enforce_Definiteness (const bool enforce_definiteness_input)
	{
		Default();
		exit (1);
	}
	virtual void Initialize_CFL() {} // used to precalculate certain things for speed
	virtual T CFL_Strain_Rate() const
	{
		return FLT_MAX;
	}
//#####################################################################
};
}
#endif
