//#####################################################################
// Copyright 2004-2005, Ron Fedkiw, Geoffrey Irving, Frank Losasso.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SOLIDS_FLUIDS_EXAMPLE
//#####################################################################
#ifndef __SOLIDS_FLUIDS_EXAMPLE__
#define __SOLIDS_FLUIDS_EXAMPLE__

#include <string>
#include "RIGID_BODY_PARAMETERS_CALLBACKS.h"
#include "../Read_Write/FILE_UTILITIES.h"
#include "../Utilities/DEBUG_UTILITIES.h"
#include "../Utilities/STRING_UTILITIES.h"
namespace PhysBAM
{

template <class T>
class SOLIDS_FLUIDS_EXAMPLE: public RIGID_BODY_PARAMETERS_CALLBACKS<T>
{
public:
	T initial_time;
	int first_frame, last_frame;
	T frame_rate;
	bool restart;
	int restart_frame;
	bool verbose, verbose_dt;
	bool write_output_files, write_substeps, write_last_frame, write_time, write_frame_title;
	std::string output_directory, data_directory;
	std::string frame_title;
	T abort_when_dt_below; //
protected:
	T minimum_surface_roughness; // needed for ray tracing, etc.
public:

	SOLIDS_FLUIDS_EXAMPLE()
		: initial_time (0),
		  first_frame (0), last_frame (24), frame_rate (24),
		  restart (false), restart_frame (0),
		  verbose (true), verbose_dt (false),
		  write_output_files (true), write_substeps (false), write_last_frame (true), write_time (false), write_frame_title (false), output_directory ("output"), data_directory ("."),
		  abort_when_dt_below (0)
	{
		Set_Minimum_Surface_Roughness();
	}

	virtual ~SOLIDS_FLUIDS_EXAMPLE()
	{}

	static void Clamp_Time_Step_With_Target_Time (const T time, const T target_time, T& dt, bool& done)
	{
		if (time + dt >= target_time)
		{
			dt = target_time - time;
			done = true;
		}
		else if (time + 2 * dt >= target_time) dt = min (dt, (T).51 * (target_time - time));
	}

	void Set_Minimum_Surface_Roughness (const T minimum_surface_roughness_input = 1e-6)
	{
		minimum_surface_roughness = minimum_surface_roughness_input;
	}

	void Write_Frame_Title (const int frame) const
	{
		if (write_frame_title)
		{
			std::ofstream output (STRING_UTILITIES::string_sprintf ("%s/frame_title.%d", output_directory.c_str(), frame).c_str());
			output << frame_title;
		}
	}

	template<class RW> bool Read_Thin_Shells_Next_Dt (T& next_dt, T& next_dt_levelset, bool& next_done, const int frame)
	{
		std::string filename = STRING_UTILITIES::string_sprintf ("%s/next_dt.%d", output_directory.c_str(), frame);

		if (FILE_UTILITIES::File_Exists (filename))
		{
			FILE_UTILITIES::Read_From_File<RW> (filename, next_dt, next_dt_levelset, next_done);
			return true;
		}
		else return false;
	}

	template<class RW> void Write_Thin_Shells_Next_Dt (const T next_dt, const T next_dt_levelset, const bool next_done, const int frame) const
	{
		FILE_UTILITIES::Write_To_File<RW> (STRING_UTILITIES::string_sprintf ("%s/next_dt.%d", output_directory.c_str(), frame), next_dt, next_dt_levelset, next_done);
	}

//#####################################################################
	virtual void Preprocess_Frame (const int frame)
	{
		WARN_IF_NOT_OVERRIDDEN();
	}
	virtual void Postprocess_Frame (const int frame)
	{
		WARN_IF_NOT_OVERRIDDEN();
	}
	virtual void Limit_Dt (T& dt)
	{
		WARN_IF_NOT_OVERRIDDEN();
	}
	virtual void Write_Output_Files (const int frame) const
	{
		WARN_IF_NOT_OVERRIDDEN();
	}
	// solids
	virtual void Initialize_Fracture()
	{
		WARN_IF_NOT_OVERRIDDEN();
	}
//#####################################################################
};
}
#endif
