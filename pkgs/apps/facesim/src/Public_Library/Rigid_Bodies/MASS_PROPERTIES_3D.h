//#####################################################################
// Copyright 2003-2004, Ron Fedkiw, Eran Guendelman, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class MASS_PROPERTIES_3D
//#####################################################################
//
// Based on Brian Mirtich's volInt.c implementation (copyright 1995) which is based on "Fast and Accurate Computation of Polyhedral Mass Properties" by Mirtich.
//
//#####################################################################
#ifndef __MASS_PROPERTIES_3D__
#define __MASS_PROPERTIES_3D__

#include "../Matrices_And_Vectors/VECTOR_3D.h"
#include "../Matrices_And_Vectors/QUATERNION.h"
#include "../Particles/SOLIDS_PARTICLE.h"
namespace PhysBAM
{

template<class T> class TRIANGULATED_SURFACE;
template<class T> class TRIANGLE_3D;
template<class T> class SYMMETRIC_MATRIX_3X3;

template<class T>
class MASS_PROPERTIES_3D
{
private:
	TRIANGULATED_SURFACE<T>& triangulated_surface;
	mutable T mass, density;
	bool given_mass; // either mass or density is prescribed
	int A, B, C; // alpha, beta, gamma (axis permutation)
	T P1, Pa, Pb, Paa, Pab, Pbb, Paaa, Paab, Pabb, Pbbb; // projection integrals
	T Fa, Fb, Fc, Faa, Fbb, Fcc, Fab, Fbc, Fca, Faaa, Fbbb, Fccc, Faab, Fbbc, Fcca; // face integrals
	T volume; // surface area if thin shell
	VECTOR_3D<T> T1, T2, TP; // volume integrals

public:
	MASS_PROPERTIES_3D (TRIANGULATED_SURFACE<T>& triangulated_surface_input, const bool thin_shell = false)
		: triangulated_surface (triangulated_surface_input)
	{
		if (!triangulated_surface.triangle_list) triangulated_surface.Update_Triangle_List();

		Set_Density();
		Set_Mass(); // leaves the given_mass=true

		if (thin_shell) Compute_Thin_Shell_Integrals();
		else Compute_Volume_Integrals();
	}

	void Set_Mass (T mass_input = 1)
	{
		mass = mass_input;
		given_mass = true;
	}

	void Set_Density (T density_input = 1)
	{
		density = density_input;
		given_mass = false;
	}

	T Get_Volume() const
	{
		return volume;
	}

	T Get_Surface_Area() const
	{
		return volume;
	}

//#####################################################################
public:
	void Get_Center_Of_Mass_And_Inertia_Tensor (VECTOR_3D<T>& center_of_mass, SYMMETRIC_MATRIX_3X3<T>& inertia_tensor) const;
	void Transform_To_Object_Frame (VECTOR_3D<T>& center_of_mass, QUATERNION<T>& orientation, DIAGONAL_MATRIX_3X3<T>& moment_of_inertia, SOLIDS_PARTICLE<T, VECTOR_3D<T> >& particles) const;
private:
	void Compute_Volume_Integrals();
	void Compute_Thin_Shell_Integrals();
	void Compute_Face_Integrals (const TRIANGLE_3D<T>& triangle);
	void Compute_Projection_Integrals (const TRIANGLE_3D<T>& triangle);
//#####################################################################
};
}
#endif
