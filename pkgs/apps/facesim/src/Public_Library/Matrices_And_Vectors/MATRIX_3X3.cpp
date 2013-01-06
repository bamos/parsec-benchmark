//#####################################################################
// Copyright 2002-2004, Silvia Salinas-Blemker, Robert Bridson, Ronald Fedkiw, Eran Guendelman, Geoffrey Irving, Neil Molino, Igor Neverov, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class MATRIX_3X3
//#####################################################################
#include "MATRIX_3X3.h"
#include "../Utilities/DEBUG_UTILITIES.h"
using namespace PhysBAM;
//#####################################################################
// Function Fast_Singular_Value_Decomposition_Double
//#####################################################################
// U and V rotations, smallest singular value possibly negative
template<class T> void MATRIX_3X3<T>::
Fast_Singular_Value_Decomposition_Double (MATRIX_3X3<double>& U, DIAGONAL_MATRIX_3X3<double>& singular_values, MATRIX_3X3<double>& V, const double tolerance) const
{
	DIAGONAL_MATRIX_3X3<double> lambda;
	MATRIX_3X3<double> A (*this);
	A.Normal_Equations_Matrix().Fast_Solve_Eigenproblem (lambda, V);
	double tiny = tolerance * maxabs (lambda.x11, lambda.x33);

	if (lambda.x33 > tiny)
	{
		singular_values = DIAGONAL_MATRIX_3X3<double> (sqrt (lambda.x11), sqrt (lambda.x22), sqrt (lambda.x33));
		U.Column (1) = A * (V.Column (1) / singular_values.x11);
		U.Column (2) = A * (V.Column (2) / singular_values.x22);
		U.Column (3) = VECTOR_3D<double>::Cross_Product (U.Column (1), U.Column (2));

		if (A.Determinant() < 0) singular_values.x33 = -singular_values.x33;
	}
	else if (lambda.x22 > tiny)
	{
		singular_values = DIAGONAL_MATRIX_3X3<double> (sqrt (lambda.x11), sqrt (lambda.x22), 0);
		U.Column (1) = A * (V.Column (1) / singular_values.x11);
		U.Column (2) = A * (V.Column (2) / singular_values.x22);
		U.Column (3) = VECTOR_3D<double>::Cross_Product (U.Column (1), U.Column (2));
	}
	else if (lambda.x11 > tiny)
	{
		singular_values = DIAGONAL_MATRIX_3X3<double> (sqrt (lambda.x11), 0, 0);
		U.Column (1) = A * (V.Column (1) / singular_values.x11);
		U.Column (2) = U.Column (1).Orthogonal_Vector().Normalized();
		U.Column (3) = VECTOR_3D<double>::Cross_Product (U.Column (1), U.Column (2));
	}
	else
	{
		singular_values = DIAGONAL_MATRIX_3X3<double>();
		U = MATRIX_3X3<double>::Identity_Matrix();
	}
}
//#####################################################################
// Function Tetrahedron_Minimum_Altitude
//#####################################################################
template<class T> T MATRIX_3X3<T>::
Tetrahedron_Minimum_Altitude() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
template class MATRIX_3X3<float>;
template class MATRIX_3X3<double>;
