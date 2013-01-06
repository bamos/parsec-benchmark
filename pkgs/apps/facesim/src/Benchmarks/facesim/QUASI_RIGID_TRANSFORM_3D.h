//#####################################################################
// Copyright 2004-2005, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class QUASI_RIGID_TRANSFORM_3D
//#####################################################################
#ifndef __QUASI_RIGID_TRANSFORM_3D__
#define __QUASI_RIGID_TRANSFORM_3D__

#include "../../Public_Library/Constitutive_Models/DIAGONALIZED_ISOTROPIC_STRESS_DERIVATIVE_3D.h"

namespace PhysBAM
{

template <class T>
class QUASI_RIGID_TRANSFORM_3D
{
public:
	MATRIX_3X3<T> affine_transform;
	VECTOR_3D<T> translation;

	QUASI_RIGID_TRANSFORM_3D()
		: affine_transform (MATRIX_3X3<T>::Identity_Matrix())
	{}

	QUASI_RIGID_TRANSFORM_3D (const MATRIX_3X3<T>& affine_transform_input, const VECTOR_3D<T>& translation_input)
		: affine_transform (affine_transform_input), translation (translation_input)
	{}

	T operator() (const int i) const
	{
		assert (1 <= i && i <= 12);
		return i <= 9 ? affine_transform.x[i - 1] : translation[i - 9];
	}

	T& operator() (const int i)
	{
		assert (1 <= i && i <= 12);
		return i <= 9 ? affine_transform.x[i - 1] : translation[i - 9];
	}

	T Rigidity_Penalty() const
	{
		SYMMETRIC_MATRIX_3X3<T> strain = affine_transform.Normal_Equations_Matrix() - 1;
		return strain.Frobenius_Norm_Squared();
	}

	T Rigidity_Penalty_Gradient (const int i) const
	{
		assert (1 <= i && i <= 12);

		if (i > 9) return 0;

		MATRIX_3X3<T> gradient = affine_transform * (affine_transform.Normal_Equations_Matrix() - 1);
		return (T) 4 * gradient.x[i - 1];
	}

	T Ridigity_Penalty_Hessian_Definite_Part (const int i, const int j) const
	{
		assert (1 <= i && i <= 12 && 1 <= j && j <= 12);

		if (i > 9 || j > 9) return 0;

		MATRIX_3X3<T> U, V;
		DIAGONAL_MATRIX_3X3<T> Sigma;
		affine_transform.Fast_Singular_Value_Decomposition (U, Sigma, V);
		DIAGONALIZED_ISOTROPIC_STRESS_DERIVATIVE_3D<T> rotated_hessian;
		rotated_hessian.x1111 = (T) 12 * sqr (Sigma.x11) - (T) 4;
		rotated_hessian.x2222 = (T) 12 * sqr (Sigma.x22) - (T) 4;
		rotated_hessian.x3333 = (T) 12 * sqr (Sigma.x33) - (T) 4;
		rotated_hessian.x2211 = 0;
		rotated_hessian.x3311 = 0;
		rotated_hessian.x3322 = 0;
		rotated_hessian.x2121 = (T) 4 * (sqr (Sigma.x11) + sqr (Sigma.x22) - (T) 1);
		rotated_hessian.x3131 = (T) 4 * (sqr (Sigma.x11) + sqr (Sigma.x33) - (T) 1);
		rotated_hessian.x3232 = (T) 4 * (sqr (Sigma.x22) + sqr (Sigma.x33) - (T) 1);
		rotated_hessian.x2112 = (T) 4 * Sigma.x11 * Sigma.x22;
		rotated_hessian.x3113 = (T) 4 * Sigma.x11 * Sigma.x33;
		rotated_hessian.x3223 = (T) 4 * Sigma.x22 * Sigma.x33;
		rotated_hessian.Fix_Indefinite_Blocks();
		MATRIX_3X3<T> dF1, dF2;
		dF1.x[i - 1] = (T) 1;
		dF2.x[j - 1] = (T) 1;
		dF1 = U.Transpose_Times (dF1) * V;
		dF2 = U.Transpose_Times (dF2) * V;
		return dF1.Transpose_Times (rotated_hessian.Differential (dF2)).Trace();
	}

	static QUASI_RIGID_TRANSFORM_3D<T> Incremental_Transform (const QUASI_RIGID_TRANSFORM_3D<T>& target_transform, const QUASI_RIGID_TRANSFORM_3D<T>& initial_transform)
	{
		MATRIX_3X3<T> affine_transform_incremental = target_transform.affine_transform * initial_transform.affine_transform.Inverse();
		VECTOR_3D<T> translation_incremental = target_transform.translation - affine_transform_incremental * initial_transform.translation;
		return QUASI_RIGID_TRANSFORM_3D<T> (affine_transform_incremental, translation_incremental);
	}

	static QUASI_RIGID_TRANSFORM_3D<T> Composite_Transform (const QUASI_RIGID_TRANSFORM_3D<T>& master_transform, const QUASI_RIGID_TRANSFORM_3D<T>& slave_transform)
	{
		MATRIX_3X3<T> affine_transform_composite = master_transform.affine_transform * slave_transform.affine_transform;
		VECTOR_3D<T> translation_composite = master_transform.affine_transform * slave_transform.translation + master_transform.translation;
		return QUASI_RIGID_TRANSFORM_3D<T> (affine_transform_composite, translation_composite);
	}

	FRAME<T> Frame()
	{
		MATRIX_3X3<T> U, V;
		DIAGONAL_MATRIX_3X3<T> Sigma;
		affine_transform.Fast_Singular_Value_Decomposition (U, Sigma, V);
		return FRAME<T> (translation, U.Multiply_With_Transpose (V));
	}

	static QUASI_RIGID_TRANSFORM_3D<T> Interpolate (const QUASI_RIGID_TRANSFORM_3D<T>& initial, const QUASI_RIGID_TRANSFORM_3D<T>& final, const T interpolation_fraction)
	{
		QUASI_RIGID_TRANSFORM_3D<T> result;
		result.translation = LINEAR_INTERPOLATION<T, VECTOR_3D<T> >::Linear (initial.translation, final.translation, interpolation_fraction);
		QUATERNION<T> initial_rotation (initial.affine_transform), final_rotation (final.affine_transform);
		result.affine_transform = QUATERNION<T>::Spherical_Linear_Interpolation (initial_rotation, final_rotation, interpolation_fraction).Matrix_3X3();
		//result.affine_transform=LINEAR_INTERPOLATION<T,MATRIX_3X3<T> >::Linear(initial.affine_transform,final.affine_transform,interpolation_fraction);result.Make_Rigid();
		return result;
	}

	void Make_Rigid()
	{
		MATRIX_3X3<T> U, V;
		DIAGONAL_MATRIX_3X3<T> Sigma;
		affine_transform.Fast_Singular_Value_Decomposition (U, Sigma, V);
		affine_transform = U.Multiply_With_Transpose (V);
	}

	void Print_Diagnostics (std::ostream& output = std::cout) const
	{
		output << "Transformation matrix :" << std::endl << affine_transform;
		MATRIX_3X3<T> U, V;
		DIAGONAL_MATRIX_3X3<T> Sigma;
		affine_transform.Fast_Singular_Value_Decomposition (U, Sigma, V);
		output << "Singular values : " << Sigma.x11 << " , " << Sigma.x22 << " , " << Sigma.x33 << std::endl;
		output << "Translation : " << translation << std::endl;
		output << "Rigidity penalty at current configuration : " << Rigidity_Penalty() << std::endl;
	}

//#####################################################################
};
}
#endif
