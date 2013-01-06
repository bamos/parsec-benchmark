//#####################################################################
// Copyright 2004, Ron Fedkiw, Eran Guendelman, Geoffrey Irving, Andrew Selle, Eftychios Sifakis, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SOLIDS_EVOLUTION_3D
//#####################################################################
#ifndef __SOLIDS_EVOLUTION_3D__
#define __SOLIDS_EVOLUTION_3D__

#include "SOLIDS_EVOLUTION_CALLBACKS.h"
namespace PhysBAM
{

template<class T> class VECTOR_3D;
template<class T> class ARRAY;
template<class T> class LIST_ARRAY;
template<class T> class SOLIDS_PARAMETERS_3D;
template<class T> class RIGID_BODY_COLLISIONS_3D;
template<class T> class RIGID_BODY_EVOLUTION_3D;

template<class T>
class SOLIDS_EVOLUTION_3D
{
public:
	SOLIDS_PARAMETERS_3D<T>& solids_parameters;
	RIGID_BODY_COLLISIONS_3D<T>* rigid_body_collisions;
	RIGID_BODY_EVOLUTION_3D<T>* rigid_body_evolution;
	T time;
	bool quasistatic;
	T newton_tolerance;
	int newton_iterations;
	bool use_partially_converged_result;
	bool verbose;
	SOLIDS_EVOLUTION_CALLBACKS<T, VECTOR_3D<T> >* solids_evolution_callbacks;
private:
	SOLIDS_EVOLUTION_CALLBACKS<T, VECTOR_3D<T> > solids_evolution_callbacks_default;
public:

	SOLIDS_EVOLUTION_3D (SOLIDS_PARAMETERS_3D<T>& solids_parameters_input, const bool verbose_input = true)
		: solids_parameters (solids_parameters_input), rigid_body_collisions (0), rigid_body_evolution (0), time (0), quasistatic (false), newton_tolerance ( (T) 1e-3),
		  newton_iterations (200), use_partially_converged_result (true), verbose (verbose_input), solids_evolution_callbacks (&solids_evolution_callbacks_default)
	{}

	~SOLIDS_EVOLUTION_3D();

	void Set_Solids_Evolution_Callbacks (SOLIDS_EVOLUTION_CALLBACKS<T, VECTOR_3D<T> >& solids_evolution_callbacks_input)
	{
		solids_evolution_callbacks = &solids_evolution_callbacks_input;
	}

//#####################################################################
	void Initialize_Deformable_Objects (const T frame_rate, const bool restart = false, const bool verbose_dt = false);
	void Initialize_Rigid_Bodies (const T frame_rate);
	void Advance_To_Target_Time (const T target_time, const bool verbose_dt);
	void Advance_Deformable_Bodies_To_Target_Time (const T target_time, const bool verbose_dt);
	void Advance_Deformable_Objects_In_Time (const T final_time, const int total_loops, const bool verbose_dt);
//#####################################################################
};
}
#endif

