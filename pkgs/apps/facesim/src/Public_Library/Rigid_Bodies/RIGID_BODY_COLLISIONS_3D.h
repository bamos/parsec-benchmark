//#####################################################################
// Copyright 2003-2004, Ronald Fedkiw, Eran Guendelman, Rachel Weinstein.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class RIGID_BODY_COLLISIONS_3D
//#####################################################################
#ifndef __RIGID_BODY_COLLISIONS_3D__
#define __RIGID_BODY_COLLISIONS_3D__

#include <float.h>
#include <stdlib.h>
#include "RIGID_BODY_COLLISIONS.h"
#include "RIGID_BODY_SPATIAL_PARTITION_3D.h"
#include "RIGID_BODY_INTERSECTIONS_3D.h"
namespace PhysBAM
{

template<class T> class ARTICULATED_RIGID_BODY_3D;

template<class T>
class RIGID_BODY_COLLISIONS_3D: public RIGID_BODY_COLLISIONS<T>
{
public:
	using RIGID_BODY_COLLISIONS<T>::verbose;
	using RIGID_BODY_COLLISIONS<T>::skip_collision_check;
	using RIGID_BODY_COLLISIONS<T>::collision_manager;
protected:
	using RIGID_BODY_COLLISIONS<T>::collision_iterations;
	using RIGID_BODY_COLLISIONS<T>::contact_iterations;
	using RIGID_BODY_COLLISIONS<T>::contact_level_iterations;
	using RIGID_BODY_COLLISIONS<T>::contact_pair_iterations;
	using RIGID_BODY_COLLISIONS<T>::epsilon_scaling;
	using RIGID_BODY_COLLISIONS<T>::epsilon_scaling_for_level;
	using RIGID_BODY_COLLISIONS<T>::shock_propagation_iterations;
	using RIGID_BODY_COLLISIONS<T>::shock_propagation_level_iterations;
	using RIGID_BODY_COLLISIONS<T>::shock_propagation_pair_iterations;
	using RIGID_BODY_COLLISIONS<T>::push_out_iterations;
	using RIGID_BODY_COLLISIONS<T>::push_out_level_iterations;
	using RIGID_BODY_COLLISIONS<T>::push_out_pair_iterations;
	using RIGID_BODY_COLLISIONS<T>::use_freezing_with_push_out;
	using RIGID_BODY_COLLISIONS<T>::use_gradual_push_out;
	using RIGID_BODY_COLLISIONS<T>::desired_separation_distance;
	using RIGID_BODY_COLLISIONS<T>::rolling_friction;
	using RIGID_BODY_COLLISIONS<T>::precomputed_contact_pairs_for_level;

public:
	LIST_ARRAY<RIGID_BODY_3D<T>*>& rigid_bodies;
	RIGID_BODY_SPATIAL_PARTITION_3D<T> spatial_partition;
	RIGID_BODY_INTERSECTIONS_3D<T> intersections;
	T min_bounding_box_width; // not used internally - but useful externally
private:
	ARRAY<VECTOR_3D<T> > position_save;
	ARRAY<QUATERNION<T> > orientation_save;

public:
	RIGID_BODY_COLLISIONS_3D (LIST_ARRAY<RIGID_BODY_3D<T>*>& rigid_bodies_input, const T voxel_size)
		: rigid_bodies (rigid_bodies_input),
		  spatial_partition (voxel_size, rigid_bodies), intersections (rigid_bodies)
	{
		// initialize bounding box information
		min_bounding_box_width = FLT_MAX;

		for (int i = 1; i <= rigid_bodies.m; i++)
		{
			BOX_3D<T>& box = *rigid_bodies (i)->triangulated_surface->bounding_box;
			VECTOR_3D<T> size = box.Size();
			min_bounding_box_width = min (min_bounding_box_width, size.x, size.y, size.z);
		}
	}

	void Initialize_Data_Structures()
	{
		intersections.Initialize_Data_Structures();
		skip_collision_check.Initialize (rigid_bodies.m);
		position_save.Resize_Array (rigid_bodies.m);
		orientation_save.Resize_Array (rigid_bodies.m);
	}

	static void Adjust_Bounding_Boxes (LIST_ARRAY<RIGID_BODY_3D<T>*>& rigid_bodies, const T false_thickness = 1, const T extra_padding_distance = 0)
	{
		for (int i = 1; i <= rigid_bodies.m; i++)
		{
			assert (rigid_bodies (i)->triangulated_surface->bounding_box); // bounding boxes should already be set up
			BOX_3D<T>& box = *rigid_bodies (i)->triangulated_surface->bounding_box;
			bool already_adjusted = false; // multiple rigid bodies can share a triangulated surface - don't adjust bounding box multiple times

			for (int j = 1; j <= i - 1; j++) if (rigid_bodies (i)->triangulated_surface->bounding_box == rigid_bodies (j)->triangulated_surface->bounding_box)
				{
					already_adjusted = true;
					break;
				}

			if (!already_adjusted)
			{
				if (box.xmin == box.xmax)
				{
					box.xmin -= false_thickness;
					box.xmax += false_thickness;
				}

				if (box.ymin == box.ymax)
				{
					box.ymin -= false_thickness;
					box.ymax += false_thickness;
				}

				if (box.zmin == box.zmax)
				{
					box.zmin -= false_thickness;
					box.zmax += false_thickness;
				}

				box.xmin -= extra_padding_distance;
				box.xmax += extra_padding_distance;
				box.ymin -= extra_padding_distance;
				box.ymax += extra_padding_distance;
				box.zmin -= extra_padding_distance;
				box.zmax += extra_padding_distance;
			}

			rigid_bodies (i)->Update_Bounding_Box();
		}
	}

	static BOX_3D<T> Scene_Bounding_Box (LIST_ARRAY<RIGID_BODY_3D<T>*>& rigid_bodies)
	{
		BOX_3D<T> scene_bounding_box = rigid_bodies (1)->Axis_Aligned_Bounding_Box();

		for (int i = 2; i <= rigid_bodies.m; i++) if (!rigid_bodies (i)->is_static && rigid_bodies (i)->add_to_spatial_partition)
				scene_bounding_box = BOX_3D<T>::Combine (scene_bounding_box, rigid_bodies (i)->Axis_Aligned_Bounding_Box());

		return scene_bounding_box;
	}

	static T Average_Bounding_Box_Size (LIST_ARRAY<RIGID_BODY_3D<T>*>& rigid_bodies)
	{
		T average_size = 0;
		int count = 0;

		for (int i = 1; i <= rigid_bodies.m; i++) if (!rigid_bodies (i)->is_static && rigid_bodies (i)->add_to_spatial_partition)
			{
				count++;
				VECTOR_3D<T> size = rigid_bodies (i)->triangulated_surface->bounding_box->Size();
				average_size += (size.x + size.y + size.z);
			}

		return (T) one_third * average_size / count;
	}

	static T Maximum_Bounding_Box_Size (LIST_ARRAY<RIGID_BODY_3D<T>*>& rigid_bodies)
	{
		T max_size = 0;

		for (int i = 1; i <= rigid_bodies.m; i++) if (!rigid_bodies (i)->is_static && rigid_bodies (i)->add_to_spatial_partition)
			{
				VECTOR_3D<T> size = rigid_bodies (i)->triangulated_surface->bounding_box->Size();
				max_size = max (max_size, size.x, size.y, size.z);
			}

		return max_size;
	}

	void Save_Positions()
	{
		for (int i = 1; i <= rigid_bodies.m; i++) Save_Position (i);
	}

	void Restore_Positions()
	{
		for (int i = 1; i <= rigid_bodies.m; i++) Restore_Position (i);
	}

	void Euler_Step_Position_With_New_Velocity (const int index, const T dt, const T time)
	{
		VECTOR_3D<T> velocity = rigid_bodies (index)->velocity, angular_momentum = rigid_bodies (index)->angular_momentum; // save velocity
		rigid_bodies (index)->Euler_Step_Velocity (dt, time); // temporarily update velocity
		Euler_Step_Position (index, dt, time);
		rigid_bodies (index)->velocity = velocity;
		rigid_bodies (index)->angular_momentum = angular_momentum; // restore velocity
		rigid_bodies (index)->Update_Angular_Velocity();
	} // re-sync this

private:
	void Save_Position (const int index)
	{
		position_save (index) = rigid_bodies (index)->position;
		orientation_save (index) = rigid_bodies (index)->orientation;
	}

	void Restore_Position (const int index)
	{
		rigid_bodies (index)->position = position_save (index);
		rigid_bodies (index)->orientation = orientation_save (index);
		rigid_bodies (index)->Update_Angular_Velocity();
		Update_Bounding_Box (index);
	}

	void Euler_Step_Position (const int index, const T dt, const T time)
	{
		rigid_bodies (index)->Euler_Step_Position (dt, time);
		Update_Bounding_Box (index);
		skip_collision_check.Set_Last_Moved (index);
	}

	void Update_Bounding_Box (const int index)
	{
		if (!rigid_bodies (index)->is_static) rigid_bodies (index)->Update_Bounding_Box();
	}

//#####################################################################
public:
	bool Check_For_Any_Interpenetration();
	void Print_Interpenetration_Statistics();
//#####################################################################
};
}
#endif
