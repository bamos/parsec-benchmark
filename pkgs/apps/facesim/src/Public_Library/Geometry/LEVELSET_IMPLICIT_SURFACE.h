//#####################################################################
// Copyright 2002, 2003, 2004, Doug Enright, Ron Fedkiw, Eran Guendelman, Geoffrey Irving, Sergey Koltakov, Neil Molino, Rachel Weinstein
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class LEVELSET_IMPLICIT_SURFACE
//#####################################################################
#ifndef __LEVELSET_IMPLICIT_SURFACE__
#define __LEVELSET_IMPLICIT_SURFACE__

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "../Level_Sets/LEVELSET_3D.h"
#include "IMPLICIT_SURFACE.h"
#include "TRIANGULATED_SURFACE.h"
namespace PhysBAM
{

template<class T>
class LEVELSET_IMPLICIT_SURFACE: public IMPLICIT_SURFACE<T>
{
public:
	using IMPLICIT_SURFACE<T>::levelset_data;
	using IMPLICIT_SURFACE<T>::octree_data;
	using IMPLICIT_SURFACE<T>::box;
	using IMPLICIT_SURFACE<T>::minimum_cell_size;

	LEVELSET_3D<T> levelset;

	LEVELSET_IMPLICIT_SURFACE (GRID_3D<T>& grid_input, ARRAYS_3D<T>& phi_input)
		: levelset (grid_input, phi_input)
	{
		levelset_data = true;
		octree_data = false;
		Update_Box();
		Update_Minimum_Cell_Size();
	}

	~LEVELSET_IMPLICIT_SURFACE()
	{}

	static LEVELSET_IMPLICIT_SURFACE* Create()
	{
		return new LEVELSET_IMPLICIT_SURFACE (* (new GRID_3D<T>), * (new ARRAYS_3D<T>));
	}

	void Destroy_Data() // this is dangerous
	{
		delete &levelset.grid;
		delete &levelset.phi;
	}

	void Set_Custom_Interpolation (INTERPOLATION<T, T>& interpolation_input)
	{
		levelset.Set_Custom_Interpolation (interpolation_input);
	}

	void Set_Custom_Secondary_Interpolation (INTERPOLATION<T, T>& interpolation_input)
	{
		levelset.Set_Custom_Secondary_Interpolation (interpolation_input);
	}

	void Set_Custom_Normal_Interpolation (INTERPOLATION<T, VECTOR_3D<T> >& interpolation_input)
	{
		levelset.Set_Custom_Normal_Interpolation (interpolation_input);
	}

	void Update_Box()
	{
		box.xmin = levelset.grid.xmin;
		box.xmax = levelset.grid.xmax;
		box.ymin = levelset.grid.ymin;
		box.ymax = levelset.grid.ymax;
		box.zmin = levelset.grid.zmin;
		box.zmax = levelset.grid.zmax;
	}

	void Update_Minimum_Cell_Size (const int maximum_depth = 0)
	{
		minimum_cell_size = levelset.grid.min_dx_dy_dz;
	}

	T operator() (const VECTOR_3D<T>& location) const
	{
		return levelset.Phi (location);
	}

	T Extended_Phi (const VECTOR_3D<T>& location) const
	{
		return levelset.Extended_Phi (location);
	}

	T Phi_Secondary (const VECTOR_3D<T>& location) const
	{
		return levelset.Phi_Secondary (location);
	}

	VECTOR_3D<T> Normal (const VECTOR_3D<T>& location, const int aggregate = -1) const
	{
		assert ( (aggregate >= 1 && aggregate <= 6) || aggregate == -1);

		if (aggregate != -1) return box.Normal (aggregate);
		else return levelset.Normal (location);
	}

	VECTOR_3D<T> Extended_Normal (const VECTOR_3D<T>& location, const int aggregate = -1) const
	{
		assert ( (aggregate >= 1 && aggregate <= 6) || aggregate == -1);

		if (aggregate != -1) return box.Normal (aggregate);
		else return levelset.Extended_Normal (location);
	}

	MATRIX_3X3<T> Hessian (const VECTOR_3D<T>& X) const
	{
		return levelset.Hessian (X);
	}

	void Principle_Curvatures (const VECTOR_3D<T>& X, T& curvature1, T& curvature2) const
	{
		levelset.Principle_Curvatures (X, curvature1, curvature2);
	}

	void Compute_Normals()
	{
		levelset.Compute_Normals();
	}

	void Compute_Cell_Minimum_And_Maximum (const bool recompute_if_exists = true)
	{
		levelset.Compute_Cell_Minimum_And_Maximum (recompute_if_exists);
	}

	void Rescale (const T scaling_factor)
	{
		levelset.grid.Initialize (levelset.grid.m, levelset.grid.n, levelset.grid.mn, scaling_factor * levelset.grid.xmin, scaling_factor * levelset.grid.xmax,
					  scaling_factor * levelset.grid.ymin, scaling_factor * levelset.grid.ymax, scaling_factor * levelset.grid.zmin, scaling_factor * levelset.grid.zmax);
		levelset.phi *= scaling_factor;
		Update_Box();
		Update_Minimum_Cell_Size();
	}

	void Translate (const VECTOR_3D<T>& translation)
	{
		levelset.grid.Initialize (levelset.grid.m, levelset.grid.n, levelset.grid.mn, levelset.grid.xmin + translation.x, levelset.grid.xmax + translation.x, levelset.grid.ymin + translation.y,
					  levelset.grid.ymax + translation.y, levelset.grid.zmin + translation.z, levelset.grid.zmax + translation.z);
		Update_Box();
		Update_Minimum_Cell_Size();
	}

	void Inflate (const T inflation_distance)
	{
		levelset.phi -= inflation_distance;
	}

	T Integration_Step (const T phi) const
	{
		T distance = fabs (phi);

		if (distance > 3 * minimum_cell_size) return (T).5 * distance;
		else if (distance > minimum_cell_size) return (T).25 * distance;
		else return (T).1 * minimum_cell_size;
	}

	bool Lazy_Inside (const VECTOR_3D<T>& location, const T contour_value = 0) const
	{
		return box.Lazy_Inside (location) && levelset.Lazy_Inside (location, contour_value);
	}

	bool Lazy_Inside_And_Value (const VECTOR_3D<T>& location, T& phi_value, const T contour_value = 0) const
	{
		return box.Lazy_Inside (location) && levelset.Lazy_Inside_And_Value (location, phi_value, contour_value);
	}

	bool Lazy_Inside_Extended_Levelset (const VECTOR_3D<T>& unclamped_X, const T contour_value = 0) const
	{
		return levelset.Lazy_Inside_Extended_Levelset (unclamped_X, contour_value);
	}

	bool Lazy_Inside_Extended_Levelset_And_Value (const VECTOR_3D<T>& unclamped_X, T& phi_value, const T contour_value = 0) const
	{
		return levelset.Lazy_Inside_Extended_Levelset_And_Value (unclamped_X, phi_value, contour_value);
	}

	bool Lazy_Outside (const VECTOR_3D<T>& location, const T contour_value = 0) const
	{
		return box.Lazy_Outside (location) || levelset.Lazy_Outside (location, contour_value);
	}

	bool Lazy_Outside_Extended_Levelset (const VECTOR_3D<T>& unclamped_X, const T contour_value = 0) const
	{
		return levelset.Lazy_Outside_Extended_Levelset (unclamped_X, contour_value);
	}

	bool Lazy_Outside_Extended_Levelset_And_Value (const VECTOR_3D<T>& unclamped_X, T& phi_value, const T contour_value = 0) const
	{
		return levelset.Lazy_Outside_Extended_Levelset_And_Value (unclamped_X, phi_value, contour_value);
	}

	virtual T Min_Phi() const
	{
		return ARRAYS_3D<T>::min (levelset.phi);
	}

	TRIANGULATED_SURFACE<T>* Generate_Triangles() const
	{
		TRIANGULATED_SURFACE<T>* triangulated_surface = TRIANGULATED_SURFACE<T>::Create();
		levelset.Calculate_Triangulated_Surface_From_Marching_Tetrahedra (*triangulated_surface);
		return triangulated_surface;
	}

	template<class RW>
	void Read (std::istream& input_stream)
	{
		levelset.template Read<RW> (input_stream);
		levelset_data = true;
		octree_data = false;
		Update_Box();
		Update_Minimum_Cell_Size();
	}

	template<class RW>
	void Write (std::ostream& output_stream) const
	{
		levelset.template Write<RW> (output_stream);
	}

//###########################################################################
};

}
#endif
