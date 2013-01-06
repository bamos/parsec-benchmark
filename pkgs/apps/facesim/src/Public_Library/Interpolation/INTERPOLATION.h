//#####################################################################
// Copyright 2003 Ronald Fedkiw, Eran Guendelman, Geoffrey Irving, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class INTERPOLATION
//#####################################################################
#ifndef __INTERPOLATION__
#define __INTERPOLATION__

#include "../Math_Tools/min.h"
#include "../Math_Tools/max.h"
#include "../Arrays/ARRAY.h"
#include "../Arrays/ARRAYS_1D.h"
#include "../Arrays/ARRAYS_2D.h"
#include "../Arrays/ARRAYS_3D.h"
#include "../Grids/GRID_1D.h"
#include "../Grids/GRID_2D.h"
#include "../Grids/GRID_3D.h"
#include "../Matrices_And_Vectors/VECTOR_1D.h"
#include "../Matrices_And_Vectors/VECTOR_2D.h"
#include "../Matrices_And_Vectors/VECTOR_3D.h"
namespace PhysBAM
{
template<class T, class T2>
class INTERPOLATION
{
public:
	static void Clamped_Index (const GRID_1D<T>& grid, const ARRAYS_1D<T2>& u, const T& location, int& i)
	{
		i = min (u.m_end, u.m_start + max (0, (int) ( (location - grid.xmin) * grid.one_over_dx - u.m_start + 1 - grid.MAC_offset)));
	}

	static void Clamped_Index (const GRID_2D<T>& grid, const ARRAYS_2D<T2>& u, const VECTOR_2D<T>& location, int& i, int& j)
	{
		i = min (u.m_end, u.m_start + max (0, (int) ( (location.x - grid.xmin) * grid.one_over_dx - u.m_start + 1 - grid.MAC_offset)));
		j = min (u.n_end, u.n_start + max (0, (int) ( (location.y - grid.ymin) * grid.one_over_dy - u.n_start + 1 - grid.MAC_offset)));
	}

	static void Clamped_Index (const GRID_2D<T>& grid, const ARRAYS_2D<T2>& u, const VECTOR_2D<T>& location, VECTOR_2D<int>& index)
	{
		Clamped_Index (grid, u, location, index.x, index.y);
	}

	static void Clamped_Index (const GRID_3D<T>& grid, const ARRAYS_3D<T2>& u, const VECTOR_3D<T>& location, int& i, int& j, int& ij)
	{
		i = min (u.m_end, u.m_start + max (0, (int) ( (location.x - grid.xmin) * grid.one_over_dx - u.m_start + 1 - grid.MAC_offset)));
		j = min (u.n_end, u.n_start + max (0, (int) ( (location.y - grid.ymin) * grid.one_over_dy - u.n_start + 1 - grid.MAC_offset)));
		ij = min (u.mn_end, u.mn_start + max (0, (int) ( (location.z - grid.zmin) * grid.one_over_dz - u.mn_start + 1 - grid.MAC_offset)));
	}

	static void Clamped_Index (const GRID_3D<T>& grid, const ARRAYS_3D<T2>& u, const VECTOR_3D<T>& location, VECTOR_3D<int>& index)
	{
		Clamped_Index (grid, u, location, index.x, index.y, index.z);
	}

	static void Clamped_Index_End_Minus_One (const GRID_1D<T>& grid, const ARRAYS_1D<T2>& u, const VECTOR_1D<T>& location, int& i)
	{
		i = min (u.m_end - 1, u.m_start + max (0, (int) ( (location.x - grid.xmin) * grid.one_over_dx - u.m_start + 1 - grid.MAC_offset)));
	}

	static void Clamped_Index_End_Minus_One (const GRID_1D<T>& grid, const ARRAYS_1D<T2>& u, const VECTOR_1D<T>& location, VECTOR_1D<int>& index)
	{
		Clamped_Index_End_Minus_One (grid, u, location, index.x);
	}

	static void Clamped_Index_End_Minus_One (const GRID_2D<T>& grid, const ARRAYS_2D<T2>& u, const VECTOR_2D<T>& location, int& i, int& j)
	{
		i = min (u.m_end - 1, u.m_start + max (0, (int) ( (location.x - grid.xmin) * grid.one_over_dx - u.m_start + 1 - grid.MAC_offset)));
		j = min (u.n_end - 1, u.n_start + max (0, (int) ( (location.y - grid.ymin) * grid.one_over_dy - u.n_start + 1 - grid.MAC_offset)));
	}

	static void Clamped_Index_End_Minus_One (const GRID_2D<T>& grid, const ARRAYS_2D<T2>& u, const VECTOR_2D<T>& location, VECTOR_2D<int>& index)
	{
		Clamped_Index_End_Minus_One (grid, u, location, index.x, index.y);
	}

	static void Clamped_Index_End_Minus_One (const GRID_3D<T>& grid, const ARRAYS_3D<T2>& u, const VECTOR_3D<T>& location, int& i, int& j, int& ij)
	{
		i = min (u.m_end - 1, u.m_start + max (0, (int) ( (location.x - grid.xmin) * grid.one_over_dx - u.m_start + 1 - grid.MAC_offset)));
		j = min (u.n_end - 1, u.n_start + max (0, (int) ( (location.y - grid.ymin) * grid.one_over_dy - u.n_start + 1 - grid.MAC_offset)));
		ij = min (u.mn_end - 1, u.mn_start + max (0, (int) ( (location.z - grid.zmin) * grid.one_over_dz - u.mn_start + 1 - grid.MAC_offset)));
	}

	static void Clamped_Index_End_Minus_One (const GRID_3D<T>& grid, const ARRAYS_3D<T2>& u, const VECTOR_3D<T>& location, VECTOR_3D<int>& index)
	{
		Clamped_Index_End_Minus_One (grid, u, location, index.x, index.y, index.z);
	}

	static void Clamped_Index_Interior (const GRID_1D<T>& grid, const ARRAYS_1D<T2>& u, const VECTOR_1D<T>& location, int& i)
	{
		i = min (u.m_end - 1, u.m_start + 1 + max (0, (int) ( (location.x - grid.xmin) * grid.one_over_dx - u.m_start - grid.MAC_offset)));
	}

	static void Clamped_Index_Interior (const GRID_2D<T>& grid, const ARRAYS_2D<T2>& u, const VECTOR_2D<T>& location, int& i, int& j)
	{
		i = min (u.m_end - 1, u.m_start + 1 + max (0, (int) ( (location.x - grid.xmin) * grid.one_over_dx - u.m_start - grid.MAC_offset)));
		j = min (u.n_end - 1, u.n_start + 1 + max (0, (int) ( (location.y - grid.ymin) * grid.one_over_dy - u.n_start - grid.MAC_offset)));
	}

	static void Clamped_Index_Interior (const GRID_3D<T>& grid, const ARRAYS_3D<T2>& u, const VECTOR_3D<T>& location, int& i, int& j, int& ij)
	{
		i = min (u.m_end - 1, u.m_start + 1 + max (0, (int) ( (location.x - grid.xmin) * grid.one_over_dx - u.m_start - grid.MAC_offset)));
		j = min (u.n_end - 1, u.n_start + 1 + max (0, (int) ( (location.y - grid.ymin) * grid.one_over_dy - u.n_start - grid.MAC_offset)));
		ij = min (u.mn_end - 1, u.mn_start + 1 + max (0, (int) ( (location.z - grid.zmin) * grid.one_over_dz - u.mn_start - grid.MAC_offset)));
	}

	static void Clamped_Index_Interior_End_Minus_One (const GRID_1D<T>& grid, const ARRAYS_1D<T2>& u, const VECTOR_1D<T>& location, int& i)
	{
		i = min (u.m_end - 2, u.m_start + 1 + max (0, (int) ( (location.x - grid.xmin) * grid.one_over_dx - u.m_start - grid.MAC_offset)));
	}

	static void Clamped_Index_Interior_End_Minus_One (const GRID_2D<T>& grid, const ARRAYS_2D<T2>& u, const VECTOR_2D<T>& location, int& i, int& j)
	{
		i = min (u.m_end - 2, u.m_start + 1 + max (0, (int) ( (location.x - grid.xmin) * grid.one_over_dx - u.m_start - grid.MAC_offset)));
		j = min (u.n_end - 2, u.n_start + 1 + max (0, (int) ( (location.y - grid.ymin) * grid.one_over_dy - u.n_start - grid.MAC_offset)));
	}

	static void Clamped_Index_Interior_End_Minus_One (const GRID_3D<T>& grid, const ARRAYS_3D<T2>& u, const VECTOR_3D<T>& location, int& i, int& j, int& ij)
	{
		i = min (u.m_end - 2, u.m_start + 1 + max (0, (int) ( (location.x - grid.xmin) * grid.one_over_dx - u.m_start - grid.MAC_offset)));
		j = min (u.n_end - 2, u.n_start + 1 + max (0, (int) ( (location.y - grid.ymin) * grid.one_over_dy - u.n_start - grid.MAC_offset)));
		ij = min (u.mn_end - 2, u.mn_start + 1 + max (0, (int) ( (location.z - grid.zmin) * grid.one_over_dz - u.mn_start - grid.MAC_offset)));
	}

	void Populate_New_Array (const GRID_1D<T>& grid, const ARRAYS_1D<T2>& u, const GRID_1D<T>& grid_new, ARRAYS_1D<T2>& u_new)
	{
		for (int i = u_new.m_start; i <= u_new.m_end; i++) u_new (i) = Clamped_To_Array (grid, u, grid_new.x (i));
	}

	void Populate_New_Array (const GRID_2D<T>& grid, const ARRAYS_2D<T2>& u, const GRID_2D<T>& grid_new, ARRAYS_2D<T2>& u_new)
	{
		for (int i = u_new.m_start; i <= u_new.m_end; i++) for (int j = u_new.n_start; j <= u_new.n_end; j++) u_new (i, j) = Clamped_To_Array (grid, u, grid_new.X (i, j));
	}

	void Populate_New_Array (const GRID_3D<T>& grid, const ARRAYS_3D<T2>& u, const GRID_3D<T>& grid_new, ARRAYS_3D<T2>& u_new)
	{
		for (int i = u_new.m_start; i <= u_new.m_end; i++) for (int j = u_new.n_start; j <= u_new.n_end; j++) for (int ij = u_new.mn_start; ij <= u_new.mn_end; ij++)
					u_new (i, j, ij) = Clamped_To_Array (grid, u, grid_new.X (i, j, ij));
	}

	void Default() const
	{
		std::cout << "THIS INTERPOLATION FUNCTION IS NOT DEFINED!" << std::endl;
	}

//#####################################################################
	virtual T2 Clamped_To_Array (const GRID_1D<T>& grid, const ARRAYS_1D<T2>& u, const VECTOR_1D<T>& X) const
	{
		Default();
		return T2();
	}
	virtual T2 Clamped_To_Array (const GRID_2D<T>& grid, const ARRAYS_2D<T2>& u, const VECTOR_2D<T>& X) const
	{
		Default();
		return T2();
	}
	virtual T2 Clamped_To_Array (const GRID_3D<T>& grid, const ARRAYS_3D<T2>& u, const VECTOR_3D<T>& X) const
	{
		Default();
		return T2();
	}
	virtual T2 From_Base_Node (const GRID_1D<T>& grid, const ARRAYS_1D<T2>& u, const VECTOR_1D<T>& X, const int i) const
	{
		Default();
		return T2();
	}
	virtual T2 From_Base_Node (const GRID_1D<T>& grid, const ARRAYS_1D<T2>& u, const VECTOR_1D<T>& X, const VECTOR_1D<int>& index) const
	{
		Default();
		return T2();
	}
	virtual T2 From_Base_Node (const GRID_2D<T>& grid, const ARRAYS_2D<T2>& u, const VECTOR_2D<T>& X, const int i, const int j) const
	{
		Default();
		return T2();
	}
	virtual T2 From_Base_Node (const GRID_2D<T>& grid, const ARRAYS_2D<T2>& u, const VECTOR_2D<T>& X, const VECTOR_2D<int>& index) const
	{
		Default();
		return T2();
	}
	virtual T2 From_Base_Node (const GRID_3D<T>& grid, const ARRAYS_3D<T2>& u, const VECTOR_3D<T>& X, const int i, const int j, const int ij) const
	{
		Default();
		return T2();
	}
	virtual T2 From_Base_Node (const GRID_3D<T>& grid, const ARRAYS_3D<T2>& u, const VECTOR_3D<T>& X, const VECTOR_3D<int>& index) const
	{
		Default();
		return T2();
	}
//#####################################################################
};
}
#endif
