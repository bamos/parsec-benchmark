//#####################################################################
// Copyright 2002-2004, Ronald Fedkiw, Geoffrey Irving, Neil Molino, Igor Neverov, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class STRAIN_MEASURE_3D
//#####################################################################
#ifndef __STRAIN_MEASURE_3D__
#define __STRAIN_MEASURE_3D__

#include "../Geometry/TETRAHEDRALIZED_VOLUME.h"
#include "../Matrices_And_Vectors/MATRIX_3X3.h"
namespace PhysBAM
{

template<class T>
class STRAIN_MEASURE_3D
{
public:
	TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume;
	TETRAHEDRON_MESH& tetrahedron_mesh;
	LIST_ARRAY<UPPER_TRIANGULAR_MATRIX_3X3<T> > Dm_inverse;
private:
	LIST_ARRAY<UPPER_TRIANGULAR_MATRIX_3X3<T> >* Dm_inverse_save;
public:

	STRAIN_MEASURE_3D (TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume_input)
		: tetrahedralized_volume (tetrahedralized_volume_input), tetrahedron_mesh (tetrahedralized_volume.tetrahedron_mesh), Dm_inverse_save (0)
	{
		Initialize_Dm_Inverse_From_Current_Positions();
	}

	virtual ~STRAIN_MEASURE_3D()
	{
		delete Dm_inverse_save;
	}

	void Initialize_Dm_Inverse_From_Current_Positions()
	{
		Initialize_Dm_Inverse (tetrahedralized_volume.particles.X.array);
	}

	void Initialize_Dm_Inverse (const ARRAY<VECTOR_3D<T> >& X)
	{
		Dm_inverse.Resize_Array (tetrahedron_mesh.tetrahedrons.m);

		for (int i = 1; i <= tetrahedron_mesh.tetrahedrons.m; i++)
		{
			UPPER_TRIANGULAR_MATRIX_3X3<T> R = Ds (X, i).R_From_Gram_Schmidt_QR_Factorization();
			assert (R.Determinant() > 0);
			Dm_inverse (i) = R.Inverse();
		}
	}

	void Initialize_Dm_Inverse_Save()
	{
		Dm_inverse_save = new LIST_ARRAY<UPPER_TRIANGULAR_MATRIX_3X3<T> > (tetrahedron_mesh.tetrahedrons.m);
		LIST_ARRAY<UPPER_TRIANGULAR_MATRIX_3X3<T> >::copy (Dm_inverse, *Dm_inverse_save);
	}

	void Copy_Dm_Inverse_Save_Into_Dm_Inverse (const LIST_ARRAY<int>& map)
	{
		Dm_inverse.Resize_Array (map.m);

		for (int t = 1; t <= map.m; t++) Dm_inverse (t) = (*Dm_inverse_save) (map (t));
	}

	void Initialize_Rest_State_To_Equilateral_Tetrahedrons (const T side_length)
	{
		Dm_inverse.Resize_Array (tetrahedron_mesh.tetrahedrons.m);
		T x = (T) root_three / 3, d = (T).5 * x, h = (T) root_six / 3;
		MATRIX_3X3<T> Dm (x, 0, -h, -d, .5, -h, -d, -.5, -h);
		Dm *= side_length;
		LIST_ARRAY<UPPER_TRIANGULAR_MATRIX_3X3<T> >::copy (Dm.R_From_Gram_Schmidt_QR_Factorization().Inverse(), Dm_inverse);
	}

	MATRIX_3X3<T> F (const int tetrahedron_index) const
	{
		return Ds (tetrahedralized_volume.particles.X.array, tetrahedron_index) * Dm_inverse (tetrahedron_index);
	}

	MATRIX_3X3<T> dF (const ARRAY<VECTOR_3D<T> >& dX, const int tetrahedron_index) const
	{
		return Ds (dX, tetrahedron_index) * Dm_inverse (tetrahedron_index);
	}

	T J (const int tetrahedron_index) const
	{
		return Ds (tetrahedralized_volume.particles.X.array, tetrahedron_index).Determinant() * Dm_inverse (tetrahedron_index).Determinant();
	}

	MATRIX_3X3<T> Velocity_Gradient (const int tetrahedron_index) const
	{
		return Ds (tetrahedralized_volume.particles.V.array, tetrahedron_index) * Dm_inverse (tetrahedron_index);
	}

	MATRIX_3X3<T> Spatial_Velocity_Gradient (const int tetrahedron_index) const
	{
		return Ds (tetrahedralized_volume.particles.V.array, tetrahedron_index) * Ds (tetrahedralized_volume.particles.X.array, tetrahedron_index).Inverse();
	}

	T Minimum_Rest_Altitude() const
	{
		T altitude = FLT_MAX;

		for (int t = 1; t <= Dm_inverse.m; t++) altitude = min (altitude, Dm_inverse (t).Inverse().Tetrahedron_Minimum_Altitude());

		return altitude;
	}

	MATRIX_3X3<T> Ds (const ARRAY<VECTOR_3D<T> >& X, const int tetrahedron_index) const
	{
		int i, j, k, l;
		tetrahedron_mesh.tetrahedrons.Get (tetrahedron_index, i, j, k, l);
		return MATRIX_3X3<T> (X (j) - X (i), X (k) - X (i), X (l) - X (i));
	}

	static MATRIX_3X3<T> Ds (const ARRAY<VECTOR_3D<T> >& X, const int i, const int j, const int k, const int l)
	{
		return MATRIX_3X3<T> (X (j) - X (i), X (k) - X (i), X (l) - X (i));
	}

private:
	void Default() const
	{
		std::cout << "THIS STRAIN_MEASURE_3D FUNCTION IS NOT DEFINED!" << std::endl;
	}

//#####################################################################
public:
	virtual SYMMETRIC_MATRIX_3X3<T> Strain (const int tetrahedron_index) const
	{
		Default();
		exit (1);
	}
	virtual SYMMETRIC_MATRIX_3X3<T> Strain_Rate (const int tetrahedron_index) const
	{
		Default();
		exit (1);
	}
	virtual SYMMETRIC_MATRIX_3X3<T> Strain_Rate (const int tetrahedron_index, const MATRIX_3X3<T>& deformation_gradient) const
	{
		Default();
		exit (1);
	}
	void Print_Altitude_Statistics();
//#####################################################################
};
}
#endif
