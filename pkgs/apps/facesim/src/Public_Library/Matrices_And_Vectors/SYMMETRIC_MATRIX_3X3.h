//#####################################################################
// Copyright 2003-2004, Ronald Fedkiw, Geoffrey Irving, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SYMMETRIC_MATRIX_3X3
//#####################################################################
#ifndef __SYMMETRIC_MATRIX_3X3__
#define __SYMMETRIC_MATRIX_3X3__

#include "VECTOR_3D.h"
#include "../Read_Write/READ_WRITE_FUNCTIONS.h"
namespace PhysBAM
{

template<class T> class MATRIX_3X3;
template<class T> class MATRIX_3X2;
template<class T> class DIAGONAL_MATRIX_3X3;
template<class T> class DIAGONAL_MATRIX_2X2;
template<class T> class UPPER_TRIANGULAR_MATRIX_3X3;

template<class T>
class SYMMETRIC_MATRIX_3X3
{
public:
	T x11, x21, x31, x22, x32, x33;

	SYMMETRIC_MATRIX_3X3()
		: x11 (0), x21 (0), x31 (0), x22 (0), x32 (0), x33 (0)
	{}

	template<class T2>
	SYMMETRIC_MATRIX_3X3 (const SYMMETRIC_MATRIX_3X3<T2>& matrix_input)
		: x11 ( (T) matrix_input.x11), x21 ( (T) matrix_input.x21), x31 ( (T) matrix_input.x31), x22 ( (T) matrix_input.x22), x32 ( (T) matrix_input.x32), x33 ( (T) matrix_input.x33)
	{}

	SYMMETRIC_MATRIX_3X3 (const T y11, const T y21, const T y31, const T y22, const T y32, const T y33)
		: x11 (y11), x21 (y21), x31 (y31), x22 (y22), x32 (y32), x33 (y33)
	{}

	VECTOR_3D<T> Column1() const
	{
		return VECTOR_3D<T> (x11, x21, x31);
	}

	VECTOR_3D<T> Column2() const
	{
		return VECTOR_3D<T> (x21, x22, x32);
	}

	VECTOR_3D<T> Column3() const
	{
		return VECTOR_3D<T> (x31, x32, x33);
	}

	SYMMETRIC_MATRIX_3X3<T> operator-() const
	{
		return SYMMETRIC_MATRIX_3X3<T> (-x11, -x21, -x31, -x22, -x32, -x33);
	}

	SYMMETRIC_MATRIX_3X3<T>& operator+= (const SYMMETRIC_MATRIX_3X3<T>& A)
	{
		x11 += A.x11;
		x21 += A.x21;
		x31 += A.x31;
		x22 += A.x22;
		x32 += A.x32;
		x33 += A.x33;
		return *this;
	}

	SYMMETRIC_MATRIX_3X3<T>& operator+= (const T& a)
	{
		x11 += a;
		x22 += a;
		x33 += a;
		return *this;
	}

	SYMMETRIC_MATRIX_3X3<T>& operator-= (const SYMMETRIC_MATRIX_3X3<T>& A)
	{
		x11 -= A.x11;
		x21 -= A.x21;
		x31 -= A.x31;
		x22 -= A.x22;
		x32 -= A.x32;
		x33 -= A.x33;
		return *this;
	}

	SYMMETRIC_MATRIX_3X3<T>& operator-= (const T& a)
	{
		x11 -= a;
		x22 -= a;
		x33 -= a;
		return *this;
	}

	SYMMETRIC_MATRIX_3X3<T>& operator*= (const T a)
	{
		x11 *= a;
		x21 *= a;
		x31 *= a;
		x22 *= a;
		x32 *= a;
		x33 *= a;
		return *this;
	}

	SYMMETRIC_MATRIX_3X3<T>& operator/= (const T a)
	{
		assert (a != 0);
		T s = 1 / a;
		x11 *= s;
		x21 *= s;
		x31 *= s;
		x22 *= s;
		x32 *= s;
		x33 *= s;
		return *this;
	}

	SYMMETRIC_MATRIX_3X3<T> operator+ (const SYMMETRIC_MATRIX_3X3<T>& A) const
	{
		return SYMMETRIC_MATRIX_3X3<T> (x11 + A.x11, x21 + A.x21, x31 + A.x31, x22 + A.x22, x32 + A.x32, x33 + A.x33);
	}

	SYMMETRIC_MATRIX_3X3<T> operator+ (const T a) const
	{
		return SYMMETRIC_MATRIX_3X3<T> (x11 + a, x21, x31, x22 + a, x32, x33 + a);
	}

	SYMMETRIC_MATRIX_3X3<T> operator- (const SYMMETRIC_MATRIX_3X3<T>& A) const
	{
		return SYMMETRIC_MATRIX_3X3<T> (x11 - A.x11, x21 - A.x21, x31 - A.x31, x22 - A.x22, x32 - A.x32, x33 - A.x33);
	}

	SYMMETRIC_MATRIX_3X3<T> operator- (const T a) const
	{
		return SYMMETRIC_MATRIX_3X3<T> (x11 - a, x21, x31, x22 - a, x32, x33 - a);
	}

	SYMMETRIC_MATRIX_3X3<T> operator* (const T a) const
	{
		return SYMMETRIC_MATRIX_3X3<T> (a * x11, a * x21, a * x31, a * x22, a * x32, a * x33);
	}

	SYMMETRIC_MATRIX_3X3<T> operator/ (const T a) const
	{
		assert (a != 0);
		T s = 1 / a;
		return SYMMETRIC_MATRIX_3X3<T> (s * x11, s * x21, s * x31, s * x22, s * x32, s * x33);
	}

	VECTOR_3D<T> operator* (const VECTOR_3D<T>& v) const
	{
		return VECTOR_3D<T> (x11 * v.x + x21 * v.y + x31 * v.z, x21 * v.x + x22 * v.y + x32 * v.z, x31 * v.x + x32 * v.y + x33 * v.z);
	}

	T Determinant() const
	{
		return x11 * (x22 * x33 - x32 * x32) + x21 * (2 * x32 * x31 - x21 * x33) - x31 * x22 * x31;
	}

	SYMMETRIC_MATRIX_3X3<T> Inverse() const
	{
		T cofactor11 = x22 * x33 - x32 * x32, cofactor12 = x32 * x31 - x21 * x33, cofactor13 = x21 * x32 - x22 * x31;
		return SYMMETRIC_MATRIX_3X3<T> (cofactor11, cofactor12, cofactor13, x11 * x33 - x31 * x31, x21 * x31 - x11 * x32, x11 * x22 - x21 * x21) / (x11 * cofactor11 + x21 * cofactor12 + x31 * cofactor13);
	}

	VECTOR_3D<T> Solve_Linear_System (const VECTOR_3D<T> b) const
	{
		T cofactor11 = x22 * x33 - x32 * x32, cofactor12 = x32 * x31 - x21 * x33, cofactor13 = x21 * x32 - x22 * x31;
		return SYMMETRIC_MATRIX_3X3<T> (cofactor11, cofactor12, cofactor13, x11 * x33 - x31 * x31, x21 * x31 - x11 * x32, x11 * x22 - x21 * x21) * b / (x11 * cofactor11 + x21 * cofactor12 + x31 * cofactor13);
	}

	SYMMETRIC_MATRIX_3X3<T> Squared() const
	{
		return SYMMETRIC_MATRIX_3X3<T> (x11 * x11 + x21 * x21 + x31 * x31, x21 * x11 + x22 * x21 + x32 * x31, x31 * x11 + x32 * x21 + x33 * x31, x21 * x21 + x22 * x22 + x32 * x32, x31 * x21 + x32 * x22 + x33 * x32, x31 * x31 + x32 * x32 + x33 * x33);
	}

	T Trace() const
	{
		return x11 + x22 + x33;
	}

	static T Inner_Product (const SYMMETRIC_MATRIX_3X3<T>& A, const SYMMETRIC_MATRIX_3X3<T>& B)
	{
		return A.x11 * B.x11 + A.x22 * B.x22 + A.x33 * B.x33 + 2 * (A.x21 * B.x21 + A.x31 * B.x31 + A.x32 * B.x32);
	}

	T Frobenius_Norm_Squared() const
	{
		return x11 * x11 + x22 * x22 + x33 * x33 + 2 * (x21 * x21 + x31 * x31 + x32 * x32);
	}

	T Frobenius_Norm() const
	{
		return sqrt (Frobenius_Norm_Squared());
	}

	SYMMETRIC_MATRIX_3X3<T> Cofactor_Matrix()
	{
		return SYMMETRIC_MATRIX_3X3<T> (x22 * x33 - x32 * x32, -x21 * x33 + x32 * x31, x21 * x32 - x22 * x31, x11 * x33 - x31 * x31, -x11 * x32 + x21 * x31, x11 * x22 - x21 * x21);
	}

	VECTOR_3D<T> Largest_Column() const
	{
		T sqr11 = sqr (x11), sqr12 = sqr (x21), sqr13 = sqr (x31), sqr22 = sqr (x22), sqr23 = sqr (x32), sqr33 = sqr (x33);
		T scale1 = sqr11 + sqr12 + sqr13, scale2 = sqr12 + sqr22 + sqr23, scale3 = sqr13 + sqr23 + sqr33;
		return scale1 > scale2 ? (scale1 > scale3 ? VECTOR_3D<T> (x11, x21, x31) : VECTOR_3D<T> (x31, x32, x33)) : (scale2 > scale3 ? VECTOR_3D<T> (x21, x22, x32) : VECTOR_3D<T> (x31, x32, x33));
	}

	VECTOR_3D<T> Largest_Column_Normalized() const
	{
		T sqr11 = sqr (x11), sqr12 = sqr (x21), sqr13 = sqr (x31), sqr22 = sqr (x22), sqr23 = sqr (x32), sqr33 = sqr (x33);
		T scale1 = sqr11 + sqr12 + sqr13, scale2 = sqr12 + sqr22 + sqr23, scale3 = sqr13 + sqr23 + sqr33;

		if (scale1 > scale2)
		{
			if (scale1 > scale3) return VECTOR_3D<T> (x11, x21, x31) / sqrt (scale1);
		}
		else if (scale2 > scale3) return VECTOR_3D<T> (x21, x22, x32) / sqrt (scale2);

		return VECTOR_3D<T> (x31, x32, x33) / sqrt (scale3);
	}

	T Max_Abs_Element() const
	{
		return max (fabs (x11), fabs (x21), fabs (x31), fabs (x22), fabs (x32), fabs (x33));
	}

	static SYMMETRIC_MATRIX_3X3<T> Outer_Product (const VECTOR_3D<T>& u)
	{
		return SYMMETRIC_MATRIX_3X3<T> (u.x * u.x, u.x * u.y, u.x * u.z, u.y * u.y, u.y * u.z, u.z * u.z);
	}

	SYMMETRIC_MATRIX_3X3<T> Log() const
	{
		DIAGONAL_MATRIX_3X3<T> D;
		MATRIX_3X3<T> Q;
		Fast_Solve_Eigenproblem (D, Q);
		return Conjugate (Q, D.Log());
	}

	SYMMETRIC_MATRIX_3X3<T> Exp() const
	{
		DIAGONAL_MATRIX_3X3<T> D;
		MATRIX_3X3<T> Q;
		Fast_Solve_Eigenproblem (D, Q);
		return Conjugate (Q, D.Exp());
	}

	static SYMMETRIC_MATRIX_3X3<T> Identity_Matrix()
	{
		return SYMMETRIC_MATRIX_3X3<T> (1, 0, 0, 1, 0, 1);
	}

	static SYMMETRIC_MATRIX_3X3<T> Unit_Matrix (const T scale = 1)
	{
		return SYMMETRIC_MATRIX_3X3<T> (scale, scale, scale, scale, scale, scale);
	}

	VECTOR_3D<T> First_Eigenvector_From_Ordered_Eigenvalues (const DIAGONAL_MATRIX_3X3<T>& eigenvalues, const T tolerance = 1e-5) const
	{
		T tiny = tolerance * maxabs (eigenvalues.x11, eigenvalues.x33);

		if (eigenvalues.x11 - eigenvalues.x22 > tiny) return (*this - eigenvalues.x11).Cofactor_Matrix().Largest_Column_Normalized();

		if (eigenvalues.x22 - eigenvalues.x33 > tiny) return (*this - eigenvalues.x33).Cofactor_Matrix().Largest_Column().Orthogonal_Vector().Normalized();
		else return VECTOR_3D<T> (1, 0, 0);
	}

	VECTOR_3D<T> Last_Eigenvector_From_Ordered_Eigenvalues (const DIAGONAL_MATRIX_3X3<T>& eigenvalues, const T tolerance = 1e-5) const
	{
		T tiny = tolerance * maxabs (eigenvalues.x11, eigenvalues.x33);

		if (eigenvalues.x22 - eigenvalues.x33 > tiny) return (*this - eigenvalues.x33).Cofactor_Matrix().Largest_Column_Normalized();

		if (eigenvalues.x11 - eigenvalues.x22 > tiny) return (*this - eigenvalues.x11).Cofactor_Matrix().Largest_Column().Orthogonal_Vector().Normalized();
		else return VECTOR_3D<T> (1, 0, 0);
	}

	void Fast_Solve_Eigenproblem (DIAGONAL_MATRIX_3X3<T>& eigenvalues, MATRIX_3X3<T>& eigenvectors) const
	{
		DIAGONAL_MATRIX_3X3<double> eigenvalues_double;
		MATRIX_3X3<double> eigenvectors_double;
		Fast_Solve_Eigenproblem_Double (eigenvalues_double, eigenvectors_double);
		eigenvalues = eigenvalues_double;
		eigenvectors = eigenvectors_double;
	}

	template<class RW>
	void Read (std::istream &input_stream)
	{
		Read_Binary<RW> (input_stream, x11, x21, x31, x22, x32, x33);
	}

	template<class RW>
	void Write (std::ostream &output_stream) const
	{
		Write_Binary<RW> (output_stream, x11, x21, x31, x22, x32, x33);
	}

private:
	static SYMMETRIC_MATRIX_3X3<T> Multiply_With_Symmetric_Result (const MATRIX_3X3<T>& A, const MATRIX_3X3<T>& B) // A*B and assume symmetric result (18 mults, 12 adds)
	{
		return SYMMETRIC_MATRIX_3X3<T> (A.x[0] * B.x[0] + A.x[3] * B.x[1] + A.x[6] * B.x[2], A.x[1] * B.x[0] + A.x[4] * B.x[1] + A.x[7] * B.x[2],
						A.x[2] * B.x[0] + A.x[5] * B.x[1] + A.x[8] * B.x[2], A.x[1] * B.x[3] + A.x[4] * B.x[4] + A.x[7] * B.x[5],
						A.x[2] * B.x[3] + A.x[5] * B.x[4] + A.x[8] * B.x[5], A.x[2] * B.x[6] + A.x[5] * B.x[7] + A.x[8] * B.x[8]);
	}

	static SYMMETRIC_MATRIX_3X3<T> Multiply_With_Transpose_With_Symmetric_Result (const MATRIX_3X3<T>& A, const MATRIX_3X3<T>& B) // A*B^t and assume symmetric result
	{
		return SYMMETRIC_MATRIX_3X3<T> (A.x[0] * B.x[0] + A.x[3] * B.x[3] + A.x[6] * B.x[6], A.x[1] * B.x[0] + A.x[4] * B.x[3] + A.x[7] * B.x[6],
						A.x[2] * B.x[0] + A.x[5] * B.x[3] + A.x[8] * B.x[6], A.x[1] * B.x[1] + A.x[4] * B.x[4] + A.x[7] * B.x[7],
						A.x[2] * B.x[1] + A.x[5] * B.x[4] + A.x[8] * B.x[7], A.x[2] * B.x[2] + A.x[5] * B.x[5] + A.x[8] * B.x[8]);
	}

	static SYMMETRIC_MATRIX_3X3<T> Multiply_With_Transpose_With_Symmetric_Result (const MATRIX_3X2<T>& A, const MATRIX_3X2<T>& B) // A*B^t and assume symmetric result
	{
		return SYMMETRIC_MATRIX_3X3<T> (A.x[0] * B.x[0] + A.x[3] * B.x[3], A.x[1] * B.x[0] + A.x[4] * B.x[3], A.x[2] * B.x[0] + A.x[5] * B.x[3],
						A.x[1] * B.x[1] + A.x[4] * B.x[4], A.x[2] * B.x[1] + A.x[5] * B.x[4], A.x[2] * B.x[2] + A.x[5] * B.x[5]);
	}

	static SYMMETRIC_MATRIX_3X3<T> Transpose_Times_With_Symmetric_Result (const MATRIX_3X3<T>& A, const MATRIX_3X3<T>& B) // A^t*B and assume symmetric result
	{
		return SYMMETRIC_MATRIX_3X3<T> (A.x[0] * B.x[0] + A.x[1] * B.x[1] + A.x[2] * B.x[2], A.x[3] * B.x[0] + A.x[4] * B.x[1] + A.x[5] * B.x[2], A.x[6] * B.x[0] + A.x[7] * B.x[1] + A.x[8] * B.x[2],
						A.x[3] * B.x[3] + A.x[4] * B.x[4] + A.x[5] * B.x[5], A.x[6] * B.x[3] + A.x[7] * B.x[4] + A.x[8] * B.x[5], A.x[6] * B.x[6] + A.x[7] * B.x[7] + A.x[8] * B.x[8]);
	}

	static SYMMETRIC_MATRIX_3X3<T> Transpose_Times_With_Symmetric_Result (const MATRIX_3X3<T>& A, const UPPER_TRIANGULAR_MATRIX_3X3<T>& B) // A^t*B and assume symmetric result
	{
		return SYMMETRIC_MATRIX_3X3<T> (A.x[0] * B.x11, A.x[3] * B.x11, A.x[6] * B.x11, A.x[3] * B.x12 + A.x[4] * B.x22, A.x[6] * B.x12 + A.x[7] * B.x22, A.x[6] * B.x13 + A.x[7] * B.x23 + A.x[8] * B.x33);
	}

public:
//#####################################################################
	MATRIX_3X3<T> operator* (const DIAGONAL_MATRIX_3X3<T>& A) const;
	MATRIX_3X3<T> operator* (const UPPER_TRIANGULAR_MATRIX_3X3<T>& A) const;
	SYMMETRIC_MATRIX_3X3<T> operator+ (const DIAGONAL_MATRIX_3X3<T>& A) const;
	static SYMMETRIC_MATRIX_3X3<T> Conjugate (const MATRIX_3X3<T>& A, const DIAGONAL_MATRIX_3X3<T>& B);
	static SYMMETRIC_MATRIX_3X3<T> Conjugate (const MATRIX_3X3<T>& A, const SYMMETRIC_MATRIX_3X3<T>& B);
	static SYMMETRIC_MATRIX_3X3<T> Conjugate (const MATRIX_3X2<T>& A, const DIAGONAL_MATRIX_2X2<T>& B);
	static SYMMETRIC_MATRIX_3X3<T> Conjugate_With_Transpose (const MATRIX_3X3<T>& A, const DIAGONAL_MATRIX_3X3<T>& B);
	static SYMMETRIC_MATRIX_3X3<T> Conjugate_With_Transpose (const MATRIX_3X3<T>& A, const SYMMETRIC_MATRIX_3X3<T>& B);
	static SYMMETRIC_MATRIX_3X3<T> Conjugate_With_Transpose (const UPPER_TRIANGULAR_MATRIX_3X3<T>& A, const SYMMETRIC_MATRIX_3X3<T>& B);
	DIAGONAL_MATRIX_3X3<double> Fast_Eigenvalues_Double() const;
	void Fast_Solve_Eigenproblem_Double (DIAGONAL_MATRIX_3X3<double>& eigenvalues, MATRIX_3X3<double>& eigenvectors, const double tolerance = 1e-7) const;
	void Fast_Eigenvectors_Double (const DIAGONAL_MATRIX_3X3<double>& eigenvalues, MATRIX_3X3<double>& eigenvectors, const double tolerance = 1e-7) const;
	void Solve_Eigenproblem (DIAGONAL_MATRIX_3X3<T>& eigenvalues, MATRIX_3X3<T>& eigenvectors) const;
private:
	static void Jacobi_Transform (const int sweep, const T threshold, T& app, T& apq, T& aqq, T& arp, T& arq, T& v1p, T& v1q, T& v2p, T& v2q, T& v3p, T& v3q);
//#####################################################################
};
// global functions
template<class T>
inline SYMMETRIC_MATRIX_3X3<T> operator* (const T a, const SYMMETRIC_MATRIX_3X3<T>& A)
{
	return A * a;
}

template<class T>
inline SYMMETRIC_MATRIX_3X3<T> operator+ (const T a, const SYMMETRIC_MATRIX_3X3<T>& A)
{
	return A + a;
}

template<class T>
inline SYMMETRIC_MATRIX_3X3<T> operator- (const T a, const SYMMETRIC_MATRIX_3X3<T>& A)
{
	return -A + a;
}

template<class T>
inline std::ostream& operator<< (std::ostream& output_stream, const SYMMETRIC_MATRIX_3X3<T>& A)
{
	output_stream << A.x11 << "\n" << A.x21 << " " << A.x22 << "\n" << A.x31 << " " << A.x32 << " " << A.x33 << "\n";
	return output_stream;
}
//#####################################################################
}
#include "MATRIX_3X2.h"
#include "MATRIX_3X3.h"
#include "UPPER_TRIANGULAR_MATRIX_3X3.h"
namespace PhysBAM
{
//#####################################################################
// Function Conjugate
//#####################################################################
template<class T> inline SYMMETRIC_MATRIX_3X3<T> SYMMETRIC_MATRIX_3X3<T>::
Conjugate (const MATRIX_3X3<T>& A, const DIAGONAL_MATRIX_3X3<T>& B) // 27 mults, 12 adds
{
	return Multiply_With_Transpose_With_Symmetric_Result (A * B, A);
}
//#####################################################################
// Function Conjugate
//#####################################################################
template<class T> inline SYMMETRIC_MATRIX_3X3<T> SYMMETRIC_MATRIX_3X3<T>::
Conjugate (const MATRIX_3X3<T>& A, const SYMMETRIC_MATRIX_3X3<T>& B)
{
	return Multiply_With_Transpose_With_Symmetric_Result (A * B, A);
}
//#####################################################################
// Function Conjugate
//#####################################################################
template<class T> inline SYMMETRIC_MATRIX_3X3<T> SYMMETRIC_MATRIX_3X3<T>::
Conjugate (const MATRIX_3X2<T>& A, const DIAGONAL_MATRIX_2X2<T>& B)
{
	return Multiply_With_Transpose_With_Symmetric_Result (A * B, A);
}
//#####################################################################
// Function Conjugate_With_Transpose
//#####################################################################
template<class T> inline SYMMETRIC_MATRIX_3X3<T> SYMMETRIC_MATRIX_3X3<T>::
Conjugate_With_Transpose (const MATRIX_3X3<T>& A, const DIAGONAL_MATRIX_3X3<T>& B)
{
	return Transpose_Times_With_Symmetric_Result (B * A, A);
}
//#####################################################################
// Function Conjugate_With_Transpose
//#####################################################################
template<class T> inline SYMMETRIC_MATRIX_3X3<T> SYMMETRIC_MATRIX_3X3<T>::
Conjugate_With_Transpose (const MATRIX_3X3<T>& A, const SYMMETRIC_MATRIX_3X3<T>& B)
{
	return Transpose_Times_With_Symmetric_Result (B * A, A);
}
//#####################################################################
// Function Conjugate_With_Transpose
//#####################################################################
template<class T> inline SYMMETRIC_MATRIX_3X3<T> SYMMETRIC_MATRIX_3X3<T>::
Conjugate_With_Transpose (const UPPER_TRIANGULAR_MATRIX_3X3<T>& A, const SYMMETRIC_MATRIX_3X3<T>& B)
{
	return Transpose_Times_With_Symmetric_Result (B * A, A);
}
//#####################################################################
// Function operator*
//#####################################################################
template<class T> inline MATRIX_3X3<T> SYMMETRIC_MATRIX_3X3<T>::
operator* (const DIAGONAL_MATRIX_3X3<T>& A) const
{
	return MATRIX_3X3<T> (x11 * A.x11, x21 * A.x11, x31 * A.x11, x21 * A.x22, x22 * A.x22, x32 * A.x22, x31 * A.x33, x32 * A.x33, x33 * A.x33);
}
//#####################################################################
// Function operator*
//#####################################################################
template<class T> inline MATRIX_3X3<T> SYMMETRIC_MATRIX_3X3<T>::
operator* (const UPPER_TRIANGULAR_MATRIX_3X3<T>& A) const
{
	return MATRIX_3X3<T> (x11 * A.x11, x21 * A.x11, x31 * A.x11, x11 * A.x12 + x21 * A.x22, x21 * A.x12 + x22 * A.x22, x31 * A.x12 + x32 * A.x22,
			      x11 * A.x13 + x21 * A.x23 + x31 * A.x33, x21 * A.x13 + x22 * A.x23 + x32 * A.x33, x31 * A.x13 + x32 * A.x23 + x33 * A.x33);
}
//#####################################################################
// Function operator*
//#####################################################################
template<class T> inline MATRIX_3X3<T>
operator* (const DIAGONAL_MATRIX_3X3<T>& D, const SYMMETRIC_MATRIX_3X3<T>& A)
{
	return MATRIX_3X3<T> (D.x11 * A.x11, D.x22 * A.x21, D.x33 * A.x31, D.x11 * A.x21, D.x22 * A.x22, D.x33 * A.x32, D.x11 * A.x31, D.x22 * A.x32, D.x33 * A.x33);
}
//#####################################################################
// Function operator*
//#####################################################################
template<class T> inline MATRIX_3X3<T>
operator* (const UPPER_TRIANGULAR_MATRIX_3X3<T>& A, const SYMMETRIC_MATRIX_3X3<T>& B)
{
	return MATRIX_3X3<T> (A.x11 * B.x11 + A.x12 * B.x21 + A.x13 * B.x31, A.x22 * B.x21 + A.x23 * B.x31, A.x33 * B.x31, A.x11 * B.x21 + A.x12 * B.x22 + A.x13 * B.x32,
			      A.x22 * B.x22 + A.x23 * B.x32, A.x33 * B.x32, A.x11 * B.x31 + A.x12 * B.x32 + A.x13 * B.x33, A.x22 * B.x32 + A.x23 * B.x33, A.x33 * B.x33);
}
//#####################################################################
// Function operator+
//#####################################################################
template<class T> inline SYMMETRIC_MATRIX_3X3<T> SYMMETRIC_MATRIX_3X3<T>::
operator+ (const DIAGONAL_MATRIX_3X3<T>& A) const
{
	return SYMMETRIC_MATRIX_3X3<T> (x11 + A.x11, x21, x31, x22 + A.x22, x32, x33 + A.x33);
}
//#####################################################################
// Function operator+
//#####################################################################
template<class T> inline SYMMETRIC_MATRIX_3X3<T>
operator- (const DIAGONAL_MATRIX_3X3<T>& A, const SYMMETRIC_MATRIX_3X3<T>& B)
{
	return SYMMETRIC_MATRIX_3X3<T> (A.x11 - B.x11, -B.x21, -B.x31, A.x22 - B.x22, -B.x32, A.x33 - B.x33);
}
//#####################################################################
}
#endif

