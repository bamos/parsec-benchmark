//#####################################################################
// Copyright 2003-2004, Zhaosheng Bao, Ronald Fedkiw, Geoffrey Irving, Neil Molino, Duc Nguyen, Eftychios Sifakis, Joseph Teran, Rachel Weinstein.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class MATRIX_2X2
//#####################################################################
#ifndef __MATRIX_2X2__
#define __MATRIX_2X2__

#include <assert.h>
#include "VECTOR_2D.h"
#include "VECTOR_3D.h"
#include "SYMMETRIC_MATRIX_2X2.h"
#include "../Math_Tools/sqr.h"
#include "../Math_Tools/constants.h"
#include "../Math_Tools/exchange.h"
namespace PhysBAM
{

template<class T>
class MATRIX_2X2
{
public:
	T x[4];

	MATRIX_2X2()
	{
		for (int i = 0; i < 4; i++) x[i] = 0;
	}

	MATRIX_2X2 (const MATRIX_2X2<T>& matrix_input)
	{
		for (int i = 0; i < 4; i++) x[i] = matrix_input.x[i];
	}

	MATRIX_2X2 (const SYMMETRIC_MATRIX_2X2<T>& matrix_input)
	{
		x[0] = matrix_input.x11;
		x[1] = x[2] = matrix_input.x21;
		x[3] = matrix_input.x22;
	}

	MATRIX_2X2 (const UPPER_TRIANGULAR_MATRIX_2X2<T>& matrix_input)
	{
		x[0] = matrix_input.x11;
		x[2] = matrix_input.x12;
		x[3] = matrix_input.x22;
		x[1] = 0;
	}

	MATRIX_2X2 (const T x11, const T x21, const T x12, const T x22)
	{
		x[0] = x11;
		x[1] = x21;
		x[2] = x12;
		x[3] = x22;
	}

	MATRIX_2X2 (const VECTOR_2D<T> & column1, const VECTOR_2D<T> & column2)
	{
		x[0] = column1.x;
		x[1] = column1.y;
		x[2] = column2.x;
		x[3] = column2.y;
	}

	T& operator() (const int i, const int j)
	{
		assert (i >= 1 && i <= 2);
		assert (j >= 1 && j <= 2);
		return x[i - 1 + 2 * (j - 1)];
	}

	const T& operator() (const int i, const int j) const
	{
		assert (i >= 1 && i <= 2);
		assert (j >= 1 && j <= 2);
		return x[i - 1 + 2 * (j - 1)];
	}

	VECTOR_2D<T>& Column (const int j)
	{
		assert (1 <= j && j <= 2);
		return * (VECTOR_2D<T>*) (x + 2 * (j - 1));
	}

	const VECTOR_2D<T>& Column (const int j) const
	{
		assert (1 <= j && j <= 2);
		return * (VECTOR_2D<T>*) (x + 2 * (j - 1));
	}

	MATRIX_2X2<T> operator-() const
	{
		return MATRIX_2X2<T> (-x[0], -x[1], -x[2], -x[3]);
	}

	MATRIX_2X2<T>& operator+= (const MATRIX_2X2<T>& A)
	{
		for (int i = 0; i < 4; i++) x[i] += A.x[i];

		return *this;
	}

	MATRIX_2X2<T>& operator+= (const T a)
	{
		x[0] += a;
		x[3] += a;
		return *this;
	}

	MATRIX_2X2<T>& operator-= (const MATRIX_2X2<T>& A)
	{
		for (int i = 0; i < 4; i++) x[i] -= A.x[i];

		return *this;
	}

	MATRIX_2X2<T>& operator-= (const T a)
	{
		x[0] -= a;
		x[3] -= a;
		return *this;
	}

	MATRIX_2X2<T>& operator*= (const MATRIX_2X2<T>& A)
	{
		return *this = *this * A;
	}

	MATRIX_2X2<T>& operator*= (const T a)
	{
		for (int i = 0; i < 4; i++) x[i] *= a;

		return *this;
	}

	MATRIX_2X2<T>& operator/= (const T a)
	{
		assert (a != 0);
		T s = 1 / a;

		for (int i = 0; i < 4; i++) x[i] *= s;

		return *this;
	}

	MATRIX_2X2<T> operator+ (const MATRIX_2X2<T>& A) const
	{
		return MATRIX_2X2<T> (x[0] + A.x[0], x[1] + A.x[1], x[2] + A.x[2], x[3] + A.x[3]);
	}

	MATRIX_2X2<T> operator+ (const T a) const
	{
		return MATRIX_2X2<T> (x[0] + a, x[1], x[2], x[3] + a);
	}

	MATRIX_2X2<T> operator- (const MATRIX_2X2<T>& A) const
	{
		return MATRIX_2X2<T> (x[0] - A.x[0], x[1] - A.x[1], x[2] - A.x[2], x[3] - A.x[3]);
	}

	MATRIX_2X2<T> operator- (const T a) const
	{
		return MATRIX_2X2<T> (x[0] - a, x[1], x[2], x[3] - a);
	}

	MATRIX_2X2<T> operator* (const MATRIX_2X2<T>& A) const
	{
		return MATRIX_2X2<T> (x[0] * A.x[0] + x[2] * A.x[1], x[1] * A.x[0] + x[3] * A.x[1], x[0] * A.x[2] + x[2] * A.x[3], x[1] * A.x[2] + x[3] * A.x[3]);
	}

	MATRIX_2X2<T> operator* (const T a) const
	{
		return MATRIX_2X2<T> (a * x[0], a * x[1], a * x[2], a * x[3]);
	}

	MATRIX_2X2<T> operator/ (const T a) const
	{
		assert (a != 0);
		T s = 1 / a;
		return MATRIX_2X2<T> (s * x[0], s * x[1], s * x[2], s * x[3]);
	}

	VECTOR_2D<T> operator* (const VECTOR_2D<T> & v) const
	{
		return VECTOR_2D<T> (x[0] * v.x + x[2] * v.y, x[1] * v.x + x[3] * v.y);
	}

	T Determinant() const
	{
		return x[0] * x[3] - x[1] * x[2];
	}

	void Invert()
	{
		*this = Inverse();
	}

	MATRIX_2X2<T> Inverse() const
	{
		T one_over_determinant = 1 / (x[0] * x[3] - x[1] * x[2]);
		return MATRIX_2X2<T> (one_over_determinant * x[3], -one_over_determinant * x[1], -one_over_determinant * x[2], one_over_determinant * x[0]);
	}

	MATRIX_2X2<T> Inverse_Transpose() const
	{
		return Inverse().Transposed();
	}

	VECTOR_2D<T> Solve_Linear_System (const VECTOR_2D<T>& b) const
	{
		T one_over_determinant = 1 / (x[0] * x[3] - x[1] * x[2]);
		return one_over_determinant * VECTOR_2D<T> (x[3] * b.x - x[2] * b.y, x[0] * b.y - x[1] * b.x);
	}

	void Transpose()
	{
		exchange (x[1], x[2]);
	}

	MATRIX_2X2<T> Transposed() const
	{
		return MATRIX_2X2<T> (x[0], x[2], x[1], x[3]);
	}

	T Trace() const
	{
		return x[0] + x[3];
	}

	MATRIX_2X2<T> Cofactor_Matrix() const
	{
		return MATRIX_2X2<T> (x[3], -x[2], -x[1], x[0]);
	}

	SYMMETRIC_MATRIX_2X2<T> Normal_Equations_Matrix() const
	{
		return SYMMETRIC_MATRIX_2X2<T> (x[0] * x[0] + x[1] * x[1], x[0] * x[2] + x[1] * x[3], x[2] * x[2] + x[3] * x[3]);
	}

	SYMMETRIC_MATRIX_2X2<T> Outer_Product_Matrix() const
	{
		return SYMMETRIC_MATRIX_2X2<T> (x[0] * x[0] + x[2] * x[2], x[0] * x[1] + x[2] * x[3], x[1] * x[1] + x[3] * x[3]);
	}

	SYMMETRIC_MATRIX_2X2<T> Symmetric_Part() const
	{
		return SYMMETRIC_MATRIX_2X2<T> (x[0], (T).5 * (x[1] + x[2]), x[3]);
	}

	SYMMETRIC_MATRIX_2X2<T> Twice_Symmetric_Part() const
	{
		return SYMMETRIC_MATRIX_2X2<T> (2 * x[0], x[1] + x[2], 2 * x[3]);
	}

	DIAGONAL_MATRIX_2X2<T> Diagonal_Part() const
	{
		return DIAGONAL_MATRIX_2X2<T> (x[0], x[3]);
	}

	static MATRIX_2X2<T> Transpose (const MATRIX_2X2<T>& A)
	{
		return MATRIX_2X2<T> (A.x[0], A.x[2], A.x[1], A.x[3]);
	}

	static MATRIX_2X2<T> Identity_Matrix()
	{
		return MATRIX_2X2<T> (1, 0, 0, 1);
	}

	static MATRIX_2X2<T> Diagonal_Matrix (const T d)
	{
		return MATRIX_2X2<T> (d, 0, 0, d);
	}

	T Antisymmetric_Part_Cross_Product_Vector_As_Scalar() const
	{
		return (T).5 * (x[1] - x[2]);
	}

	VECTOR_3D<T> Antisymmetric_Part_Cross_Product_Vector() const
	{
		return VECTOR_3D<T> (0, 0, (T).5 * (x[1] - x[2]));
	}

	static MATRIX_2X2<T> Rotation_Matrix (const T radians)
	{
		T c = cos (radians), s = sin (radians);
		return MATRIX_2X2<T> (c, s, -s, c);
	}

	static MATRIX_2X2<T> Derivative_Rotation_Matrix (const T radians)
	{
		T c = cos (radians), s = sin (radians);
		return MATRIX_2X2<T> (-s, c, -c, -s);
	}

	static MATRIX_2X2<T> Outer_Product (const VECTOR_2D<T>& u, const VECTOR_2D<T>& v)
	{
		return MATRIX_2X2<T> (u.x * v.x, u.y * v.x, u.x * v.y, u.y * v.y);
	}

	T Frobenius_Norm() const
	{
		return sqrt (sqr (x[0]) + sqr (x[1]) + sqr (x[2]) + sqr (x[3]));
	}

	T Max_Abs_Element() const
	{
		return max (fabs (x[0]), fabs (x[1]), fabs (x[2]), fabs (x[3]));
	}

	MATRIX_2X2<T> operator* (const DIAGONAL_MATRIX_2X2<T>& A) const
	{
		return MATRIX_2X2<T> (x[0] * A.x11, x[1] * A.x11, x[2] * A.x22, x[3] * A.x22);
	}

	MATRIX_2X2<T> operator* (const UPPER_TRIANGULAR_MATRIX_2X2<T>& A) const
	{
		return MATRIX_2X2<T> (x[0] * A.x11, x[1] * A.x11, x[0] * A.x12 + x[2] * A.x22, x[1] * A.x12 + x[3] * A.x22);
	}

	MATRIX_2X2<T> operator* (const SYMMETRIC_MATRIX_2X2<T>& A) const
	{
		return MATRIX_2X2<T> (x[0] * A.x11 + x[2] * A.x21, x[1] * A.x11 + x[3] * A.x21, x[0] * A.x21 + x[2] * A.x22, x[1] * A.x21 + x[3] * A.x22);
	}

	MATRIX_2X2<T> Multiply_With_Transpose (const UPPER_TRIANGULAR_MATRIX_2X2<T>& A) const
	{
		return MATRIX_2X2<T> (x[0] * A.x11 + x[2] * A.x12, x[1] * A.x11 + x[3] * A.x12, x[2] * A.x22, x[3] * A.x22);
	}

	MATRIX_2X2<T> Multiply_With_Transpose (const MATRIX_2X2<T>& A) const
	{
		return MATRIX_2X2<T> (x[0] * A.x[0] + x[2] * A.x[2], x[1] * A.x[0] + x[3] * A.x[2], x[0] * A.x[1] + x[2] * A.x[3], x[1] * A.x[1] + x[3] * A.x[3]);
	}

	MATRIX_2X2<T> Transpose_Times (const MATRIX_2X2<T>& A) const
	{
		return MATRIX_2X2<T> (x[0] * A.x[0] + x[1] * A.x[1], x[2] * A.x[0] + x[3] * A.x[1], x[0] * A.x[2] + x[1] * A.x[3], x[2] * A.x[2] + x[3] * A.x[3]);
	}

	VECTOR_2D<T> Transpose_Times (const VECTOR_2D<T>& v) const
	{
		return VECTOR_2D<T> (x[0] * v.x + x[1] * v.y, x[2] * v.x + x[3] * v.y);
	}

	UPPER_TRIANGULAR_MATRIX_2X2<T> R_From_QR_Factorization() const
	{
		if (x[1] == 0) return UPPER_TRIANGULAR_MATRIX_2X2<T> (x[0], x[2], x[3]);

		T c, s;

		if (fabs (x[1]) > fabs (x[0]))
		{
			T t = -x[0] / x[1];
			s = 1 / sqrt (1 + t * t);
			c = s * t;
		}
		else
		{
			T t = -x[1] / x[0];
			c = 1 / sqrt (1 + t * t);
			s = c * t;
		}

		return UPPER_TRIANGULAR_MATRIX_2X2<T> (c * x[0] - s * x[1], c * x[2] - s * x[3], s * x[2] + c * x[3]);
	}

	void Indefinite_Polar_Decomposition (MATRIX_2X2<T>& Q, SYMMETRIC_MATRIX_2X2<T>& S) const
	{
		T x03 = x[0] + x[3], cosine, sine;

		if (x03 == 0)
		{
			cosine = 0;
			sine = 1;
		}
		else
		{
			T t = (x[1] - x[2]) / x03;
			cosine = 1 / sqrt (1 + t * t);
			sine = t * cosine;
		}

		Q = MATRIX_2X2<T> (cosine, sine, -sine, cosine);
		S = SYMMETRIC_MATRIX_2X2<T> (Q.x[0] * x[0] + Q.x[1] * x[1], Q.x[0] * x[2] + Q.x[1] * x[3], Q.x[2] * x[2] + Q.x[3] * x[3]);
	}

	void Fast_Singular_Value_Decomposition (MATRIX_2X2<T>& U, DIAGONAL_MATRIX_2X2<T>& singular_values, MATRIX_2X2<T>& V) const
	{
		MATRIX_2X2<T> Q;
		SYMMETRIC_MATRIX_2X2<T> S;
		Indefinite_Polar_Decomposition (Q, S);
		S.Solve_Eigenproblem (singular_values, V);

		if (singular_values.x22 < 0 && fabs (singular_values.x22) >= fabs (singular_values.x11))
		{
			singular_values = DIAGONAL_MATRIX_2X2<T> (-singular_values.x22, -singular_values.x11);
			Q = -Q;
			V = MATRIX_2X2<T> (V.x[2], V.x[3], -V.x[0], -V.x[1]);
		}

		U = Q * V;
	}

	static T Determinant_Differential (const MATRIX_2X2<T>& A, const MATRIX_2X2<T>& dA)
	{
		return dA.x[0] * A.x[3] + A.x[0] * dA.x[3] - dA.x[1] * A.x[2] - A.x[1] * dA.x[2];
	}

	static MATRIX_2X2<T> Cofactor_Differential (const MATRIX_2X2<T>& dA)
	{
		return dA.Cofactor_Matrix();
	}

	T Triangle_Minimum_Altitude() const
	{
		return Determinant() / sqrt (max (Column (1).Magnitude_Squared(), Column (2).Magnitude_Squared(), (Column (1) - Column (2)).Magnitude_Squared()));
	}

//#####################################################################
};
// global functions
template<class T>
inline MATRIX_2X2<T> operator* (const T a, const MATRIX_2X2<T>& A)
{
	return MATRIX_2X2<T> (a * A.x[0], a * A.x[1], a * A.x[2], a * A.x[3]);
}

template<class T>
inline VECTOR_2D<T> operator* (const VECTOR_2D<T>& v, const MATRIX_2X2<T>& A)
{
	return VECTOR_2D<T> (v.x * A.x[0] + v.y * A.x[2], v.x * A.x[1] + v.y * A.x[3]);
}

template<class T>
inline MATRIX_2X2<T> operator* (const DIAGONAL_MATRIX_2X2<T>& A, const MATRIX_2X2<T>& B)
{
	return MATRIX_2X2<T> (A.x11 * B.x[0], A.x22 * B.x[1], A.x11 * B.x[2], A.x22 * B.x[3]);
}

template<class T>
inline MATRIX_2X2<T> operator* (const UPPER_TRIANGULAR_MATRIX_2X2<T>& A, const MATRIX_2X2<T>& B)
{
	return MATRIX_2X2<T> (A.x11 * B.x[0] + A.x12 * B.x[1], A.x22 * B.x[1], A.x11 * B.x[2] + A.x12 * B.x[3], A.x22 * B.x[3]);
}

template<class T>
inline std::istream& operator>> (std::istream& input_stream, MATRIX_2X2<T>& A)
{
	for (int i = 0; i < 2; i++) for (int j = 0; j < 2; j++) input_stream >> A.x[i + j * 2];

	return input_stream;
}

template<class T>
inline std::ostream& operator<< (std::ostream& output_stream, const MATRIX_2X2<T>& A)
{
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++) output_stream << A.x[i + j * 2] << " ";

		output_stream << std::endl;
	}

	return output_stream;
}
}
#endif
