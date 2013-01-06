//#####################################################################
// Copyright 2004, Ron Fedkiw, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SOLIDS_FLUIDS_DRIVER
//#####################################################################
#ifndef __SOLIDS_FLUIDS_DRIVER__
#define __SOLIDS_FLUIDS_DRIVER__

#include "SOLIDS_FLUIDS_EXAMPLE.h"
#include "../Read_Write/FILE_UTILITIES.h"
namespace PhysBAM
{

template <class T>
class SOLIDS_FLUIDS_DRIVER
{
public:
	SOLIDS_FLUIDS_EXAMPLE<T>& example;
	int current_frame;
	T time;
	int output_number;

	SOLIDS_FLUIDS_DRIVER (SOLIDS_FLUIDS_EXAMPLE<T>& example_input)
		: example (example_input)
	{}

	virtual ~SOLIDS_FLUIDS_DRIVER()
	{}

	T Time_At_Frame (const int frame) const
	{
		return example.initial_time + (frame - example.first_frame) / example.frame_rate;
	}

	virtual void Preprocess_Frame (const int frame)
	{
		example.Preprocess_Frame (frame);
	}

	virtual void Postprocess_Frame (const int frame)
	{
		example.Postprocess_Frame (frame);
	}

	virtual void Write_Output_Files (const int frame)
	{
		FILE_UTILITIES::Create_Directory (example.output_directory);
		example.Write_Output_Files (frame);
		Write_Last_Frame (frame);
	}

	void Write_Last_Frame (const int frame) const
	{
		if (example.write_last_frame) FILE_UTILITIES::Write_To_Text_File (example.output_directory + "/last_frame", frame);
	}

	void Write_Time (const int frame) const
	{
		if (example.write_time)
		{
			FILE_UTILITIES::template Write_To_File<float> (
				STRING_UTILITIES::string_sprintf ("%s/time.%d",
						example.output_directory.c_str(), frame), time);
		}
	}

	void Read_Time (const int frame)
	{
		std::string filename =
			STRING_UTILITIES::string_sprintf ("%s/time.%d",
					example.output_directory.c_str(), frame);

		if (FILE_UTILITIES::File_Exists (filename))
		{
			FILE_UTILITIES::template Read_From_File<float> (filename, time);
			example.initial_time =
				time - (frame - example.first_frame) / example.frame_rate;
		}
	} // adjust initial time so that Simulate_To_Frame() returns correct time (essential when writing substeps)

//#####################################################################
	virtual void Advance_To_Target_Time (const T target_time) {}
	virtual void Execute_Main_Program();
	virtual void Initialize();
	virtual void Simulate_To_Frame (const int frame_input);
//#####################################################################
};
}
#endif

