//#####################################################################
// Copyright 2003-2004, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class UPPER_TRIANGULAR_MATRIX_3X3
//#####################################################################
#ifndef __UPPER_TRIANGULAR_MATRIX_3X3__
#define __UPPER_TRIANGULAR_MATRIX_3X3__

#include "DIAGONAL_MATRIX_3X3.h"
namespace PhysBAM
{

template<class T>
class UPPER_TRIANGULAR_MATRIX_3X3
{
public:
	T x11, x12, x22, x13, x23, x33;

	UPPER_TRIANGULAR_MATRIX_3X3()
	{
		x11 = x12 = x22 = x13 = x23 = x33 = 0;
	}

	template<class T2>
	UPPER_TRIANGULAR_MATRIX_3X3 (const UPPER_TRIANGULAR_MATRIX_3X3<T2>& matrix_input)
		: x11 (matrix_input.x11), x12 (matrix_input.x12), x22 (matrix_input.x22), x13 (matrix_input.x13), x23 (matrix_input.x23), x33 (matrix_input.x33)
	{}

	UPPER_TRIANGULAR_MATRIX_3X3 (const T x11_input, const T x12_input, const T x22_input, const T x13_input, const T x23_input, const T x33_input)
		: x11 (x11_input), x12 (x12_input), x22 (x22_input), x13 (x13_input), x23 (x23_input), x33 (x33_input)
	{}

	UPPER_TRIANGULAR_MATRIX_3X3<T> operator-() const
	{
		return UPPER_TRIANGULAR_MATRIX_3X3<T> (-x11, -x12, -x22, -x13, -x23, -x33);
	}

	UPPER_TRIANGULAR_MATRIX_3X3<T>& operator*= (const UPPER_TRIANGULAR_MATRIX_3X3<T>& A)
	{
		return *this = *this * A;
	}

	UPPER_TRIANGULAR_MATRIX_3X3<T>& operator*= (const T a)
	{
		x11 *= a;
		x12 *= a;
		x22 *= a;
		x13 *= a;
		x23 *= a;
		x33 *= a;
		return *this;
	}

	UPPER_TRIANGULAR_MATRIX_3X3<T>& operator/= (const T a)
	{
		assert (a != 0);
		T s = 1 / a;
		x11 *= s;
		x12 *= s;
		x22 *= s;
		x13 *= s;
		x23 *= s;
		x33 *= s;
		return *this;
	}

	UPPER_TRIANGULAR_MATRIX_3X3<T> operator* (const T a) const
	{
		return UPPER_TRIANGULAR_MATRIX_3X3<T> (a * x11, a * x12, a * x22, a * x13, a * x23, a * x33);
	}

	UPPER_TRIANGULAR_MATRIX_3X3<T> operator/ (const T a) const
	{
		assert (a != 0);
		T s = 1 / a;
		return UPPER_TRIANGULAR_MATRIX_3X3<T> (s * x11, s * x12, s * x22, s * x13, s * x23, s * x33);
	}

	UPPER_TRIANGULAR_MATRIX_3X3<T> operator* (const UPPER_TRIANGULAR_MATRIX_3X3<T>& A) const
	{
		return UPPER_TRIANGULAR_MATRIX_3X3<T> (x11 * A.x11, x11 * A.x12 + x12 * A.x22, x22 * A.x22, x11 * A.x13 + x12 * A.x23 + x13 * A.x33, x22 * A.x23 + x23 * A.x33, x33 * A.x33);
	}

	UPPER_TRIANGULAR_MATRIX_3X3<T> operator* (const DIAGONAL_MATRIX_3X3<T>& A) const
	{
		return UPPER_TRIANGULAR_MATRIX_3X3<T> (x11 * A.x11, x12 * A.x22, x22 * A.x22, x13 * A.x33, x23 * A.x33, x33 * A.x33);
	}

	T Determinant() const
	{
		return x11 * x22 * x33;
	}

	T Trace() const
	{
		return x11 + x22 + x33;
	}

	UPPER_TRIANGULAR_MATRIX_3X3<T> Inverse() const
	{
		T determinant = x11 * x22 * x33;
		assert (determinant != 0);
		T s = 1 / determinant;
		return s * UPPER_TRIANGULAR_MATRIX_3X3<T> (x22 * x33, -x12 * x33, x11 * x33, x12 * x23 - x22 * x13, -x11 * x23, x11 * x22);
	}

	static UPPER_TRIANGULAR_MATRIX_3X3<T> Identity_Matrix()
	{
		return UPPER_TRIANGULAR_MATRIX_3X3<T> (1, 0, 1, 0, 0, 1);
	}

	T Max_Abs_Element() const
	{
		return max (fabs (x11), fabs (x12), fabs (x22), fabs (x13), fabs (x23), fabs (x33));
	}

	T Tetrahedron_Minimum_Altitude() const
	{
		return MATRIX_3X3<T> (*this).Tetrahedron_Minimum_Altitude();
	}

//#####################################################################
};
// global functions
template<class T>
inline UPPER_TRIANGULAR_MATRIX_3X3<T> operator* (const T a, const UPPER_TRIANGULAR_MATRIX_3X3<T>& A)
{
	return A * a;
}

template<class T>
inline UPPER_TRIANGULAR_MATRIX_3X3<T> operator* (const DIAGONAL_MATRIX_3X3<T>& A, const UPPER_TRIANGULAR_MATRIX_3X3<T>& B)
{
	return UPPER_TRIANGULAR_MATRIX_3X3<T> (A.x11 * B.x11, A.x11 * B.x12, A.x22 * B.x22, A.x11 * B.x13, A.x22 * B.x23, A.x33 * B.x33);
}

template<class T>
inline std::ostream& operator<< (std::ostream& output_stream, const UPPER_TRIANGULAR_MATRIX_3X3<T>& A)
{
	return output_stream << A.x11 << " " << A.x12 << " " << A.x13 << "\n0 " << A.x22 << " " << A.x23 << "\n0 0 " << A.x33 << "\n";
}
//#####################################################################
}
#endif

