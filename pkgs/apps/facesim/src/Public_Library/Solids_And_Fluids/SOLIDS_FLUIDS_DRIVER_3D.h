//#####################################################################
// Copyright 2004, Ron Fedkiw, Eran Guendelman, Geoffrey Irving, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SOLIDS_FLUIDS_DRIVER_3D
//#####################################################################
#ifndef __SOLIDS_FLUIDS_DRIVER_3D__
#define __SOLIDS_FLUIDS_DRIVER_3D__

#include "SOLIDS_FLUIDS_DRIVER.h"
#include "SOLIDS_FLUIDS_EXAMPLE_3D.h"
#include "SOLIDS_EVOLUTION_3D.h"
namespace PhysBAM
{

template <class T, class RW>
class SOLIDS_FLUIDS_DRIVER_3D: public SOLIDS_FLUIDS_DRIVER<T>
{
public:
	using SOLIDS_FLUIDS_DRIVER<T>::time;
	using SOLIDS_FLUIDS_DRIVER<T>::current_frame;
	using SOLIDS_FLUIDS_DRIVER<T>::Time_At_Frame;

	SOLIDS_FLUIDS_EXAMPLE_3D<T, RW>& example;
	SOLIDS_EVOLUTION_3D<T> solids_evolution;

	SOLIDS_FLUIDS_DRIVER_3D (SOLIDS_FLUIDS_EXAMPLE_3D<T, RW>& example_input)
		: SOLIDS_FLUIDS_DRIVER<T> (example_input), example (example_input), solids_evolution (example.solids_parameters, example.verbose)
	{}

	virtual ~SOLIDS_FLUIDS_DRIVER_3D()
	{}

//#####################################################################
	void Initialize();
	void Preroll_Solids (const int preroll_frame);
	void Advance_To_Target_Time (const T target_time);
	void Postprocess_Frame (const int frame);
//#####################################################################
};
}
#endif

