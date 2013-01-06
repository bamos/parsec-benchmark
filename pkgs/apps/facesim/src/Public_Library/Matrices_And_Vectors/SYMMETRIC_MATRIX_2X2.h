//#####################################################################
// Copyright 2003-2004, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SYMMETRIC_MATRIX_2X2
//#####################################################################
#ifndef __SYMMETRIC_MATRIX_2X2__
#define __SYMMETRIC_MATRIX_2X2__

#include "VECTOR_2D.h"
namespace PhysBAM
{

template<class T> class MATRIX_2X2;
template<class T> class DIAGONAL_MATRIX_2X2;
template<class T> class UPPER_TRIANGULAR_MATRIX_2X2;

template<class T>
class SYMMETRIC_MATRIX_2X2
{
public:
	T x11, x21, x22;

	SYMMETRIC_MATRIX_2X2()
		: x11 (0), x21 (0), x22 (0)
	{}

	template<class T2>
	SYMMETRIC_MATRIX_2X2 (const SYMMETRIC_MATRIX_2X2<T2>& matrix_input)
		: x11 ( (T) matrix_input.x11), x21 ( (T) matrix_input.x21), x22 ( (T) matrix_input.x22)
	{}

	SYMMETRIC_MATRIX_2X2 (const T y11, const T y21, const T y22)
		: x11 (y11), x21 (y21), x22 (y22)
	{}

	VECTOR_2D<T> Column1() const
	{
		return VECTOR_2D<T> (x11, x21);
	}

	VECTOR_2D<T> Column2() const
	{
		return VECTOR_2D<T> (x21, x22);
	}

	SYMMETRIC_MATRIX_2X2<T> operator-() const
	{
		return SYMMETRIC_MATRIX_2X2<T> (-x11, -x21, -x22);
	}

	SYMMETRIC_MATRIX_2X2<T>& operator+= (const SYMMETRIC_MATRIX_2X2<T>& A)
	{
		x11 += A.x11;
		x21 += A.x21;
		x22 += A.x22;
		return *this;
	}

	SYMMETRIC_MATRIX_2X2<T>& operator+= (const T& a)
	{
		x11 += a;
		x22 += a;
		return *this;
	}

	SYMMETRIC_MATRIX_2X2<T>& operator-= (const SYMMETRIC_MATRIX_2X2<T>& A)
	{
		x11 -= A.x11;
		x21 -= A.x21;
		x22 -= A.x22;
		return *this;
	}

	SYMMETRIC_MATRIX_2X2<T>& operator-= (const T& a)
	{
		x11 -= a;
		x22 -= a;
		return *this;
	}

	SYMMETRIC_MATRIX_2X2<T>& operator*= (const T a)
	{
		x11 *= a;
		x21 *= a;
		x22 *= a;
		return *this;
	}

	SYMMETRIC_MATRIX_2X2<T>& operator/= (const T a)
	{
		assert (a != 0);
		T s = 1 / a;
		x11 *= s;
		x21 *= s;
		x22 *= s;
		return *this;
	}

	SYMMETRIC_MATRIX_2X2<T> operator+ (const SYMMETRIC_MATRIX_2X2<T>& A) const
	{
		return SYMMETRIC_MATRIX_2X2<T> (x11 + A.x11, x21 + A.x21, x22 + A.x22);
	}

	SYMMETRIC_MATRIX_2X2<T> operator+ (const T a) const
	{
		return SYMMETRIC_MATRIX_2X2<T> (x11 + a, x21, x22 + a);
	}

	SYMMETRIC_MATRIX_2X2<T> operator- (const SYMMETRIC_MATRIX_2X2<T>& A) const
	{
		return SYMMETRIC_MATRIX_2X2<T> (x11 - A.x11, x21 - A.x21, x22 - A.x22);
	}

	SYMMETRIC_MATRIX_2X2<T> operator- (const T a) const
	{
		return SYMMETRIC_MATRIX_2X2<T> (x11 - a, x21, x22 - a);
	}

	SYMMETRIC_MATRIX_2X2<T> operator* (const T a) const
	{
		return SYMMETRIC_MATRIX_2X2<T> (a * x11, a * x21, a * x22);
	}

	SYMMETRIC_MATRIX_2X2<T> operator/ (const T a) const
	{
		assert (a != 0);
		return *this * (1 / a);
	}

	VECTOR_2D<T> operator* (const VECTOR_2D<T>& v) const
	{
		return VECTOR_2D<T> (x11 * v.x + x21 * v.y, x21 * v.x + x22 * v.y);
	}

	T Determinant() const
	{
		return x11 * x22 - x21 * x21;
	}

	SYMMETRIC_MATRIX_2X2<T> Inverse() const
	{
		return SYMMETRIC_MATRIX_2X2<T> (x22, -x21, x11) / Determinant();
	}

	VECTOR_2D<T> Solve_Linear_System (const VECTOR_2D<T>& b) const
	{
		return SYMMETRIC_MATRIX_2X2<T> (x22, -x21, x11) * b / Determinant();
	}

	T Trace() const
	{
		return x11 + x22;
	}

	static T Inner_Product (const SYMMETRIC_MATRIX_2X2<T>& A, const SYMMETRIC_MATRIX_2X2<T>& B)
	{
		return A.x11 * B.x11 + A.x22 * B.x22 + 2 * A.x21 * B.x21;
	}

	T Frobenius_Norm_Squared() const
	{
		return x11 * x11 + x22 * x22 + 2 * x21 * x21;
	}

	T Frobenius_Norm() const
	{
		return sqrt (Frobenius_Norm_Squared());
	}

	SYMMETRIC_MATRIX_2X2<T> Cofactor_Matrix()
	{
		return SYMMETRIC_MATRIX_2X2<T> (x22, -x21, x11);
	}

	T Max_Abs_Element() const
	{
		return max (fabs (x11), fabs (x21), fabs (x22));
	}

	static SYMMETRIC_MATRIX_2X2<T> Outer_Product (const VECTOR_2D<T>& u)
	{
		return SYMMETRIC_MATRIX_2X2<T> (u.x * u.x, u.x * u.y, u.y * u.y);
	}

	SYMMETRIC_MATRIX_2X2<T> Log() const
	{
		DIAGONAL_MATRIX_2X2<T> D;
		MATRIX_2X2<T> Q;
		Solve_Eigenproblem (D, Q);
		return Conjugate (Q, D.Log());
	}

	SYMMETRIC_MATRIX_2X2<T> Exp() const
	{
		DIAGONAL_MATRIX_2X2<T> D;
		MATRIX_2X2<T> Q;
		Solve_Eigenproblem (D, Q);
		return Conjugate (Q, D.Exp());
	}

	static SYMMETRIC_MATRIX_2X2<T> Identity_Matrix()
	{
		return SYMMETRIC_MATRIX_2X2<T> (1, 0, 1);
	}

	template<class RW>
	void Read (std::istream &input_stream)
	{
		Read_Binary<RW> (input_stream, x11, x21, x22);
	}

	template<class RW>
	void Write (std::ostream &output_stream) const
	{
		Write_Binary<RW> (output_stream, x11, x21, x22);
	}

//#####################################################################
	void Solve_Eigenproblem (DIAGONAL_MATRIX_2X2<T>& eigenvalues, MATRIX_2X2<T>& eigenvectors) const;
	MATRIX_2X2<T> operator* (const UPPER_TRIANGULAR_MATRIX_2X2<T>& A) const;
	MATRIX_2X2<T> Multiply_With_Transpose (const UPPER_TRIANGULAR_MATRIX_2X2<T>& A) const;
	static SYMMETRIC_MATRIX_2X2<T> Conjugate (const MATRIX_2X2<T>& A, const DIAGONAL_MATRIX_2X2<T>& B);
	static SYMMETRIC_MATRIX_2X2<T> Conjugate (const MATRIX_2X2<T>& A, const SYMMETRIC_MATRIX_2X2<T>& B);
	static SYMMETRIC_MATRIX_2X2<T> Conjugate (const UPPER_TRIANGULAR_MATRIX_2X2<T>& A, const SYMMETRIC_MATRIX_2X2<T>& B);
//#####################################################################
};
// global functions
template<class T>
inline SYMMETRIC_MATRIX_2X2<T> operator* (const T a, const SYMMETRIC_MATRIX_2X2<T>& A)
{
	return A * a;
}

template<class T>
inline SYMMETRIC_MATRIX_2X2<T> operator- (const T a, const SYMMETRIC_MATRIX_2X2<T>& A)
{
	return -A + a;
}

template<class T>
inline std::ostream& operator<< (std::ostream& output_stream, const SYMMETRIC_MATRIX_2X2<T>& A)
{
	output_stream << A.x11 << "\n" << A.x21 << " " << A.x22 << "\n";
	return output_stream;
}
//#####################################################################
}
#include "MATRIX_2X2.h"
#include "UPPER_TRIANGULAR_MATRIX_2X2.h"
namespace PhysBAM
{
//#####################################################################
// Function Solve_Eigenproblem
//#####################################################################
template<class T> void SYMMETRIC_MATRIX_2X2<T>::
Solve_Eigenproblem (DIAGONAL_MATRIX_2X2<T>& eigenvalues, MATRIX_2X2<T>& eigenvectors) const
{
	T cosine, sine, da;

	if (x21 == 0)
	{
		cosine = 1;
		sine = 0;
		da = 0;
	}
	else
	{
		T theta = (T).5 * (x22 - x11) / x21, t = 1 / (fabs (theta) + sqrt (1 + sqr (theta)));

		if (theta < 0) t = -t;

		cosine = 1 / sqrt (1 + t * t);
		sine = t * cosine;
		da = t * x21;
	}

	eigenvalues = DIAGONAL_MATRIX_2X2<T> (x11 - da, x22 + da);

	if (eigenvalues.x11 >= eigenvalues.x22) eigenvectors = MATRIX_2X2<T> (cosine, -sine, sine, cosine);
	else
	{
		exchange (eigenvalues.x11, eigenvalues.x22);
		eigenvectors = MATRIX_2X2<T> (sine, cosine, -cosine, sine);
	}
}
//#####################################################################
// Function Conjugate
//#####################################################################
template<class T> SYMMETRIC_MATRIX_2X2<T> SYMMETRIC_MATRIX_2X2<T>::
Conjugate (const MATRIX_2X2<T>& A, const DIAGONAL_MATRIX_2X2<T>& B)
{
	MATRIX_2X2<T> BA = B * A.Transposed();
	return SYMMETRIC_MATRIX_2X2<T> (A.x[0] * BA.x[0] + A.x[2] * BA.x[1], A.x[1] * BA.x[0] + A.x[3] * BA.x[1], A.x[1] * BA.x[2] + A.x[3] * BA.x[3]);
}
//#####################################################################
// Function Conjugate
//#####################################################################
template<class T> SYMMETRIC_MATRIX_2X2<T> SYMMETRIC_MATRIX_2X2<T>::
Conjugate (const MATRIX_2X2<T>& A, const SYMMETRIC_MATRIX_2X2<T>& B)
{
	MATRIX_2X2<T> BA = (A * B).Transposed();
	return SYMMETRIC_MATRIX_2X2<T> (A.x[0] * BA.x[0] + A.x[2] * BA.x[1], A.x[1] * BA.x[0] + A.x[3] * BA.x[1], A.x[1] * BA.x[2] + A.x[3] * BA.x[3]);
}
//#####################################################################
// Function Conjugate
//#####################################################################
template<class T> SYMMETRIC_MATRIX_2X2<T> SYMMETRIC_MATRIX_2X2<T>::
Conjugate (const UPPER_TRIANGULAR_MATRIX_2X2<T>& A, const SYMMETRIC_MATRIX_2X2<T>& B)
{
	MATRIX_2X2<T> BA = B.Multiply_With_Transpose (A);
	return SYMMETRIC_MATRIX_2X2<T> (A.x11 * BA.x[0] + A.x12 * BA.x[1], A.x22 * BA.x[1], A.x22 * BA.x[3]);
}
//#####################################################################
// Function Multiply_With_Transpose
//#####################################################################
template<class T> MATRIX_2X2<T> SYMMETRIC_MATRIX_2X2<T>::
Multiply_With_Transpose (const UPPER_TRIANGULAR_MATRIX_2X2<T>& A) const
{
	return (A**this).Transposed();
}
//#####################################################################
// Function operator*
//#####################################################################
template<class T> MATRIX_2X2<T> SYMMETRIC_MATRIX_2X2<T>::
operator* (const UPPER_TRIANGULAR_MATRIX_2X2<T>& A) const
{
	return MATRIX_2X2<T> (x11 * A.x11, x21 * A.x11, x11 * A.x12 + x21 * A.x22, x21 * A.x12 + x22 * A.x22);
}
//#####################################################################
// Function operator*
//#####################################################################
template<class T> MATRIX_2X2<T> operator* (const DIAGONAL_MATRIX_2X2<T>& D, const SYMMETRIC_MATRIX_2X2<T>& A)
{
	return MATRIX_2X2<T> (D.x11 * A.x11, D.x22 * A.x21, D.x11 * A.x21, D.x22 * A.x22);
}
//#####################################################################
// Function operator*
//#####################################################################
template<class T>
inline MATRIX_2X2<T> operator* (const UPPER_TRIANGULAR_MATRIX_2X2<T>& A, const SYMMETRIC_MATRIX_2X2<T>& B)
{
	return MATRIX_2X2<T> (A.x11 * B.x11 + A.x12 * B.x21, A.x22 * B.x21, A.x11 * B.x21 + A.x12 * B.x22, A.x22 * B.x22);
}
//#####################################################################
// Function operator*
//#####################################################################
template<class T>
inline MATRIX_2X2<T> operator* (const SYMMETRIC_MATRIX_2X2<T>& A, const MATRIX_2X2<T>& B)
{
	return MATRIX_2X2<T> (A.x11 * B.x[0] + A.x21 * B.x[1], A.x21 * B.x[0] + A.x22 * B.x[1], A.x11 * B.x[2] + A.x21 * B.x[3], A.x21 * B.x[2] + A.x22 * B.x[3]);
}
//#####################################################################
}
#endif

