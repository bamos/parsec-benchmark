//#####################################################################
// Copyright 2002-2004, Ronald Fedkiw, Duc Nguyen, Eran Guendelman, Robert Bridson, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class GRID_1D
//#####################################################################
// A number of functions (e.g. the Clamp functions) assume the grid indexing starts at 1.  This way we can use truncation rather than floor because floor is really slow.
//#####################################################################
#ifndef __GRID_1D__
#define __GRID_1D__

#include "../Matrices_And_Vectors/VECTOR_1D.h"
#include "../Math_Tools/clamp.h"
namespace PhysBAM
{

template<class T>
class GRID_1D
{
public:
	int m; // # of points: x direction
	T xmin, xmax; // left and right wall
	T dx; // cell size
	T one_over_dx;
	T MAC_offset; // 0 for a regular grid and .5 for a MAC grid
	int number_of_cells_x; // not saved to file

	GRID_1D()
	{
		Initialize (0);
	}

	GRID_1D (const int m_input, const T xmin_input, const T xmax_input)
	{
		Initialize (m_input, xmin_input, xmax_input);
	}

	GRID_1D (const int m_input)
	{
		Initialize (m_input);
	}

	GRID_1D (const GRID_1D<T>& grid_input)
	{
		Initialize (grid_input.m, grid_input.xmin, grid_input.xmax);

		if (grid_input.MAC_offset) Set_MAC_Grid();
	}

	template<class T2>
	GRID_1D (const GRID_1D<T2>& grid_input)
	{
		Initialize (grid_input.m, (T) grid_input.xmin, (T) grid_input.xmax);

		if (grid_input.MAC_offset) Set_MAC_Grid();
	}

	GRID_1D<T>& operator= (const GRID_1D<T>& grid_input)
	{
		if (this == &grid_input) return *this;

		Initialize (grid_input.m, grid_input.xmin, grid_input.xmax);

		if (grid_input.MAC_offset) Set_MAC_Grid();

		return *this;
	}

	void Initialize (const int m_input, const T xmin_input = 0, const T xmax_input = 1)
	{
		m = m_input;
		xmin = xmin_input;
		xmax = xmax_input;
		Set_Regular_Grid();
	}

	void Set_Regular_Grid()
	{
		if (m > 1)
		{
			MAC_offset = 0;
			dx = (xmax - xmin) / (m - 1);
			one_over_dx = 1 / dx;
			number_of_cells_x = m - 1;
		}
		else
		{
			MAC_offset = 0;
			dx = 0;
			one_over_dx = 0;
			number_of_cells_x = 0;
		}
	}

	void Set_MAC_Grid()
	{
		if (m > 0)
		{
			MAC_offset = .5;
			dx = (xmax - xmin) / m;
			one_over_dx = 1 / dx;
			number_of_cells_x = m;
		}
		else
		{
			MAC_offset = 0;
			dx = 0;
			one_over_dx = 0;
			number_of_cells_x = 0;
		}
	}

	bool Is_MAC_Grid() const
	{
		return MAC_offset == .5;
	}

	T x (const int i) const // x at the i=1 to i=m grid points
	{
		return xmin + (i - 1 + MAC_offset) * dx;
	}

	T x_plus_half (const int i) const
	{
		return xmin + (i - .5 + MAC_offset) * dx;
	}

	T x_minus_half (const int i) const
	{
		return xmin + (i - 1.5 + MAC_offset) * dx;
	}

	VECTOR_1D<T> X (const int i) const
	{
		return VECTOR_1D<T> (x (i));
	}

	T Node (const int i) const
	{
		return xmin + (i - (T) 1) * dx;
	}

	T Center (const int i) const
	{
		return xmin + (i - (T).5) * dx;
	}

	void Index (const VECTOR_1D<T>& location, int& i) const // returns the left index
	{
		i = (int) floor ( (location.x - xmin) * one_over_dx + 1 - MAC_offset);       // note that "floor" is expensive
	}

	void Cell (const VECTOR_1D<T>& location, int& i, const int number_of_ghost_cells) const // returns the left
	{
		i = (int) ( (location.x - xmin) * one_over_dx + number_of_ghost_cells + 1) - number_of_ghost_cells;
	}

	T Clamp (const VECTOR_1D<T>& location) const
	{
		return clamp (location.x, xmin, xmax);
	}

	void Clamp (int& i) const
	{
		i = clamp (i, 1, m);
	}

	void Clamp_End_Minus_One (int& i) const
	{
		i = clamp (i, 1, m - 1);
	}

	void Clamped_Index (const VECTOR_1D<T>& location, int& i) const
	{
		i = min (m, 1 + max (0, (int) ( (location.x - xmin) * one_over_dx - MAC_offset)));
	}

	void Clamped_Index_End_Minus_One (const VECTOR_1D<T>& location, int& i) const
	{
		i = min (m - 1, 1 + max (0, (int) ( (location.x - xmin) * one_over_dx - MAC_offset)));
	}

	void Clamp_To_Cell (const VECTOR_1D<T>& location, int& i) const
	{
		i = min (number_of_cells_x, 1 + max (0, (int) ( (location.x - xmin) * one_over_dx)));
	}

	void Clamp_To_Cell (const VECTOR_1D<T>& location, VECTOR_1D<int>& index) const
	{
		index.x = min (number_of_cells_x, 1 + max (0, (int) ( (location.x - xmin) * one_over_dx)));
	}

	void Clamp_To_Cell (const VECTOR_1D<T>& location, int& i, const int number_of_ghost_cells) const
	{
		i = min (number_of_cells_x + number_of_ghost_cells, 1 - number_of_ghost_cells + max (0, (int) ( (location.x - xmin) * one_over_dx + number_of_ghost_cells)));
	}

	bool Outside (const T location) const
	{
		return location < xmin || location > xmax;
	}

	bool Outside (const VECTOR_1D<T>& location) const
	{
		return location.x < xmin || location.x > xmax;
	}

	GRID_1D<T> Get_MAC_Grid() const
	{
		GRID_1D<T> new_grid (number_of_cells_x, xmin, xmax);
		new_grid.Set_MAC_Grid();
		return new_grid;
	}

	GRID_1D<T> Get_X_Face_Grid() const
	{
		return GRID_1D<T> (number_of_cells_x + 1, xmin, xmax);
	}

	GRID_1D<T> Get_Regular_Grid_At_MAC_Positions() const
	{
		assert (MAC_offset == (T).5);
		return GRID_1D<T> (m, xmin + (T).5 * dx, xmax - (T).5 * dx);
	}

	template<class RW>
	void Read (std::istream& input_stream)
	{
		Read_Binary<RW> (input_stream, m);
		assert (m > 0);
		Read_Binary<RW> (input_stream, xmin, xmax);
		Initialize (m, xmin, xmax);
		Read_Binary<RW> (input_stream, MAC_offset);

		if (MAC_offset) Set_MAC_Grid();
	}

	template<class RW>
	void Write (std::ostream& output_stream) const
	{
		Write_Binary<RW> (output_stream, m);
		Write_Binary<RW> (output_stream, xmin, xmax);
		Write_Binary<RW> (output_stream, MAC_offset);
	}

//#####################################################################
};
}
#endif
