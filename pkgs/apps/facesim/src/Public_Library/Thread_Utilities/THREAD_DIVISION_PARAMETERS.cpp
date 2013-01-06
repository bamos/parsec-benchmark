//#####################################################################
// Copyright 2005, Eftychios sifakis
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "THREAD_DIVISION_PARAMETERS.h"
using namespace PhysBAM;

extern bool PHYSBAM_THREADED_RUN;
//#####################################################################
template <class T> bool THREAD_DIVISION_PARAMETERS<T>::
Thread_Divisions_Enabled()
{
#ifndef NEW_SERIAL_IMPLEMENTATIOM
	return PHYSBAM_THREADED_RUN;
#else
	return true;
#endif
}
//#####################################################################
// Function Initialize_Array_Divisions
//#####################################################################
template <class T> void THREAD_DIVISION_PARAMETERS<T>::
Initialize_Array_Divisions (const int length, const int divisions, ARRAY<VECTOR_2D<int> >& ranges) const
{
	ranges.Resize_Array (divisions);

	for (int i = 1; i <= divisions; i++)
	{
		int quotient = length / divisions, remainder = length % divisions;
		ranges (i) = VECTOR_2D<int> ( (i - 1) * quotient + min (i - 1, remainder) + 1, i * quotient + min (i, remainder));
	}
}
//#####################################################################
// Function Initialize_Array_Divisions_With_Equal_Extents
//#####################################################################
template <class T> void THREAD_DIVISION_PARAMETERS<T>::
Initialize_Array_Divisions_With_Equal_Extents (const int length, const int divisions, const int overlap, ARRAY<VECTOR_2D<int> >& ranges) const
{
	assert ( (length - 2 * overlap) % divisions == 0); // need specific length to guarantee exact partitioning
	int span = (length - 2 * overlap) / divisions + 2 * overlap - 1;
	ranges.Resize_Array (divisions);

	for (int i = 1; i <= divisions; i++) ranges (i) = VECTOR_2D<int> ( (i - 1) * span - (2 * i - 3) * overlap + i, i * span - (2 * i - 1) * overlap + i);

	ranges (1).x = 1;
	ranges (divisions).y = length;
}
//#####################################################################
// Function Initialize_Array_Divisions_With_Equal_Extents
//#####################################################################
template <class T> void THREAD_DIVISION_PARAMETERS<T>::
Initialize_Array_Divisions_With_Equal_Extents (const VECTOR_2D<T>& length, const VECTOR_2D<int>& divisions, const VECTOR_2D<int>& overlap, ARRAY<BOX_2D<int> >& ranges) const
{
	ARRAY<VECTOR_2D<int> > x_ranges, y_ranges;
	Initialize_Array_Divisions_With_Equal_Extents (length.x, divisions.x, overlap.x, x_ranges);
	Initialize_Array_Divisions_With_Equal_Extents (length.y, divisions.y, overlap.y, y_ranges);
	ranges.Resize_Array (divisions.x * divisions.y);

	for (int j = 1; j <= divisions.y; j++) for (int i = 1; i <= divisions.x; i++)
			ranges ( (j - 1) *divisions.x + i) = BOX_2D<int> (x_ranges (i).x, x_ranges (i).y, y_ranges (j).x, y_ranges (j).y);
}
//#####################################################################
// Function Initialize_Grid_Divisions
//#####################################################################
template <class T> void THREAD_DIVISION_PARAMETERS<T>::
Initialize_Grid_Divisions (const GRID_2D<T>& grid, ARRAY<BOX_2D<int> >& ranges, ARRAY<BOX_2D<int> >* ranges_padded, const int ghost_cells, const int range_overlap) const
{
	assert (grid_divisions_2d.x && grid_divisions_2d.y);
	ranges.Resize_Array (grid_divisions_2d.x * grid_divisions_2d.y);

	if (ranges_padded) ranges_padded->Resize_Array (grid_divisions_2d.x * grid_divisions_2d.y);

	for (int i = 1; i <= grid_divisions_2d.x; i++)
	{
		int m_quotient = grid.m / grid_divisions_2d.x, m_remainder = grid.m % grid_divisions_2d.x;

		for (int j = 1; j <= grid_divisions_2d.y; j++)
		{
			int n_quotient = grid.n / grid_divisions_2d.y, n_remainder = grid.n % grid_divisions_2d.y;
			BOX_2D<int> range ( (i - 1) *m_quotient + min (i - 1, m_remainder) + 1, i * m_quotient + min (i, m_remainder),
					    (j - 1) *n_quotient + min (j - 1, n_remainder) + 1, j * n_quotient + min (j, n_remainder));
			BOX_2D<int> range_padded (max (range.xmin - range_overlap, 1 - ghost_cells), min (range.xmax + range_overlap, grid.m + ghost_cells),
						  max (range.ymin - range_overlap, 1 - ghost_cells), min (range.ymax + range_overlap, grid.n + ghost_cells));
			ranges ( (i - 1) *grid_divisions_2d.y + j) = range;

			if (ranges_padded) (*ranges_padded) ( (i - 1) *grid_divisions_2d.y + j) = range_padded;
		}
	}
}
//#####################################################################
// Function Initialize_Grid_Divisions
//#####################################################################
template <class T> void THREAD_DIVISION_PARAMETERS<T>::
Initialize_Grid_Divisions (const GRID_3D<T>& grid, ARRAY<BOX_3D<int> >& ranges, ARRAY<BOX_3D<int> >* ranges_padded, const int ghost_cells, const int range_overlap) const
{
	assert (grid_divisions_3d.x && grid_divisions_3d.y && grid_divisions_3d.z);
	ranges.Resize_Array (grid_divisions_3d.x * grid_divisions_3d.y * grid_divisions_3d.z);

	if (ranges_padded) ranges_padded->Resize_Array (grid_divisions_3d.x * grid_divisions_3d.y * grid_divisions_3d.z);

	for (int i = 1; i <= grid_divisions_3d.x; i++)
	{
		int m_quotient = grid.m / grid_divisions_3d.x, m_remainder = grid.m % grid_divisions_3d.x;

		for (int j = 1; j <= grid_divisions_3d.y; j++)
		{
			int n_quotient = grid.n / grid_divisions_3d.y, n_remainder = grid.n % grid_divisions_3d.y;

			for (int ij = 1; ij <= grid_divisions_3d.z; ij++)
			{
				int mn_quotient = grid.mn / grid_divisions_3d.z, mn_remainder = grid.mn % grid_divisions_3d.z;
				BOX_3D<int> range ( (i - 1) *m_quotient + min (i - 1, m_remainder) + 1, i * m_quotient + min (i, m_remainder),
						    (j - 1) *n_quotient + min (j - 1, n_remainder) + 1, j * n_quotient + min (j, n_remainder),
						    (ij - 1) *mn_quotient + min (ij - 1, mn_remainder) + 1, ij * mn_quotient + min (ij, mn_remainder));
				BOX_3D<int> range_padded (max (range.xmin - range_overlap, 1 - ghost_cells), min (range.xmax + range_overlap, grid.m + ghost_cells),
							  max (range.ymin - range_overlap, 1 - ghost_cells), min (range.ymax + range_overlap, grid.n + ghost_cells),
							  max (range.zmin - range_overlap, 1 - ghost_cells), min (range.zmax + range_overlap, grid.mn + ghost_cells));
				ranges ( (i - 1) *grid_divisions_3d.y * grid_divisions_3d.z + (j - 1) *grid_divisions_3d.z + ij) = range;

				if (ranges_padded) (*ranges_padded) ( (i - 1) *grid_divisions_3d.y * grid_divisions_3d.z + (j - 1) *grid_divisions_3d.z + ij) = range_padded;
			}
		}
	}
}
//#####################################################################


//#####################################################################
// Function Initialize_Grid_Divisions
//#####################################################################
template <class T> void THREAD_DIVISION_PARAMETERS<T>::
Initialize_Grid_Divisions (const BOX_2D<T>& grid_box, ARRAY<BOX_2D<int> >& ranges) const
{
	assert (grid_divisions_2d.x && grid_divisions_2d.y);
	ranges.Resize_Array (grid_divisions_2d.x * grid_divisions_2d.y);
	int size_x = grid_box.xmax - grid_box.xmin, size_y = grid_box.ymax - grid_box.ymin;

	for (int i = 1; i <= grid_divisions_2d.x; i++)
	{
		int m_quotient = size_x / grid_divisions_2d.x, m_remainder = size_x % grid_divisions_2d.x;

		for (int j = 1; j <= grid_divisions_2d.y; j++)
		{
			int n_quotient = size_y / grid_divisions_2d.y, n_remainder = size_y % grid_divisions_2d.y;
			BOX_2D<int> range (grid_box.xmin + (i - 1) *m_quotient + min (i - 1, m_remainder) + 1, grid_box.xmin + i * m_quotient + min (i, m_remainder),
					   grid_box.ymin + (j - 1) *n_quotient + min (j - 1, n_remainder) + 1, grid_box.ymin + j * n_quotient + min (j, n_remainder));
			ranges ( (i - 1) *grid_divisions_2d.y + j) = range;
		}
	}
}
//#####################################################################
// Function Initialize_Grid_Divisions
//#####################################################################
template <class T> void THREAD_DIVISION_PARAMETERS<T>::
Initialize_Grid_Divisions (const BOX_3D<T>& grid_box, ARRAY<BOX_3D<int> >& ranges) const
{
	assert (grid_divisions_3d.x && grid_divisions_3d.y && grid_divisions_3d.z);
	ranges.Resize_Array (grid_divisions_3d.x * grid_divisions_3d.y * grid_divisions_3d.z);
	int size_x = grid_box.xmax - grid_box.xmin, size_y = grid_box.ymax - grid_box.ymin, size_z = grid_box.zmax - grid_box.zmin;

	for (int i = 1; i <= grid_divisions_3d.x; i++)
	{
		int m_quotient = size_x / grid_divisions_3d.x, m_remainder = size_x % grid_divisions_3d.x;

		for (int j = 1; j <= grid_divisions_3d.y; j++)
		{
			int n_quotient = size_y / grid_divisions_3d.y, n_remainder = size_y % grid_divisions_3d.y;

			for (int ij = 1; ij <= grid_divisions_3d.z; ij++)
			{
				int mn_quotient = size_z / grid_divisions_3d.z, mn_remainder = size_z % grid_divisions_3d.z;
				BOX_3D<int> range (grid_box.xmin + (i - 1) *m_quotient + min (i - 1, m_remainder) + 1, grid_box.xmin + i * m_quotient + min (i, m_remainder),
						   grid_box.ymin + (j - 1) *n_quotient + min (j - 1, n_remainder) + 1, grid_box.ymin + j * n_quotient + min (j, n_remainder),
						   grid_box.zmin + (ij - 1) *mn_quotient + min (ij - 1, mn_remainder) + 1, grid_box.zmin + ij * mn_quotient + min (ij, mn_remainder));
				ranges ( (i - 1) *grid_divisions_3d.y * grid_divisions_3d.z + (j - 1) *grid_divisions_3d.z + ij) = range;
			}
		}
	}
}
//#####################################################################


template class THREAD_DIVISION_PARAMETERS<float>;
template class THREAD_DIVISION_PARAMETERS<double>;
