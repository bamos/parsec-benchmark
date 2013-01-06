//#####################################################################
// Copyright 2003-2004, Geoffrey Irving, Igor Neverov.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class DIAGONAL_MATRIX_2X2
//#####################################################################
#ifndef __DIAGONAL_MATRIX_2X2__
#define __DIAGONAL_MATRIX_2X2__

#include "VECTOR_2D.h"
#include "../Math_Tools/sign.h"
namespace PhysBAM
{

template<class T> class MATRIX_2X2;
template<class T> class SYMMETRIC_MATRIX_2X2;

template<class T>
class DIAGONAL_MATRIX_2X2
{
public:
	T x11, x22;

	DIAGONAL_MATRIX_2X2()
		: x11 (0), x22 (0)
	{}

	template<class T2>
	DIAGONAL_MATRIX_2X2 (const DIAGONAL_MATRIX_2X2<T2>& matrix_input)
		: x11 ( (T) matrix_input.x11), x22 ( (T) matrix_input.x22)
	{}

	DIAGONAL_MATRIX_2X2 (const T y11, const T y22)
		: x11 (y11), x22 (y22)
	{}

	DIAGONAL_MATRIX_2X2<T> operator-() const
	{
		return DIAGONAL_MATRIX_2X2<T> (-x11, -x22);
	}

	DIAGONAL_MATRIX_2X2<T>& operator+= (const DIAGONAL_MATRIX_2X2<T>& A)
	{
		x11 += A.x11;
		x22 += A.x22;
		return *this;
	}

	DIAGONAL_MATRIX_2X2<T>& operator+= (const T& a)
	{
		x11 += a;
		x22 += a;
		return *this;
	}

	DIAGONAL_MATRIX_2X2<T>& operator-= (const DIAGONAL_MATRIX_2X2<T>& A)
	{
		x11 -= A.x11;
		x22 -= A.x22;
		return *this;
	}

	DIAGONAL_MATRIX_2X2<T>& operator-= (const T& a)
	{
		x11 -= a;
		x22 -= a;
		return *this;
	}

	DIAGONAL_MATRIX_2X2<T>& operator*= (const T a)
	{
		x11 *= a;
		x22 *= a;
		return *this;
	}

	DIAGONAL_MATRIX_2X2<T>& operator/= (const T a)
	{
		assert (a != 0);
		T s = 1 / a;
		x11 *= s;
		x22 *= s;
		return *this;
	}

	DIAGONAL_MATRIX_2X2<T> operator+ (const DIAGONAL_MATRIX_2X2<T>& A) const
	{
		return DIAGONAL_MATRIX_2X2<T> (x11 + A.x11, x22 + A.x22);
	}

	DIAGONAL_MATRIX_2X2<T> operator+ (const T a) const
	{
		return DIAGONAL_MATRIX_2X2<T> (x11 + a, x22 + a);
	}

	DIAGONAL_MATRIX_2X2<T> operator- (const DIAGONAL_MATRIX_2X2<T>& A) const
	{
		return DIAGONAL_MATRIX_2X2<T> (x11 - A.x11, x22 - A.x22);
	}

	DIAGONAL_MATRIX_2X2<T> operator- (const T a) const
	{
		return DIAGONAL_MATRIX_2X2<T> (x11 - a, x22 - a);
	}

	DIAGONAL_MATRIX_2X2<T> operator* (const T a) const
	{
		return DIAGONAL_MATRIX_2X2<T> (a * x11, a * x22);
	}

	DIAGONAL_MATRIX_2X2<T> operator/ (const T a) const
	{
		assert (a != 0);
		return *this * (1 / a);
	}

	VECTOR_2D<T> operator* (const VECTOR_2D<T>& v) const
	{
		return VECTOR_2D<T> (x11 * v.x, x22 * v.y);
	}

	DIAGONAL_MATRIX_2X2<T> operator* (const DIAGONAL_MATRIX_2X2<T>& A) const
	{
		return DIAGONAL_MATRIX_2X2<T> (x11 * A.x11, x22 * A.x22);
	}

	DIAGONAL_MATRIX_2X2<T> operator/ (const DIAGONAL_MATRIX_2X2<T>& A) const
	{
		return DIAGONAL_MATRIX_2X2<T> (x11 / A.x11, x22 / A.x22);
	}

	T Determinant() const
	{
		return x11 * x22;
	}

	DIAGONAL_MATRIX_2X2<T> Inverse() const
	{
		assert (x11 != 0 && x22 != 0);
		return DIAGONAL_MATRIX_2X2<T> (1 / x11, 1 / x22);
	}

	T Trace() const
	{
		return x11 + x22;
	}

	T Min_Element() const
	{
		return min (x11, x22);
	}

	T Max_Element() const
	{
		return max (x11, x22);
	}

	T Max_Abs_Element() const
	{
		return max (fabs (x11), fabs (x22));
	}

	static T Inner_Product (const DIAGONAL_MATRIX_2X2<T>& A, const DIAGONAL_MATRIX_2X2<T>& B)
	{
		return A.x11 * B.x11 + A.x22 * B.x22 + A.x33 * B.x33;
	}

	T Frobenius_Norm_Squared() const
	{
		return sqr (x11) + sqr (x22);
	}

	T Frobenius_Norm() const
	{
		return sqrt (Frobenius_Norm_Squared());
	}

	DIAGONAL_MATRIX_2X2<T> Sqrt() const
	{
		return DIAGONAL_MATRIX_2X2<T> (sqrt (x11), sqrt (x22));
	}

	DIAGONAL_MATRIX_2X2<T> Log() const
	{
		return DIAGONAL_MATRIX_2X2<T> (log (x11), log (x22));
	}

	DIAGONAL_MATRIX_2X2<T> Exp() const
	{
		return DIAGONAL_MATRIX_2X2<T> (exp (x11), exp (x22));
	}

	DIAGONAL_MATRIX_2X2<T> Min (const T a) const
	{
		return DIAGONAL_MATRIX_2X2<T> (min (x11, a), min (x22, a));
	}

	DIAGONAL_MATRIX_2X2<T> Max (const T a) const
	{
		return DIAGONAL_MATRIX_2X2<T> (max (x11, a), max (x22, a));
	}

	DIAGONAL_MATRIX_2X2<T> Abs() const
	{
		return DIAGONAL_MATRIX_2X2<T> (fabs (x11), fabs (x22));
	}

	DIAGONAL_MATRIX_2X2<T> Sign() const
	{
		return DIAGONAL_MATRIX_2X2<T> (sign (x11), sign (x22));
	}

	static DIAGONAL_MATRIX_2X2<T> Identity_Matrix()
	{
		return DIAGONAL_MATRIX_2X2<T> (1, 1);
	}

//#####################################################################
	static T Inner_Product_Conjugate (const DIAGONAL_MATRIX_2X2<T>& A, const MATRIX_2X2<T>& Q, const DIAGONAL_MATRIX_2X2<T> B);
//#####################################################################
};
// global functions
template<class T>
inline DIAGONAL_MATRIX_2X2<T> operator* (const T a, const DIAGONAL_MATRIX_2X2<T>& A)
{
	return A * a;
}

template<class T>
inline DIAGONAL_MATRIX_2X2<T> operator- (const T a, const DIAGONAL_MATRIX_2X2<T>& A)
{
	return -A + a;
}

template<class T>
inline std::ostream& operator<< (std::ostream& output_stream, const DIAGONAL_MATRIX_2X2<T>& A)
{
	return output_stream << A.x11 << " 0\n0 " << A.x22 << " 0\n";
}
//#####################################################################
}
#include "SYMMETRIC_MATRIX_2X2.h"
namespace PhysBAM
{
//#####################################################################
// Function Inner_Product_Conjugate
//#####################################################################
template<class T> T DIAGONAL_MATRIX_2X2<T>::
Inner_Product_Conjugate (const DIAGONAL_MATRIX_2X2<T>& A, const MATRIX_2X2<T>& Q, const DIAGONAL_MATRIX_2X2<T> B)
{
	MATRIX_2X2<T> BQ = B * Q.Transposed();
	return A.x11 * (Q.x[0] * BQ.x[0] + Q.x[2] * BQ.x[1]) + A.x22 * (Q.x[1] * BQ.x[2] + Q.x[3] * BQ.x[3]);
}
//#####################################################################
}
#endif

