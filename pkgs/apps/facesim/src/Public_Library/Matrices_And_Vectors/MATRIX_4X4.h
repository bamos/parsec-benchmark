//#####################################################################
// Copyright 2002-2004, Robert Bridson, Ronald Fedkiw, Geoffrey Irving, Rachel Weinstein.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class MATRIX_4X4
//#####################################################################
#ifndef __MATRIX_4X4__
#define __MATRIX_4X4__

#include <assert.h>
#include "VECTOR_3D.h"
#include "MATRIX_3X3.h"
#include "../Math_Tools/sqr.h"
#include "../Math_Tools/constants.h"
#include <math.h>
namespace PhysBAM
{

template<class T>
class MATRIX_4X4
{
public:
	T x[16];

	MATRIX_4X4()
	{
		for (int i = 0; i < 16; i++) x[i] = 0;
	}

	MATRIX_4X4 (const MATRIX_4X4<T>& matrix_input)
	{
		for (int i = 0; i < 16; i++) x[i] = matrix_input.x[i];
	}

	MATRIX_4X4 (const T x11, const T x21, const T x31, const T x41, const T x12, const T x22, const T x32, const T x42, const T x13, const T x23, const T x33, const T x43, const T x14, const T x24, const T x34,
		    const T x44)
	{
		x[0] = x11;
		x[1] = x21;
		x[2] = x31;
		x[3] = x41;
		x[4] = x12;
		x[5] = x22;
		x[6] = x32;
		x[7] = x42;
		x[8] = x13;
		x[9] = x23;
		x[10] = x33;
		x[11] = x43;
		x[12] = x14;
		x[13] = x24;
		x[14] = x34;
		x[15] = x44;
	}

	explicit MATRIX_4X4 (const MATRIX_3X3<T>& matrix_input) // Create a homogeneous 4x4 matrix corresponding to 3x3 transform
	{
		x[0] = matrix_input.x[0];
		x[1] = matrix_input.x[1];
		x[2] = matrix_input.x[2];
		x[3] = (T) 0;
		x[4] = matrix_input.x[3];
		x[5] = matrix_input.x[4];
		x[6] = matrix_input.x[5];
		x[7] = (T) 0;
		x[8] = matrix_input.x[6];
		x[9] = matrix_input.x[7];
		x[10] = matrix_input.x[8];
		x[11] = (T) 0;
		x[12] = (T) 0;
		x[13] = (T) 0;
		x[14] = (T) 0;
		x[15] = (T) 1;
	}

	bool operator== (const MATRIX_4X4<T>& A) const
	{
		for (int i = 0; i < 16; i++) if (x[i] != A.x[i]) return false;

		return true;
	}

	T& operator() (const int i, const int j)
	{
		assert (i >= 1 && i <= 4);
		assert (j >= 1 && j <= 4);
		return x[i - 1 + 4 * (j - 1)];
	}

	const T& operator() (const int i, const int j) const
	{
		assert (i >= 1 && i <= 4);
		assert (j >= 1 && j <= 4);
		return x[i - 1 + 4 * (j - 1)];
	}

	MATRIX_4X4<T> operator-() const
	{
		return MATRIX_4X4<T> (-x[0], -x[1], -x[2], -x[3], -x[4], -x[5], -x[6], -x[7], -x[8], -x[9], -x[10], -x[11], -x[12], -x[13], -x[14], -x[15]);
	}

	MATRIX_4X4<T>& operator+= (const MATRIX_4X4<T>& A)
	{
		for (int i = 0; i < 16; i++) x[i] += A.x[i];

		return *this;
	}

	MATRIX_4X4<T>& operator-= (const MATRIX_4X4<T>& A)
	{
		for (int i = 0; i < 16; i++) x[i] -= A.x[i];

		return *this;
	}

	MATRIX_4X4<T>& operator*= (const MATRIX_4X4<T>& A)
	{
		return *this = *this * A;
	}

	MATRIX_4X4<T>& operator*= (const T a)
	{
		for (int i = 0; i < 16; i++) x[i] *= a;

		return *this;
	}

	MATRIX_4X4<T>& operator/= (const T a)
	{
		assert (a != 0);
		T s = 1 / a;

		for (int i = 0; i < 16; i++) x[i] *= s;

		return *this;
	}

	MATRIX_4X4<T> operator+ (const MATRIX_4X4<T>& A) const
	{
		return MATRIX_4X4<T> (x[0] + A.x[0], x[1] + A.x[1], x[2] + A.x[2], x[3] + A.x[3], x[4] + A.x[4], x[5] + A.x[5], x[6] + A.x[6], x[7] + A.x[7], x[8] + A.x[8], x[9] + A.x[9], x[10] + A.x[10],
				      x[11] + A.x[11], x[12] + A.x[12], x[13] + A.x[13], x[14] + A.x[14], x[15] + A.x[15]);
	}

	MATRIX_4X4<T> operator- (const MATRIX_4X4<T>& A) const
	{
		return MATRIX_4X4<T> (x[0] - A.x[0], x[1] - A.x[1], x[2] - A.x[2], x[3] - A.x[3], x[4] - A.x[4], x[5] - A.x[5], x[6] - A.x[6], x[7] - A.x[7], x[8] - A.x[8], x[9] - A.x[9], x[10] - A.x[10], x[11] - A.x[11],
				      x[12] - A.x[12], x[13] - A.x[13], x[14] - A.x[14], x[15] - A.x[15]);
	}

	MATRIX_4X4<T> operator* (const MATRIX_4X4<T>& A) const
	{
		return MATRIX_4X4<T> (x[0] * A.x[0] + x[4] * A.x[1] + x[8] * A.x[2] + x[12] * A.x[3], x[1] * A.x[0] + x[5] * A.x[1] + x[9] * A.x[2] + x[13] * A.x[3],
				      x[2] * A.x[0] + x[6] * A.x[1] + x[10] * A.x[2] + x[14] * A.x[3], x[3] * A.x[0] + x[7] * A.x[1] + x[11] * A.x[2] + x[15] * A.x[3],
				      x[0] * A.x[4] + x[4] * A.x[5] + x[8] * A.x[6] + x[12] * A.x[7], x[1] * A.x[4] + x[5] * A.x[5] + x[9] * A.x[6] + x[13] * A.x[7],
				      x[2] * A.x[4] + x[6] * A.x[5] + x[10] * A.x[6] + x[14] * A.x[7], x[3] * A.x[4] + x[7] * A.x[5] + x[11] * A.x[6] + x[15] * A.x[7],
				      x[0] * A.x[8] + x[4] * A.x[9] + x[8] * A.x[10] + x[12] * A.x[11], x[1] * A.x[8] + x[5] * A.x[9] + x[9] * A.x[10] + x[13] * A.x[11],
				      x[2] * A.x[8] + x[6] * A.x[9] + x[10] * A.x[10] + x[14] * A.x[11], x[3] * A.x[8] + x[7] * A.x[9] + x[11] * A.x[10] + x[15] * A.x[11],
				      x[0] * A.x[12] + x[4] * A.x[13] + x[8] * A.x[14] + x[12] * A.x[15], x[1] * A.x[12] + x[5] * A.x[13] + x[9] * A.x[14] + x[13] * A.x[15],
				      x[2] * A.x[12] + x[6] * A.x[13] + x[10] * A.x[14] + x[14] * A.x[15], x[3] * A.x[12] + x[7] * A.x[13] + x[11] * A.x[14] + x[15] * A.x[15]);
	}

	MATRIX_4X4<T> operator* (const T a) const
	{
		return MATRIX_4X4<T> (a * x[0], a * x[1], a * x[2], a * x[3], a * x[4], a * x[5], a * x[6], a * x[7], a * x[8], a * x[9], a * x[10], a * x[11], a * x[12], a * x[13], a * x[14], a * x[15]);
	}

	MATRIX_4X4<T> operator/ (const T a) const
	{
		assert (a != 0);
		T s = 1 / a;
		return MATRIX_4X4<T> (s * x[0], s * x[1], s * x[2], s * x[3], s * x[4], s * x[5], s * x[6], s * x[7], s * x[8], s * x[9], s * x[10], s * x[11], s * x[12], s * x[13], s * x[14], s * x[15]);
	}

	VECTOR_3D<T> operator* (const VECTOR_3D<T>& v) const // assumes w=1 is the 4th coordinate of v
	{
		T w = x[3] * v.x + x[7] * v.y + x[11] * v.z + x[15];
		assert (w != 0);

		if (w == 1) return VECTOR_3D<T> (x[0] * v.x + x[4] * v.y + x[8] * v.z + x[12], x[1] * v.x + x[5] * v.y + x[9] * v.z + x[13], x[2] * v.x + x[6] * v.y + x[10] * v.z + x[14]);
		else
		{
			T s = 1 / w; // rescale so w=1
			return VECTOR_3D<T> (s * (x[0] * v.x + x[4] * v.y + x[8] * v.z + x[12]), s * (x[1] * v.x + x[5] * v.y + x[9] * v.z + x[13]), s * (x[2] * v.x + x[6] * v.y + x[10] * v.z + x[14]));
		}
	}

	VECTOR_3D<T> Transform_3X3 (const VECTOR_3D<T>& v) const // multiplies vector by upper 3x3 of matrix only
	{
		return VECTOR_3D<T> (x[0] * v.x + x[4] * v.y + x[8] * v.z, x[1] * v.x + x[5] * v.y + x[9] * v.z, x[2] * v.x + x[6] * v.y + x[10] * v.z);
	}

	void Invert()
	{
		*this = Inverse();
	}

	MATRIX_4X4<T> Inverse()
	{
		int pivot_row;
		T p1 = fabs (x[0]), p2 = fabs (x[1]), p3 = fabs (x[2]), p4 = fabs (x[3]);

		if (p1 > p2)
		{
			if (p1 > p3)
			{
				if (p1 > p4) pivot_row = 1;
				else pivot_row = 4;
			}
			else if (p3 > p4) pivot_row = 3;
			else pivot_row = 4;
		}
		else if (p2 > p3)
		{
			if (p2 > p4) pivot_row = 2;
			else pivot_row = 4;
		}
		else if (p3 > p4) pivot_row = 3;
		else pivot_row = 4;

		T a = x[pivot_row - 1];
		assert (a != 0);
		VECTOR_3D<T> b (x[3 + pivot_row], x[7 + pivot_row], x[11 + pivot_row]), c;
		MATRIX_3X3<T> d;

		switch (pivot_row)
		{
		case 1:
			c = VECTOR_3D<T> (x[1], x[2], x[3]);
			d = MATRIX_3X3<T> (x[5], x[6], x[7], x[9], x[10], x[11], x[13], x[14], x[15]);
			break;
		case 2:
			c = VECTOR_3D<T> (x[0], x[2], x[3]);
			d = MATRIX_3X3<T> (x[4], x[6], x[7], x[8], x[10], x[11], x[12], x[14], x[15]);
			break;
		case 3:
			c = VECTOR_3D<T> (x[0], x[1], x[3]);
			d = MATRIX_3X3<T> (x[4], x[5], x[7], x[8], x[9], x[11], x[12], x[13], x[15]);
			break;
		default:
			c = VECTOR_3D<T> (x[0], x[1], x[2]);
			d = MATRIX_3X3<T> (x[4], x[5], x[6], x[8], x[9], x[10], x[12], x[13], x[14]);
		}

		T m = -1 / a;
		b *= m;
		d += MATRIX_3X3<T>::Outer_Product (c, b); // find schur complement
		MATRIX_3X3<T> h = d.Inverse();
		VECTOR_3D<T> g = h * (c * m);
		VECTOR_3D<T> f = b * h;
		T e = VECTOR_3D<T>::Dot_Product (b, g) - m; // compute parts of inverse

		switch (pivot_row)
		{
		case 1:
			return MATRIX_4X4<T> (e, g.x, g.y, g.z, f.x, h (1, 1), h (2, 1), h (3, 1), f.y, h (1, 2), h (2, 2), h (3, 2), f.z, h (1, 3), h (2, 3), h (3, 3));
			break;
		case 2:
			return MATRIX_4X4<T> (f.x, h (1, 1), h (2, 1), h (3, 1), e, g.x, g.y, g.z, f.y, h (1, 2), h (2, 2), h (3, 2), f.z, h (1, 3), h (2, 3), h (3, 3));
			break;
		case 3:
			return MATRIX_4X4<T> (f.x, h (1, 1), h (2, 1), h (3, 1), f.y, h (1, 2), h (2, 2), h (3, 2), e, g.x, g.y, g.z, f.z, h (1, 3), h (2, 3), h (3, 3));
			break;
		default:
			return MATRIX_4X4<T> (f.x, h (1, 1), h (2, 1), h (3, 1), f.y, h (1, 2), h (2, 2), h (3, 2), f.z, h (1, 3), h (2, 3), h (3, 3), e, g.x, g.y, g.z);
		}
	}

	MATRIX_4X4<T> Cofactor_Matrix() const
	{
		T y[16];
		y[0] = x[5] * x[10] * x[15] + x[9] * x[14] * x[7] + x[13] * x[6] * x[11] - x[5] * x[14] * x[11] - x[9] * x[6] * x[15] - x[13] * x[10] * x[7];
		y[1] = -x[6] * x[11] * x[12] - x[10] * x[15] * x[4] - x[14] * x[7] * x[8] + x[6] * x[15] * x[8] + x[10] * x[7] * x[12] + x[14] * x[11] * x[4];
		y[2] = x[7] * x[8] * x[13] + x[11] * x[12] * x[5] + x[15] * x[4] * x[9] - x[7] * x[12] * x[9] - x[11] * x[4] * x[13] - x[15] * x[8] * x[5];
		y[3] = -x[4] * x[9] * x[14] - x[8] * x[13] * x[6] - x[12] * x[5] * x[10] + x[4] * x[13] * x[10] + x[8] * x[5] * x[14] + x[12] * x[9] * x[6];
		y[4] = -x[9] * x[14] * x[3] - x[13] * x[2] * x[11] - x[1] * x[10] * x[15] + x[9] * x[2] * x[15] + x[13] * x[10] * x[3] + x[1] * x[14] * x[11];
		y[5] = x[10] * x[15] * x[0] + x[14] * x[3] * x[8] + x[2] * x[11] * x[12] - x[10] * x[3] * x[12] - x[14] * x[11] * x[0] - x[2] * x[15] * x[8];
		y[6] = -x[11] * x[12] * x[1] - x[15] * x[0] * x[9] - x[3] * x[8] * x[13] + x[11] * x[0] * x[13] + x[15] * x[8] * x[1] + x[3] * x[12] * x[9];
		y[7] = x[8] * x[13] * x[2] + x[12] * x[1] * x[10] + x[0] * x[9] * x[14] - x[8] * x[1] * x[14] - x[12] * x[9] * x[2] - x[0] * x[13] * x[10];
		y[8] = x[13] * x[2] * x[7] + x[1] * x[6] * x[15] + x[5] * x[14] * x[3] - x[13] * x[6] * x[3] - x[1] * x[14] * x[7] - x[5] * x[2] * x[15];
		y[9] = -x[14] * x[3] * x[4] - x[2] * x[7] * x[12] - x[6] * x[15] * x[0] + x[14] * x[7] * x[0] + x[2] * x[15] * x[4] + x[6] * x[3] * x[12];
		y[10] = x[15] * x[0] * x[5] + x[3] * x[4] * x[13] + x[7] * x[12] * x[1] - x[15] * x[4] * x[1] - x[3] * x[12] * x[5] - x[7] * x[0] * x[13];
		y[11] = -x[12] * x[1] * x[6] - x[0] * x[5] * x[14] - x[4] * x[13] * x[2] + x[12] * x[5] * x[2] + x[0] * x[13] * x[6] + x[4] * x[1] * x[14];
		y[12] = -x[1] * x[6] * x[11] - x[5] * x[10] * x[3] - x[9] * x[2] * x[7] + x[1] * x[10] * x[7] + x[5] * x[2] * x[11] + x[9] * x[6] * x[3];
		y[13] = x[2] * x[7] * x[8] + x[6] * x[11] * x[0] + x[10] * x[3] * x[4] - x[2] * x[11] * x[4] - x[6] * x[3] * x[8] - x[10] * x[7] * x[0];
		y[14] = -x[3] * x[4] * x[9] - x[7] * x[8] * x[1] - x[11] * x[0] * x[5] + x[3] * x[8] * x[5] + x[7] * x[0] * x[9] + x[11] * x[4] * x[1];
		y[15] = x[0] * x[5] * x[10] + x[4] * x[9] * x[2] + x[8] * x[1] * x[6] - x[0] * x[9] * x[6] - x[4] * x[1] * x[10] - x[8] * x[5] * x[2];
		return MATRIX_4X4<T> (y[0], y[1], y[2], y[3], y[4], y[5], y[6], y[7], y[8], y[9], y[10], y[11], y[12], y[13], y[14], y[15]);
	}

	void Invert_Rotation_And_Translation()
	{
		*this = Inverse_Rotation_And_Translation();
	}

	MATRIX_4X4<T> Inverse_Rotation_And_Translation()
	{
		return MATRIX_4X4<T> (x[0], x[4], x[8], 0, x[1], x[5], x[9], 0, x[2], x[6], x[10], 0, -x[0] * x[12] - x[1] * x[13] - x[2] * x[14], -x[4] * x[12] - x[5] * x[13] - x[6] * x[14], -x[8] * x[12] - x[9] * x[13] - x[10] * x[14], 1);
	}

	MATRIX_4X4<T> Rotation_Only() const
	{
		return MATRIX_4X4<T> (x[0], x[1], x[2], 0, x[4], x[5], x[6], 0, x[8], x[9], x[10], 0, 0, 0, 0, 1);
	}

	MATRIX_3X3<T> Upper_3X3() const
	{
		return MATRIX_3X3<T> (x[0], x[1], x[2], x[4], x[5], x[6], x[8], x[9], x[10]);
	}

	void Transpose()
	{
		exchange (x[1], x[4]);
		exchange (x[2], x[8]);
		exchange (x[3], x[12]);
		exchange (x[6], x[9]);
		exchange (x[7], x[13]);
		exchange (x[11], x[14]);
	}

	static MATRIX_4X4<T> Transpose (const MATRIX_4X4<T>& A)
	{
		return MATRIX_4X4<T> (A.x[0], A.x[4], A.x[8], A.x[12], A.x[1], A.x[5], A.x[9], A.x[13], A.x[2], A.x[6], A.x[10], A.x[14], A.x[3], A.x[7], A.x[11], A.x[15]);
	}

	static MATRIX_4X4<T> Translation_Matrix (const VECTOR_3D<T>& translation)
	{
		return MATRIX_4X4<T> (1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, translation.x, translation.y, translation.z, 1);
	}

	static MATRIX_4X4<T> Identity_Matrix()
	{
		return MATRIX_4X4<T> (1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
	}

	static MATRIX_4X4<T> Rotation_Matrix_X_Axis (const T radians)
	{
		T c = cos (radians), s = sin (radians);
		return MATRIX_4X4<T> (1, 0, 0, 0, 0, c, s, 0, 0, -s, c, 0, 0, 0, 0, 1);
	}

	static MATRIX_4X4<T> Rotation_Matrix_Y_Axis (const T radians)
	{
		T c = cos (radians), s = sin (radians);
		return MATRIX_4X4<T> (c, 0, -s, 0, 0, 1, 0, 0, s, 0, c, 0, 0, 0, 0, 1);
	}

	static MATRIX_4X4<T> Rotation_Matrix_Z_Axis (const T radians)
	{
		T c = cos (radians), s = sin (radians);
		return MATRIX_4X4<T> (c, s, 0, 0, -s, c, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
	}

	static MATRIX_4X4<T> Rotation_Matrix (const VECTOR_3D<T>& axis, const T radians)
	{
		T c = cos (radians), s = sin (radians);
		return MATRIX_4X4<T> (sqr (axis.x) + (1 - sqr (axis.x)) * c, axis.x * axis.y * (1 - c) + axis.z * s, axis.x * axis.z * (1 - c) - axis.y * s, 0,
				      axis.x * axis.y * (1 - c) - axis.z * s, sqr (axis.y) + (1 - sqr (axis.y)) * c, axis.y * axis.z * (1 - c) + axis.x * s, 0,
				      axis.x * axis.z * (1 - c) + axis.y * s, axis.y * axis.z * (1 - c) - axis.x * s, sqr (axis.z) + (1 - sqr (axis.z)) * c, 0,
				      0, 0, 0, 1);
	}

	static MATRIX_4X4<T> Rotation_Matrix (const VECTOR_3D<T>& x_final, const VECTOR_3D<T>& y_final, const VECTOR_3D<T>& z_final)
	{
		return MATRIX_4X4<T> (x_final.x, x_final.y, x_final.z, 0, y_final.x, y_final.y, y_final.z, 0, z_final.x, z_final.y, z_final.z, 0, 0, 0, 0, 1);
	}

	static MATRIX_4X4<T> Rotation_Matrix (const VECTOR_3D<T>& initial_vector, const VECTOR_3D<T>& final_vector, const T tolerance = 1e-14)
	{
		VECTOR_3D<T> initial_unit = initial_vector / initial_vector.Magnitude(), final_unit = final_vector / final_vector.Magnitude();
		T cos_theta = VECTOR_3D<T>::Dot_Product (initial_unit, final_unit);

		if (cos_theta > 1 - tolerance) return Identity_Matrix();

		if (cos_theta < -1 + tolerance) return MATRIX_4X4<T> (-1, 0, 0, 0, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1); // note-this is actually a reflection

		VECTOR_3D<T> axis = VECTOR_3D<T>::Cross_Product (initial_unit, final_unit);
		axis.Normalize();
		return Rotation_Matrix (axis, acos (cos_theta));
	}

	static MATRIX_4X4<T> Scale_Matrix (const VECTOR_3D<T>& scale_vector)
	{
		return MATRIX_4X4<T> (scale_vector.x, 0, 0, 0, 0, scale_vector.y, 0, 0, 0, 0, scale_vector.z, 0, 0, 0, 0, 1);
	}

	static MATRIX_4X4<T> Scale_Matrix (const T scale)
	{
		return MATRIX_4X4<T> (scale, 0, 0, 0, 0, scale, 0, 0, 0, 0, scale, 0, 0, 0, 0, 1);
	}

//#####################################################################
	template<class S> friend MATRIX_4X4<S> operator* (const S a, const MATRIX_4X4<S>& A);
};
template<class T>
inline MATRIX_4X4<T> operator* (const T a, const MATRIX_4X4<T>& A)
{
	return MATRIX_4X4<T> (a * A.x[0], a * A.x[1], a * A.x[2], a * A.x[3], a * A.x[4], a * A.x[5], a * A.x[6], a * A.x[7], a * A.x[8], a * A.x[9], a * A.x[10], a * A.x[11], a * A.x[12], a * A.x[13], a * A.x[14], a * A.x[15]);
}

template<class T>
inline std::istream& operator>> (std::istream& input_stream, MATRIX_4X4<T>& A)
{
	for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) input_stream >> A.x[i + j * 4];

	return input_stream;
}

template<class T>
inline std::ostream& operator<< (std::ostream& output_stream, const MATRIX_4X4<T>& A)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++) output_stream << A.x[i + j * 4] << " ";

		output_stream << std::endl;
	}

	return output_stream;
}
}
#endif
