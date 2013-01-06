//#####################################################################
// Copyright 2005, Eftychios sifakis
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#ifndef __THREAD_DIVISION_PARAMETERS__
#define __THREAD_DIVISION_PARAMETERS__

#include "../Arrays/ARRAY.h"
#include "../Grids/GRID_2D.h"
#include "../Grids/GRID_3D.h"
namespace PhysBAM
{

template <class T>
class THREAD_DIVISION_PARAMETERS
{
private:
	static THREAD_DIVISION_PARAMETERS<T>* singleton_instance;
public:
	VECTOR_2D<int> grid_divisions_2d;
	VECTOR_3D<int> grid_divisions_3d;
	int box_hierarchy_intersection_divisions;
	int adjust_velocity_divisions;

	static inline THREAD_DIVISION_PARAMETERS<T>* Singleton()
	{
		if (!singleton_instance) singleton_instance = new THREAD_DIVISION_PARAMETERS<T>();

		return singleton_instance;
	}

//#####################################################################
	static bool Thread_Divisions_Enabled();
	void Initialize_Array_Divisions (const int length, const int divisions, ARRAY<VECTOR_2D<int> >& ranges) const;
	void Initialize_Array_Divisions_With_Equal_Extents (const int length, const int divisions, const int overlap, ARRAY<VECTOR_2D<int> >& ranges) const;
	void Initialize_Array_Divisions_With_Equal_Extents (const VECTOR_2D<T>& length, const VECTOR_2D<int>& divisions, const VECTOR_2D<int>& overlap, ARRAY<BOX_2D<int> >& ranges) const;
	void Initialize_Grid_Divisions (const GRID_2D<T>& grid, ARRAY<BOX_2D<int> >& ranges, ARRAY<BOX_2D<int> >* ranges_padded, const int ghost_cells, const int range_overlap) const;
	void Initialize_Grid_Divisions (const GRID_3D<T>& grid, ARRAY<BOX_3D<int> >& ranges, ARRAY<BOX_3D<int> >* ranges_padded, const int ghost_cells, const int range_overlap) const;
	void Initialize_Grid_Divisions (const BOX_2D<T>& grid_box, ARRAY<BOX_2D<int> >& ranges)  const;
	void Initialize_Grid_Divisions (const BOX_3D<T>& grid_box, ARRAY<BOX_3D<int> >& ranges) const;
//#####################################################################
};

template <class T> THREAD_DIVISION_PARAMETERS<T>* THREAD_DIVISION_PARAMETERS<T>::singleton_instance = 0;

}
#endif
