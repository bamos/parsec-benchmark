//#####################################################################
// Copyright 2003-2004, Ron Fedkiw, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class UPPER_TRIANGULAR_MATRIX_2X2
//#####################################################################
#ifndef __UPPER_TRIANGULAR_MATRIX_2X2__
#define __UPPER_TRIANGULAR_MATRIX_2X2__

#include "DIAGONAL_MATRIX_2X2.h"
namespace PhysBAM
{

template<class T>
class UPPER_TRIANGULAR_MATRIX_2X2
{
public:
	T x11, x12, x22;

	UPPER_TRIANGULAR_MATRIX_2X2()
	{
		x11 = x12 = x22 = 0;
	}

	template<class T2>
	UPPER_TRIANGULAR_MATRIX_2X2 (const UPPER_TRIANGULAR_MATRIX_2X2<T2>& matrix_input)
		: x11 (matrix_input.x11), x12 (matrix_input.x12), x22 (matrix_input.x22)
	{}

	UPPER_TRIANGULAR_MATRIX_2X2 (const T x11_input, const T x12_input, const T x22_input)
		: x11 (x11_input), x12 (x12_input), x22 (x22_input)
	{}

	UPPER_TRIANGULAR_MATRIX_2X2<T> operator-() const
	{
		return UPPER_TRIANGULAR_MATRIX_2X2<T> (-x11, -x12, -x22);
	}

	UPPER_TRIANGULAR_MATRIX_2X2<T>& operator*= (const UPPER_TRIANGULAR_MATRIX_2X2<T>& A)
	{
		return *this = *this * A;
	}

	UPPER_TRIANGULAR_MATRIX_2X2<T>& operator*= (const T a)
	{
		x11 *= a;
		x12 *= a;
		x22 *= a;
		return *this;
	}

	UPPER_TRIANGULAR_MATRIX_2X2<T>& operator/= (const T a)
	{
		assert (a != 0);
		T s = 1 / a;
		x11 *= s;
		x12 *= s;
		x22 *= s;
		return *this;
	}

	UPPER_TRIANGULAR_MATRIX_2X2<T> operator* (const T a) const
	{
		return UPPER_TRIANGULAR_MATRIX_2X2<T> (a * x11, a * x12, a * x22);
	}

	UPPER_TRIANGULAR_MATRIX_2X2<T> operator/ (const T a) const
	{
		assert (a != 0);
		return *this * (1 / a);
	}

	UPPER_TRIANGULAR_MATRIX_2X2<T> operator* (const UPPER_TRIANGULAR_MATRIX_2X2<T>& A) const
	{
		return UPPER_TRIANGULAR_MATRIX_2X2<T> (x11 * A.x11, x11 * A.x12 + x12 * A.x22, x22 * A.x22);
	}

	UPPER_TRIANGULAR_MATRIX_2X2<T> operator* (const DIAGONAL_MATRIX_2X2<T>& A) const
	{
		return UPPER_TRIANGULAR_MATRIX_2X2<T> (x11 * A.x11, x12 * A.x22, x22 * A.x22);
	}

	SYMMETRIC_MATRIX_2X2<T> Outer_Product_Matrix() const
	{
		return SYMMETRIC_MATRIX_2X2<T> (x11 * x11 + x12 * x12, x12 * x22, x22 * x22);
	}

	T Determinant() const
	{
		return x11 * x22;
	}

	T Trace() const
	{
		return x11 + x22;
	}

	UPPER_TRIANGULAR_MATRIX_2X2<T> Inverse() const
	{
		assert (x11 != 0 && x22 != 0);
		T one_over_x11 = 1 / x11, one_over_x22 = 1 / x22;
		return UPPER_TRIANGULAR_MATRIX_2X2<T> (one_over_x11, -x12 * one_over_x11 * one_over_x22, one_over_x22);
	}

	static UPPER_TRIANGULAR_MATRIX_2X2<T> Identity_Matrix()
	{
		return UPPER_TRIANGULAR_MATRIX_2X2<T> (1, 0, 1);
	}

	T Max_Abs_Element() const
	{
		return max (fabs (x11), fabs (x12), fabs (x22));
	}

	T Triangle_Minimum_Altitude() const
	{
		return Determinant() / max (fabs (x11), sqrt (sqr (x22) + max (sqr (x12), sqr (x12 - x11))));
	}

//#####################################################################
};
// global functions
template<class T>
inline UPPER_TRIANGULAR_MATRIX_2X2<T> operator* (const T a, const UPPER_TRIANGULAR_MATRIX_2X2<T>& A)
{
	return A * a;
}

template<class T>
inline UPPER_TRIANGULAR_MATRIX_2X2<T> operator* (const DIAGONAL_MATRIX_2X2<T>& A, const UPPER_TRIANGULAR_MATRIX_2X2<T>& B)
{
	return UPPER_TRIANGULAR_MATRIX_2X2<T> (A.x11 * B.x11, A.x11 * B.x12, A.x22 * B.x22);
}

template<class T>
inline std::ostream& operator<< (std::ostream& output_stream, const UPPER_TRIANGULAR_MATRIX_2X2<T>& A)
{
	return output_stream << A.x11 << " " << A.x12 << "\n0 " << A.x22 << "\n";
}
//#####################################################################
}
#endif

