//#####################################################################
// Copyright 2003, Geoffrey Irving, Neil Molino.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class DIAGONAL_MATRIX_3X3
//#####################################################################
#ifndef __DIAGONAL_MATRIX_3X3__
#define __DIAGONAL_MATRIX_3X3__

#include "VECTOR_3D.h"
namespace PhysBAM
{

template<class T> class MATRIX_3X3;
template<class T> class SYMMETRIC_MATRIX_3X3;

template<class T>
class DIAGONAL_MATRIX_3X3
{
public:
	T x11, x22, x33;

	DIAGONAL_MATRIX_3X3()
		: x11 (0), x22 (0), x33 (0)
	{}

	template<class T2>
	DIAGONAL_MATRIX_3X3 (const DIAGONAL_MATRIX_3X3<T2>& matrix_input)
		: x11 ( (T) matrix_input.x11), x22 ( (T) matrix_input.x22), x33 ( (T) matrix_input.x33)
	{}

	DIAGONAL_MATRIX_3X3 (const T y11, const T y22, const T y33)
		: x11 (y11), x22 (y22), x33 (y33)
	{}

	DIAGONAL_MATRIX_3X3<T> operator-() const
	{
		return DIAGONAL_MATRIX_3X3<T> (-x11, -x22, -x33);
	}

	DIAGONAL_MATRIX_3X3<T>& operator+= (const DIAGONAL_MATRIX_3X3<T>& A)
	{
		x11 += A.x11;
		x22 += A.x22;
		x33 += A.x33;
		return *this;
	}

	DIAGONAL_MATRIX_3X3<T>& operator+= (const T& a)
	{
		x11 += a;
		x22 += a;
		x33 += a;
		return *this;
	}

	DIAGONAL_MATRIX_3X3<T>& operator-= (const DIAGONAL_MATRIX_3X3<T>& A)
	{
		x11 -= A.x11;
		x22 -= A.x22;
		x33 -= A.x33;
		return *this;
	}

	DIAGONAL_MATRIX_3X3<T>& operator-= (const T& a)
	{
		x11 -= a;
		x22 -= a;
		x33 -= a;
		return *this;
	}

	DIAGONAL_MATRIX_3X3<T>& operator*= (const T a)
	{
		x11 *= a;
		x22 *= a;
		x33 *= a;
		return *this;
	}

	DIAGONAL_MATRIX_3X3<T>& operator/= (const T a)
	{
		assert (a != 0);
		T s = 1 / a;
		x11 *= s;
		x22 *= s;
		x33 *= s;
		return *this;
	}

	DIAGONAL_MATRIX_3X3<T> operator+ (const DIAGONAL_MATRIX_3X3<T>& A) const
	{
		return DIAGONAL_MATRIX_3X3<T> (x11 + A.x11, x22 + A.x22, x33 + A.x33);
	}

	MATRIX_3X3<T> operator+ (const MATRIX_3X3<T>& A) const
	{
		return MATRIX_3X3<T> (x11 + A.x[0], A.x[1], A.x[2], A.x[3], x22 + A.x[4], A.x[5], A.x[6], A.x[7], x33 + A.x[8]);
	}

	DIAGONAL_MATRIX_3X3<T> operator+ (const T a) const
	{
		return DIAGONAL_MATRIX_3X3<T> (x11 + a, x22 + a, x33 + a);
	}

	DIAGONAL_MATRIX_3X3<T> operator- (const DIAGONAL_MATRIX_3X3<T>& A) const
	{
		return DIAGONAL_MATRIX_3X3<T> (x11 - A.x11, x22 - A.x22, x33 - A.x33);
	}

	DIAGONAL_MATRIX_3X3<T> operator- (const T a) const
	{
		return DIAGONAL_MATRIX_3X3<T> (x11 - a, x22 - a, x33 - a);
	}

	DIAGONAL_MATRIX_3X3<T> operator* (const T a) const
	{
		return DIAGONAL_MATRIX_3X3<T> (a * x11, a * x22, a * x33);
	}

	DIAGONAL_MATRIX_3X3<T> operator/ (const T a) const
	{
		assert (a != 0);
		return *this * (1 / a);
	}

	VECTOR_3D<T> operator* (const VECTOR_3D<T>& v) const
	{
		return VECTOR_3D<T> (x11 * v.x, x22 * v.y, x33 * v.z);
	}

	DIAGONAL_MATRIX_3X3<T> operator* (const DIAGONAL_MATRIX_3X3<T>& A) const
	{
		return DIAGONAL_MATRIX_3X3<T> (x11 * A.x11, x22 * A.x22, x33 * A.x33);
	}

	DIAGONAL_MATRIX_3X3<T> operator/ (const DIAGONAL_MATRIX_3X3<T>& A) const
	{
		return DIAGONAL_MATRIX_3X3<T> (x11 / A.x11, x22 / A.x22, x33 / A.x33);
	}

	T Determinant() const
	{
		return x11 * x22 * x33;
	}

	DIAGONAL_MATRIX_3X3<T> Inverse() const
	{
		assert (x11 != 0 && x22 != 0 && x33 != 0);
		return DIAGONAL_MATRIX_3X3<T> (1 / x11, 1 / x22, 1 / x33);
	}

	VECTOR_3D<T> Inverse_Times (const VECTOR_3D<T>& v) const
	{
		assert (x11 != 0 && x22 != 0 && x33 != 0);
		return VECTOR_3D<T> (v.x / x11, v.y / x22, v.z / x33);
	}

	T Trace() const
	{
		return x11 + x22 + x33;
	}

	T Min_Element() const
	{
		return min (x11, x22, x33);
	}

	T Max_Element() const
	{
		return max (x11, x22, x33);
	}

	T Max_Abs_Element() const
	{
		return max (fabs (x11), fabs (x22), fabs (x33));
	}

	static T Inner_Product (const DIAGONAL_MATRIX_3X3<T>& A, const DIAGONAL_MATRIX_3X3<T>& B)
	{
		return A.x11 * B.x11 + A.x22 * B.x22 + A.x33 * B.x33;
	}

	T Inner_Product (const VECTOR_3D<T>& a, const VECTOR_3D<T>& b) const // inner product with respect to this matrix
	{
		return a.x * x11 * b.x + a.y * x22 * b.y + a.z * x33 * b.z;
	}

	T Inverse_Inner_Product (const VECTOR_3D<T>& a, const VECTOR_3D<T>& b) const // inner product with respect to the inverse of this matrix
	{
		assert (x11 != 0 && x22 != 0 && x33 != 0);
		return a.x / x11 * b.x + a.y / x22 * b.y + a.z / x33 * b.z;
	}

	T Frobenius_Norm_Squared() const
	{
		return sqr (x11) + sqr (x22) + sqr (x33);
	}

	T Frobenius_Norm() const
	{
		return sqrt (Frobenius_Norm_Squared());
	}

	DIAGONAL_MATRIX_3X3<T> Sqrt() const
	{
		return DIAGONAL_MATRIX_3X3<T> (sqrt (x11), sqrt (x22), sqrt (x33));
	}

	DIAGONAL_MATRIX_3X3<T> Log() const
	{
		return DIAGONAL_MATRIX_3X3<T> (log (x11), log (x22), log (x33));
	}

	DIAGONAL_MATRIX_3X3<T> Exp() const
	{
		return DIAGONAL_MATRIX_3X3<T> (exp (x11), exp (x22), exp (x33));
	}

	DIAGONAL_MATRIX_3X3<T> Min (const T a) const
	{
		return DIAGONAL_MATRIX_3X3<T> (min (x11, a), min (x22, a), min (x33, a));
	}

	DIAGONAL_MATRIX_3X3<T> Max (const T a) const
	{
		return DIAGONAL_MATRIX_3X3<T> (max (x11, a), max (x22, a), max (x33, a));
	}

	DIAGONAL_MATRIX_3X3<T> Abs() const
	{
		return DIAGONAL_MATRIX_3X3<T> (fabs (x11), fabs (x22), fabs (x33));
	}

	DIAGONAL_MATRIX_3X3<T> Sign() const
	{
		return DIAGONAL_MATRIX_3X3<T> (sign (x11), sign (x22), sign (x33));
	}

	static DIAGONAL_MATRIX_3X3<T> Identity_Matrix()
	{
		return DIAGONAL_MATRIX_3X3<T> (1, 1, 1);
	}

	template<class RW>
	void Read (std::istream &input_stream)
	{
		Read_Binary<RW> (input_stream, x11, x22, x33);
	}

	template<class RW>
	void Write (std::ostream &output_stream) const
	{
		Write_Binary<RW> (output_stream, x11, x22, x33);
	}

//#####################################################################
	MATRIX_3X3<T> Multiply_With_Transpose (const MATRIX_3X3<T>& A) const;
	static T Inner_Product_Conjugate (const DIAGONAL_MATRIX_3X3<T>& A, const MATRIX_3X3<T>& Q, const DIAGONAL_MATRIX_3X3<T> B);
//#####################################################################
};
// global functions
template<class T>
inline DIAGONAL_MATRIX_3X3<T> operator* (const T a, const DIAGONAL_MATRIX_3X3<T>& A)
{
	return A * a;
}

template<class T>
inline DIAGONAL_MATRIX_3X3<T> operator+ (const T a, const DIAGONAL_MATRIX_3X3<T>& A)
{
	return A + a;
}

template<class T>
inline DIAGONAL_MATRIX_3X3<T> operator- (const T a, const DIAGONAL_MATRIX_3X3<T>& A)
{
	return -A + a;
}

template<class T>
inline std::ostream& operator<< (std::ostream& output_stream, const DIAGONAL_MATRIX_3X3<T>& A)
{
	return output_stream << A.x11 << " " << A.x22 << " " << A.x33;
}
//#####################################################################
}
#include "SYMMETRIC_MATRIX_3X3.h"
namespace PhysBAM
{
//#####################################################################
// Function Multiply_With_Transpose
//#####################################################################
template<class T> MATRIX_3X3<T> DIAGONAL_MATRIX_3X3<T>::
Multiply_With_Transpose (const MATRIX_3X3<T>& A) const
{
	return MATRIX_3X3<T> (x11 * A.x[0], x22 * A.x[3], x33 * A.x[6], x11 * A.x[1], x22 * A.x[4], x33 * A.x[7], x11 * A.x[2], x22 * A.x[5], x33 * A.x[8]);
}
//#####################################################################
// Function Inner_Product_Conjugate
//#####################################################################
template<class T> T DIAGONAL_MATRIX_3X3<T>::
Inner_Product_Conjugate (const DIAGONAL_MATRIX_3X3<T>& A, const MATRIX_3X3<T>& Q, const DIAGONAL_MATRIX_3X3<T> B)
{
	MATRIX_3X3<T> BQ = B * Q.Transposed();
	return A.x11 * (Q.x[0] * BQ.x[0] + Q.x[3] * BQ.x[1] + Q.x[6] * BQ.x[2]) + A.x22 * (Q.x[1] * BQ.x[3] + Q.x[4] * BQ.x[4] + Q.x[7] * BQ.x[5]) + A.x33 * (Q.x[2] * BQ.x[6] + Q.x[5] * BQ.x[7] + Q.x[8] * BQ.x[8]);
}
//#####################################################################
}
#endif

