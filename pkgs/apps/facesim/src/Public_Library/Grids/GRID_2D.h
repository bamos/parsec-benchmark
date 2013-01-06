//#####################################################################
// Copyright 2002-2004, Robert Bridson, Ronald Fedkiw, Eran Guendelman, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class GRID_2D
//#####################################################################
// A number of functions (e.g. the Clamp functions) assume the grid indexing starts at 1.  This way we can use truncation rather than floor because floor is really slow.
//#####################################################################
#ifndef __GRID_2D__
#define __GRID_2D__

#include "../Geometry/BOX_2D.h"
#include "../Matrices_And_Vectors/VECTOR_2D.h"
#include "../Math_Tools/clamp.h"
namespace PhysBAM
{

template<class T>
class GRID_2D
{
public:
	int m, n; // # of points: x and y direction
	T xmin, xmax, ymin, ymax; // left and right wall, bottom and top wall
	T dx, dy; // cell sizes
	T one_over_dx, one_over_dy, min_dx_dy, max_dx_dy;
	T MAC_offset; // 0 for a regular grid and .5 for a MAC grid
	int number_of_cells_x, number_of_cells_y; // not saved to file

	GRID_2D()
	{
		Initialize (0, 0);
	}

	GRID_2D (const int m_input, const int n_input, const T xmin_input, const T xmax_input, const T ymin_input, const T ymax_input)
	{
		Initialize (m_input, n_input, xmin_input, xmax_input, ymin_input, ymax_input);
	}

	GRID_2D (const int m_input, const int n_input, const BOX_2D<T>& box_input)
	{
		Initialize (m_input, n_input, box_input.xmin, box_input.xmax, box_input.ymin, box_input.ymax);
	}

	GRID_2D (const int m_input, const int n_input)
	{
		Initialize (m_input, n_input);
	}

	GRID_2D (const GRID_2D<T>& grid_input)
	{
		Initialize (grid_input.m, grid_input.n, grid_input.xmin, grid_input.xmax, grid_input.ymin, grid_input.ymax);

		if (grid_input.MAC_offset) Set_MAC_Grid();
	}

	template<class T2>
	GRID_2D (const GRID_2D<T2>& grid_input)
	{
		Initialize (grid_input.m, grid_input.n, (T) grid_input.xmin, (T) grid_input.xmax, (T) grid_input.ymin, (T) grid_input.ymax);

		if (grid_input.MAC_offset) Set_MAC_Grid();
	}

	GRID_2D<T>& operator= (const GRID_2D<T>& grid_input)
	{
		if (this == &grid_input) return *this;

		Initialize (grid_input.m, grid_input.n, grid_input.xmin, grid_input.xmax, grid_input.ymin, grid_input.ymax);

		if (grid_input.MAC_offset) Set_MAC_Grid();

		return *this;
	}

	void Initialize (const int m_input, const int n_input, const BOX_2D<T>& box)
	{
		m = m_input;
		n = n_input;
		xmin = box.xmin;
		xmax = box.xmax;
		ymin = box.ymin;
		ymax = box.ymax;
		Set_Regular_Grid();
	}

	void Initialize (const int m_input, const int n_input, const T xmin_input = 0, const T xmax_input = 1, const T ymin_input = 0, const T ymax_input = 1)
	{
		m = m_input;
		n = n_input;
		xmin = xmin_input;
		xmax = xmax_input;
		ymin = ymin_input;
		ymax = ymax_input;
		Set_Regular_Grid();
	}

	void Set_Regular_Grid()
	{
		if (m > 1 && n > 1)
		{
			MAC_offset = 0;
			dx = (xmax - xmin) / (m - 1);
			dy = (ymax - ymin) / (n - 1);
			one_over_dx = 1 / dx;
			one_over_dy = 1 / dy;
			min_dx_dy = min (dx, dy);
			max_dx_dy = max (dx, dy);
			number_of_cells_x = m - 1;
			number_of_cells_y = n - 1;
		}
		else
		{
			MAC_offset = 0;
			dx = 0;
			dy = 0;
			one_over_dx = 0;
			one_over_dy = 0;
			min_dx_dy = 0;
			max_dx_dy = 0;
			number_of_cells_x = 0;
			number_of_cells_y = 0;
		}
	}

	void Set_MAC_Grid()
	{
		if (m > 0 && n > 0)
		{
			MAC_offset = (T).5;
			dx = (xmax - xmin) / m;
			dy = (ymax - ymin) / n;
			one_over_dx = 1 / dx;
			one_over_dy = 1 / dy;
			min_dx_dy = min (dx, dy);
			max_dx_dy = max (dx, dy);
			number_of_cells_x = m;
			number_of_cells_y = n;
		}
		else
		{
			MAC_offset = 0;
			dx = 0;
			dy = 0;
			one_over_dx = 0;
			one_over_dy = 0;
			min_dx_dy = 0;
			max_dx_dy = 0;
			number_of_cells_x = 0;
			number_of_cells_y = 0;
		}
	}

	bool Is_MAC_Grid() const
	{
		return MAC_offset == .5;
	}

	T x (const int i) const // x at the i=1 to i=m grid points
	{
		return xmin + (i - (T) 1 + MAC_offset) * dx;
	}

	T y (const int j) const // y at the j=1 to j=n grid points
	{
		return ymin + (j - (T) 1 + MAC_offset) * dy;
	}

	T x_plus_half (const int i) const
	{
		return xmin + (i - (T).5 + MAC_offset) * dx;
	}

	T x_minus_half (const int i) const
	{
		return xmin + (i - (T) 1.5 + MAC_offset) * dx;
	}

	T y_plus_half (const int j) const
	{
		return ymin + (j - (T).5 + MAC_offset) * dy;
	}

	T y_minus_half (const int j) const
	{
		return ymin + (j - (T) 1.5 + MAC_offset) * dy;
	}

	VECTOR_2D<T> X (const int i, const int j) const
	{
		return VECTOR_2D<T> (x (i), y (j));
	}

	VECTOR_2D<T> X (const VECTOR_2D<int>& index) const
	{
		return VECTOR_2D<T> (x (index.x), y (index.y));
	}

	VECTOR_2D<T> Node (const int i, const int j) const
	{
		return VECTOR_2D<T> (xmin + (i - (T) 1) * dx, ymin + (j - (T) 1) * dy);
	}

	VECTOR_2D<T> Node (const VECTOR_2D<int>& index) const
	{
		return VECTOR_2D<T> (xmin + (index.x - (T) 1) * dx, ymin + (index.y - (T) 1) * dy);
	}

	VECTOR_2D<T> Center (const int i, const int j) const
	{
		return VECTOR_2D<T> (xmin + (i - (T).5) * dx, ymin + (j - (T).5) * dy);
	}

	VECTOR_2D<T> Center (const VECTOR_2D<int>& index) const
	{
		return VECTOR_2D<T> (xmin + (index.x - (T).5) * dx, ymin + (index.y - (T).5) * dy);
	}

	VECTOR_2D<T> X_Face (const int i, const int j) const
	{
		return VECTOR_2D<T> (xmin + (i - (T) 1) * dx, ymin + (j - (T).5) * dy);
	}

	VECTOR_2D<T> X_Face (const VECTOR_2D<int>& index) const
	{
		return VECTOR_2D<T> (xmin + (index.x - (T) 1) * dx, ymin + (index.y - (T).5) * dy);
	}

	VECTOR_2D<T> Y_Face (const int i, const int j) const
	{
		return VECTOR_2D<T> (xmin + (i - (T).5) * dx, ymin + (j - (T) 1) * dy);
	}

	VECTOR_2D<T> Y_Face (const VECTOR_2D<int>& index) const
	{
		return VECTOR_2D<T> (xmin + (index.x - (T).5) * dx, ymin + (index.y - (T) 1) * dy);
	}

	void Index (const VECTOR_2D<T>& location, int& i, int& j) const // returns the left and bottom indices
	{
		i = (int) floor ( (location.x - xmin) * one_over_dx + 1 - MAC_offset); // note that "floor" is expensive
		j = (int) floor ( (location.y - ymin) * one_over_dy + 1 - MAC_offset);
	}

	void Cell (const VECTOR_2D<T>& location, int& i, int& j, const int number_of_ghost_cells) const // returns the left and bottom
	{
		int number_of_ghost_cells_plus_one = number_of_ghost_cells + 1;
		i = (int) ( (location.x - xmin) * one_over_dx + number_of_ghost_cells_plus_one) - number_of_ghost_cells;
		j = (int) ( (location.y - ymin) * one_over_dy + number_of_ghost_cells_plus_one) - number_of_ghost_cells;
	}

	VECTOR_2D<T> Clamp (const VECTOR_2D<T>& location) const
	{
		return VECTOR_2D<T> (clamp (location.x, xmin, xmax), clamp (location.y, ymin, ymax));
	}

	VECTOR_2D<T> Clamp (const VECTOR_2D<T>& location, int number_of_ghost_cells) const // clamps to the grid (with ghost cells)
	{
		T extra_x = number_of_ghost_cells * dx, extra_y = number_of_ghost_cells * dy;
		return VECTOR_2D<T> (clamp (location.x, xmin - extra_x, xmax + extra_x), clamp (location.y, ymin - extra_y, ymax + extra_y));
	}

	void Clamp (int& i, int& j) const
	{
		i = clamp (i, 1, m);
		j = clamp (j, 1, n);
	}

	void Clamp_End_Minus_One (int& i, int& j) const
	{
		i = clamp (i, 1, m - 1);
		j = clamp (j, 1, n - 1);
	}

	void Clamped_Index (const VECTOR_2D<T>& location, int& i, int& j) const
	{
		i = min (m, 1 + max (0, (int) ( (location.x - xmin) * one_over_dx - MAC_offset)));
		j = min (n, 1 + max (0, (int) ( (location.y - ymin) * one_over_dy - MAC_offset)));
	}

	void Clamped_Index (const VECTOR_2D<T>& location, VECTOR_2D<int>& index) const
	{
		Clamped_Index (location, index.x, index.y);
	}

	void Clamped_Index_End_Minus_One (const VECTOR_2D<T>& location, int& i, int& j) const
	{
		i = min (m - 1, 1 + max (0, (int) ( (location.x - xmin) * one_over_dx - MAC_offset)));
		j = min (n - 1, 1 + max (0, (int) ( (location.y - ymin) * one_over_dy - MAC_offset)));
	}

	void Clamped_Index_End_Minus_One (const VECTOR_2D<T>& location, VECTOR_2D<int>& index) const
	{
		Clamped_Index_End_Minus_One (location, index.x, index.y);
	}

	void Clamp_To_Cell (const VECTOR_2D<T>& location, int& i, int& j) const
	{
		i = min (number_of_cells_x, 1 + max (0, (int) ( (location.x - xmin) * one_over_dx)));
		j = min (number_of_cells_y, 1 + max (0, (int) ( (location.y - ymin) * one_over_dy)));
	}

	void Clamp_To_Cell (const VECTOR_2D<T>& location, VECTOR_2D<int>& index) const
	{
		Clamp_To_Cell (location, index.x, index.y);
	}

	void Clamp_To_Cell (const VECTOR_2D<T>& location, int& i, int& j, const int number_of_ghost_cells) const
	{
		i = min (number_of_cells_x + number_of_ghost_cells, 1 - number_of_ghost_cells + max (0, (int) ( (location.x - xmin) * one_over_dx + number_of_ghost_cells)));
		j = min (number_of_cells_y + number_of_ghost_cells, 1 - number_of_ghost_cells + max (0, (int) ( (location.y - ymin) * one_over_dy + number_of_ghost_cells)));
	}

	void Clamp_To_Cell (const VECTOR_2D<T>& location, VECTOR_2D<int>& index, const int number_of_ghost_cells) const
	{
		Clamp_To_Cell (location, index.x, index.y, number_of_ghost_cells);
	}

	void Closest_Node (const VECTOR_2D<T>& location, int& i, int& j) const
	{
		i = min (m, 1 + max (0, (int) ( (location.x - xmin) * one_over_dx + (T).5)));
		j = min (n, 1 + max (0, (int) ( (location.y - ymin) * one_over_dy + (T).5)));
	}

	void Closest_Node (const VECTOR_2D<T>& location, VECTOR_2D<int>& index) const
	{
		Closest_Node (location, index.x, index.y);
	}

	BOX_2D<T> Domain() const
	{
		return BOX_2D<T> (xmin, xmax, ymin, ymax);
	}

	bool Outside (const VECTOR_2D<T>& location) const
	{
		return location.x < xmin || location.x > xmax || location.y < ymin || location.y > ymax;
	}

	GRID_2D<T> Get_MAC_Grid() const
	{
		GRID_2D<T> new_grid (number_of_cells_x, number_of_cells_y, xmin, xmax, ymin, ymax);
		new_grid.Set_MAC_Grid();
		return new_grid;
	}

	GRID_2D<T> Get_X_Face_Grid() const
	{
		return GRID_2D<T> (number_of_cells_x + 1, number_of_cells_y, xmin, xmax, ymin + (T).5 * dy, ymax - (T).5 * dy);
	}

	GRID_2D<T> Get_Y_Face_Grid() const
	{
		return GRID_2D<T> (number_of_cells_x, number_of_cells_y + 1, xmin + (T).5 * dx, xmax - (T).5 * dx, ymin, ymax);
	}

	GRID_2D<T> Get_Regular_Grid_At_MAC_Positions() const
	{
		assert (MAC_offset == (T).5);
		return GRID_2D<T> (m, n, xmin + (T).5 * dx, xmax - (T).5 * dx, ymin + (T).5 * dy, ymax - (T).5 * dy);
	}

	static GRID_2D<T> Create_Grid_Given_Cell_Size (const BOX_2D<T>& domain, const T cell_size, const bool mac_grid, const int boundary_nodes = 0)
	{
		int number_of_cells_x = (int) ceil ( (domain.xmax - domain.xmin) / cell_size) + 2 * boundary_nodes, number_of_cells_y = (int) ceil ( (domain.ymax - domain.ymin) / cell_size) + 2 * boundary_nodes;
		VECTOR_2D<T> domain_center = domain.Center(), actual_domain_half_size ( (T).5 * number_of_cells_x * cell_size, (T).5 * number_of_cells_y * cell_size);
		BOX_2D<T> actual_domain (domain_center - actual_domain_half_size, domain_center + actual_domain_half_size);

		if (mac_grid)
		{
			GRID_2D<T> grid (number_of_cells_x, number_of_cells_y, actual_domain);
			grid.Set_MAC_Grid();
			return grid;
		}
		else return GRID_2D<T> (number_of_cells_x + 1, number_of_cells_y + 1, actual_domain);
	}

	template<class RW>
	void Read (std::istream& input_stream)
	{
		Read_Binary<RW> (input_stream, m, n);
		assert (m > 0 && n > 0);
		Read_Binary<RW> (input_stream, xmin, xmax, ymin, ymax);
		Initialize (m, n, xmin, xmax, ymin, ymax);
		Read_Binary<RW> (input_stream, MAC_offset);

		if (MAC_offset) Set_MAC_Grid();
	}

	template<class RW>
	void Write (std::ostream& output_stream) const
	{
		Write_Binary<RW> (output_stream, m, n);
		Write_Binary<RW> (output_stream, xmin, xmax, ymin, ymax);
		Write_Binary<RW> (output_stream, MAC_offset);
	}

//#####################################################################
};
// global functions
template<class T>
inline std::ostream& operator<< (std::ostream& output_stream, const GRID_2D<T>& grid)
{
	output_stream << grid.Domain() << " divided by (" << grid.m << "," << grid.n << ")";
	return output_stream;
}
}
#endif
