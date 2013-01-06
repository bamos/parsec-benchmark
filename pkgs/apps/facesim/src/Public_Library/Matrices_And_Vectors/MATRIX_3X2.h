//#####################################################################
// Copyright 2003, Zhaosheng Bao, Ronald Fedkiw, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class MATRIX_3X2
//#####################################################################
#ifndef __MATRIX_3X2__
#define __MATRIX_3X2__

#include <assert.h>
#include "MATRIX_2X2.h"
#include "MATRIX_3X3.h"
#include "VECTOR_2D.h"
#include "VECTOR_3D.h"
#include "../Math_Tools/sqr.h"
#include "../Math_Tools/constants.h"
#include "../Math_Tools/exchange.h"
namespace PhysBAM
{

template<class T>
class MATRIX_3X2
{
public:
	T x[6];

	MATRIX_3X2()
	{
		for (int i = 0; i < 6; i++) x[i] = 0;
	}

	MATRIX_3X2 (const MATRIX_3X2<T>& matrix_input)
	{
		for (int i = 0; i < 6; i++) x[i] = matrix_input.x[i];
	}

	MATRIX_3X2 (const T x11, const T x21, const T x31, const T x12, const T x22, const T x32)
	{
		x[0] = x11;
		x[1] = x21;
		x[2] = x31;
		x[3] = x12;
		x[4] = x22;
		x[5] = x32;
	}

	MATRIX_3X2 (const VECTOR_3D<T>& column1, const VECTOR_3D<T>& column2)
	{
		x[0] = column1.x;
		x[1] = column1.y;
		x[2] = column1.z;
		x[3] = column2.x;
		x[4] = column2.y;
		x[5] = column2.z;
	}

	void Initialize_With_Column_Vectors (const VECTOR_3D<T>& column1, const VECTOR_3D<T>& column2)
	{
		x[0] = column1.x;
		x[1] = column1.y;
		x[2] = column1.z;
		x[3] = column2.x;
		x[4] = column2.y;
		x[5] = column2.z;
	}

	T& operator[] (const int i)
	{
		assert (i >= 0 && i <= 5);
		return x[i];
	}

	T& operator() (const int i, const int j)
	{
		assert (i >= 1 && i <= 3);
		assert (j >= 1 && j <= 2);
		return x[i - 1 + 3 * (j - 1)];
	}

	const T& operator() (const int i, const int j) const
	{
		assert (i >= 1 && i <= 3);
		assert (j >= 1 && j <= 2);
		return x[i - 1 + 3 * (j - 1)];
	}

	VECTOR_3D<T>& Column (const int j)
	{
		assert (1 <= j && j <= 2);
		return * (VECTOR_3D<T>*) (x + 3 * (j - 1));
	}

	const VECTOR_3D<T>& Column (const int j) const
	{
		assert (1 <= j && j <= 2);
		return * (VECTOR_3D<T>*) (x + 3 * (j - 1));
	}

	MATRIX_3X2<T> operator-() const
	{
		return MATRIX_3X2 (-x[0], -x[1], -x[2], -x[3], -x[4], -x[5]);
	}

	MATRIX_3X2<T> operator+= (const MATRIX_3X2<T> A)
	{
		for (int i = 0; i < 6; i++) x[i] += A.x[i];

		return *this;
	}

	MATRIX_3X2<T> operator-= (const MATRIX_3X2<T> A)
	{
		for (int i = 0; i < 6; i++) x[i] -= A.x[i];

		return *this;
	}

	MATRIX_3X2<T> operator*= (const MATRIX_2X2<T> & A)
	{
		return *this = *this * A;
	}

	MATRIX_3X2<T> operator*= (const T a)
	{
		for (int i = 0; i < 6; i++) x[i] *= a;

		return *this;
	}

	MATRIX_3X2<T> operator/= (const T a)
	{
		assert (a != 0);
		T s = 1 / a;

		for (int i = 0; i < 6; i++) x[i] *= s;

		return *this;
	}

	MATRIX_3X2<T> operator+ (const MATRIX_3X2<T> A) const
	{
		return MATRIX_3X2 (x[0] + A.x[0], x[1] + A.x[1], x[2] + A.x[2], x[3] + A.x[3], x[4] + A.x[4], x[5] + A.x[5]);
	}

	MATRIX_3X2<T> operator- (const MATRIX_3X2<T> A) const
	{
		return MATRIX_3X2 (x[0] - A.x[0], x[1] - A.x[1], x[2] - A.x[2], x[3] - A.x[3], x[4] - A.x[4], x[5] - A.x[5]);
	}

	MATRIX_3X2<T> operator* (const MATRIX_2X2<T>& A) const
	{
		return MATRIX_3X2 (x[0] * A.x[0] + x[3] * A.x[1], x[1] * A.x[0] + x[4] * A.x[1], x[2] * A.x[0] + x[5] * A.x[1], x[0] * A.x[2] + x[3] * A.x[3], x[1] * A.x[2] + x[4] * A.x[3], x[2] * A.x[2] + x[5] * A.x[3]);
	}

	MATRIX_3X2<T> Multiply_With_Transpose (const UPPER_TRIANGULAR_MATRIX_2X2<T>& A) const
	{
		return MATRIX_3X2 (x[0] * A.x11 + x[3] * A.x12, x[1] * A.x11 + x[4] * A.x12, x[2] * A.x11 + x[5] * A.x12, x[3] * A.x22, x[4] * A.x22, x[5] * A.x22);
	}

	MATRIX_3X2<T> operator* (const T a) const
	{
		return MATRIX_3X2 (a * x[0], a * x[1], a * x[2], a * x[3], a * x[4], a * x[5]);
	}

	MATRIX_3X2<T> operator/ (const T a) const
	{
		assert (a != 0);
		T s = 1 / a;
		return MATRIX_3X2 (s * x[0], s * x[1], s * x[2], s * x[3], s * x[4], s * x[5]);
	}

	VECTOR_3D<T> operator* (const VECTOR_2D<T>& v) const
	{
		return VECTOR_3D<T> (x[0] * v.x + x[3] * v.y, x[1] * v.x + x[4] * v.y, x[2] * v.x + x[5] * v.y);
	}

	UPPER_TRIANGULAR_MATRIX_2X2<T> R_From_Gram_Schmidt_QR_Factorization() const
	{
		T x_dot_x = Column (1).Magnitude_Squared(), x_dot_y = VECTOR_3D<T>::Dot_Product (Column (1), Column (2)), y_dot_y = Column (2).Magnitude_Squared();
		T r11 = sqrt (x_dot_x), r12 = r11 ? x_dot_y / r11 : 0, r22 = sqrt (y_dot_y - r12 * r12);
		return UPPER_TRIANGULAR_MATRIX_2X2<T> (r11, r12, r22);
	}

	SYMMETRIC_MATRIX_2X2<T> Normal_Equations_Matrix() const
	{
		return SYMMETRIC_MATRIX_2X2<T> (x[0] * x[0] + x[1] * x[1] + x[2] * x[2], x[3] * x[0] + x[4] * x[1] + x[5] * x[2], x[3] * x[3] + x[4] * x[4] + x[5] * x[5]);
	}

	MATRIX_2X2<T> Transpose_Times (const MATRIX_3X2<T>& A) const
	{
		return MATRIX_2X2<T> (x[0] * A.x[0] + x[1] * A.x[1] + x[2] * A.x[2], x[3] * A.x[0] + x[4] * A.x[1] + x[5] * A.x[2], x[0] * A.x[3] + x[1] * A.x[4] + x[2] * A.x[5], x[3] * A.x[3] + x[4] * A.x[4] + x[5] * A.x[5]);
	}

	VECTOR_2D<T> Transpose_Times (const VECTOR_3D<T>& v) const
	{
		return VECTOR_2D<T> (x[0] * v.x + x[1] * v.y + x[2] * v.z, x[3] * v.x + x[4] * v.y + x[5] * v.z);
	}

	T Max_Abs_Element() const
	{
		return max (fabs (x[0]), fabs (x[1]), fabs (x[2]), fabs (x[3]), fabs (x[4]), fabs (x[5]));
	}

	MATRIX_3X2<T> operator* (const UPPER_TRIANGULAR_MATRIX_2X2<T>& A) const
	{
		return MATRIX_3X2<T> (x[0] * A.x11, x[1] * A.x11, x[2] * A.x11, x[0] * A.x12 + x[3] * A.x22, x[1] * A.x12 + x[4] * A.x22, x[2] * A.x12 + x[5] * A.x22);
	}

	MATRIX_3X2<T> operator* (const SYMMETRIC_MATRIX_2X2<T>& A) const
	{
		return MATRIX_3X2<T> (x[0] * A.x11 + x[3] * A.x21, x[1] * A.x11 + x[4] * A.x21, x[2] * A.x11 + x[5] * A.x21, x[0] * A.x21 + x[3] * A.x22, x[1] * A.x21 + x[4] * A.x22, x[2] * A.x21 + x[5] * A.x22);
	}

	MATRIX_3X2<T> operator* (const DIAGONAL_MATRIX_2X2<T>& A) const
	{
		return MATRIX_3X2<T> (x[0] * A.x11, x[1] * A.x11, x[2] * A.x11, x[3] * A.x22, x[4] * A.x22, x[5] * A.x22);
	}

	VECTOR_3D<T> Normal() const
	{
		return VECTOR_3D<T>::Cross_Product (Column (1), Column (2));
	}

	void Fast_Singular_Value_Decomposition (MATRIX_3X2<T>& U, DIAGONAL_MATRIX_2X2<T>& singular_values, MATRIX_2X2<T>& V, const T tolerance = 1e-5) const
	{
		DIAGONAL_MATRIX_2X2<T> lambda;
		Normal_Equations_Matrix().Solve_Eigenproblem (lambda, V);

		if (lambda.x11 <= 0)
		{
			singular_values = DIAGONAL_MATRIX_2X2<T>();
			U = MATRIX_3X2<T>();
		}
		else if (lambda.x22 > tolerance * lambda.x11)
		{
			singular_values = lambda.Sqrt();
			U = MATRIX_3X2<T> (*this * (V.Column (1) / singular_values.x11), *this * (V.Column (2) / singular_values.x22));
		}
		else
		{
			singular_values = DIAGONAL_MATRIX_2X2<T> (sqrt (lambda.x11), 0);
			U.Column (1) = *this * (V.Column (1) / singular_values.x11);
			U.Column (2) = U.Column (1).Orthogonal_Vector();
		}
	}

	void Consistent_Singular_Value_Decomposition (MATRIX_3X2<T>& U, DIAGONAL_MATRIX_2X2<T>& singular_values, MATRIX_2X2<T>& V, const VECTOR_3D<T> normal, const T tolerance = 1e-5) const
	{
		Fast_Singular_Value_Decomposition (U, singular_values, V, tolerance);

		if (VECTOR_3D<T>::Dot_Product (normal, Normal()) < 0)
		{
			singular_values.x22 = -singular_values.x22;
			U.Column (2) = -U.Column (2);
		}
	}

//#####################################################################
};
// global functions
template<class T>
inline MATRIX_3X2<T> operator* (MATRIX_3X3<T> B, const MATRIX_3X2<T> A)
{
	return MATRIX_3X2<T> (B.x[0] * A.x[0] + B.x[3] * A.x[1] + B.x[6] * A.x[2], B.x[1] * A.x[0] + B.x[4] * A.x[1] + B.x[7] * A.x[2], B.x[2] * A.x[0] + B.x[5] * A.x[1] + B.x[8] * A.x[2],
			      B.x[0] * A.x[3] + B.x[3] * A.x[4] + B.x[6] * A.x[5], B.x[1] * A.x[3] + B.x[4] * A.x[4] + B.x[7] * A.x[5], B.x[2] * A.x[3] + B.x[5] * A.x[4] + B.x[8] * A.x[5]);
}

template<class T>
inline MATRIX_3X2<T> operator* (const T a, const MATRIX_3X2<T> A)
{
	return MATRIX_3X2<T> (a * A.x[0], a * A.x[1], a * A.x[2], a * A.x[3], a * A.x[4], a * A.x[5]);
}

template<class T>
inline VECTOR_3D<T> operator* (const VECTOR_2D<T>& v, const MATRIX_3X2<T> A)
{
	return VECTOR_3D<T> (v.x * A.x[0] + v.y * A.x[3], v.x * A.x[1] + v.y * A.x[4], v.x * A.x[2] + v.y * A.x[5]);
}

template<class T>
inline std::istream& operator>> (std::istream& input_stream, MATRIX_3X2<T> A)
{
	for (int i = 0; i < 3; i++) for (int j = 0; j < 2; j++) input_stream >> A.x[i + j * 3];

	return input_stream;
}

template<class T>
inline std::ostream& operator<< (std::ostream& output_stream, const MATRIX_3X2<T> A)
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 2; j++) output_stream << A.x[i + j * 3] << " ";

		output_stream << std::endl;
	}

	return output_stream;
}
}
#endif
