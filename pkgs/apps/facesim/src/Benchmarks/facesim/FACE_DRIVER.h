//#####################################################################
// Copyright 2004-2005, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class FACE_DRIVER
//#####################################################################
#ifndef __FACE_DRIVER__
#define __FACE_DRIVER__

#include "QUASISTATICS_DRIVER.h"
#include "FACE_EXAMPLE.h"
namespace PhysBAM
{

template <class T, class RW>
class FACE_DRIVER: public QUASISTATICS_DRIVER<T, RW>
{
public:
	using SOLIDS_FLUIDS_DRIVER_3D<T, RW>::solids_evolution;
	using SOLIDS_FLUIDS_DRIVER<T>::current_frame;
	FACE_EXAMPLE<T, RW>& example;

	FACE_DRIVER (FACE_EXAMPLE<T, RW>& example_input)
		: QUASISTATICS_DRIVER<T, RW> (example_input), example (example_input)
	{}

	virtual ~FACE_DRIVER()
	{}

	void Write_Output_Files (const int frame)
	{
		QUASISTATICS_DRIVER<T, RW>::Write_Output_Files (frame);

		if (example.optimization && example.write_last_step) Write_Last_Step (frame, 0);
	}

	void Write_Last_Step (const int frame, const int step) const
	{
		std::string f = STRING_UTILITIES::string_sprintf (".%d", frame);
		FILE_UTILITIES::Write_To_Text_File<int> (example.output_directory + "/last_step" + f, step);
	}

//#####################################################################
	void Initialize();
	void Advance_To_Target_Time (const T target_time);
//#####################################################################
};
}
#endif
