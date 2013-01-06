//#####################################################################
// Copyright 2004, Ron Fedkiw, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class RIGID_BODY_PARAMETERS
//#####################################################################
#ifndef __RIGID_BODY_PARAMETERS__
#define __RIGID_BODY_PARAMETERS__

#include "../Math_Tools/constants.h"
#include "../Arrays/ARRAYS_2D.h"
namespace PhysBAM
{

template<class T> class RIGID_BODY_PARAMETERS_CALLBACKS;

template <class T>
class RIGID_BODY_PARAMETERS
{
public:
	bool simulate, write;
	T ether_viscosity;
	T artificial_maximum_speed; // artificial limit on the maximum possible speed
	bool spatial_partition_based_on_scene_size;
	int spatial_partition_number_of_cells;
	bool spatial_partition_based_on_object_size, spatial_partition_with_max_size;
	bool use_particle_partition, use_particle_partition_center_phi_test;
	int particle_partition_size;
	bool use_triangle_hierarchy, use_triangle_hierarchy_center_phi_test;
	bool use_edge_intersection;
	bool print_interpenetration_statistics;
	T max_rotation_per_time_step;
	T max_linear_movement_fraction_per_time_step;
	bool use_collision_matrix;
	RIGID_BODY_PARAMETERS_CALLBACKS<T>* callbacks;

	RIGID_BODY_PARAMETERS()
		: simulate (false), write (true),
		  ether_viscosity (0), artificial_maximum_speed (0),
		  spatial_partition_based_on_scene_size (false), spatial_partition_number_of_cells (100), spatial_partition_based_on_object_size (true), spatial_partition_with_max_size (true),
		  use_particle_partition (true), use_particle_partition_center_phi_test (true), particle_partition_size (2),
		  use_triangle_hierarchy (false), use_triangle_hierarchy_center_phi_test (false), use_edge_intersection (false), print_interpenetration_statistics (false),
		  max_rotation_per_time_step ( (T).1 * (T) pi), max_linear_movement_fraction_per_time_step ( (T).1),
		  use_collision_matrix (false), callbacks (0)
	{}

	virtual ~RIGID_BODY_PARAMETERS()
	{}

	void Set_Rigid_Body_Parameters_Callbacks (RIGID_BODY_PARAMETERS_CALLBACKS<T>& callbacks_input)
	{
		callbacks = &callbacks_input;
	}

//#####################################################################
};
}
#endif
