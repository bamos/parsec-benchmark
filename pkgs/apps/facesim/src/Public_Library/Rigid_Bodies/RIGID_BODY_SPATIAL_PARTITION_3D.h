//#####################################################################
// Copyright 2002, 2003, Ronald Fedkiw, Eran Guendelman.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class RIGID_BODY_SPATIAL_PARTITION_3D
//#####################################################################
#ifndef __RIGID_BODY_SPATIAL_PARTITION_3D__
#define __RIGID_BODY_SPATIAL_PARTITION_3D__

#include "../Arrays/ARRAY.h"
#include "../Data_Structures/HASHTABLE_3D.h"
#include "../Arrays/LIST_ARRAY.h"
#include "../Matrices_And_Vectors/VECTOR_3D.h"
#include "RIGID_BODY_3D.h"
#include <limits.h> // for INT_MAX (for old gcc)
namespace PhysBAM
{

template<class T>
class RIGID_BODY_SPATIAL_PARTITION_3D
{
private:
	T voxel_size, one_over_voxel_size;
	LIST_ARRAY<RIGID_BODY_3D<T>*>& rigid_bodies;
	ARRAYS<VECTOR_3D<int> > voxel_range;
	HASHTABLE_3D<LIST_ARRAY<int>*> hashtable;
	ARRAY<int> bodies_not_in_partition;
	int reinitialize_counter;

public:
	RIGID_BODY_SPATIAL_PARTITION_3D (T voxel_size_input, LIST_ARRAY<RIGID_BODY_3D<T>*>& rigid_bodies_input)
		: voxel_size (voxel_size_input), one_over_voxel_size (1 / voxel_size),
		  rigid_bodies (rigid_bodies_input), voxel_range (2, rigid_bodies.m),
		  hashtable (10 * rigid_bodies.m), reinitialize_counter (0)
	{
		Reinitialize();
	}

	~RIGID_BODY_SPATIAL_PARTITION_3D()
	{
		hashtable.Delete_Pointers_Stored_In_Table();
		hashtable.Delete_All_Entries();
	}

	void Reinitialize()
	{
		if (reinitialize_counter % 10 == 0)
		{
			hashtable.Delete_Pointers_Stored_In_Table();
			hashtable.Delete_All_Entries();
		}
		else hashtable.Reset_List_Arrays_Stored_In_Table();

		reinitialize_counter++;
		voxel_range.Resize_Array (2, rigid_bodies.m);
		int count = 0, index = 0;

		for (int i = 1; i <= rigid_bodies.m; i++) if (rigid_bodies (i)->add_to_spatial_partition)
			{
				Voxel_Range (i, voxel_range (1, i), voxel_range (2, i));
				Insert_Into_Hashtable (i);
				count++;
			}

		bodies_not_in_partition.Resize_Array (rigid_bodies.m - count);

		for (int i = 1; i <= rigid_bodies.m; i++) if (!rigid_bodies (i)->add_to_spatial_partition)
			{
				assert (rigid_bodies (i)->is_static);
				bodies_not_in_partition (++index) = i;
			}
	}

	void Print_Initial_Statistics() const
	{
		int min_size = INT_MAX, max_size = 0, number = 0;
		T average_size = 0;

		for (int i = 1; i <= rigid_bodies.m; i++) if (rigid_bodies (i)->add_to_spatial_partition)
			{
				number++;
				int size = Number_Of_Voxels_Occupied (i);
				min_size = min (min_size, size);
				max_size = max (max_size, size);
				average_size += size;
			}

		average_size /= number;
		std::cout << "Spatial partition statistics: " << std::endl;
		std::cout << "Voxel size = " << voxel_size << std::endl;
		std::cout << "Number of voxels occupied: min = " << min_size << ", average = " << average_size << ", max = " << max_size << std::endl;
		std::cout << bodies_not_in_partition.m << " bodies not in spatial partition" << std::endl;
		std::cout << "Number of hash entries = " << hashtable.number_of_entries << std::endl;
	}

private:
	void Voxel_Range (const int index, VECTOR_3D<int>& voxel_min, VECTOR_3D<int>& voxel_max) const
	{
		BOX_3D<T> box = rigid_bodies (index)->Axis_Aligned_Bounding_Box();
		voxel_min.x = (int) floor (box.xmin * one_over_voxel_size);
		voxel_min.y = (int) floor (box.ymin * one_over_voxel_size);
		voxel_min.z = (int) floor (box.zmin * one_over_voxel_size);
		voxel_max.x = (int) floor (box.xmax * one_over_voxel_size);
		voxel_max.y = (int) floor (box.ymax * one_over_voxel_size);
		voxel_max.z = (int) floor (box.zmax * one_over_voxel_size);
	}

	int Number_Of_Voxels_Occupied (const int index) const
	{
		const VECTOR_3D<int> &voxel_min = voxel_range (1, index), &voxel_max = voxel_range (2, index);
		return (voxel_max.x - voxel_min.x + 1) * (voxel_max.y - voxel_min.y + 1) * (voxel_max.z - voxel_min.z + 1);
	}

	void Insert_Into_Hashtable (const int index)
	{
		assert (rigid_bodies (index)->add_to_spatial_partition);
		const VECTOR_3D<int> &voxel_min = voxel_range (1, index), &voxel_max = voxel_range (2, index);

		for (int i = voxel_min.x; i <= voxel_max.x; i++) for (int j = voxel_min.y; j <= voxel_max.y; j++) for (int k = voxel_min.z; k <= voxel_max.z; k++)
				{
					LIST_ARRAY<int>* occupancy_list = 0;

					if (!hashtable.Get_Entry (i, j, k, occupancy_list))
					{
						occupancy_list = new LIST_ARRAY<int>();
						occupancy_list->Preallocate (5);
						hashtable.Insert_Entry (i, j, k, occupancy_list);
					}

					occupancy_list->Append_Element (index);
				}
	}
//#####################################################################
};
}
#endif
