//#####################################################################
// Copyright 2004, Igor Neverov, Eftychios Sifakis
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class FACE_LANDMARK_OPTIMIZATION_GOAL
//#####################################################################
#include "FACE_LANDMARK_OPTIMIZATION_GOAL.h"
using namespace PhysBAM;
template class FACE_LANDMARK_OPTIMIZATION_GOAL<float>;
template class FACE_LANDMARK_OPTIMIZATION_GOAL<double>;
//#####################################################################
// Function Update_Target
//#####################################################################
template<class T> void FACE_LANDMARK_OPTIMIZATION_GOAL<T>::
Update_Target (const int frame)
{
	int target_index = target_start + (frame - 1) * target_stride;
	assert (target_index <= target_end);
	std::cout << "Target frame : " << target_index << std::endl;

	if (target_landmark_trajectories.m)
	{
		target_landmarks = target_landmark_trajectories (target_index);
		return;
	}

	std::string f = STRING_UTILITIES::string_sprintf (".%d", target_index);
	FILE_UTILITIES::Read_From_File<float> (input_prefix + f, target_landmarks);
}
//#####################################################################
// Function Initialize_Embedded_Landmarks_To_Particles
//#####################################################################
template<class T> void FACE_LANDMARK_OPTIMIZATION_GOAL<T>::
Initialize_Embedded_Landmarks_To_Particles (const int number_of_particles)
{
	embedded_landmarks.Resize_Array (number_of_particles);

	for (int i = 1; i <= number_of_particles; i++) embedded_landmarks (i) = LANDMARK_3D<T> (i, i, i, i, VECTOR_3D<T> (0, 0, 0));
}
//#####################################################################
// Function Initialize_Embedded_Landmarks_To_Particles
//#####################################################################
template <class T> void FACE_LANDMARK_OPTIMIZATION_GOAL<T>::
Initialize_Embedded_Landmarks_From_File (const std::string& filename)
{
	FILE_UTILITIES::Read_From_File<float> (filename, embedded_landmarks);
}
//#####################################################################
