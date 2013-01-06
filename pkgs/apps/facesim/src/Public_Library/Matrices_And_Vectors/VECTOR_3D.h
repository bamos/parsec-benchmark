//#####################################################################
// Copyright 2002-2004, Robert Bridson, Doug Enright, Ronald Fedkiw, Eran Guendelman, Geoffrey Irving, Sergey Koltakov, Igor Neverov, Duc Nguyen, Joseph Teran, Rachel Weinstein.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class VECTOR_3D
//#####################################################################
#ifndef __VECTOR_3D__
#define __VECTOR_3D__

#include <iostream>
#include <assert.h>
#include <math.h>
#include "../Math_Tools/sqr.h"
#include "../Math_Tools/max.h"
#include "../Math_Tools/min.h"
#include "../Math_Tools/constants.h"
#include "VECTOR_2D.h"
#include "VECTOR_ND.h"
#include "../Read_Write/READ_WRITE_FUNCTIONS.h"
namespace PhysBAM
{

template<class T>
class VECTOR_3D
{
public:
	T x, y, z;

	VECTOR_3D()
		: x (T()), y (T()), z (T())
	{}

	VECTOR_3D (const T x_input, const T y_input, const T z_input)
		: x (x_input), y (y_input), z (z_input)
	{}

	VECTOR_3D (const VECTOR_3D<T>& vector_input)
		: x (vector_input.x), y (vector_input.y), z (vector_input.z)
	{}

	VECTOR_3D (const VECTOR_ND<T>& vector_input)
		: x (vector_input (1)), y (vector_input (2)), z (vector_input (3))
	{
		assert (vector_input.n == 3);
	}

	template<class T2> VECTOR_3D (const VECTOR_3D<T2>& vector_input)
		: x ( (T) vector_input.x), y ( (T) vector_input.y), z ( (T) vector_input.z)
	{}

	explicit VECTOR_3D (const VECTOR_2D<T>& vector_input)
		: x (vector_input.x), y (vector_input.y), z (0)
	{}

	VECTOR_2D<T> Vector_2D() const
	{
		return VECTOR_2D<T> (x, y);
	}

	T operator[] (const int i) const
	{
		assert (1 <= i && i <= 3);
		return * ( (const T*) (this) + i - 1);
	}

	T& operator[] (const int i)
	{
		assert (1 <= i && i <= 3);
		return * ( (T*) (this) + i - 1);
	}

	bool operator== (const VECTOR_3D<T>& v) const
	{
		return x == v.x && y == v.y && z == v.z;
	}

	bool operator!= (const VECTOR_3D<T>& v) const
	{
		return x != v.x || y != v.y || z != v.z;
	}

	VECTOR_3D<T> operator-() const
	{
		return VECTOR_3D<T> (-x, -y, -z);
	}

	VECTOR_3D<T>& operator+= (const VECTOR_3D<T>& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	VECTOR_3D<T>& operator-= (const VECTOR_3D<T>& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	VECTOR_3D<T>& operator*= (const VECTOR_3D<T>& v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return *this;
	}

	VECTOR_3D<T>& operator*= (const T a)
	{
		x *= a;
		y *= a;
		z *= a;
		return *this;
	}

	VECTOR_3D<T>& operator/= (const T a)
	{
		assert (a != 0);
		T one_over_a = 1 / a;
		x *= one_over_a;
		y *= one_over_a;
		z *= one_over_a;
		return *this;
	}

	VECTOR_3D<T>& operator/= (const VECTOR_3D<T>& v)
	{
		x /= v.x;
		y /= v.y;
		z /= v.z;
		return *this;
	}

	VECTOR_3D<T> operator+ (const VECTOR_3D<T>& v) const
	{
		return VECTOR_3D<T> (x + v.x, y + v.y, z + v.z);
	}

	VECTOR_3D<T> operator- (const VECTOR_3D<T>& v) const
	{
		return VECTOR_3D<T> (x - v.x, y - v.y, z - v.z);
	}

	VECTOR_3D<T> operator* (const VECTOR_3D<T>& v) const
	{
		return VECTOR_3D<T> (x * v.x, y * v.y, z * v.z);
	}

	VECTOR_3D<T> operator/ (const VECTOR_3D<T>& v) const
	{
		return VECTOR_3D<T> (x / v.x, y / v.y, z / v.z);
	}

	VECTOR_3D<T> operator* (const T a) const
	{
		return VECTOR_3D<T> (a * x, a * y, a * z);
	}

	VECTOR_3D<T> operator/ (const T a) const
	{
		assert (a != 0);
		T s = 1 / a;
		return VECTOR_3D<T> (s * x, s * y, s * z);
	}

	VECTOR_3D<T> fabs() const
	{
		return VECTOR_3D<T> (fabs (x), fabs (y), fabs (z));
	}

	T Magnitude_Squared() const
	{
		return x * x + y * y + z * z;
	}

	T Magnitude() const
	{
		return sqrt (x * x + y * y + z * z);
	}

	T Lp_Norm (const T p) const
	{
		return pow (pow (::fabs (x), p) + pow (::fabs (y), p) + pow (::fabs (z), p), 1 / p);
	}

	T Normalize()
	{
		T magnitude = Magnitude();
		assert (magnitude != 0);
		T s = 1 / magnitude;
		x *= s;
		y *= s;
		z *= s;
		return magnitude;
	}

	T Robust_Normalize (T tolerance = (T) 1e-8, const VECTOR_3D<T>& fallback = VECTOR_3D<T> (1, 0, 0))
	{
		T magnitude = Magnitude();

		if (magnitude > tolerance)
		{
			T s = 1 / magnitude;
			x *= s;
			y *= s;
			z *= s;
		}
		else
		{
			*this = fallback;
		}

		return magnitude;
	}

	VECTOR_3D<T> Normalized() const
	{
		T magnitude = Magnitude();
		assert (magnitude != 0);
		T s = 1 / magnitude;
		return VECTOR_3D (x * s, y * s, z * s);
	}

	VECTOR_3D<T> Robust_Normalized (T tolerance = (T) 1e-8, const VECTOR_3D<T>& fallback = VECTOR_3D<T> (1, 0, 0)) const
	{
		T magnitude = Magnitude();

		if (magnitude > tolerance) return *this / magnitude;
		else return fallback;
	}

	VECTOR_3D<T> Orthogonal_Vector() const
	{
		T abs_x =::fabs ( (T) x), abs_y =::fabs ( (T) y), abs_z =::fabs ( (T) z);

		if (abs_x < abs_y) return abs_x < abs_z ? VECTOR_3D<T> (0, z, -y) : VECTOR_3D<T> (y, -x, 0);
		else return abs_y < abs_z ? VECTOR_3D<T> (-z, 0, x) : VECTOR_3D<T> (y, -x, 0);
	}

	T Min_Element() const
	{
		return min (x, y, z);
	}

	T Max_Element() const
	{
		return max (x, y, z);
	}

	T Max_Abs_Element() const
	{
		return maxabs (x, y, z);
	}

	int Dominant_Axis() const
	{
		T abs_x =::fabs (x), abs_y =::fabs (y), abs_z =::fabs (z);
		return (abs_x > abs_y && abs_x > abs_z) ? 1 : ( (abs_y > abs_z) ? 2 : 3);
	}

	static T Dot_Product (const VECTOR_3D<T>& v1, const VECTOR_3D<T>& v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	VECTOR_3D<T> Projected_On_Unit_Direction (const VECTOR_3D<T>& direction) const
	{
		return Dot_Product (*this, direction) * direction;
	}

	VECTOR_3D<T> Projected (const VECTOR_3D<T>& direction) const // un-normalized direction
	{
		return Dot_Product (*this, direction) / Dot_Product (direction, direction) * direction;
	}

	void Project_On_Unit_Direction (const VECTOR_3D<T>& direction)
	{
		*this = Dot_Product (*this, direction) * direction;
	}

	void Project (const VECTOR_3D<T>& direction) // un-normalized direction
	{
		*this = Dot_Product (*this, direction) / Dot_Product (direction, direction) * direction;
	}

	VECTOR_3D<T> Projected_Orthogonal_To_Unit_Direction (const VECTOR_3D<T>& direction) const
	{
		T dot_product = Dot_Product (*this, direction);
		return VECTOR_3D<T> (x - dot_product * direction.x, y - dot_product * direction.y, z - dot_product * direction.z);
	}

	static VECTOR_3D<T> Cross_Product (const VECTOR_3D<T>& v1, const VECTOR_3D<T>& v2)
	{
		return VECTOR_3D<T> (v1.y * v2.z - v2.y * v1.z, -v1.x * v2.z + v2.x * v1.z, v1.x * v2.y - v2.x * v1.y);
	}

	static T Angle_Between (const VECTOR_3D<T>& v1, const VECTOR_3D<T>& v2)
	{
		return acos (max (T (-1), min (T (1), Dot_Product (v1, v2) / (v1.Magnitude() * v2.Magnitude()))));
	}

	static T Triple_Product (const VECTOR_3D<T>& u, const VECTOR_3D<T>& v, const VECTOR_3D<T>& w)
	{
		return Dot_Product (u, Cross_Product (v, w));
	}

	static T Element_Average (const VECTOR_3D<T>& a)
	{
		return (T) one_third * (a.x + a.y + a.z);
	}

	static VECTOR_3D<T> Exponent (const VECTOR_3D<T>& a)
	{
		return VECTOR_3D<T> (exp (a.x), exp (a.y), exp (a.z));
	}

	static VECTOR_3D<T> Square_Root (const VECTOR_3D<T>& a)
	{
		return VECTOR_3D<T> (sqrt (a.x), sqrt (a.y), sqrt (a.z));
	}

	static VECTOR_3D<T> Square (const VECTOR_3D<T>& a)
	{
		return VECTOR_3D<T> (sqr (a.x), sqr (a.y), sqr (a.z));
	}

	static VECTOR_3D<T> Cube (const VECTOR_3D<T>& a)
	{
		return VECTOR_3D<T> (cube (a.x), cube (a.y), cube (a.z));
	}

	static VECTOR_3D<T> Reciprocal (const VECTOR_3D<T>& a)
	{
		return VECTOR_3D<T> (1 / a.x, 1 / a.y, 1 / a.z);
	}

	void Print() const
	{
		std::cout << "VECTOR_3D: " << x << " " << y << " " << z << std::endl;
	}

	template<class RW>
	void Read (std::istream &input_stream)
	{
		Read_Binary<RW> (input_stream, x, y, z);
	}

	template<class RW>
	void Write (std::ostream &output_stream) const
	{
		Write_Binary<RW> (output_stream, x, y, z);
	}

//#####################################################################
};
template<>
inline VECTOR_3D<int> VECTOR_3D<int>::operator/ (const int a) const
{
	return VECTOR_3D<int> (x / a, y / a, z / a);
}

template<>
inline VECTOR_3D<int>& VECTOR_3D<int>::operator/= (const int a)
{
	x /= a;
	y /= a;
	z /= a;
	return *this;
}

template<class T>
inline VECTOR_3D<T> operator* (const T a, const VECTOR_3D<T>& v)
{
	return VECTOR_3D<T> (a * v.x, a * v.y, a * v.z);
}

template<class T>
inline std::istream& operator>> (std::istream& input_stream, VECTOR_3D<T>& v)
{
	input_stream >> v.x >> v.y >> v.z;
	return input_stream;
}

template<class T>
inline std::ostream& operator<< (std::ostream& output_stream, const VECTOR_3D<T>& v)
{
	output_stream << v.x << " " << v.y << " " << v.z;
	return output_stream;
}
}
#endif
