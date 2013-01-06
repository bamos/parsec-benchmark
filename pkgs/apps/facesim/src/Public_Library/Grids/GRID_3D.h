//#####################################################################
// Copyright 2002-2004, Zhaosheng Bao, Robert Bridson, Ronald Fedkiw, Eran Guendelman, Geoffrey Irving, Sergey Koltakov, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class GRID_3D
//#####################################################################
// A number of functions (e.g. the Clamp functions) assume the grid indexing starts at 1.  This way we can use truncation rather than floor because floor is really slow.
//#####################################################################
#ifndef __GRID_3D__
#define __GRID_3D__

#include "../Geometry/BOX_3D.h"
#include "../Matrices_And_Vectors/VECTOR_3D.h"
#include "../Math_Tools/clamp.h"
namespace PhysBAM
{

template<class T>
class GRID_3D
{
public:
	int m, n, mn; // # of points: x, y and z direction
	T xmin, xmax, ymin, ymax, zmin, zmax; // left and right wall, bottom and top wall, front and back wall
	T dx, dy, dz; // cell sizes
	T one_over_dx, one_over_dy, one_over_dz, min_dx_dy_dz, max_dx_dy_dz;
	T MAC_offset; // 0 for a regular grid and .5 for a MAC grid
	int number_of_cells_x, number_of_cells_y, number_of_cells_z; // not saved to file

	GRID_3D()
	{
		Initialize (0, 0, 0);
	}

	GRID_3D (const int m_input, const int n_input, const int mn_input, const T xmin_input, const T xmax_input, const T ymin_input, const T ymax_input, const T zmin_input, const T zmax_input,
		 const bool MAC_grid = false)
	{
		Initialize (m_input, n_input, mn_input, xmin_input, xmax_input, ymin_input, ymax_input, zmin_input, zmax_input);

		if (MAC_grid) Set_MAC_Grid();
	}

	GRID_3D (const int m_input, const int n_input, const int mn_input, const BOX_3D<T>& box)
	{
		Initialize (m_input, n_input, mn_input, box);
	}

	GRID_3D (const int m_input, const int n_input, const int mn_input)
	{
		Initialize (m_input, n_input, mn_input);
	}

	GRID_3D (const GRID_3D<T>& grid_input)
	{
		Initialize (grid_input.m, grid_input.n, grid_input.mn, grid_input.xmin, grid_input.xmax, grid_input.ymin, grid_input.ymax, grid_input.zmin, grid_input.zmax);

		if (grid_input.MAC_offset) Set_MAC_Grid();
	}

	template<class T2>
	GRID_3D (const GRID_3D<T2>& grid_input)
	{
		Initialize (grid_input.m, grid_input.n, grid_input.mn, (T) grid_input.xmin, (T) grid_input.xmax, (T) grid_input.ymin, (T) grid_input.ymax, (T) grid_input.zmin, (T) grid_input.zmax);

		if (grid_input.MAC_offset) Set_MAC_Grid();
	}

	GRID_3D<T>& operator= (const GRID_3D<T>& grid_input)
	{
		if (this == &grid_input) return *this;

		Initialize (grid_input.m, grid_input.n, grid_input.mn, grid_input.xmin, grid_input.xmax, grid_input.ymin, grid_input.ymax, grid_input.zmin, grid_input.zmax);

		if (grid_input.MAC_offset) Set_MAC_Grid();

		return *this;
	}

	bool operator== (const GRID_3D<T>& grid) const
	{
		return (grid.m == m && grid.n == n && grid.mn == mn && grid.xmin == xmin && grid.xmax == xmax && grid.ymin == ymin && grid.ymax == ymax && grid.zmin == zmin && grid.zmax == zmax &&
			grid.MAC_offset == MAC_offset);
	}

	inline bool operator!= (const GRID_3D<T>& grid) const
	{
		return !operator== (grid);
	}

	void Initialize (const int m_input, const int n_input, const int mn_input, const BOX_3D<T>& box)
	{
		m = m_input;
		n = n_input;
		mn = mn_input;
		xmin = box.xmin;
		xmax = box.xmax;
		ymin = box.ymin;
		ymax = box.ymax;
		zmin = box.zmin;
		zmax = box.zmax;
		Set_Regular_Grid();
	}

	void Initialize (const int m_input, const int n_input, const int mn_input, const T xmin_input = 0, const T xmax_input = 1, const T ymin_input = 0, const T ymax_input = 1, const T zmin_input = 0,
			 const T zmax_input = 1)
	{
		m = m_input;
		n = n_input;
		mn = mn_input;
		xmin = xmin_input;
		xmax = xmax_input;
		ymin = ymin_input;
		ymax = ymax_input;
		zmin = zmin_input;
		zmax = zmax_input;
		Set_Regular_Grid();
	}

	void Set_Regular_Grid()
	{
		if (m > 1 && n > 1 && mn > 1)
		{
			MAC_offset = 0;
			dx = (xmax - xmin) / (m - 1);
			dy = (ymax - ymin) / (n - 1);
			dz = (zmax - zmin) / (mn - 1);
			one_over_dx = 1 / dx;
			one_over_dy = 1 / dy;
			one_over_dz = 1 / dz;
			min_dx_dy_dz = min (dx, dy, dz);
			max_dx_dy_dz = max (dx, dy, dz);
			number_of_cells_x = m - 1;
			number_of_cells_y = n - 1;
			number_of_cells_z = mn - 1;
		}
		else
		{
			MAC_offset = 0;
			dx = 0;
			dy = 0;
			dz = 0;
			one_over_dx = 0;
			one_over_dy = 0;
			one_over_dz = 0;
			min_dx_dy_dz = 0;
			max_dx_dy_dz = 0;
			number_of_cells_x = 0;
			number_of_cells_y = 0;
			number_of_cells_z = 0;
		}
	}

	void Set_MAC_Grid()
	{
		if (m > 0 && n > 0 && mn > 0)
		{
			MAC_offset = (T).5;
			dx = (xmax - xmin) / m;
			dy = (ymax - ymin) / n;
			dz = (zmax - zmin) / mn;
			one_over_dx = 1 / dx;
			one_over_dy = 1 / dy;
			one_over_dz = 1 / dz;
			min_dx_dy_dz = min (dx, dy, dz);
			max_dx_dy_dz = max (dx, dy, dz);
			number_of_cells_x = m;
			number_of_cells_y = n;
			number_of_cells_z = mn;
		}
		else
		{
			MAC_offset = 0;
			dx = 0;
			dy = 0;
			dz = 0;
			one_over_dx = 0;
			one_over_dy = 0;
			one_over_dz = 0;
			min_dx_dy_dz = 0;
			max_dx_dy_dz = 0;
			number_of_cells_x = 0;
			number_of_cells_y = 0;
			number_of_cells_z = 0;
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

	T y (const int j) const // y at the j=1 to j=n grid points
	{
		return ymin + (j - 1 + MAC_offset) * dy;
	}

	T z (const int ij) const // z at the ij=1 to ij=mn grid points
	{
		return zmin + (ij - 1 + MAC_offset) * dz;
	}

	T x_plus_half (const int i) const
	{
		return xmin + (i - T (.5) + MAC_offset) * dx;
	}

	T x_minus_half (const int i) const
	{
		return xmin + (i - T (1.5) + MAC_offset) * dx;
	}

	T y_plus_half (const int j) const
	{
		return ymin + (j - T (.5) + MAC_offset) * dy;
	}

	T y_minus_half (const int j) const
	{
		return ymin + (j - T (1.5) + MAC_offset) * dy;
	}

	T z_plus_half (const int ij) const
	{
		return zmin + (ij - T (.5) + MAC_offset) * dz;
	}

	T z_minus_half (const int ij) const
	{
		return zmin + (ij - T (1.5) + MAC_offset) * dz;
	}

	VECTOR_3D<T> X (const VECTOR_3D<int>& index) const
	{
		return VECTOR_3D<T> (x (index.x), y (index.y), z (index.z));
	}

	VECTOR_3D<T> X (const int i, const int j, const int ij) const
	{
		return VECTOR_3D<T> (x (i), y (j), z (ij));
	}

	VECTOR_3D<T> UX (const int i, const int j, const int ij) const
	{
		return VECTOR_3D<T> (x (i) - MAC_offset * dx, y (j), z (ij));
	}

	VECTOR_3D<T> VX (const int i, const int j, const int ij) const
	{
		return VECTOR_3D<T> (x (i), y (j) - MAC_offset * dy, z (ij));
	}

	VECTOR_3D<T> WX (const int i, const int j, const int ij) const
	{
		return VECTOR_3D<T> (x (i), y (j), z (ij) - MAC_offset * dz);
	}

	VECTOR_3D<T> Node (const int i, const int j, const int ij) const
	{
		return VECTOR_3D<T> (xmin + (i - (T) 1) * dx, ymin + (j - (T) 1) * dy, zmin + (ij - (T) 1) * dz);
	}

	VECTOR_3D<T> Node (const VECTOR_3D<int>& index) const
	{
		return VECTOR_3D<T> (xmin + (index.x - (T) 1) * dx, ymin + (index.y - (T) 1) * dy, zmin + (index.z - (T) 1) * dz);
	}

	VECTOR_3D<T> Center (const int i, const int j, const int ij) const
	{
		return VECTOR_3D<T> (xmin + (i - (T).5) * dx, ymin + (j - (T).5) * dy, zmin + (ij - (T).5) * dz);
	}

	VECTOR_3D<T> Center (const VECTOR_3D<int>& index) const
	{
		return VECTOR_3D<T> (xmin + (index.x - (T).5) * dx, ymin + (index.y - (T).5) * dy, zmin + (index.z - (T).5) * dz);
	}

	VECTOR_3D<T> X_Face (const int i, const int j, const int ij) const
	{
		return VECTOR_3D<T> (xmin + (i - (T) 1) * dx, ymin + (j - (T).5) * dy, zmin + (ij - (T).5) * dz);
	}

	VECTOR_3D<T> X_Face (const VECTOR_3D<int>& index) const
	{
		return VECTOR_3D<T> (xmin + (index.x - (T) 1) * dx, ymin + (index.y - (T).5) * dy, zmin + (index.z - (T).5) * dz);
	}

	VECTOR_3D<T> Y_Face (const int i, const int j, const int ij) const
	{
		return VECTOR_3D<T> (xmin + (i - (T).5) * dx, ymin + (j - (T) 1) * dy, zmin + (ij - (T).5) * dz);
	}

	VECTOR_3D<T> Y_Face (const VECTOR_3D<int>& index) const
	{
		return VECTOR_3D<T> (xmin + (index.x - (T).5) * dx, ymin + (index.y - (T) 1) * dy, zmin + (index.z - (T).5) * dz);
	}

	VECTOR_3D<T> Z_Face (const int i, const int j, const int ij) const
	{
		return VECTOR_3D<T> (xmin + (i - (T).5) * dx, ymin + (j - (T).5) * dy, zmin + (ij - (T) 1) * dz);
	}

	VECTOR_3D<T> Z_Face (const VECTOR_3D<int>& index) const
	{
		return VECTOR_3D<T> (xmin + (index.x - (T).5) * dx, ymin + (index.y - (T).5) * dy, zmin + (index.z - (T) 1) * dz);
	}

	void Index (const VECTOR_3D<T>& location, int& i, int& j, int& ij) const // returns the left, bottom and front indices on a regular grid
	{
		i = (int) floor ( (location.x - xmin) * one_over_dx + 1 - MAC_offset); // note that "floor" is expensive
		j = (int) floor ( (location.y - ymin) * one_over_dy + 1 - MAC_offset);
		ij = (int) floor ( (location.z - zmin) * one_over_dz + 1 - MAC_offset);
	}

	void Cell (const VECTOR_3D<T>& location, int& i, int& j, int& ij, const int number_of_ghost_cells) const // returns the left, bottom and front
	{
		int number_of_ghost_cells_plus_one = number_of_ghost_cells + 1;
		i = (int) ( (location.x - xmin) * one_over_dx + number_of_ghost_cells_plus_one) - number_of_ghost_cells;
		j = (int) ( (location.y - ymin) * one_over_dy + number_of_ghost_cells_plus_one) - number_of_ghost_cells;
		ij = (int) ( (location.z - zmin) * one_over_dz + number_of_ghost_cells_plus_one) - number_of_ghost_cells;
	}

	VECTOR_3D<T> Clamp (const VECTOR_3D<T>& location) const
	{
		return VECTOR_3D<T> (clamp (location.x, xmin, xmax), clamp (location.y, ymin, ymax), clamp (location.z, zmin, zmax));
	}

	VECTOR_3D<T> Clamp (const VECTOR_3D<T>& location, int number_of_ghost_cells) const // clamps to the grid (with ghost cells)
	{
		T extra_x = number_of_ghost_cells * dx, extra_y = number_of_ghost_cells * dy, extra_z = number_of_ghost_cells * dz;
		return VECTOR_3D<T> (clamp (location.x, xmin - extra_x, xmax + extra_x), clamp (location.y, ymin - extra_y, ymax + extra_y), clamp (location.z, zmin - extra_z, zmax + extra_z));
	}

	void Clamp (int& i, int& j, int& ij) const
	{
		i = clamp (i, 1, m);
		j = clamp (j, 1, n);
		ij = clamp (ij, 1, mn);
	}

	void Clamp_End_Minus_One (int& i, int& j, int& ij) const
	{
		i = clamp (i, 1, m - 1);
		j = clamp (j, 1, n - 1);
		ij = clamp (ij, 1, mn - 1);
	}

	void Clamped_Index (const VECTOR_3D<T>& location, int& i, int& j, int& ij) const
	{
		i = min (m, 1 + max (0, (int) ( (location.x - xmin) * one_over_dx - MAC_offset)));
		j = min (n, 1 + max (0, (int) ( (location.y - ymin) * one_over_dy - MAC_offset)));
		ij = min (mn, 1 + max (0, (int) ( (location.z - zmin) * one_over_dz - MAC_offset)));
	}

	void Clamped_Index (const VECTOR_3D<T>& location, VECTOR_3D<int>& index) const
	{
		Clamped_Index (location, index.x, index.y, index.z);
	}

	void Clamped_Index_End_Minus_One (const VECTOR_3D<T>& location, int& i, int& j, int& ij) const
	{
		i = min (m - 1, 1 + max (0, (int) ( (location.x - xmin) * one_over_dx - MAC_offset)));
		j = min (n - 1, 1 + max (0, (int) ( (location.y - ymin) * one_over_dy - MAC_offset)));
		ij = min (mn - 1, 1 + max (0, (int) ( (location.z - zmin) * one_over_dz - MAC_offset)));
	}

	void Clamped_Index_End_Minus_One (const VECTOR_3D<T>& location, VECTOR_3D<int>& index) const
	{
		Clamped_Index_End_Minus_One (location, index.x, index.y, index.z);
	}

	void Clamp_To_Cell (const VECTOR_3D<T>& location, int& i, int& j, int& ij) const
	{
		i = min (number_of_cells_x, 1 + max (0, (int) ( (location.x - xmin) * one_over_dx)));
		j = min (number_of_cells_y, 1 + max (0, (int) ( (location.y - ymin) * one_over_dy)));
		ij = min (number_of_cells_z, 1 + max (0, (int) ( (location.z - zmin) * one_over_dz)));
	}

	void Clamp_To_Cell (const VECTOR_3D<T>& location, VECTOR_3D<int>& index) const
	{
		Clamp_To_Cell (location, index.x, index.y, index.z);
	}

	void Clamp_To_Cell (const VECTOR_3D<T>& location, int& i, int& j, int& ij, const int number_of_ghost_cells) const
	{
		i = min (number_of_cells_x + number_of_ghost_cells, 1 - number_of_ghost_cells + max (0, (int) ( (location.x - xmin) * one_over_dx + number_of_ghost_cells)));
		j = min (number_of_cells_y + number_of_ghost_cells, 1 - number_of_ghost_cells + max (0, (int) ( (location.y - ymin) * one_over_dy + number_of_ghost_cells)));
		ij = min (number_of_cells_z + number_of_ghost_cells, 1 - number_of_ghost_cells + max (0, (int) ( (location.z - zmin) * one_over_dz + number_of_ghost_cells)));
	}

	void Clamp_To_Cell (const VECTOR_3D<T>& location, VECTOR_3D<int>& index, const int number_of_ghost_cells) const
	{
		Clamp_To_Cell (location, index.x, index.y, index.z, number_of_ghost_cells);
	}

	void Closest_Node (const VECTOR_3D<T>& location, int& i, int& j, int& ij) const
	{
		i = min (m, 1 + max (0, (int) ( (location.x - xmin) * one_over_dx + (T).5)));
		j = min (n, 1 + max (0, (int) ( (location.y - ymin) * one_over_dy + (T).5)));
		ij = min (mn, 1 + max (0, (int) ( (location.z - zmin) * one_over_dz + (T).5)));
	}

	void Closest_Node (const VECTOR_3D<T>& location, VECTOR_3D<int>& index) const
	{
		Closest_Node (location, index.x, index.y, index.z);
	}

	BOX_3D<T> Domain() const
	{
		return BOX_3D<T> (xmin, xmax, ymin, ymax, zmin, zmax);
	}

	bool Outside (const VECTOR_3D<T>& location) const
	{
		return location.x < xmin || location.x > xmax || location.y < ymin || location.y > ymax || location.z < zmin || location.z > zmax;
	}

	GRID_3D<T> Get_MAC_Grid() const
	{
		GRID_3D<T> new_grid (number_of_cells_x, number_of_cells_y, number_of_cells_z, xmin, xmax, ymin, ymax, zmin, zmax);
		new_grid.Set_MAC_Grid();
		return new_grid;
	}

	GRID_3D<T> Get_X_Face_Grid() const
	{
		return GRID_3D<T> (number_of_cells_x + 1, number_of_cells_y, number_of_cells_z, xmin, xmax, ymin + (T).5 * dy, ymax - (T).5 * dy, zmin + (T).5 * dz, zmax - (T).5 * dz);
	}

	GRID_3D<T> Get_Y_Face_Grid() const
	{
		return GRID_3D<T> (number_of_cells_x, number_of_cells_y + 1, number_of_cells_z, xmin + (T).5 * dx, xmax - (T).5 * dx, ymin, ymax, zmin + (T).5 * dz, zmax - (T).5 * dz);
	}

	GRID_3D<T> Get_Z_Face_Grid() const
	{
		return GRID_3D<T> (number_of_cells_x, number_of_cells_y, number_of_cells_z + 1, xmin + (T).5 * dx, xmax - (T).5 * dx, ymin + (T).5 * dy, ymax - (T).5 * dy, zmin, zmax);
	}

	GRID_3D<T> Get_Regular_Grid_At_MAC_Positions() const
	{
		assert (MAC_offset == (T).5);
		return GRID_3D<T> (m, n, mn, xmin + (T).5 * dx, xmax - (T).5 * dx, ymin + (T).5 * dy, ymax - (T).5 * dy, zmin + (T).5 * dz, zmax - (T).5 * dz);
	}

	static GRID_3D<T> Create_Grid_Given_Cell_Size (const BOX_3D<T>& domain, const T cell_size, const bool mac_grid, const int boundary_nodes = 0)
	{
		int number_of_cells_x = (int) ceil ( (domain.xmax - domain.xmin) / cell_size) + 2 * boundary_nodes, number_of_cells_y = (int) ceil ( (domain.ymax - domain.ymin) / cell_size) + 2 * boundary_nodes,
		    number_of_cells_z = (int) ceil ( (domain.zmax - domain.zmin) / cell_size) + 2 * boundary_nodes;
		VECTOR_3D<T> domain_center = domain.Center(), actual_domain_half_size = (T).5 * cell_size * VECTOR_3D<T> ( (T) number_of_cells_x, (T) number_of_cells_y, (T) number_of_cells_z);
		BOX_3D<T> actual_domain (domain_center - actual_domain_half_size, domain_center + actual_domain_half_size);

		if (mac_grid)
		{
			GRID_3D<T> grid (number_of_cells_x, number_of_cells_y, number_of_cells_z, actual_domain);
			grid.Set_MAC_Grid();
			return grid;
		}
		else return GRID_3D<T> (number_of_cells_x + 1, number_of_cells_y + 1, number_of_cells_z + 1, actual_domain);
	}

	static GRID_3D<T> Create_Even_Sized_Grid_Given_Cell_Size (const BOX_3D<T>& domain, const T cell_size, const bool mac_grid, const int boundary_nodes = 0)
	{
		int number_of_cells_x = (int) ceil ( (domain.xmax - domain.xmin) / cell_size), number_of_cells_y = (int) ceil ( (domain.ymax - domain.ymin) / cell_size),
		    number_of_cells_z = (int) ceil ( (domain.zmax - domain.zmin) / cell_size);
		T cell_size_x, cell_size_y, cell_size_z;

		if (number_of_cells_x % 2 == 1)
		{
			number_of_cells_x++;
			cell_size_x = (domain.xmax - domain.xmin) / number_of_cells_x;
		}
		else cell_size_x = cell_size;

		if (number_of_cells_y % 2 == 1)
		{
			number_of_cells_y++;
			cell_size_y = (domain.ymax - domain.ymin) / number_of_cells_y;
		}
		else cell_size_y = cell_size;

		if (number_of_cells_z % 2 == 1)
		{
			number_of_cells_z++;
			cell_size_z = (domain.zmax - domain.zmin) / number_of_cells_z;
		}
		else cell_size_z = cell_size;

		number_of_cells_x += 2 * boundary_nodes;
		number_of_cells_y += 2 * boundary_nodes;
		number_of_cells_z += 2 * boundary_nodes;
		VECTOR_3D<T> domain_center = domain.Center(), actual_domain_half_size = (T).5 * VECTOR_3D<T> (cell_size_x * number_of_cells_x, cell_size_y * number_of_cells_y, cell_size_z * number_of_cells_z);
		BOX_3D<T> actual_domain (domain_center - actual_domain_half_size, domain_center + actual_domain_half_size);

		if (mac_grid)
		{
			GRID_3D<T> grid (number_of_cells_x, number_of_cells_y, number_of_cells_z, actual_domain);
			grid.Set_MAC_Grid();
			return grid;
		}
		else return GRID_3D<T> (number_of_cells_x + 1, number_of_cells_y + 1, number_of_cells_z + 1, actual_domain);
	}

	template<class RW>
	void Read (std::istream& input_stream)
	{
		Read_Binary<RW> (input_stream, m, n, mn);
		assert (m > 0 && n > 0 && mn > 0);
		Read_Binary<RW> (input_stream, xmin, xmax, ymin, ymax, zmin, zmax);
		Initialize (m, n, mn, xmin, xmax, ymin, ymax, zmin, zmax);
		Read_Binary<RW> (input_stream, MAC_offset);

		if (MAC_offset) Set_MAC_Grid();
	}

	template<class RW>
	void Write (std::ostream& output_stream) const
	{
		Write_Binary<RW> (output_stream, m, n, mn);
		Write_Binary<RW> (output_stream, xmin, xmax, ymin, ymax, zmin, zmax);
		Write_Binary<RW> (output_stream, MAC_offset);
	}

//#####################################################################
};
// global functions
template<class T>
inline std::ostream& operator<< (std::ostream& output_stream, const GRID_3D<T>& grid)
{
	output_stream << grid.Domain() << " divided by (" << grid.m << "," << grid.n << "," << grid.mn << ")";
	return output_stream;
}
}
#endif
