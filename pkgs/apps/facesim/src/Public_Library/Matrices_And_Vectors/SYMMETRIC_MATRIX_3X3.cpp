//#####################################################################
// Copyright 2003-2004, Ronald Fedkiw, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SYMMETRIC_MATRIX_3X3
//#####################################################################
#include "SYMMETRIC_MATRIX_3X3.h"
#include "../Math_Tools/max.h"
#include "../Math_Tools/abs.h"
#include "../Math_Tools/exchange_sort.h"
#include "../Utilities/DEBUG_UTILITIES.h"
using namespace PhysBAM;
//#####################################################################
// Function Fast_Eigenvalues_Double
//#####################################################################
// lambda_x > lambda_y > lambda_z
template<class T> DIAGONAL_MATRIX_3X3<double> SYMMETRIC_MATRIX_3X3<T>::
Fast_Eigenvalues_Double() const
{
	double m = one_third * ( (double) x11 + x22 + x33);
	double a11 = x11 - m, a22 = x22 - m, a33 = x33 - m, a12_sqr = (double) x21 * x21, a13_sqr = (double) x31 * x31, a23_sqr = (double) x32 * x32;
	double p = one_sixth * (a11 * a11 + a22 * a22 + a33 * a33 + 2 * (a12_sqr + a13_sqr + a23_sqr));
	double q = .5 * (a11 * (a22 * a33 - a23_sqr) - a22 * a13_sqr - a33 * a12_sqr) + (double) x21 * x31 * x32;
	double sqrt_p = sqrt (p), disc = p * p * p - q * q;
	double phi = one_third * atan2 (sqrt (max ( (double) 0, disc)), q), c = cos (phi), s = sin (phi);
	double sqrt_p_cos = sqrt_p * c, root_three_sqrt_p_sin = root_three * sqrt_p * s;
	DIAGONAL_MATRIX_3X3<double> lambda (m + 2 * sqrt_p_cos, m - sqrt_p_cos - root_three_sqrt_p_sin, m - sqrt_p_cos + root_three_sqrt_p_sin);
	exchange_sort (lambda.x33, lambda.x22, lambda.x11);
	return lambda;
}
//#####################################################################
// Function Fast_Eigenvectors_Double
//#####################################################################
template<class T> void SYMMETRIC_MATRIX_3X3<T>::
Fast_Eigenvectors_Double (const DIAGONAL_MATRIX_3X3<double>& lambda, MATRIX_3X3<double>& eigenvectors, const double tolerance) const
{
	double tiny = tolerance * max (abs (lambda.x11), abs (lambda.x33));
	SYMMETRIC_MATRIX_3X3<double> A (*this);

	if (lambda.x11 - lambda.x22 <= tiny)
	{
		if (lambda.x22 - lambda.x33 <= tiny) eigenvectors = MATRIX_3X3<double>::Identity_Matrix();
		else
		{
			VECTOR_3D<double> v3 = (A - lambda.x33).Cofactor_Matrix().Largest_Column_Normalized();
			VECTOR_3D<double> v2 = v3.Orthogonal_Vector().Normalized();
			eigenvectors = MATRIX_3X3<double> (VECTOR_3D<double>::Cross_Product (v2, v3), v2, v3);
		}
	}
	else if (lambda.x22 - lambda.x33 <= tiny)
	{
		VECTOR_3D<double> v1 = (A - lambda.x11).Cofactor_Matrix().Largest_Column_Normalized();
		VECTOR_3D<double> v2 = v1.Orthogonal_Vector().Normalized();
		eigenvectors = MATRIX_3X3<double> (v1, v2, VECTOR_3D<double>::Cross_Product (v1, v2));
	}
	else
	{
		VECTOR_3D<double> v1 = (A - lambda.x11).Cofactor_Matrix().Largest_Column_Normalized();
		VECTOR_3D<double> v3 = (A - lambda.x33).Cofactor_Matrix().Largest_Column_Normalized();
		eigenvectors = MATRIX_3X3<double> (v1, VECTOR_3D<double>::Cross_Product (v3, v1), v3);
	}
}
//#####################################################################
// Function Fast_Solve_Eigenproblem_Double
//#####################################################################
template<class T> void SYMMETRIC_MATRIX_3X3<T>::
Fast_Solve_Eigenproblem_Double (DIAGONAL_MATRIX_3X3<double>& eigenvalues, MATRIX_3X3<double>& eigenvectors, const double tolerance) const
{
	eigenvalues = Fast_Eigenvalues_Double();
	Fast_Eigenvectors_Double (eigenvalues, eigenvectors, tolerance);
}
//#####################################################################
// Function Solve_Eigenproblem
//####################################################################
template<class T> void SYMMETRIC_MATRIX_3X3<T>::
Solve_Eigenproblem (DIAGONAL_MATRIX_3X3<T>& eigenvalues, MATRIX_3X3<T>& eigenvectors) const
{
	eigenvectors = MATRIX_3X3<T>::Identity_Matrix();
	T a11 = x11, a12 = x21, a13 = x31, a22 = x22, a23 = x32, a33 = x33;
	T v11 = 1, v12 = 0, v13 = 0, v21 = 0, v22 = 1, v23 = 0, v31 = 0, v32 = 0, v33 = 1;
	int sweep;

	for (sweep = 1; sweep <= 50; sweep++)
	{
		T sum = (T) fabs (a12) + (T) fabs (a13) + (T) fabs (a23);

		if (sum == 0) break;

		T threshold = sweep < 4 ? (T) (1. / 45) * sum : 0;
		Jacobi_Transform (sweep, threshold, a11, a12, a22, a13, a23, v11, v12, v21, v22, v31, v32);
		Jacobi_Transform (sweep, threshold, a11, a13, a33, a12, a23, v11, v13, v21, v23, v31, v33);
		Jacobi_Transform (sweep, threshold, a22, a23, a33, a12, a13, v12, v13, v22, v23, v32, v33);
	}

	assert (sweep <= 50);
	eigenvalues = DIAGONAL_MATRIX_3X3<T> (a11, a22, a33);
	eigenvectors = MATRIX_3X3<T> (v11, v21, v31, v12, v22, v32, v13, v23, v33);
}
//#####################################################################
// Function Jacobi_Transform
//#####################################################################
template<class T> inline void SYMMETRIC_MATRIX_3X3<T>::
Jacobi_Transform (const int sweep, const T threshold, T& app, T& apq, T& aqq, T& arp, T& arq, T& v1p, T& v1q, T& v2p, T& v2q, T& v3p, T& v3q)
{
	T g = 100 * apq;

	if (sweep > 4 && app + g == app && aqq + g == aqq) apq = 0;
	else if (fabs (apq) > threshold)
	{
		T h = aqq - app, t;

		if (h + g == h) t = apq / h;
		else
		{
			T theta = (T).5 * h / apq;
			t = 1 / (fabs (theta) + sqrt (1 + sqr (theta)));

			if (theta < 0) t = -t;
		}

		T cosine = 1 / sqrt (1 + t * t), sine = t * cosine, tau = sine / (1 + cosine), da = t * apq;
		app -= da;
		aqq += da;
		apq = 0;
		T arp_tmp = arp - sine * (arq + tau * arp);
		arq = arq + sine * (arp - tau * arq);
		arp = arp_tmp;
		T v1p_tmp = v1p - sine * (v1q + tau * v1p);
		v1q = v1q + sine * (v1p - tau * v1q);
		v1p = v1p_tmp;
		T v2p_tmp = v2p - sine * (v2q + tau * v2p);
		v2q = v2q + sine * (v2p - tau * v2q);
		v2p = v2p_tmp;
		T v3p_tmp = v3p - sine * (v3q + tau * v3p);
		v3q = v3q + sine * (v3p - tau * v3q);
		v3p = v3p_tmp;
	}
}
//#####################################################################
template class SYMMETRIC_MATRIX_3X3<float>;
template class SYMMETRIC_MATRIX_3X3<double>;
