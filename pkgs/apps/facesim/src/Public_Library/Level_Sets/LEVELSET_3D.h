//#####################################################################
// Copyright 2002, 2003, 2004, Zhaosheng Bao, Ronald Fedkiw, Eran Guendelman, Sergey Koltakov, Neil Molino, Igor Neverov, Robert Bridson, Rachel Weinstein.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class LEVELSET_3D
//#####################################################################
#ifndef __LEVELSET_3D__
#define __LEVELSET_3D__

#include "LEVELSET.h"
#include "../Grids/GRID_3D.h"
#include "../Arrays/ARRAYS_3D.h"
#include "../Arrays/LIST_ARRAY.h"
#include "../Interpolation/LINEAR_INTERPOLATION.h"
#include "../Matrices_And_Vectors/VECTOR_3D.h"
#include "../Matrices_And_Vectors/MATRIX_3X3.h"
namespace PhysBAM
{

template<class T> class OCTREE_LEVELSET;
template<class T> class OCTREE_GRID;
template<class T> class OCTREE_CELL;
template<class T> class TRIANGULATED_SURFACE;
template<class T> class COLLISION_BODY_LIST_3D;

template<class T>
class LEVELSET_3D: public LEVELSET<T, VECTOR_3D<T> >
{
protected:
	using LEVELSET<T, VECTOR_3D<T> >::refine_fmm_initialization_with_iterative_solver;
	using LEVELSET<T, VECTOR_3D<T> >::fmm_initialization_iterations;
	using LEVELSET<T, VECTOR_3D<T> >::fmm_initialization_iterative_tolerance;
	using LEVELSET<T, VECTOR_3D<T> >::fmm_initialization_iterative_drift_fraction;
public:
	using LEVELSET<T, VECTOR_3D<T> >::small_number;
	using LEVELSET<T, VECTOR_3D<T> >::max_time_step;
	using LEVELSET<T, VECTOR_3D<T> >::reinitialization_cfl;
	using LEVELSET<T, VECTOR_3D<T> >::reinitialization_runge_kutta_order;
	using LEVELSET<T, VECTOR_3D<T> >::reinitialization_spatial_order;
	using LEVELSET<T, VECTOR_3D<T> >::curvature_motion;
	using LEVELSET<T, VECTOR_3D<T> >::sigma;
	using LEVELSET<T, VECTOR_3D<T> >::interpolation;
	using LEVELSET<T, VECTOR_3D<T> >::curvature_interpolation;
	using LEVELSET<T, VECTOR_3D<T> >::normal_interpolation;
	using LEVELSET<T, VECTOR_3D<T> >::secondary_interpolation;
	using LEVELSET<T, VECTOR_3D<T> >::collision_aware_interpolation_plus_default;
	using LEVELSET<T, VECTOR_3D<T> >::collision_aware_interpolation_minus_default;

	GRID_3D<T>& grid;
	ARRAYS_3D<T>& phi;
	ARRAYS_3D<VECTOR_3D<T> >* normals;
	ARRAYS_3D<T> *curvature;
	ARRAYS_3D<T> *cell_maximum, *cell_minimum;
	ARRAYS_3D<bool>* narrowband;
	const COLLISION_BODY_LIST_3D<T>* collision_body_list;
	const ARRAYS_3D<bool>* node_neighbors_visible;
	bool clamp_phi_with_collision_bodies;
	int collision_aware_signed_distance_material_sign;

	LEVELSET_3D (GRID_3D<T>& grid_input, ARRAYS_3D<T>& phi_input)
		: grid (grid_input), phi (phi_input), normals (0), curvature (0), cell_maximum (0), cell_minimum (0), narrowband (0), collision_body_list (0), node_neighbors_visible (0)
	{}

	~LEVELSET_3D()
	{
		delete normals;
		delete curvature;
		delete cell_maximum;
		delete cell_minimum;
		delete narrowband;
	}

	void Set_Collision_Aware_Signed_Distance (const COLLISION_BODY_LIST_3D<T>& collision_body_list_input, const ARRAYS_3D<bool>* node_neighbors_visible_input, const bool clamp_phi_with_collision_bodies_input = false, const int collision_aware_signed_distance_material_sign_input = -1)
	{
		collision_body_list = &collision_body_list_input;
		node_neighbors_visible = node_neighbors_visible_input;
		clamp_phi_with_collision_bodies = clamp_phi_with_collision_bodies_input;
		collision_aware_signed_distance_material_sign = collision_aware_signed_distance_material_sign_input;
	}

	T Phi (const VECTOR_3D<T>& location) const
	{
		return interpolation->Clamped_To_Array (grid, phi, location);
	}

	T Phi_Secondary (const VECTOR_3D<T>& location) const
	{
		return secondary_interpolation->Clamped_To_Array (grid, phi, location);
	}

	T Extended_Phi (const VECTOR_3D<T>& location) const
	{
		VECTOR_3D<T> clamped_location (grid.Clamp (location));
		return interpolation->Clamped_To_Array (grid, phi, clamped_location) + (clamped_location - location).Magnitude();
	}

	VECTOR_3D<T> Normal (const VECTOR_3D<T>& location) const
	{
		if (normals)
		{
			VECTOR_3D<T> approximate_normal (normal_interpolation->Clamped_To_Array (grid, *normals, location));
			T magnitude = approximate_normal.Magnitude();

			if (magnitude > small_number) return approximate_normal / magnitude;
			else return VECTOR_3D<T> (1, 0, 0);
		}
		else
		{
			VECTOR_3D<T> approximate_normal ( (Phi (VECTOR_3D<T> (location.x + grid.dx, location.y, location.z)) - Phi (VECTOR_3D<T> (location.x - grid.dx, location.y, location.z))) / (2 * grid.dx),
							  (Phi (VECTOR_3D<T> (location.x, location.y + grid.dy, location.z)) - Phi (VECTOR_3D<T> (location.x, location.y - grid.dy, location.z))) / (2 * grid.dy),
							  (Phi (VECTOR_3D<T> (location.x, location.y, location.z + grid.dz)) - Phi (VECTOR_3D<T> (location.x, location.y, location.z - grid.dz))) / (2 * grid.dz));
			T magnitude = approximate_normal.Magnitude();

			if (magnitude > small_number) return approximate_normal / magnitude;
			else return VECTOR_3D<T> (1, 0, 0);
		}
	} // default normal points to the right

	VECTOR_3D<T> Extended_Normal (const VECTOR_3D<T>& location) const
	{
		if (normals)
		{
			VECTOR_3D<T> approximate_normal (normal_interpolation->Clamped_To_Array (grid, *normals, grid.Clamp (location)));
			T magnitude = approximate_normal.Magnitude();

			if (magnitude > small_number) return approximate_normal / magnitude;
			else return VECTOR_3D<T> (1, 0, 0);
		}
		else
		{
			VECTOR_3D<T> approximate_normal ( (Extended_Phi (VECTOR_3D<T> (location.x + grid.dx, location.y, location.z)) - Extended_Phi (VECTOR_3D<T> (location.x - grid.dx, location.y, location.z))) / (2 * grid.dx),
							  (Extended_Phi (VECTOR_3D<T> (location.x, location.y + grid.dy, location.z)) - Extended_Phi (VECTOR_3D<T> (location.x, location.y - grid.dy, location.z))) / (2 * grid.dy),
							  (Extended_Phi (VECTOR_3D<T> (location.x, location.y, location.z + grid.dz)) - Extended_Phi (VECTOR_3D<T> (location.x, location.y, location.z - grid.dz))) / (2 * grid.dz));
			T magnitude = approximate_normal.Magnitude();

			if (magnitude > small_number) return approximate_normal / magnitude;
			else return VECTOR_3D<T> (1, 0, 0);
		}
	} // default normal points to the right

	T Curvature (const VECTOR_3D<T>& location) const // later add finite difference for curvature, like in normal above
	{
		assert (curvature);
		return curvature_interpolation->Clamped_To_Array (grid, *curvature, location);
	}

	MATRIX_3X3<T> Hessian (const VECTOR_3D<T>& X) const
	{
		T one_over_dx = 1 / grid.dx, one_over_dy = 1 / grid.dy, one_over_dz = 1 / grid.dz;
		T two_phi_center = 2 * Phi (X);
		T phi_xx = (Phi (VECTOR_3D<T> (X.x + grid.dx, X.y, X.z)) - two_phi_center + Phi (VECTOR_3D<T> (X.x - grid.dx, X.y, X.z))) * sqr (one_over_dx),
		  phi_yy = (Phi (VECTOR_3D<T> (X.x, X.y + grid.dy, X.z)) - two_phi_center + Phi (VECTOR_3D<T> (X.x, X.y - grid.dy, X.z))) * sqr (one_over_dy),
		  phi_zz = (Phi (VECTOR_3D<T> (X.x, X.y, X.z + grid.dz)) - two_phi_center + Phi (VECTOR_3D<T> (X.x, X.y, X.z - grid.dz))) * sqr (one_over_dz),
		  phi_xy = (Phi (VECTOR_3D<T> (X.x + grid.dx, X.y + grid.dy, X.z)) - Phi (VECTOR_3D<T> (X.x + grid.dx, X.y - grid.dy, X.z))
			    - Phi (VECTOR_3D<T> (X.x - grid.dx, X.y + grid.dy, X.z)) + Phi (VECTOR_3D<T> (X.x - grid.dx, X.y - grid.dy, X.z))) * (T).25 * one_over_dx * one_over_dy,
			   phi_xz = (Phi (VECTOR_3D<T> (X.x + grid.dx, X.y, X.z + grid.dz)) - Phi (VECTOR_3D<T> (X.x + grid.dx, X.y, X.z - grid.dz))
				     - Phi (VECTOR_3D<T> (X.x - grid.dx, X.y, X.z + grid.dz)) + Phi (VECTOR_3D<T> (X.x - grid.dx, X.y, X.z - grid.dz))) * (T).25 * one_over_dx * one_over_dz,
				    phi_yz = (Phi (VECTOR_3D<T> (X.x, X.y + grid.dy, X.z + grid.dz)) - Phi (VECTOR_3D<T> (X.x, X.y + grid.dy, X.z - grid.dz))
					      - Phi (VECTOR_3D<T> (X.x, X.y - grid.dy, X.z + grid.dz)) + Phi (VECTOR_3D<T> (X.x, X.y - grid.dy, X.z - grid.dz))) * (T).25 * one_over_dy * one_over_dz;
		return MATRIX_3X3<T> (phi_xx, phi_xy, phi_xz, phi_xy, phi_yy, phi_yz, phi_xz, phi_yz, phi_zz);
	}

	void Principle_Curvatures (const VECTOR_3D<T>& X, T& curvature1, T& curvature2) const
	{
		NOT_IMPLEMENTED();
	}

	bool Lazy_Inside (const VECTOR_3D<T>& clamped_X, const T contour_value = 0) const
	{
		assert (cell_maximum && cell_minimum);
		int i, j, ij;
		INTERPOLATION<T, T>::Clamped_Index_End_Minus_One (grid, phi, clamped_X, i, j, ij);

		if ( (*cell_minimum) (i, j, ij) > contour_value) return false;
		else if ( (*cell_maximum) (i, j, ij) <= contour_value) return true;

		return (interpolation->From_Base_Node (grid, phi, clamped_X, i, j, ij) <= contour_value);
	}

	bool Lazy_Inside_And_Value (const VECTOR_3D<T>& clamped_X, T& phi_value, const T contour_value = 0) const
	{
		assert (cell_maximum && cell_minimum);
		int i, j, ij;
		INTERPOLATION<T, T>::Clamped_Index_End_Minus_One (grid, phi, clamped_X, i, j, ij);

		if ( (*cell_minimum) (i, j, ij) > contour_value) return false;

		phi_value = interpolation->From_Base_Node (grid, phi, clamped_X, i, j, ij);
		return (phi_value <= contour_value);
	}

	bool Lazy_Inside_Extended_Levelset (const VECTOR_3D<T>& unclamped_X, const T contour_value = 0) const
	{
		assert (cell_maximum && cell_minimum);
		VECTOR_3D<T> clamped_X (grid.Clamp (unclamped_X));
		T magnitude_squared = (unclamped_X - clamped_X).Magnitude_Squared();
		int i, j, ij;
		INTERPOLATION<T, T>::Clamped_Index_End_Minus_One (grid, phi, clamped_X, i, j, ij);

		if (magnitude_squared == 0 && (*cell_maximum) (i, j, ij) <= contour_value) return true;

		T phi_value = interpolation->From_Base_Node (grid, phi, clamped_X, i, j, ij);

		if (magnitude_squared > 0) phi_value = sqrt (magnitude_squared + sqr (max ( (T) 0, phi_value)));

		return (phi_value <= contour_value);
	}

	// Only sets phi_value if it also returns true
	bool Lazy_Inside_Extended_Levelset_And_Value (const VECTOR_3D<T>& unclamped_X, T& phi_value, const T contour_value = 0) const
	{
		assert (cell_maximum && cell_minimum);
		VECTOR_3D<T> clamped_X (grid.Clamp (unclamped_X));
		T magnitude_squared = (unclamped_X - clamped_X).Magnitude_Squared();

		if (contour_value <= 0)
		{
			if (magnitude_squared > 0) return false;
		}
		else if (magnitude_squared > sqr (contour_value)) return false;

		int i, j, ij;
		INTERPOLATION<T, T>::Clamped_Index_End_Minus_One (grid, phi, clamped_X, i, j, ij);

		if ( (*cell_minimum) (i, j, ij) > contour_value) return false;

		phi_value = interpolation->From_Base_Node (grid, phi, clamped_X, i, j, ij);

		if (magnitude_squared > 0) phi_value = sqrt (magnitude_squared + sqr (max ( (T) 0, phi_value)));

		return (phi_value <= contour_value);
	}

	bool Lazy_Outside (const VECTOR_3D<T>& clamped_X, const T contour_value = 0) const
	{
		return !Lazy_Inside (clamped_X, contour_value);
	}

	bool Lazy_Outside_Extended_Levelset (const VECTOR_3D<T>& unclamped_X, const T contour_value = 0) const
	{
		assert (cell_maximum && cell_minimum);
		VECTOR_3D<T> clamped_X (grid.Clamp (unclamped_X));
		T magnitude_squared = (unclamped_X - clamped_X).Magnitude_Squared();

		if (magnitude_squared > sqr (max ( (T) 0, contour_value))) return true; // we're outside the box beyond contour value

		int i, j, ij;
		INTERPOLATION<T, T>::Clamped_Index_End_Minus_One (grid, phi, clamped_X, i, j, ij);

		if ( (*cell_minimum) (i, j, ij) > contour_value) return true;

		T phi_value = interpolation->From_Base_Node (grid, phi, clamped_X, i, j, ij);

		if (magnitude_squared > 0) phi_value = sqrt (magnitude_squared + sqr (max ( (T) 0, phi_value)));

		return (phi_value > contour_value);
	}

	// Only sets phi_value if it also returns true
	bool Lazy_Outside_Extended_Levelset_And_Value (const VECTOR_3D<T>& unclamped_X, T& phi_value, const T contour_value = 0) const
	{
		assert (cell_maximum && cell_minimum);
		VECTOR_3D<T> clamped_X (grid.Clamp (unclamped_X));
		T magnitude_squared = (unclamped_X - clamped_X).Magnitude_Squared();
		int i, j, ij;
		INTERPOLATION<T, T>::Clamped_Index_End_Minus_One (grid, phi, clamped_X, i, j, ij);

		if (magnitude_squared == 0 && (*cell_maximum) (i, j, ij) <= contour_value) return false;

		phi_value = interpolation->From_Base_Node (grid, phi, clamped_X, i, j, ij);

		if (magnitude_squared > 0) phi_value = sqrt (magnitude_squared + sqr (max ( (T) 0, phi_value)));

		return (phi_value > contour_value);
	}

	template<class RW>
	void Read (std::istream& input_stream)
	{
		grid.template Read<RW> (input_stream);
		phi.template Read<RW> (input_stream);
	}

	template<class RW>
	void Write (std::ostream& output_stream) const
	{
		grid.template Write<RW> (output_stream);
		phi.template Write<RW> (output_stream);
	}

//#####################################################################
	void Use_Collision_Aware_Interpolation (COLLISION_BODY_LIST_3D<T>& body_list_input, ARRAYS_3D<bool>* valid_value_mask_input = 0, ARRAYS_3D<bool>* occupied_cell_input = 0);
	void Compute_Normals (const T time = 0);
	void Compute_Curvature (const T time = 0);
	void Compute_Cell_Minimum_And_Maximum (const bool recompute_if_exists = true);
	void Euler_Step (const ARRAYS_3D<T>& u, const ARRAYS_3D<T>& v, const ARRAYS_3D<T>& w, const T dt, const T time = 0);
	void Euler_Step (const ARRAYS_3D<VECTOR_3D<T> >& velocity, const T dt, const T time = 0);
	T CFL (const ARRAYS_3D<T>& u, const ARRAYS_3D<T>& v, const ARRAYS_3D<T>& w) const;
	T CFL (const ARRAYS_3D<VECTOR_3D<T> >& velocity) const;
	void Reinitialize (const int time_steps = 10, const T time = 0);
private:
	void Euler_Step_Of_Reinitialization (const ARRAYS_3D<T>& sign_phi, const T dt, const T time = 0);
public:
	void Fast_Marching_Method (const T time = 0, const T stopping_distance = 0, const LIST_ARRAY<VECTOR_3D<int> >* seed_indices = 0);
	void Get_Signed_Distance_Using_FMM (ARRAYS_3D<T>& signed_distance, const T time = 0, const T stopping_distance = 0, const LIST_ARRAY<VECTOR_3D<int> >* seed_indices = 0);
	void Fast_Marching_Method_Outside_Band (const T half_band_width, const T time = 0, const T stopping_distance = 0);
	void Fast_Sweeping_Method (const T time = 0, const T stopping_distance = 0, const LIST_ARRAY<VECTOR_3D<int> >* seed_indices = 0);
	void Get_Signed_Distance_Using_FSM (ARRAYS_3D<T>& signed_distance, const T time = 0, const T stopping_distance = 0, const LIST_ARRAY<VECTOR_3D<int> >* seed_indices = 0);
	void Fast_Sweeping_Method_Outside_Band (const T half_band_width, const T time = 0, const T stopping_distance = 0);
private:
	void Fast_Marching_Method (ARRAYS_3D<T>& phi_ghost, const T stopping_distance = 0, const LIST_ARRAY<VECTOR_3D<int> >* seed_indices = 0);
	void Fast_Sweeping_Method (ARRAYS_3D<T>& phi_ghost, const T stopping_distance = 0);
	void Solve_Quadratic_3D (ARRAYS_3D<T>& phi_ghost, ARRAYS_3D<bool>& done, int i, int j, int ij);
	void Update_Or_Add_Neighbor (ARRAYS_3D<T>& phi_ghost, ARRAYS_3D<bool>& done, ARRAYS_3D<int>& close_k, ARRAYS_1D<VECTOR_3D<int> >& heap, int& heap_length,
				     const VECTOR_3D<int>& neighbor);
	void Initialize_Interface (ARRAYS_3D<T>& phi, ARRAYS_3D<bool>& done, ARRAYS_3D<int>& close_k, ARRAYS_1D<VECTOR_3D<int> >& heap, int& heap_length, const LIST_ARRAY<VECTOR_3D<int> >* seed_indices = 0);
	void Initialize_Narrowband (ARRAYS_3D<T>& phi, const T stopping_distance);
	void Update_Close_Point (ARRAYS_3D<T>& phi, const ARRAYS_3D<bool>& done, const int i, const int j, const int ij);
public:
	void Add_To_Initial (ARRAYS_3D<bool>& done, ARRAYS_3D<int>& close_k, const int index, const int i, const int j, const int ij);
	T Approximate_Surface_Area (const T interface_thickness = 3, const T time = 0) const;
	T Approximate_Negative_Volume (const T interface_thickness = 3, const T time = 0) const;
	T Approximate_Positive_Volume (const T interface_thickness = 3, const T time = 0) const;
	void Calculate_Signed_Distance_Function (OCTREE_GRID<T>& octree_grid, ARRAY<T>& phi, bool output_statistics = false) const;
	void Calculate_Signed_Distance_Function_Based_On_Curvature (OCTREE_GRID<T>& octree_grid, ARRAYS_1D<T>& phi, const int levels, bool output_statistics = false);
private:
	bool Compare_Curvature_To_DX (OCTREE_GRID<T>& octree_grid, const OCTREE_CELL<T>* cell_input) const;
public:
	void Calculate_Triangulated_Surface_From_Marching_Tetrahedra (TRIANGULATED_SURFACE<T>& triangulated_surface, const bool include_ghost_values = false) const;
	void Calculate_Triangulated_Surface_From_Marching_Tetrahedra (const GRID_3D<T>& tet_grid, TRIANGULATED_SURFACE<T>& triangulated_surface) const;
private:
	int If_Zero_Crossing_Add_Particle_By_Index (TRIANGULATED_SURFACE<T>& triangulated_surface, const VECTOR_3D<int>& index1, const VECTOR_3D<int>& index2) const;
	int If_Zero_Crossing_Add_Particle (TRIANGULATED_SURFACE<T>& triangulated_surface, const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2) const;
	void Append_Triangles (TRIANGULATED_SURFACE<T>& triangulated_surface, const int e1, const int e2, const int e3, const int e4, const int e5, const int e6, const T phi1) const;
//#####################################################################
};
}
#endif


