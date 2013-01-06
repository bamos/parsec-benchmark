//#####################################################################
// Copyright 2003-2004, Ronald Fedkiw, Eran Guendelman, Robert Bridson.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class PARTICLE_PARTITION
//#####################################################################
#ifndef __PARTICLE_PARTITION__
#define __PARTICLE_PARTITION__

#include "../Math_Tools/clamp.h"
#include "../Geometry/BOX_3D.h"
#include "../Geometry/IMPLICIT_SURFACE.h"
namespace PhysBAM
{

template<class T>
class PARTICLE_PARTITION
{
public:
	BOX_3D<T>& box;
	GRID_3D<T> grid;
	ARRAYS_3D<ARRAY<int> > partition;
private:
	bool use_radius;
	ARRAYS_3D<T> radius;
public:

	PARTICLE_PARTITION (BOX_3D<T>& box_input, const int m, const int n, const int mn, const SOLIDS_PARTICLE<T, VECTOR_3D<T> >& particles, const bool use_radius_input = true)
		: box (box_input), grid (m, n, mn, box.xmin, box.xmax, box.ymin, box.ymax, box.zmin, box.zmax, true), partition (grid), use_radius (use_radius_input)
	{
		if (use_radius) radius.Resize_Array (grid);

		for (int i = 1; i <= particles.number; i++) Add_To_Partition (particles.X (i), i);
	}

	void Add_To_Partition (const VECTOR_3D<T>& location, const int particle_id)
	{
		assert (box.Lazy_Inside (location));
		int i, j, k;
		grid.Clamped_Index (location, i, j, k);
		partition (i, j, k).Append_Element (particle_id);

		if (use_radius) radius (i, j, k) = max (radius (i, j, k), (location - grid.X (i, j, k)).Magnitude());
	}

	void Range (const BOX_3D<T>& box, int& i_start, int& i_end, int& j_start, int& j_end, int& k_start, int& k_end)
	{
		grid.Clamped_Index (VECTOR_3D<T> (box.xmin, box.ymin, box.zmin), i_start, j_start, k_start);
		grid.Clamped_Index (VECTOR_3D<T> (box.xmax, box.ymax, box.zmax), i_end, j_end, k_end);
	}

	void Intersection_List (const IMPLICIT_SURFACE<T>& test_surface, const MATRIX_3X3<T>& rotation, const VECTOR_3D<T>& translation, LIST_ARRAYS<int>& intersection_list, const T contour_value = 0)
	{
		assert (use_radius);
		intersection_list.Reset_Current_Size_To_Zero();

		for (int i = 1; i <= partition.m; i++) for (int j = 1; j <= partition.n; j++) for (int k = 1; k <= partition.mn; k++)
					if (partition (i, j, k).m && !test_surface.Lazy_Outside_Extended_Levelset (rotation * grid.X (i, j, k) + translation, radius (i, j, k) + contour_value)) intersection_list.Append_Element (i, j, k);
	}

//#####################################################################
};
}

#endif
