//#####################################################################
// Copyright 2002-2004, Silvia Salinas-Blemker, Robert Bridson, Ronald Fedkiw, Eran Guendelman, Geoffrey Irving, Neil Molino, Igor Neverov, Joseph Teran, Rachel Weinstein.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class MATRIX_3X3
//#####################################################################
#ifndef __MATRIX_3X3__
#define __MATRIX_3X3__

#include <assert.h>
#include "VECTOR_3D.h"
#include "../Math_Tools/sqr.h"
#include "../Math_Tools/constants.h"
#include "../Math_Tools/exchange.h"
#include "UPPER_TRIANGULAR_MATRIX_3X3.h"
#include "SYMMETRIC_MATRIX_3X3.h"
#include "MATRIX_NXN.h"
#include "MATRIX_MXN.h"
namespace PhysBAM
{

template<class T>
class MATRIX_3X3
{
public:
	T x[9];

	MATRIX_3X3()
	{
		for (int i = 0; i < 9; i++) x[i] = 0;
	}

	template<class T2>
	MATRIX_3X3 (const MATRIX_3X3<T2>& matrix_input)
	{
		for (int i = 0; i < 9; i++) x[i] = (T) matrix_input.x[i];
	}

	MATRIX_3X3 (const DIAGONAL_MATRIX_3X3<T>& matrix_input)
	{
		x[0] = matrix_input.x11;
		x[4] = matrix_input.x22;
		x[8] = matrix_input.x33;
		x[1] = x[2] = x[3] = x[5] = x[6] = x[7] = 0;
	}

	MATRIX_3X3 (const SYMMETRIC_MATRIX_3X3<T>& matrix_input)
	{
		x[0] = matrix_input.x11;
		x[1] = x[3] = matrix_input.x21;
		x[2] = x[6] = matrix_input.x31;
		x[4] = matrix_input.x22;
		x[5] = x[7] = matrix_input.x32;
		x[8] = matrix_input.x33;
	}

	MATRIX_3X3 (const UPPER_TRIANGULAR_MATRIX_3X3<T>& matrix_input)
	{
		x[0] = matrix_input.x11;
		x[3] = matrix_input.x12;
		x[4] = matrix_input.x22;
		x[6] = matrix_input.x13;
		x[7] = matrix_input.x23;
		x[8] = matrix_input.x33;
		x[1] = x[2] = x[5] = 0;
	}

	MATRIX_3X3 (const T x11, const T x21, const T x31, const T x12, const T x22, const T x32, const T x13, const T x23, const T x33)
	{
		x[0] = x11;
		x[1] = x21;
		x[2] = x31;
		x[3] = x12;
		x[4] = x22;
		x[5] = x32;
		x[6] = x13;
		x[7] = x23;
		x[8] = x33;
	}

	MATRIX_3X3 (const VECTOR_3D<T>& column1, const VECTOR_3D<T>& column2, const VECTOR_3D<T>& column3)
	{
		x[0] = column1.x;
		x[1] = column1.y;
		x[2] = column1.z;
		x[3] = column2.x;
		x[4] = column2.y;
		x[5] = column2.z;
		x[6] = column3.x;
		x[7] = column3.y;
		x[8] = column3.z;
	}

	MATRIX_3X3 (const MATRIX_NXN<T>& matrix_input)
	{
		assert (matrix_input.n == 3);

		for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) x[i + 3 * j] = (T) matrix_input (i + 1, j + 1);
	}

	MATRIX_3X3 (const MATRIX_MXN<T>& matrix_input)
	{
		assert (matrix_input.n == 3 && matrix_input.m == 3);

		for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) x[i + 3 * j] = (T) matrix_input (i + 1, j + 1);
	}

	void Initialize_With_Column_Vectors (const VECTOR_3D<T>& column1, const VECTOR_3D<T>& column2, const VECTOR_3D<T>& column3)
	{
		x[0] = column1.x;
		x[1] = column1.y;
		x[2] = column1.z;
		x[3] = column2.x;
		x[4] = column2.y;
		x[5] = column2.z;
		x[6] = column3.x;
		x[7] = column3.y;
		x[8] = column3.z;
	}

	T& operator() (const int i, const int j)
	{
		assert (i >= 1 && i <= 3);
		assert (j >= 1 && j <= 3);
		return x[i - 1 + 3 * (j - 1)];
	}

	const T& operator() (const int i, const int j) const
	{
		assert (i >= 1 && i <= 3);
		assert (j >= 1 && j <= 3);
		return x[i - 1 + 3 * (j - 1)];
	}

	VECTOR_3D<T>& Column (const int j)
	{
		assert (1 <= j && j <= 3);
		return * (VECTOR_3D<T>*) (x + 3 * (j - 1));
	}

	const VECTOR_3D<T>& Column (const int j) const
	{
		assert (1 <= j && j <= 3);
		return * (VECTOR_3D<T>*) (x + 3 * (j - 1));
	}

	MATRIX_3X3<T> operator-() const
	{
		return MATRIX_3X3<T> (-x[0], -x[1], -x[2], -x[3], -x[4], -x[5], -x[6], -x[7], -x[8]);
	}

	MATRIX_3X3<T>& operator+= (const MATRIX_3X3<T>& A)
	{
		for (int i = 0; i < 9; i++) x[i] += A.x[i];

		return *this;
	}

	MATRIX_3X3<T>& operator+= (const T& a)
	{
		x[0] += a;
		x[4] += a;
		x[8] += a;
		return *this;
	}

	MATRIX_3X3<T>& operator-= (const MATRIX_3X3<T>& A)
	{
		for (int i = 0; i < 9; i++) x[i] -= A.x[i];

		return *this;
	}

	MATRIX_3X3<T>& operator-= (const T& a)
	{
		x[0] -= a;
		x[4] -= a;
		x[8] -= a;
		return *this;
	}

	MATRIX_3X3<T>& operator*= (const MATRIX_3X3<T>& A)
	{
		return *this = *this * A;
	}

	MATRIX_3X3<T>& operator*= (const T a)
	{
		for (int i = 0; i < 9; i++) x[i] *= a;

		return *this;
	}

	MATRIX_3X3<T>& operator/= (const T a)
	{
		assert (a != 0);
		T s = 1 / a;

		for (int i = 0; i < 9; i++) x[i] *= s;

		return *this;
	}

	MATRIX_3X3<T> operator+ (const MATRIX_3X3<T>& A) const
	{
		return MATRIX_3X3<T> (x[0] + A.x[0], x[1] + A.x[1], x[2] + A.x[2], x[3] + A.x[3], x[4] + A.x[4], x[5] + A.x[5], x[6] + A.x[6], x[7] + A.x[7], x[8] + A.x[8]);
	}

	MATRIX_3X3<T> operator+ (const T a) const
	{
		return MATRIX_3X3<T> (x[0] + a, x[1], x[2], x[3], x[4] + a, x[5], x[6], x[7], x[8] + a);
	}

	MATRIX_3X3<T> operator- (const MATRIX_3X3<T>& A) const
	{
		return MATRIX_3X3<T> (x[0] - A.x[0], x[1] - A.x[1], x[2] - A.x[2], x[3] - A.x[3], x[4] - A.x[4], x[5] - A.x[5], x[6] - A.x[6], x[7] - A.x[7], x[8] - A.x[8]);
	}

	MATRIX_3X3<T> operator- (const T a) const
	{
		return MATRIX_3X3<T> (x[0] - a, x[1], x[2], x[3], x[4] - a, x[5], x[6], x[7], x[8] - a);
	}

	MATRIX_3X3<T> operator* (const MATRIX_3X3<T>& A) const // 27 mults, 18 adds
	{
		return MATRIX_3X3<T> (x[0] * A.x[0] + x[3] * A.x[1] + x[6] * A.x[2], x[1] * A.x[0] + x[4] * A.x[1] + x[7] * A.x[2], x[2] * A.x[0] + x[5] * A.x[1] + x[8] * A.x[2],
				      x[0] * A.x[3] + x[3] * A.x[4] + x[6] * A.x[5], x[1] * A.x[3] + x[4] * A.x[4] + x[7] * A.x[5], x[2] * A.x[3] + x[5] * A.x[4] + x[8] * A.x[5],
				      x[0] * A.x[6] + x[3] * A.x[7] + x[6] * A.x[8], x[1] * A.x[6] + x[4] * A.x[7] + x[7] * A.x[8], x[2] * A.x[6] + x[5] * A.x[7] + x[8] * A.x[8]);
	}

	MATRIX_3X3<T> operator* (const T a) const
	{
		return MATRIX_3X3<T> (a * x[0], a * x[1], a * x[2], a * x[3], a * x[4], a * x[5], a * x[6], a * x[7], a * x[8]);
	}

	MATRIX_3X3<T> operator/ (const T a) const
	{
		assert (a != 0);
		T s = 1 / a;
		return MATRIX_3X3<T> (s * x[0], s * x[1], s * x[2], s * x[3], s * x[4], s * x[5], s * x[6], s * x[7], s * x[8]);
	}

	VECTOR_3D<T> operator* (const VECTOR_3D<T>& v) const // 9 mults, 6 adds
	{
		return VECTOR_3D<T> (x[0] * v.x + x[3] * v.y + x[6] * v.z, x[1] * v.x + x[4] * v.y + x[7] * v.z, x[2] * v.x + x[5] * v.y + x[8] * v.z);
	}

	VECTOR_2D<T> operator* (const VECTOR_2D<T>& v) const // assumes w=1 is the 3rd coordinate of v
	{
		T w = x[2] * v.x + x[5] * v.y + x[8];
		assert (w != 0);

		if (w == 1) return VECTOR_2D<T> (x[0] * v.x + x[3] * v.y + x[6], x[1] * v.x + x[4] * v.y + x[7]);
		else
		{
			T s = 1 / w;        // rescale so w=1
			return VECTOR_2D<T> (s * (x[0] * v.x + x[3] * v.y + x[6]), s * (x[1] * v.x + x[4] * v.y + x[7]));
		}
	}

	VECTOR_2D<T> Transform_2X2 (const VECTOR_2D<T>& v) const // multiplies vector by upper 2x2 of matrix only
	{
		return VECTOR_2D<T> (x[0] * v.x + x[3] * v.y, x[1] * v.x + x[4] * v.y);
	}

	T Determinant() const
	{
		return x[0] * (x[4] * x[8] - x[7] * x[5]) + x[3] * (x[7] * x[2] - x[1] * x[8]) + x[6] * (x[1] * x[5] - x[4] * x[2]);
	}

	void Invert()
	{
		*this = Inverse();
	}

	MATRIX_3X3<T> Inverse() const
	{
		T cofactor11 = x[4] * x[8] - x[7] * x[5], cofactor12 = x[7] * x[2] - x[1] * x[8], cofactor13 = x[1] * x[5] - x[4] * x[2];
		T determinant = x[0] * cofactor11 + x[3] * cofactor12 + x[6] * cofactor13;
		assert (determinant != 0);
		T s = 1 / determinant;
		return s * MATRIX_3X3<T> (cofactor11, cofactor12, cofactor13, x[6] * x[5] - x[3] * x[8], x[0] * x[8] - x[6] * x[2], x[3] * x[2] - x[0] * x[5], x[3] * x[7] - x[6] * x[4], x[6] * x[1] - x[0] * x[7], x[0] * x[4] - x[3] * x[1]);
	}

	MATRIX_3X3<T> Inverse_Transpose() const
	{
		return Inverse().Transposed();
	}

	MATRIX_3X3<T> Deviatoric() const
	{
		T one_third_trace = (T) one_third * (x[0] + x[4] + x[8]);
		return MATRIX_3X3<T> (x[0] - one_third_trace, x[1], x[2], x[3], x[4] - one_third_trace, x[5], x[6], x[7], x[8] - one_third_trace);
	}

	MATRIX_3X3<T> Dilational() const
	{
		T one_third_trace = (T) one_third * (x[0] + x[4] + x[8]);
		return MATRIX_3X3<T> (one_third_trace, 0, 0, 0, one_third_trace, 0, 0, 0, one_third_trace);
	}

	VECTOR_3D<T> Solve_Linear_System (const VECTOR_3D<T> b) const // 33 mults, 17 adds, 1 div
	{
		T cofactor11 = x[4] * x[8] - x[7] * x[5], cofactor12 = x[7] * x[2] - x[1] * x[8], cofactor13 = x[1] * x[5] - x[4] * x[2];
		T determinant = x[0] * cofactor11 + x[3] * cofactor12 + x[6] * cofactor13;
		assert (determinant != 0);
		return MATRIX_3X3<T> (cofactor11, cofactor12, cofactor13, (x[6] * x[5] - x[3] * x[8]), (x[0] * x[8] - x[6] * x[2]), (x[3] * x[2] - x[0] * x[5]), (x[3] * x[7] - x[6] * x[4]), (x[6] * x[1] - x[0] * x[7]), (x[0] * x[4] - x[3] * x[1])) * b / determinant;
	}

	void Transpose()
	{
		exchange (x[1], x[3]);
		exchange (x[2], x[6]);
		exchange (x[5], x[7]);
	}

	MATRIX_3X3<T> Transposed() const
	{
		return MATRIX_3X3<T> (x[0], x[3], x[6], x[1], x[4], x[7], x[2], x[5], x[8]);
	}

	T Trace() const
	{
		return x[0] + x[4] + x[8];
	}

	MATRIX_3X3<T> Q_From_Gram_Schmidt_QR_Factorization() const
	{
		int k;
		MATRIX_3X3<T> Q = *this;
		T one_over_r11 = 1 / sqrt ( (sqr (Q.x[0]) + sqr (Q.x[1]) + sqr (Q.x[2])));

		for (k = 0; k <= 2; k++) Q.x[k] = one_over_r11 * Q.x[k];

		T r12 = Q.x[0] * Q.x[3] + Q.x[1] * Q.x[4] + Q.x[2] * Q.x[5];
		Q.x[3] -= r12 * Q.x[0];
		Q.x[4] -= r12 * Q.x[1];
		Q.x[5] -= r12 * Q.x[2];
		T r13 = Q.x[0] * Q.x[6] + Q.x[1] * Q.x[7] + Q.x[2] * Q.x[8];
		Q.x[6] -= r13 * Q.x[0];
		Q.x[7] -= r13 * Q.x[1];
		Q.x[8] -= r13 * Q.x[2];
		T one_over_r22 = 1 / sqrt ( (sqr (Q.x[3]) + sqr (Q.x[4]) + sqr (Q.x[5])));

		for (k = 3; k <= 5; k++) Q.x[k] = one_over_r22 * Q.x[k];

		T r23 = Q.x[3] * Q.x[6] + Q.x[4] * Q.x[7] + Q.x[5] * Q.x[8];
		Q.x[6] -= r23 * Q.x[3];
		Q.x[7] -= r23 * Q.x[4];
		Q.x[8] -= r23 * Q.x[5];
		T one_over_r33 = 1 / sqrt ( (sqr (Q.x[6]) + sqr (Q.x[7]) + sqr (Q.x[8])));

		for (k = 6; k <= 8; k++) Q.x[k] = one_over_r33 * Q.x[k];

		return Q;
	}

	UPPER_TRIANGULAR_MATRIX_3X3<T> R_From_Gram_Schmidt_QR_Factorization() const
	{
		int k;
		MATRIX_3X3<T> Q = *this;
		UPPER_TRIANGULAR_MATRIX_3X3<T> R;
		R.x11 = sqrt ( (sqr (Q.x[0]) + sqr (Q.x[1]) + sqr (Q.x[2])));
		T one_over_r11 = 1 / R.x11;

		for (k = 0; k <= 2; k++) Q.x[k] = one_over_r11 * Q.x[k];

		R.x12 = Q.x[0] * Q.x[3] + Q.x[1] * Q.x[4] + Q.x[2] * Q.x[5];
		Q.x[3] -= R.x12 * Q.x[0];
		Q.x[4] -= R.x12 * Q.x[1];
		Q.x[5] -= R.x12 * Q.x[2];
		R.x13 = Q.x[0] * Q.x[6] + Q.x[1] * Q.x[7] + Q.x[2] * Q.x[8];
		Q.x[6] -= R.x13 * Q.x[0];
		Q.x[7] -= R.x13 * Q.x[1];
		Q.x[8] -= R.x13 * Q.x[2];
		R.x22 = sqrt ( (sqr (Q.x[3]) + sqr (Q.x[4]) + sqr (Q.x[5])));
		T one_over_r22 = 1 / R.x22;

		for (k = 3; k <= 5; k++) Q.x[k] = one_over_r22 * Q.x[k];

		R.x23 = Q.x[3] * Q.x[6] + Q.x[4] * Q.x[7] + Q.x[5] * Q.x[8];
		Q.x[6] -= R.x23 * Q.x[3];
		Q.x[7] -= R.x23 * Q.x[4];
		Q.x[8] -= R.x23 * Q.x[5];
		R.x33 = sqrt ( (sqr (Q.x[6]) + sqr (Q.x[7]) + sqr (Q.x[8])));
		return R;
	}

	MATRIX_3X3<T> Higham_Iterate (const T tolerance = 1e-5, const int max_iterations = 20, const bool exit_on_max_iterations = false) const
	{
		MATRIX_3X3<T> X = *this, Y;
		int iterations = 0;

		for (;;)
		{
			Y = (T).5 * (X + X.Inverse_Transpose());

			if ( (X - Y).Max_Abs_Element() < tolerance) return Y;

			X = Y;

			if (++iterations >= max_iterations)
			{
				if (exit_on_max_iterations)
				{
					std::cout << "Failed in Higham iteration!" << std::endl;
					exit (1);
				}
				else return X;
			}
		}
	}

	MATRIX_3X3<T> Cofactor_Matrix() const
	{
		return MATRIX_3X3<T> (x[4] * x[8] - x[5] * x[7], -x[3] * x[8] + x[5] * x[6], x[3] * x[7] - x[4] * x[6],
				      -x[1] * x[8] + x[2] * x[7], x[0] * x[8] - x[2] * x[6], -x[0] * x[7] + x[1] * x[6],
				      x[1] * x[5] - x[2] * x[4], -x[0] * x[5] + x[2] * x[3], x[0] * x[4] - x[1] * x[3]);
	}

	SYMMETRIC_MATRIX_3X3<T> Outer_Product_Matrix() const
	{
		return SYMMETRIC_MATRIX_3X3<T> (x[0] * x[0] + x[3] * x[3] + x[6] * x[6], x[1] * x[0] + x[4] * x[3] + x[7] * x[6], x[2] * x[0] + x[5] * x[3] + x[8] * x[6],
						x[1] * x[1] + x[4] * x[4] + x[7] * x[7], x[2] * x[1] + x[5] * x[4] + x[8] * x[7], x[2] * x[2] + x[5] * x[5] + x[8] * x[8]);
	}

	SYMMETRIC_MATRIX_3X3<T> Normal_Equations_Matrix() const // 18 mults, 12 adds
	{
		return SYMMETRIC_MATRIX_3X3<T> (x[0] * x[0] + x[1] * x[1] + x[2] * x[2], x[3] * x[0] + x[4] * x[1] + x[5] * x[2], x[6] * x[0] + x[7] * x[1] + x[8] * x[2],
						x[3] * x[3] + x[4] * x[4] + x[5] * x[5], x[6] * x[3] + x[7] * x[4] + x[8] * x[5], x[6] * x[6] + x[7] * x[7] + x[8] * x[8]);
	}

	SYMMETRIC_MATRIX_3X3<T> Symmetric_Part() const
	{
		return SYMMETRIC_MATRIX_3X3<T> (x[0], (T).5 * (x[1] + x[3]), (T).5 * (x[2] + x[6]), x[4], (T).5 * (x[5] + x[7]), x[8]);
	}

	SYMMETRIC_MATRIX_3X3<T> Twice_Symmetric_Part() const // 3 mults, 3 adds
	{
		return SYMMETRIC_MATRIX_3X3<T> (2 * x[0], x[1] + x[3], x[2] + x[6], 2 * x[4], x[5] + x[7], 2 * x[8]);
	}

	DIAGONAL_MATRIX_3X3<T> Diagonal_Part() const
	{
		return DIAGONAL_MATRIX_3X3<T> (x[0], x[4], x[8]);
	}

	void Normalize_Columns()
	{
		T magnitude = sqrt (sqr (x[0]) + sqr (x[1]) + sqr (x[2]));
		assert (magnitude != 0);
		T s = 1 / magnitude;
		x[0] *= s;
		x[1] *= s;
		x[2] *= s;
		magnitude = sqrt (sqr (x[3]) + sqr (x[4]) + sqr (x[5]));
		assert (magnitude != 0);
		s = 1 / magnitude;
		x[3] *= s;
		x[4] *= s;
		x[5] *= s;
		magnitude = sqrt (sqr (x[6]) + sqr (x[7]) + sqr (x[8]));
		assert (magnitude != 0);
		s = 1 / magnitude;
		x[6] *= s;
		x[7] *= s;
		x[8] *= s;
	}

	VECTOR_3D<T> Largest_Normalized_Column() const
	{
		T scale1 = sqr (x[0]) + sqr (x[1]) + sqr (x[2]), scale2 = sqr (x[3]) + sqr (x[4]) + sqr (x[5]), scale3 = sqr (x[6]) + sqr (x[7]) + sqr (x[8]);

		if (scale1 > scale2)
		{
			if (scale1 > scale3) return VECTOR_3D<T> (x[0], x[1], x[2]) / sqrt (scale1);
		}
		else if (scale2 > scale3) return VECTOR_3D<T> (x[3], x[4], x[5]) / sqrt (scale2);

		return VECTOR_3D<T> (x[6], x[7], x[8]) / sqrt (scale3);
	}

	static MATRIX_3X3<T> Transpose (const MATRIX_3X3<T>& A)
	{
		return MATRIX_3X3<T> (A.x[0], A.x[3], A.x[6], A.x[1], A.x[4], A.x[7], A.x[2], A.x[5], A.x[8]);
	}

	static MATRIX_3X3<T> Translation_Matrix (const VECTOR_2D<T>& translation) // treating the 3x3 matrix as a homogeneous transformation on 2d vectors
	{
		return MATRIX_3X3<T> (1, 0, 0, 0, 1, 0, translation.x, translation.y, 1);
	}

	static MATRIX_3X3<T> Identity_Matrix()
	{
		return MATRIX_3X3<T> (1, 0, 0, 0, 1, 0, 0, 0, 1);
	}

	static MATRIX_3X3<T> Diagonal_Matrix (const T d)
	{
		return MATRIX_3X3<T> (d, 0, 0, 0, d, 0, 0, 0, d);
	}

	static MATRIX_3X3<T> Rotation_Matrix_X_Axis (const T radians)
	{
		T c = cos (radians), s = sin (radians);
		return MATRIX_3X3<T> (1, 0, 0, 0, c, s, 0, -s, c);
	}

	static MATRIX_3X3<T> Rotation_Matrix_Y_Axis (const T radians)
	{
		T c = cos (radians), s = sin (radians);
		return MATRIX_3X3<T> (c, 0, -s, 0, 1, 0, s, 0, c);
	}

	static MATRIX_3X3<T> Rotation_Matrix_Z_Axis (const T radians)
	{
		T c = cos (radians), s = sin (radians);
		return MATRIX_3X3<T> (c, s, 0, -s, c, 0, 0, 0, 1);
	}

	static MATRIX_3X3<T> Rotation_Matrix (const VECTOR_3D<T>& axis, const T radians)
	{
		T c = cos (radians), s = sin (radians);
		return MATRIX_3X3<T> (sqr (axis.x) + (1 - sqr (axis.x)) * c, axis.x * axis.y * (1 - c) + axis.z * s, axis.x * axis.z * (1 - c) - axis.y * s,
				      axis.x * axis.y * (1 - c) - axis.z * s, sqr (axis.y) + (1 - sqr (axis.y)) * c, axis.y * axis.z * (1 - c) + axis.x * s,
				      axis.x * axis.z * (1 - c) + axis.y * s, axis.y * axis.z * (1 - c) - axis.x * s, sqr (axis.z) + (1 - sqr (axis.z)) * c);
	}

	static MATRIX_3X3<T> Rotation_Matrix (const VECTOR_3D<T>& rotation)
	{
		T angle = rotation.Magnitude();
		return angle ? Rotation_Matrix (rotation / angle, angle) : Identity_Matrix();
	}

	static MATRIX_3X3<T> Rotation_Matrix (const VECTOR_3D<T>& x_final, const VECTOR_3D<T>& y_final, const VECTOR_3D<T>& z_final)
	{
		return MATRIX_3X3<T> (x_final.x, y_final.x, z_final.x, x_final.y, y_final.y, z_final.y, x_final.z, y_final.z, z_final.z);
	}

	static MATRIX_3X3<T> Rotation_Matrix (const VECTOR_3D<T>& initial_vector, const VECTOR_3D<T>& final_vector)
	{
		VECTOR_3D<T> initial_unit = initial_vector / initial_vector.Magnitude(), final_unit = final_vector / final_vector.Magnitude();
		T cos_theta = VECTOR_3D<T>::Dot_Product (initial_unit, final_unit);

		if (cos_theta > 1 - 1e-14) return Identity_Matrix();

		if (cos_theta < -1 + 1e-14) return MATRIX_3X3<T> (-1, 0, 0, 0, -1, 0, 0, 0, -1); // note-this is actually a reflection

		VECTOR_3D<T> axis = VECTOR_3D<T>::Cross_Product (initial_unit, final_unit);
		axis.Normalize();
		return Rotation_Matrix (axis, acos (cos_theta));
	}

	static MATRIX_3X3<T> Outer_Product (const VECTOR_3D<T>& u, const VECTOR_3D<T>& v)
	{
		return MATRIX_3X3<T> (u.x * v.x, u.y * v.x, u.z * v.x, u.x * v.y, u.y * v.y, u.z * v.y, u.x * v.z, u.y * v.z, u.z * v.z);
	}

	static T Inner_Product (const MATRIX_3X3<T>& A, const MATRIX_3X3<T>& B)
	{
		return A.x[0] * B.x[0] + A.x[1] * B.x[1] + A.x[2] * B.x[2] + A.x[3] * B.x[3] + A.x[4] * B.x[4] + A.x[5] * B.x[5] + A.x[6] * B.x[6] + A.x[7] * B.x[7] + A.x[8] * B.x[8];
	}

	static MATRIX_3X3<T> Cross_Product_Matrix (const VECTOR_3D<T>& v)
	{
		return MATRIX_3X3<T> (0, v.z, -v.y, -v.z, 0, v.x, v.y, -v.x, 0);
	}

	VECTOR_3D<T> Antisymmetric_Part_Cross_Product_Vector() const
	{
		return (T).5 * VECTOR_3D<T> (x[5] - x[7], x[6] - x[2], x[1] - x[3]);
	}

	static SYMMETRIC_MATRIX_3X3<T> Right_Multiply_With_Symmetric_Result (const MATRIX_3X3<T>& A, const DIAGONAL_MATRIX_3X3<T>& B)
	{
		return SYMMETRIC_MATRIX_3X3<T> (B.x11 * A.x[0], B.x11 * A.x[1], B.x11 * A.x[2], B.x22 * A.x[4], B.x22 * A.x[5], B.x33 * A.x[8]);
	}

	T Max_Abs_Element() const
	{
		return max (fabs (x[0]), fabs (x[1]), fabs (x[2]), fabs (x[3]), fabs (x[4]), fabs (x[5]), fabs (x[6]), fabs (x[7]), fabs (x[8]));
	}

	T Frobenius_Norm() const
	{
		return sqrt (sqr (x[0]) + sqr (x[1]) + sqr (x[2]) + sqr (x[3]) + sqr (x[4]) + sqr (x[5]) + sqr (x[6]) + sqr (x[7]) + sqr (x[8]));
	}

	MATRIX_3X3<T> operator* (const DIAGONAL_MATRIX_3X3<T>& A) const // 9 mults
	{
		return MATRIX_3X3<T> (x[0] * A.x11, x[1] * A.x11, x[2] * A.x11, x[3] * A.x22, x[4] * A.x22, x[5] * A.x22, x[6] * A.x33, x[7] * A.x33, x[8] * A.x33);
	}

	MATRIX_3X3<T> operator* (const UPPER_TRIANGULAR_MATRIX_3X3<T>& A) const // 18 mults, 8 adds
	{
		return MATRIX_3X3<T> (x[0] * A.x11, x[1] * A.x11, x[2] * A.x11, x[0] * A.x12 + x[3] * A.x22, x[1] * A.x12 + x[4] * A.x22, x[2] * A.x12 + x[5] * A.x22,
				      x[0] * A.x13 + x[3] * A.x23 + x[6] * A.x33, x[1] * A.x13 + x[4] * A.x23 + x[7] * A.x33, x[2] * A.x13 + x[5] * A.x23 + x[8] * A.x33);
	}

	MATRIX_3X3<T> operator* (const SYMMETRIC_MATRIX_3X3<T>& A) const // 27 mults, 18 adds
	{
		return MATRIX_3X3<T> (x[0] * A.x11 + x[3] * A.x21 + x[6] * A.x31, x[1] * A.x11 + x[4] * A.x21 + x[7] * A.x31, x[2] * A.x11 + x[5] * A.x21 + x[8] * A.x31,
				      x[0] * A.x21 + x[3] * A.x22 + x[6] * A.x32, x[1] * A.x21 + x[4] * A.x22 + x[7] * A.x32, x[2] * A.x21 + x[5] * A.x22 + x[8] * A.x32,
				      x[0] * A.x31 + x[3] * A.x32 + x[6] * A.x33, x[1] * A.x31 + x[4] * A.x32 + x[7] * A.x33, x[2] * A.x31 + x[5] * A.x32 + x[8] * A.x33);
	}

	MATRIX_3X3<T> Multiply_With_Transpose (const UPPER_TRIANGULAR_MATRIX_3X3<T>& A) const
	{
		return MATRIX_3X3<T> (x[0] * A.x11 + x[3] * A.x12 + x[6] * A.x13, x[1] * A.x11 + x[4] * A.x12 + x[7] * A.x13, x[2] * A.x11 + x[5] * A.x12 + x[8] * A.x13,
				      x[3] * A.x22 + x[6] * A.x23, x[4] * A.x22 + x[7] * A.x23, x[5] * A.x22 + x[8] * A.x23, x[6] * A.x33, x[7] * A.x33, x[8] * A.x33);
	}

	MATRIX_3X3<T> Multiply_With_Transpose (const MATRIX_3X3<T>& A) const
	{
		return MATRIX_3X3<T> (x[0] * A.x[0] + x[3] * A.x[3] + x[6] * A.x[6], x[1] * A.x[0] + x[4] * A.x[3] + x[7] * A.x[6], x[2] * A.x[0] + x[5] * A.x[3] + x[8] * A.x[6],
				      x[0] * A.x[1] + x[3] * A.x[4] + x[6] * A.x[7], x[1] * A.x[1] + x[4] * A.x[4] + x[7] * A.x[7], x[2] * A.x[1] + x[5] * A.x[4] + x[8] * A.x[7],
				      x[0] * A.x[2] + x[3] * A.x[5] + x[6] * A.x[8], x[1] * A.x[2] + x[4] * A.x[5] + x[7] * A.x[8], x[2] * A.x[2] + x[5] * A.x[5] + x[8] * A.x[8]);
	}

	MATRIX_3X3<T> Transpose_Times (const MATRIX_3X3<T>& A) const
	{
		return MATRIX_3X3<T> (x[0] * A.x[0] + x[1] * A.x[1] + x[2] * A.x[2], x[3] * A.x[0] + x[4] * A.x[1] + x[5] * A.x[2], x[6] * A.x[0] + x[7] * A.x[1] + x[8] * A.x[2],
				      x[0] * A.x[3] + x[1] * A.x[4] + x[2] * A.x[5], x[3] * A.x[3] + x[4] * A.x[4] + x[5] * A.x[5], x[6] * A.x[3] + x[7] * A.x[4] + x[8] * A.x[5],
				      x[0] * A.x[6] + x[1] * A.x[7] + x[2] * A.x[8], x[3] * A.x[6] + x[4] * A.x[7] + x[5] * A.x[8], x[6] * A.x[6] + x[7] * A.x[7] + x[8] * A.x[8]);
	}

	VECTOR_3D<T> Transpose_Times (const VECTOR_3D<T>& v) const
	{
		return VECTOR_3D<T> (x[0] * v.x + x[1] * v.y + x[2] * v.z, x[3] * v.x + x[4] * v.y + x[5] * v.z, x[6] * v.x + x[7] * v.y + x[8] * v.z);
	}

	static MATRIX_3X3<T> Left_Procrustes_Rotation (const MATRIX_3X3<T>& A, const MATRIX_3X3<T>& B)
	{
		MATRIX_3X3<T> U, V;
		DIAGONAL_MATRIX_3X3<T> D;
		A.Multiply_With_Transpose (B).Fast_Singular_Value_Decomposition (U, D, V);
		return U.Multiply_With_Transpose (V);
	}

	void Fast_Singular_Value_Decomposition (MATRIX_3X3<T>& U, DIAGONAL_MATRIX_3X3<T>& singular_values, MATRIX_3X3<T>& V) const
	{
		MATRIX_3X3<double> U_double, V_double;
		DIAGONAL_MATRIX_3X3<double> singular_values_double;
		Fast_Singular_Value_Decomposition_Double (U_double, singular_values_double, V_double);
		U = U_double;
		singular_values = singular_values_double;
		V = V_double;
	}

	template<class RW>
	void Read (std::istream &input_stream)
	{
		Read_Binary<RW> (input_stream, x[0], x[1], x[2], x[3], x[4], x[5], x[6], x[7], x[8]);
	}

	template<class RW>
	void Write (std::ostream &output_stream) const
	{
		Write_Binary<RW> (output_stream, x[0], x[1], x[2], x[3], x[4], x[5], x[6], x[7], x[8]);
	}

//#####################################################################
	void Fast_Singular_Value_Decomposition_Double (MATRIX_3X3<double>& U, DIAGONAL_MATRIX_3X3<double>& singular_values, MATRIX_3X3<double>& V, const double tolerance = 1e-7) const;
	T Tetrahedron_Minimum_Altitude() const;
//#####################################################################
};
// global functions
template<class T>
inline MATRIX_3X3<T> operator+ (const T a, const MATRIX_3X3<T>& A)
{
	return A + a;
}

template<class T>
inline MATRIX_3X3<T> operator* (const T a, const MATRIX_3X3<T>& A)
{
	return A * a;
}

template<class T>
inline VECTOR_3D<T> operator* (const VECTOR_3D<T>& v, const MATRIX_3X3<T>& A)
{
	return VECTOR_3D<T> (v.x * A.x[0] + v.y * A.x[1] + v.z * A.x[2], v.x * A.x[3] + v.y * A.x[4] + v.z * A.x[5], v.x * A.x[6] + v.y * A.x[7] + v.z * A.x[8]);
}

template<class T>
inline MATRIX_3X3<T> operator* (const DIAGONAL_MATRIX_3X3<T>& A, const MATRIX_3X3<T>& B)
{
	return MATRIX_3X3<T> (A.x11 * B.x[0], A.x22 * B.x[1], A.x33 * B.x[2], A.x11 * B.x[3], A.x22 * B.x[4], A.x33 * B.x[5], A.x11 * B.x[6], A.x22 * B.x[7], A.x33 * B.x[8]);
}

template<class T>
inline MATRIX_3X3<T> operator* (const UPPER_TRIANGULAR_MATRIX_3X3<T>& A, const MATRIX_3X3<T>& B)
{
	return MATRIX_3X3<T> (A.x11 * B.x[0] + A.x12 * B.x[1] + A.x13 * B.x[2], A.x22 * B.x[1] + A.x23 * B.x[2], A.x33 * B.x[2], A.x11 * B.x[3] + A.x12 * B.x[4] + A.x13 * B.x[5],
			      A.x22 * B.x[4] + A.x23 * B.x[5], A.x33 * B.x[5], A.x11 * B.x[6] + A.x12 * B.x[7] + A.x13 * B.x[8], A.x22 * B.x[7] + A.x23 * B.x[8], A.x33 * B.x[8]);
}

template<class T>
inline MATRIX_3X3<T> operator* (const SYMMETRIC_MATRIX_3X3<T>& A, const MATRIX_3X3<T>& B)
{
	return MATRIX_3X3<T> (A.x11 * B.x[0] + A.x21 * B.x[1] + A.x31 * B.x[2], A.x21 * B.x[0] + A.x22 * B.x[1] + A.x32 * B.x[2], A.x31 * B.x[0] + A.x32 * B.x[1] + A.x33 * B.x[2],
			      A.x11 * B.x[3] + A.x21 * B.x[4] + A.x31 * B.x[5], A.x21 * B.x[3] + A.x22 * B.x[4] + A.x32 * B.x[5], A.x31 * B.x[3] + A.x32 * B.x[4] + A.x33 * B.x[5],
			      A.x11 * B.x[6] + A.x21 * B.x[7] + A.x31 * B.x[8], A.x21 * B.x[6] + A.x22 * B.x[7] + A.x32 * B.x[8], A.x31 * B.x[6] + A.x32 * B.x[7] + A.x33 * B.x[8]);
}

template<class T>
inline std::istream& operator>> (std::istream& input_stream, MATRIX_3X3<T>& A)
{
	for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) input_stream >> A.x[i + j * 3];

	return input_stream;
}

template<class T>
inline std::ostream& operator<< (std::ostream& output_stream, const MATRIX_3X3<T>& A)
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++) output_stream << A.x[i + j * 3] << " ";

		output_stream << std::endl;
	}

	return output_stream;
}
//#####################################################################
}
#endif

