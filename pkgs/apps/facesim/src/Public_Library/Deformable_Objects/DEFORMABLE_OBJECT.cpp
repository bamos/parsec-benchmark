//#####################################################################
// Copyright 2003-2006, Zhaosheng Bao, Ron Fedkiw, Geoffrey Irving, Igor Neverov, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class DEFORMABLE_OBJECT
//#####################################################################
#include "DEFORMABLE_OBJECT.h"
#include "../Forces_And_Torques/SOLIDS_FORCES.h"
#include "../Arrays/ARRAY_PARALLEL_OPERATIONS.h"
#include "../Arrays/ARRAY_RANGE.h"
#include "../Utilities/LOG.h"
#include "../Utilities/DEBUG_UTILITIES.h"
#include "../Thread_Utilities/THREAD_POOL.h"
using namespace PhysBAM;
extern bool PHYSBAM_THREADED_RUN;
//#####################################################################
// Function Save_Velocity
//#####################################################################
template<class T, class TV> void DEFORMABLE_OBJECT<T, TV>::
Save_Velocity()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Euler_Step_Position
//#####################################################################
template<class T, class TV> void DEFORMABLE_OBJECT<T, TV>::
Euler_Step_Position (const T dt)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Euler_Step_Velocity
//#####################################################################
template<class T, class TV> void DEFORMABLE_OBJECT<T, TV>::
Euler_Step_Velocity (const T dt, const T time)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Euler_Step_Position_And_Velocity
//#####################################################################
template<class T, class TV> void DEFORMABLE_OBJECT<T, TV>::
Euler_Step_Position_And_Velocity (const T dt, const T time)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Predictor_Corrector_Step_Velocity
//#####################################################################
template<class T, class TV> void DEFORMABLE_OBJECT<T, TV>::
Predictor_Corrector_Step_Velocity (const T dt, const T time, const int corrector_steps)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Predictor_Corrector_Integrate_Velocity
//#####################################################################
template<class T, class TV> void DEFORMABLE_OBJECT<T, TV>::
Predictor_Corrector_Integrate_Velocity (const T start_time, const T end_time, const int corrector_steps)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Backward_Euler_Step_Velocity_With_Fallback
//#####################################################################
template<class T, class TV> void DEFORMABLE_OBJECT<T, TV>::
Backward_Euler_Step_Velocity_With_Fallback (const T dt, const T time, const T convergence_tolerance, const int max_iterations, const bool verbose)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Backward_Euler_Step_Velocity
//#####################################################################
// returns true if converged, false if not (in which case particles->V is reset to old values)
// assumes all solids_forces are linear in velocity, with a symmetric positive definite Jacobian.
#ifndef AGGREGATE_CG_OPERATIONS
template<class T, class TV> bool DEFORMABLE_OBJECT<T, TV>::
Backward_Euler_Step_Velocity (const T dt, const T time, const T convergence_tolerance, const int max_iterations, const bool use_forward_euler_initial_guess, int* iterations_used,
			      const bool damping_only)
{
	NOT_IMPLEMENTED();
}

#else
template<class T, class TV> struct CONJUGATE_GRADIENTS_HELPER
{
	DEFORMABLE_OBJECT<T, TV>* deformable_object;
	int partition_id;
	T time, dt;
	T alpha, beta;
	ARRAY<TV>* dX_full;
	ARRAY<double> *S_dot_Q_partial, *rho_new_partial, *supnorm_partial;
};
template<class T, class TV> void DEFORMABLE_OBJECT<T, TV>::
Backward_Euler_Step_Velocity_CG_Helper_I (long thread_id, void* helper_raw)
{
	NOT_IMPLEMENTED();
}

template<class T, class TV> void DEFORMABLE_OBJECT<T, TV>::
Backward_Euler_Step_Velocity_CG_Helper_II (long thread_id, void* helper_raw)
{
	NOT_IMPLEMENTED();
}

template<class T, class TV> bool DEFORMABLE_OBJECT<T, TV>::
Backward_Euler_Step_Velocity (const T dt, const T time, const T convergence_tolerance, const int max_iterations, const bool use_forward_euler_initial_guess, int* iterations_used,
			      const bool damping_only)
{
	NOT_IMPLEMENTED();
}

#endif
//#####################################################################
// Function One_Newton_Step_Toward_Steady_State
//#####################################################################
#ifndef AGGREGATE_CG_OPERATIONS
template<class T, class TV> bool DEFORMABLE_OBJECT<T, TV>::
One_Newton_Step_Toward_Steady_State (const T convergence_tolerance, const int max_iterations, const T time, ARRAY<TV>& dX, const bool balance_external_forces_only,
				     int* iterations_used, const bool update_positions_and_state)
{
	NOT_IMPLEMENTED();
}

#else
template<class T, class TV> void DEFORMABLE_OBJECT<T, TV>::
One_Newton_Step_Toward_Steady_State_CG_Helper_I (long thread_id, void* helper_raw)
{
	CONJUGATE_GRADIENTS_HELPER<T, TV>const& helper = * (CONJUGATE_GRADIENTS_HELPER<T, TV>*) helper_raw;
	DEFORMABLE_OBJECT<T, TV>& deformable_object = *helper.deformable_object;
	int partition_id = helper.partition_id;
	VECTOR_2D<int> particle_range = (*deformable_object.particles.particle_ranges) (partition_id);
	T beta = helper.beta;
//	ARRAY<TV>& negative_Q_full = deformable_object.F_full;
	ARRAY<TV>& S_full = deformable_object.S_full;
	ARRAY_RANGE<ARRAY<TV> > S (deformable_object.S_full, particle_range);
	ARRAY_RANGE<ARRAY<TV> > R (deformable_object.R_full, particle_range);
//	double& S_dot_Q = (*helper.S_dot_Q_partial) (partition_id);

	for (int i = 1; i <= S.m; i++) S (i) = beta * S (i) + R (i);
}

template<class T,class TV> void DEFORMABLE_OBJECT<T,TV>::
One_Newton_Step_Toward_Steady_State_CG_Helper_II(long thread_id, void * helper_raw)
{
	CONJUGATE_GRADIENTS_HELPER<T,TV>const& helper=*(CONJUGATE_GRADIENTS_HELPER<T,TV>*)helper_raw;
	DEFORMABLE_OBJECT<T,TV>& deformable_object=*helper.deformable_object;
	int partition_id = helper.partition_id;
	VECTOR_2D<int> particle_range=(*deformable_object.particles.particle_ranges)(partition_id);
	ARRAY<TV>& negative_Q_full=deformable_object.F_full;
	ARRAY<TV>& S_full = deformable_object.S_full;
	ARRAY_RANGE<ARRAY<TV> > S(deformable_object.S_full,particle_range);
	double & S_dot_Q=(*helper.S_dot_Q_partial)(partition_id);

	deformable_object.Force_Differential (S_full, negative_Q_full, partition_id);
	deformable_object.external_forces_and_velocities->Zero_Out_Enslaved_Position_Nodes (negative_Q_full, helper.time, deformable_object.id_number, partition_id);
	S_dot_Q = -ARRAY<TV>::template Vector_Dot_Product<double> (S, negative_Q_full.Range (particle_range));
}
template<class T, class TV> void DEFORMABLE_OBJECT<T, TV>::
One_Newton_Step_Toward_Steady_State_CG_Helper_III (long thread_id, void* helper_raw)
{
	CONJUGATE_GRADIENTS_HELPER<T, TV>const& helper = * (CONJUGATE_GRADIENTS_HELPER<T, TV>*) helper_raw;
	DEFORMABLE_OBJECT<T, TV>& deformable_object = *helper.deformable_object;
	int partition_id = helper.partition_id;
	VECTOR_2D<int>& particle_range = (*deformable_object.particles.particle_ranges) (partition_id);
	T alpha = helper.alpha;
	ARRAY_RANGE<ARRAY<TV> > dX (*helper.dX_full, particle_range);
	ARRAY_RANGE<ARRAY<TV> > S (deformable_object.S_full, particle_range);
	ARRAY_RANGE<ARRAY<TV> > R (deformable_object.R_full, particle_range);
	ARRAY_RANGE<ARRAY<TV> > negative_Q (deformable_object.F_full, particle_range);
	double& rho_new = (*helper.rho_new_partial) (partition_id);
	double& supnorm = (*helper.supnorm_partial) (partition_id);
	double local_rho_new = rho_new;
	double local_supnorm = supnorm;

	for (int i = 1; i <= dX.m; i++)
	{
		dX (i) += alpha * S (i);
		R (i) += alpha * negative_Q (i);
		double s2 = R (i).Magnitude_Squared();
		local_rho_new += s2;
		local_supnorm = max (local_supnorm, s2);
	}

	rho_new = local_rho_new;
	supnorm = local_supnorm;

// False sharing version
//    for(int i=1;i<=dX.m;i++){dX(i)+=alpha*S(i);R(i)+=alpha*negative_Q(i);double s2=R(i).Magnitude_Squared();rho_new+=s2;supnorm=max(supnorm,s2);}
}
template<class T, class TV> bool DEFORMABLE_OBJECT<T, TV>::
One_Newton_Step_Toward_Steady_State (const T convergence_tolerance, const int max_iterations, const T time, ARRAY<TV>& dX_full, const bool balance_external_forces_only,
				     int* iterations_used, const bool update_positions_and_state)
{
	LOG::Push_Scope ("NRS", "NRS");
	int i, N = particles.number;
	LOG::Time ("NRS - Initialize");
	dX_full.Resize_Array (N); // an initial guess might be passed in for dX, otherwise it's zero
	R_full.Resize_Array (N, false, false);
	LOG::Time ("NRS - Boundary conditions 1");
	external_forces_and_velocities->Zero_Out_Enslaved_Position_Nodes (dX_full, time, id_number);

	if (update_positions_and_state) external_forces_and_velocities->Set_External_Positions (particles.X.array, time, id_number);

	LOG::Stop_Time();

	if (update_positions_and_state) Update_Position_Based_State();

	Force_Differential (dX_full, R_full);

	if (!balance_external_forces_only) Add_Velocity_Independent_Forces (R_full);

	LOG::Time ("NRS - Boundary conditions 2");
	external_forces_and_velocities->Add_External_Forces (R_full, time, id_number);
	external_forces_and_velocities->Zero_Out_Enslaved_Position_Nodes (R_full, time, id_number);
	LOG::Stop_Time();
	LOG::Push_Scope ("NRS - Compute residual", "NRS - Compute residual");
	double rho = 0, supnorm = 0;

#ifndef NEW_SERIAL_IMPLEMENTATIOM

	if (PHYSBAM_THREADED_RUN)
	{
#endif
		rho = ARRAY_PARALLEL_OPERATIONS<TV, T, TV>::Dot_Product_Parallel (R_full, R_full, *particles.particle_ranges);
		supnorm = ARRAY_PARALLEL_OPERATIONS<TV, T, TV>::Maximum_Magnitude_Squared_Parallel (R_full, *particles.particle_ranges);
#ifndef NEW_SERIAL_IMPLEMENTATIOM
	}
	else
	{
		for (i = 1; i <= N; i++)
		{
			double s2 = R_full (i).Magnitude_Squared();
			rho += s2;
			supnorm = max (supnorm, s2);
		}
	}

#endif

	supnorm = sqrt (supnorm);
	LOG::Pop_Scope(); // from Compute residual

	if (supnorm <= convergence_tolerance) //if(print_diagnostics)LOG::cout<<"no cg iterations needed"<<std::endl;
	{
		if (iterations_used) *iterations_used = 0;

		LOG::Pop_Scope(); // from NRS
		return true;
	} // exit early

	LOG::Time ("NRS - Copy initial guess");
	ARRAY<TV>& negative_Q_full = F_full;
	negative_Q_full.Resize_Array (N, false, false);
	S_full.Resize_Array (N, false, false);
	LOG::Stop_Time();
	//LOG::Push_Scope ("CGI", "CGI");
	int iterations;
	T beta = 0;

	for (iterations = 1; iterations <= max_iterations; iterations++)
	{
//        LOG::Time("CGI - Update solution I");
		double S_dot_Q = 0;

		if (PHYSBAM_THREADED_RUN)
		{
			typedef CONJUGATE_GRADIENTS_HELPER<T, TV> T_CG_HELPER;
			ARRAY<double> S_dot_Q_partial (particles.particle_ranges->m);
			THREAD_POOL& pool = *THREAD_POOL::Singleton();
			ARRAY<T_CG_HELPER> helpers (particles.particle_ranges->m);

			for (int p = 1; p <= particles.particle_ranges->m; p++)
			{
				helpers (p).deformable_object = this;
				helpers (p).partition_id = p;
				helpers (p).time = time;
				helpers (p).beta = beta;
				helpers (p).S_dot_Q_partial = &S_dot_Q_partial;
				pool.Add_Task (One_Newton_Step_Toward_Steady_State_CG_Helper_I, &helpers (p));
			}

			pool.Wait_For_Completion();

		        for(int p = 1; p <= particles.particle_ranges->m; p++){
				pool.Add_Task(One_Newton_Step_Toward_Steady_State_CG_Helper_II,&helpers(p));}
			pool.Wait_For_Completion();

			S_dot_Q = ARRAY<double>::sum (S_dot_Q_partial);
		}
		else
		{
#ifndef NEW_SERIAL_IMPLEMENTATIOM
			ARRAY<TV>::copy_up_to (beta, S_full, R_full, S_full, N);
			Force_Differential (S_full, negative_Q_full);
			external_forces_and_velocities->Zero_Out_Enslaved_Position_Nodes (negative_Q_full, time, id_number);

			for (i = 1; i <= N; i++) S_dot_Q -= TV::Dot_Product (S_full (i), negative_Q_full (i));

#else
			typedef CONJUGATE_GRADIENTS_HELPER<T, TV> T_CG_HELPER;
			ARRAY<double> S_dot_Q_partial (particles.particle_ranges->m);
			ARRAY<T_CG_HELPER> helpers (particles.particle_ranges->m);

			helpers (1).deformable_object = this;
			helpers (1).partition_id = 1;
			helpers (1).time = time;
			helpers (1).beta = beta;
			helpers (1).S_dot_Q_partial = &S_dot_Q_partial;
			One_Newton_Step_Toward_Steady_State_CG_Helper_I (1, &helpers (1));
			One_Newton_Step_Toward_Steady_State_CG_Helper_II (1, &helpers (1));

			S_dot_Q = ARRAY<double>::sum (S_dot_Q_partial);
#endif
		}

		T alpha = (T) (rho / S_dot_Q);
		double rho_new = 0;

//        LOG::Stop_Time();
//        LOG::Time("CGI - Update solution II");
		if (PHYSBAM_THREADED_RUN)
		{
			typedef CONJUGATE_GRADIENTS_HELPER<T, TV> T_CG_HELPER;
			ARRAY<double> rho_new_partial (particles.particle_ranges->m);
			ARRAY<double> supnorm_partial (particles.particle_ranges->m);
			THREAD_POOL& pool = *THREAD_POOL::Singleton();
			ARRAY<T_CG_HELPER> helpers (particles.particle_ranges->m);

			for (int p = 1; p <= particles.particle_ranges->m; p++)
			{
				helpers (p).deformable_object = this;
				helpers (p).partition_id = p;
				helpers (p).alpha = alpha;
				helpers (p).dX_full = &dX_full;
				helpers (p).rho_new_partial = &rho_new_partial;
				helpers (p).supnorm_partial = &supnorm_partial;
				pool.Add_Task (One_Newton_Step_Toward_Steady_State_CG_Helper_III, &helpers (p));
			}

			pool.Wait_For_Completion();
			rho_new = ARRAY<double>::sum (rho_new_partial);
			supnorm = ARRAY<double>::max (supnorm_partial);
		}
		else
		{
#ifndef NEW_SERIAL_IMPLEMENTATIOM
			supnorm = 0;

			for (i = 1; i <= N; i++)
			{
				dX_full (i) += alpha * S_full (i);
				R_full (i) += alpha * negative_Q_full (i);
				double s2 = R_full (i).Magnitude_Squared();
				rho_new += s2;
				supnorm = max (supnorm, s2);
			}

#else
			typedef CONJUGATE_GRADIENTS_HELPER<T, TV> T_CG_HELPER;
			ARRAY<double> rho_new_partial (particles.particle_ranges->m);
			ARRAY<double> supnorm_partial (particles.particle_ranges->m);
			ARRAY<T_CG_HELPER> helpers (particles.particle_ranges->m);

			helpers (1).deformable_object = this;
			helpers (1).partition_id = 1;
			helpers (1).alpha = alpha;
			helpers (1).dX_full = &dX_full;
			helpers (1).rho_new_partial = &rho_new_partial;
			helpers (1).supnorm_partial = &supnorm_partial;
			One_Newton_Step_Toward_Steady_State_CG_Helper_III (1, &helpers (1));

			rho_new = ARRAY<double>::sum (rho_new_partial);
			supnorm = ARRAY<double>::max (supnorm_partial);
#endif
		}

		supnorm = sqrt (supnorm);

//        LOG::Stop_Time();
		//if(print_residuals) LOG::cout << supnorm << std::endl;
		if (supnorm <= convergence_tolerance) break;

		beta = (T) (rho_new / rho);
		rho = rho_new;
	}

	if (iterations_used) *iterations_used = min (iterations, max_iterations);

	//LOG::Pop_Scope(); // from CGI

	if (iterations <= max_iterations)
	{
//        if(print_diagnostics) LOG::cout << "cg iterations = " << iterations << std::endl;
		LOG::Pop_Scope(); // from NRS
		return true;
	}
	else
	{
//        if(print_diagnostics) LOG::cout << "cg not converged after " << max_iterations << " iterations  - error = " << supnorm << std::endl;
		LOG::Pop_Scope(); // from NRS
		return false;
	}

	LOG::Pop_Scope();
}
#endif
//#####################################################################
// Function Update_Position_Based_State
//#####################################################################
template<class T, class TV> void DEFORMABLE_OBJECT<T, TV>::
Update_Position_Based_State()
{
	LOG::Push_Scope ("UPBS", "UPBS");

	for (int k = 1; k <= solids_forces.m; k++) if (solids_forces (k)->use_position_based_state) solids_forces (k)->Update_Position_Based_State();

	LOG::Pop_Scope();
}
//#####################################################################
// Function Delete_Position_Based_State
//#####################################################################
template<class T, class TV> void DEFORMABLE_OBJECT<T, TV>::
Delete_Position_Based_State()
{
	for (int k = 1; k <= solids_forces.m; k++) if (solids_forces (k)->use_position_based_state) solids_forces (k)->Delete_Position_Based_State();
}
//#####################################################################
// Function Add_Velocity_Independent_Forces
//#####################################################################
template<class T, class TV> void DEFORMABLE_OBJECT<T, TV>::
Add_Velocity_Independent_Forces (ARRAY<TV>& F) const
{
	LOG::Push_Scope ("AVIF", "AVIF");

	for (int k = 1; k <= solids_forces.m; k++) if (solids_forces (k)->use_velocity_independent_forces) solids_forces (k)->Add_Velocity_Independent_Forces (F);

	LOG::Pop_Scope();
}
//#####################################################################
// Function Add_Velocity_Dependent_Forces
//#####################################################################
// can depend on position too
template<class T, class TV> void DEFORMABLE_OBJECT<T, TV>::
Add_Velocity_Dependent_Forces (ARRAY<TV>& F) const
{
	LOG::Push_Scope ("AVDF", "AVDF");

	for (int k = 1; k <= solids_forces.m; k++) if (solids_forces (k)->use_velocity_dependent_forces) solids_forces (k)->Add_Velocity_Dependent_Forces (F);

	LOG::Pop_Scope();
}
template<class T, class TV> void DEFORMABLE_OBJECT<T, TV>::
Add_Velocity_Dependent_Forces (ARRAY<TV>& F, const int partition_id) const
{
	for (int k = 1; k <= solids_forces.m; k++) if (solids_forces (k)->use_velocity_dependent_forces) solids_forces (k)->Add_Velocity_Dependent_Forces (F, partition_id);
}
//#####################################################################
// Function Force_Differential
//#####################################################################
template<class T, class TV> void DEFORMABLE_OBJECT<T, TV>::
Force_Differential (const ARRAY<TV>& dX, ARRAY<TV>& dF) const
{
	//LOG::Push_Scope("AFD","AFD");
	//LOG::Time("AFD - Initialize");
	dF.Resize_Array (particles.number);

	if (PHYSBAM_THREADED_RUN) ARRAY_PARALLEL_OPERATIONS<TV, T, TV>::Clear_Parallel (dF, *particles.particle_ranges);

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	else ARRAY<TV>::copy (TV(), dF);

#else
	else ARRAY_PARALLEL_OPERATIONS<TV, T, TV>::Clear_Parallel (dF, *particles.particle_ranges);

#endif

	//LOG::Stop_Time();
	for (int k = 1; k <= solids_forces.m; k++) solids_forces (k)->Add_Force_Differential (dX, dF);

	//LOG::Pop_Scope();
}
template<class T, class TV> void DEFORMABLE_OBJECT<T, TV>::
Force_Differential (const ARRAY<TV>& dX, ARRAY<TV>& dF, const int partition_id) const
{
	assert (dF.m == particles.number);
	ARRAY<TV>::copy (TV(), dF.Range ( (*particles.particle_ranges) (partition_id)));

	for (int k = 1; k <= solids_forces.m; k++) solids_forces (k)->Add_Force_Differential (dX, dF, partition_id);
}
//#####################################################################
// Function Enforce_Definiteness
//#####################################################################
template<class T, class TV> void DEFORMABLE_OBJECT<T, TV>::
Enforce_Definiteness (const bool enforce_definiteness_input)
{
	for (int i = 1; i <= solids_forces.m; i++) solids_forces (i)->Enforce_Definiteness (enforce_definiteness_input);
}
//#####################################################################
// Function CFL
//#####################################################################
template<class T, class TV> T DEFORMABLE_OBJECT<T, TV>::
CFL (const bool verbose)
{
	T dt_elastic_and_damping = CFL_Elastic_And_Damping(), dt_strain_rate = CFL_Strain_Rate();

	if (verbose)
	{
		LOG::cout << "dt_elastic_and_damping = " << dt_elastic_and_damping << std::endl;
		LOG::cout << "dt_strain_rate = " << dt_strain_rate << std::endl;
		LOG::cout << "min = " << min (dt_elastic_and_damping, dt_strain_rate) << std::endl;
	}

	return min (dt_elastic_and_damping, dt_strain_rate);
}
//#####################################################################
// Function CFL_Elastic_And_Damping
//#####################################################################
template<class T, class TV> T DEFORMABLE_OBJECT<T, TV>::
CFL_Elastic_And_Damping()
{
	T dt_elastic = CFL_Elastic();
	T dt_damping = FLT_MAX;

	if (!implicit_damping) dt_damping = CFL_Damping();

	T one_over_dt_full = (1 / dt_elastic + 1 / dt_damping) / cfl_number;

	if (one_over_dt_full  > 1 / FLT_MAX) return 1 / one_over_dt_full;
	else return FLT_MAX;
}
//#####################################################################
// Function CFL_Elastic
//#####################################################################
template<class T, class TV> T DEFORMABLE_OBJECT<T, TV>::
CFL_Elastic()
{
	T hertz = 0;

	for (int k = 1; k <= solids_forces.m; k++) if (solids_forces (k)->use_velocity_independent_forces) hertz += 1 / solids_forces (k)->CFL_Velocity_Independent();

	if (hertz > 1 / FLT_MAX) return 1 / hertz;
	else return FLT_MAX;
}
//#####################################################################
// Function CFL_Damping
//#####################################################################
template<class T, class TV> T DEFORMABLE_OBJECT<T, TV>::
CFL_Damping()
{
	T hertz = 0;

	for (int k = 1; k <= solids_forces.m; k++) if (solids_forces (k)->use_velocity_dependent_forces) hertz += 1 / solids_forces (k)->CFL_Velocity_Dependent();

	if (hertz > 1 / FLT_MAX) return 1 / hertz;
	else return FLT_MAX;
}
//#####################################################################
// Function CFL_Strain_Rate
//#####################################################################
template<class T, class TV> T DEFORMABLE_OBJECT<T, TV>::
CFL_Strain_Rate()
{
	T hertz = 0;

	for (int k = 1; k <= solids_forces.m; k++) if (solids_forces (k)->limit_time_step_by_strain_rate) hertz = max (hertz, 1 / solids_forces (k)->CFL_Strain_Rate()); // otherwise not included

	if (hertz > 1 / FLT_MAX) return 1 / hertz;
	else return FLT_MAX;
}
//#####################################################################


template class DEFORMABLE_OBJECT<float, VECTOR_3D<float> >;
template class DEFORMABLE_OBJECT<double, VECTOR_3D<double> >;
template class DEFORMABLE_OBJECT<float, VECTOR_2D<float> >;
template class DEFORMABLE_OBJECT<double, VECTOR_2D<double> >;
