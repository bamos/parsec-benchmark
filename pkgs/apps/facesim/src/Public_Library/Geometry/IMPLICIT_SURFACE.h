//#####################################################################
// Copyright 2002, 2003, 2004, Ronald Fedkiw, Sergey Koltakov, Eran Guendelman, Neil Molino, Rachel Weinstein.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class IMPLICIT_SURFACE
//#####################################################################
#ifndef __IMPLICIT_SURFACE__
#define __IMPLICIT_SURFACE__

#include "BOX_3D.h"
#include "RAY_3D.h" // could replace with forward declaration but kept here to avoid breaking too much code
#include "../Interpolation/INTERPOLATION.h"
#include "../Matrices_And_Vectors/MATRIX_3X3.h"
#include "../Level_Sets/LEVELSET.h"
namespace PhysBAM
{

template<class T> class  TRIANGULATED_SURFACE;

template<class T>
class IMPLICIT_SURFACE
{
public:
	BOX_3D<T> box; // box containing the voxelized implicit surface
	T minimum_cell_size;
	bool levelset_data, octree_data;
	bool use_secondary_interpolation;

	IMPLICIT_SURFACE()
		: levelset_data (false), octree_data (false)
	{
		Use_Secondary_Interpolation (false);
	}

	virtual ~IMPLICIT_SURFACE()
	{}

	virtual void Destroy_Data() // this is dangerous
	{
		Default();
		exit (1);
	}

	void Use_Secondary_Interpolation (const bool use_secondary_interpolation_input = true)
	{
		use_secondary_interpolation = use_secondary_interpolation_input;
	}

	void Default() const
	{
		std::cout << "THIS IMPLICIT_SURFACE FUNCTION IS NOT DEFINED!" << std::endl;
	}

//#####################################################################
	virtual void Set_Custom_Interpolation (INTERPOLATION<T, T>& interpolation_input)
	{
		Default();
	}
	virtual void Set_Custom_Secondary_Interpolation (INTERPOLATION<T, T>& interpolation_input)
	{
		Default();
	}
	virtual void Set_Custom_Normal_Interpolation (INTERPOLATION<T, VECTOR_3D<T> >& interpolation_input)
	{
		Default();
	}
	virtual void Update_Box()
	{
		Default();
	}
	virtual void Update_Minimum_Cell_Size (const int maximum_depth = 0)
	{
		Default();
	}
	virtual T operator() (const VECTOR_3D<T>& location) const
	{
		Default();
		return 0;
	}
	virtual T Extended_Phi (const VECTOR_3D<T>& location) const
	{
		Default();
		return 0;
	}
	virtual T Phi_Secondary (const VECTOR_3D<T>& location) const
	{
		Default();
		return 0;
	}
	virtual VECTOR_3D<T> Normal (const VECTOR_3D<T>& location, const int aggregate = -1) const
	{
		Default();
		return VECTOR_3D<T>();
	}
	virtual VECTOR_3D<T> Extended_Normal (const VECTOR_3D<T>& location, const int aggregate = -1) const
	{
		Default();
		return VECTOR_3D<T>();
	}
	virtual MATRIX_3X3<T> Hessian (const VECTOR_3D<T>& X) const
	{
		Default();
		return MATRIX_3X3<T>();
	}
	virtual void Principle_Curvatures (const VECTOR_3D<T>& X, T& curvature1, T& curvature2) const
	{
		Default();
	}
	virtual void Compute_Normals()
	{
		Default();
	}
	virtual void Compute_Cell_Minimum_And_Maximum (const bool recompute_if_exists = true)
	{
		Default();
	}
	virtual void Rescale (const T scaling_factor)
	{
		Default();
	}
	virtual void Inflate (const T inflation_distance)
	{
		Default();
	}
	bool Intersection (RAY_3D<T>& ray, const T thickness = 0) const;
	virtual T Integration_Step (const T phi) const
	{
		Default();
		return 0;
	}
	bool Inside (const VECTOR_3D<T>& location, T thickness_over_two = 0) const;
	virtual bool Lazy_Inside (const VECTOR_3D<T>& location, const T contour_value = 0) const
	{
		Default();
		return false;
	}
	virtual bool Lazy_Inside_And_Value (const VECTOR_3D<T>& location, T& phi_value, const T contour_value = 0) const
	{
		Default();
		return false;
	}
	virtual bool Lazy_Inside_Extended_Levelset (const VECTOR_3D<T>& unclamped_X, const T contour_value = 0) const
	{
		Default();
		return false;
	}
	virtual bool Lazy_Inside_Extended_Levelset_And_Value (const VECTOR_3D<T>& unclamped_X, T& phi_value, const T contour_value = 0) const
	{
		Default();
		return false;
	}
	bool Outside (const VECTOR_3D<T>& location, T thickness_over_two = 0) const;
	virtual bool Lazy_Outside (const VECTOR_3D<T>& location, const T contour_value = 0) const
	{
		Default();
		return false;
	}
	virtual bool Lazy_Outside_Extended_Levelset (const VECTOR_3D<T>& unclamped_X, const T contour_value = 0) const
	{
		Default();
		return false;
	}
	virtual bool Lazy_Outside_Extended_Levelset_And_Value (const VECTOR_3D<T>& unclamped_X, T& phi_value, const T contour_value = 0) const
	{
		Default();
		return false;
	}
	bool Boundary (const VECTOR_3D<T>& location, T thickness_over_two = 0) const;
	VECTOR_3D<T> Surface (const VECTOR_3D<T>& location, T tolerance = 0, int max_iterations = 1) const;
	virtual T Min_Phi() const
	{
		Default();
		return 0;
	}
	virtual TRIANGULATED_SURFACE<T>* Generate_Triangles() const
	{
		Default();
		return 0;
	}
//#####################################################################
};
}
namespace PhysBAM
{
//#####################################################################
// Function Inside
//#####################################################################
template<class T> inline bool IMPLICIT_SURFACE<T>::
Inside (const VECTOR_3D<T>& location, const T thickness_over_two) const
{
	return box.Inside (location, thickness_over_two) && (*this) (location) <= -thickness_over_two;
}
//#####################################################################
// Function Outside
//#####################################################################
template<class T> inline bool IMPLICIT_SURFACE<T>::
Outside (const VECTOR_3D<T>& location, const T thickness_over_two) const
{
	return box.Outside (location, thickness_over_two) || (*this) (location) >= thickness_over_two;
}
//#####################################################################
// Function Boundary
//#####################################################################
template<class T> inline bool IMPLICIT_SURFACE<T>::
Boundary (const VECTOR_3D<T>& location, const T thickness_over_two) const
{
	return !Inside (location, thickness_over_two) && !Outside (location, thickness_over_two);
}
//#####################################################################
// Function Surface
//#####################################################################
template<class T> VECTOR_3D<T> IMPLICIT_SURFACE<T>::
Surface (const VECTOR_3D<T>& location, const T tolerance, const int max_iterations) const
{
	if (!tolerance) return location - (*this) (location) * Normal (location); // only take one iteration
	else
	{
		int iterations = 1;
		VECTOR_3D<T> new_location (location - (*this) (location) *Normal (location));

		while (iterations < max_iterations && fabs ( (*this) (new_location)) > tolerance)
		{
			iterations++;
			new_location -= (*this) (new_location) * Normal (new_location);
		}

		return new_location;
	}
}
//#####################################################################
}
#endif

