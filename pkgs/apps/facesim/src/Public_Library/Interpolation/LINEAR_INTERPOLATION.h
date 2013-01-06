//#####################################################################
// Copyright 2003, 2004, Ronald Fedkiw, Eran Guendelman, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class LINEAR_INTERPOLATION
//#####################################################################
#ifndef __LINEAR_INTERPOLATION__
#define __LINEAR_INTERPOLATION__

#include "INTERPOLATION.h"
namespace PhysBAM
{

template<class T, class T2>
class LINEAR_INTERPOLATION: public INTERPOLATION<T, T2>
{
public:
	static T2 Linear (const T2& u_left, const T2& u_right, const T x)
	{
		return (1 - x) * u_left + x * u_right;
	}

	static T2 Linear (const T x_left, const T x_right, const T2& u_left, const T2& u_right, const T x)
	{
		return u_left + (x - x_left) * (u_right - u_left) / (x_right - x_left);
	}

	static T2 Linear (const T x_left, const T2& u_left, const T2& u_slope, const T x)
	{
		return u_left + (x - x_left) * u_slope;
	}

	static T2 Linear (const T x_left, const T2& u_left, const T2& u_slope, const VECTOR_1D<T> X)
	{
		return u_left + (X.x - x_left) * u_slope;
	}

	static T2 Linear_Predivided (const T x_left, const T one_over_x_right_minus_x_left, const T2& u_left, const T2& u_right, const T x)
	{
		return u_left + (x - x_left) * one_over_x_right_minus_x_left * (u_right - u_left);
	}

	static T2 Linear_Normalized (const T2& u_left, const T2& u_slope, const T x)
	{
		return u_left + x * u_slope;
	}

	// X in [0,1]x[0,1]
	static T2 Bilinear (const T2& u1, const T2& u2, const T2& u3, const T2& u4, const VECTOR_2D<T>& X)
	{
		T2 u_bottom = Linear_Normalized (u1, u2 - u1, X.x), u_top = Linear_Normalized (u3, u4 - u3, X.x);
		return Linear_Normalized (u_bottom, u_top - u_bottom, X.y);
	}

	static T2 Bilinear (const T2& u1, const T2& u3, T one_over_y_top_minus_y_bottom, const T x_left, const T y_bottom, const T2& slope12, const T2& slope34, const VECTOR_2D<T>& X)
	{
		T2 u_bottom = Linear (x_left, u1, slope12, X.x), u_top = Linear (x_left, u3, slope34, X.x);
		return Linear_Predivided (y_bottom, one_over_y_top_minus_y_bottom, u_bottom, u_top, X.y);
	}

	// X in [0,1]x[0,1]x[0,1]
	static T2 Trilinear (const T2& u1, const T2& u2, const T2& u3, const T2& u4, const T2& u5, const T2& u6, const T2& u7, const T2& u8, const VECTOR_3D<T>& X)
	{
		T2 u_bottom = Linear_Normalized (u1, u2 - u1, X.x), u_top = Linear_Normalized (u3, u4 - u3, X.x), u_front = Linear_Normalized (u_bottom, u_top - u_bottom, X.y);
		u_bottom = Linear_Normalized (u5, u6 - u5, X.x);
		u_top = Linear_Normalized (u7, u8 - u7, X.x);
		T2 u_back = Linear_Normalized (u_bottom, u_top - u_bottom, X.y);
		return Linear_Normalized (u_front, u_back - u_front, X.z);
	}

	static T2 Trilinear (const T2& u1, const T2& u3, const T2& u5, const T2& u7, T one_over_y_top_minus_y_bottom, T one_over_z_back_minus_z_front, const T x_left, const T y_bottom, const T z_front,
			     const T2& slope12, const T2& slope34, const T2& slope56, const T2& slope78, const VECTOR_3D<T>& X)
	{
		T2 u_bottom = Linear (x_left, u1, slope12, X.x), u_top = Linear (x_left, u3, slope34, X.x);
		T2 u_front = Linear_Predivided (y_bottom, one_over_y_top_minus_y_bottom, u_bottom, u_top, X.y);
		u_bottom = Linear (x_left, u5, slope56, X.x);
		u_top = Linear (x_left, u7, slope78, X.x);
		T2 u_back = Linear_Predivided (y_bottom, one_over_y_top_minus_y_bottom, u_bottom, u_top, X.y);
		return Linear_Predivided (z_front, one_over_z_back_minus_z_front, u_front, u_back, X.z);
	}

	T2 Clamped_To_Array (const GRID_1D<T>& grid, const ARRAYS_1D<T2>& u, const VECTOR_1D<T>& X) const
	{
		int i;
		INTERPOLATION<T, T2>::Clamped_Index_End_Minus_One (grid, u, X, i);
		return From_Base_Node (grid, u, X, i);
	}

	T2 Clamped_To_Array (const GRID_2D<T>& grid, const ARRAYS_2D<T2>& u, const VECTOR_2D<T>& X) const
	{
		int i, j;
		INTERPOLATION<T, T2>::Clamped_Index_End_Minus_One (grid, u, X, i, j);
		return From_Base_Node (grid, u, X, i, j);
	}

	T2 Clamped_To_Array (const GRID_3D<T>& grid, const ARRAYS_3D<T2>& u, const VECTOR_3D<T>& X) const
	{
		int i, j, ij;
		INTERPOLATION<T, T2>::Clamped_Index_End_Minus_One (grid, u, X, i, j, ij);
		return From_Base_Node (grid, u, X, i, j, ij);
	}

	T2 From_Base_Node (const GRID_1D<T>& grid, const ARRAYS_1D<T2>& u, const VECTOR_1D<T>& X, const int i) const
	{
		T2 u_i = u (i);
		return Linear (grid.x (i), u_i, grid.one_over_dx * (u (i + 1) - u_i), X.x);
	}

	T2 From_Base_Node (const GRID_1D<T>& grid, const ARRAYS_1D<T2>& u, const VECTOR_1D<T>& X, const VECTOR_1D<int>& index) const
	{
		return From_Base_Node (grid, u, X, index.x);
	}

	T2 From_Base_Node (const GRID_2D<T>& grid, const ARRAYS_2D<T2>& u, const VECTOR_2D<T>& X, const int i, const int j) const
	{
		T x_i = grid.x (i);
		int index = u.Standard_Index (i, j);
		T2 u_i_j = u.array[index], u_i_j1 = u.array[u.J_Plus_One (index)];
		T2 u_bottom = Linear (x_i, u_i_j, grid.one_over_dx * (u.array[u.I_Plus_One (index)] - u_i_j), X.x),
		   u_top = Linear (x_i, u_i_j1, grid.one_over_dx * (u.array[u.I_Plus_One_J_Plus_One (index)] - u_i_j1), X.x);
		return Linear (grid.y (j), u_bottom, grid.one_over_dy * (u_top - u_bottom), X.y);
	}

	T2 From_Base_Node (const GRID_2D<T>& grid, const ARRAYS_2D<T2>& u, const VECTOR_2D<T>& X, const VECTOR_2D<int>& index) const
	{
		return From_Base_Node (grid, u, X, index.x, index.y);
	}

	T2 From_Base_Node (const GRID_3D<T>& grid, const ARRAYS_3D<T2>& u, const VECTOR_3D<T>& X, const int i, const int j, const int ij) const
	{
		T dx = (X.x - grid.x (i)) * grid.one_over_dx, dy = (X.y - grid.y (j)) * grid.one_over_dy;
		int index = u.Standard_Index (i, j, ij);
		T2 u_i_j_ij = u.array[index], u_i_j1_ij = u.array[u.J_Plus_One (index)], u_i_j_ij1 = u.array[u.IJ_Plus_One (index)], u_i_j1_ij1 = u.array[u.J_Plus_One_IJ_Plus_One (index)];
		T2 u_bottom = Linear_Normalized (u_i_j_ij, u.array[u.I_Plus_One (index)] - u_i_j_ij, dx), u_top = Linear_Normalized (u_i_j1_ij, u.array[u.I_Plus_One_J_Plus_One (index)] - u_i_j1_ij, dx);
		T2 u_front = Linear_Normalized (u_bottom, u_top - u_bottom, dy);
		u_bottom = Linear_Normalized (u_i_j_ij1, u.array[u.I_Plus_One_IJ_Plus_One (index)] - u_i_j_ij1, dx);
		u_top = Linear_Normalized (u_i_j1_ij1, u.array[u.I_Plus_One_J_Plus_One_IJ_Plus_One (index)] - u_i_j1_ij1, dx);
		T2 u_back = Linear_Normalized (u_bottom, u_top - u_bottom, dy);
		return Linear (grid.z (ij), u_front, grid.one_over_dz * (u_back - u_front), X.z);
	}

	T2 From_Base_Node (const GRID_3D<T>& grid, const ARRAYS_3D<T2>& u, const VECTOR_3D<T>& X, const VECTOR_3D<int>& index) const
	{
		return From_Base_Node (grid, u, X, index.x, index.y, index.z);
	}
};
}
#endif
