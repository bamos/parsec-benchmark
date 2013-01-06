//#####################################################################
// Copyright 2004, Eftychios Sifakis, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class QUASISTATICS_DRIVER
//#####################################################################
#ifndef __QUASISTATICS_DRIVER__
#define __QUASISTATICS_DRIVER__

#include "../../Public_Library/Forces_And_Torques/EXTERNAL_FORCES_AND_VELOCITIES.h"
#include "../../Public_Library/Rigid_Bodies/RIGID_BODY_COLLISIONS_3D.h"
#include "../../Public_Library/Solids_And_Fluids/SOLIDS_FLUIDS_DRIVER_3D.h"
#include "QUASISTATICS_EXAMPLE.h"
namespace PhysBAM
{

template <class T, class RW>
class QUASISTATICS_DRIVER: public SOLIDS_FLUIDS_DRIVER_3D<T, RW>
{
public:
	using SOLIDS_FLUIDS_DRIVER_3D<T, RW>::solids_evolution;
	QUASISTATICS_EXAMPLE<T, RW>& example;
	RIGID_BODY_COLLISIONS_3D<T>* rigid_body_collisions;

	QUASISTATICS_DRIVER (QUASISTATICS_EXAMPLE<T, RW>& example_input)
		: SOLIDS_FLUIDS_DRIVER_3D<T, RW> (example_input), example (example_input)
	{
		solids_evolution.quasistatic = true;
		solids_evolution.newton_tolerance = example.newton_tolerance;
		solids_evolution.newton_iterations = example.newton_iterations;
		solids_evolution.use_partially_converged_result = example.use_partially_converged_result;
	}

	virtual ~QUASISTATICS_DRIVER()
	{}
//#####################################################################
};
}
#endif

