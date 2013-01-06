//#####################################################################
// Copyright 2004-2005, Igor Neverov, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class FACE_LANDMARK_OPTIMIZATION_GOAL
//#####################################################################
#ifndef __FACE_LANDMARK_OPTIMIZATION_GOAL__
#define __FACE_LANDMARK_OPTIMIZATION_GOAL__

#include "FACE_OPTIMIZATION_GOAL.h"
#include "LANDMARK_3D.h"

namespace PhysBAM
{
template <class T>
class FACE_LANDMARK_OPTIMIZATION_GOAL: public FACE_OPTIMIZATION_GOAL<T>
{
public:
	ARRAY<LANDMARK_3D<T> > embedded_landmarks;
	ARRAY<VECTOR_3D<T> > target_landmarks;
	ARRAYS_1D<ARRAY<VECTOR_3D<T> > > target_landmark_trajectories;
	int target_start, target_end, target_stride;
	std::string input_prefix;

	FACE_LANDMARK_OPTIMIZATION_GOAL (const std::string& input_path, const bool preload_target_sequence = true,
					 const int target_start_input = 1, const int target_end_input = 1, const int target_stride_input = 1)
		: target_start (target_start_input), target_end (target_end_input), target_stride (target_stride_input)
	{
		if (preload_target_sequence) FILE_UTILITIES::Read_From_File<float> (input_path, target_landmark_trajectories);
		else input_prefix = input_path;
	}

	void Write_Goal_Data (const std::string& output_prefix, const int frame_input) const
	{
		std::string f = STRING_UTILITIES::string_sprintf (".%d", frame_input);
		FILE_UTILITIES::Write_To_File<T> (output_prefix + "target_markers" + f, target_landmarks);
	}

	int Last_Frame() const
	{
		return (target_end - target_start) / target_stride + 1;
	}

	void Default() const
	{
		std::cout << "THIS FACE_LANDMARK_OPTIMIZATION_GOAL FUNCTION IS NOT DEFINED!" << std::endl;
	}

//#####################################################################
	void Update_Target (const int frame);
	void Initialize_Embedded_Landmarks_To_Particles (const int number_of_particles);
	void Initialize_Embedded_Landmarks_From_File (const std::string& filename);
//#####################################################################
};
}
#endif
