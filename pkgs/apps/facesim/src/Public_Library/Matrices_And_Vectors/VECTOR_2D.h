//#####################################################################
// Copyright 2002-2004, Robert Bridson, Doug Enright, Ronald Fedkiw, Geoffrey Irving, Sergey Koltakov, Igor Neverov, Eran Guendelman, Duc Nguyen.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class VECTOR_2D
//#####################################################################
#ifndef __VECTOR_2D__
#define __VECTOR_2D__

#include <iostream>
#include <assert.h>
#include <math.h>
#include "../Math_Tools/sqr.h"
#include "../Math_Tools/max.h"
#include "../Math_Tools/min.h"
#include "../Read_Write/READ_WRITE_FUNCTIONS.h"
namespace PhysBAM
{

template<class T>
class VECTOR_2D
{
public:
	T x, y;

	VECTOR_2D()
		: x (T()), y (T())
	{}

	VECTOR_2D (const T x_input, const T y_input)
		: x (x_input), y (y_input)
	{}

	VECTOR_2D (const VECTOR_2D<T>& vector_input)
		: x (vector_input.x), y (vector_input.y)
	{}

	template<class T2> VECTOR_2D (const VECTOR_2D<T2>& vector_input)
		: x ( (T) vector_input.x), y ( (T) vector_input.y)
	{}

	T operator[] (const int i) const
	{
		assert (1 <= i && i <= 2);
		return * ( (const T*) (this) + i - 1);
	}

	T& operator[] (const int i)
	{
		assert (1 <= i && i <= 2);
		return * ( (T*) (this) + i - 1);
	}

	bool operator== (const VECTOR_2D<T>& v) const
	{
		return x == v.x && y == v.y;
	}

	bool operator!= (const VECTOR_2D<T>& v) const
	{
		return x != v.x || y != v.y;
	}

	VECTOR_2D<T> operator-() const
	{
		return VECTOR_2D<T> (-x, -y);
	}

	VECTOR_2D<T>& operator+= (const VECTOR_2D<T>& v)
	{
		x += v.x;
		y += v.y;
		return *this;
	}

	VECTOR_2D<T>& operator-= (const VECTOR_2D<T>& v)
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}

	VECTOR_2D<T>& operator*= (const VECTOR_2D<T>& v)
	{
		x *= v.x;
		y *= v.y;
		return *this;
	}

	VECTOR_2D<T>& operator*= (const T a)
	{
		x *= a;
		y *= a;
		return *this;
	}

	VECTOR_2D<T>& operator/= (const T a)
	{
		assert (a != 0);
		T one_over_a = 1 / a;
		x *= one_over_a;
		y *= one_over_a;
		return *this;
	}

	VECTOR_2D<T>& operator/= (const VECTOR_2D<T>& v)
	{
		x /= v.x;
		y /= v.y;
		return *this;
	}

	VECTOR_2D<T> operator+ (const VECTOR_2D<T>& v) const
	{
		return VECTOR_2D<T> (x + v.x, y + v.y);
	}

	VECTOR_2D<T> operator- (const VECTOR_2D<T>& v) const
	{
		return VECTOR_2D<T> (x - v.x, y - v.y);
	}

	VECTOR_2D<T> operator* (const VECTOR_2D<T>& v) const
	{
		return VECTOR_2D<T> (x * v.x, y * v.y);
	}

	VECTOR_2D<T> operator/ (const VECTOR_2D<T>& v) const
	{
		return VECTOR_2D<T> (x / v.x, y / v.y);
	}

	VECTOR_2D<T> operator* (const T a) const
	{
		return VECTOR_2D<T> (a * x, a * y);
	}

	VECTOR_2D<T> operator/ (const T a) const
	{
		assert (a != 0);
		T s = 1 / a;
		return VECTOR_2D<T> (s * x, s * y);
	}

	VECTOR_2D<T> fabs() const
	{
		return VECTOR_2D<T> (fabs (x), fabs (y));
	}

	T Magnitude_Squared() const
	{
		return sqr (x) + sqr (y);
	}

	T Magnitude() const
	{
		return sqrt (sqr (x) + sqr (y));
	}

	T Normalize()
	{
		T magnitude = Magnitude();
		assert (magnitude != 0);
		T s = 1 / magnitude;
		x *= s;
		y *= s;
		return magnitude;
	}

	T Robust_Normalize (T tolerance = (T) 1e-8, const VECTOR_2D<T>& fallback = VECTOR_2D<T> (1, 0))
	{
		T magnitude = Magnitude();

		if (magnitude > tolerance)
		{
			T s = 1 / magnitude;
			x *= s;
			y *= s;
		}
		else
		{
			*this = fallback;
		}

		return magnitude;
	}

	VECTOR_2D<T> Normalized() const
	{
		T magnitude = Magnitude();
		assert (magnitude != 0);
		T s = 1 / magnitude;
		return VECTOR_2D<T> (x * s, y * s);
	}

	VECTOR_2D<T> Robust_Normalized (T tolerance = (T) 1e-8, const VECTOR_2D<T>& fallback = VECTOR_2D<T> (1, 0))
	{
		T magnitude = Magnitude();

		if (magnitude > tolerance) return *this / magnitude;
		else return fallback;
	}

	VECTOR_2D<T> Rotate_Clockwise_90() const
	{
		return VECTOR_2D<T> (y, -x);
	}

	VECTOR_2D<T> Rotate_Counterclockwise_90() const
	{
		return VECTOR_2D<T> (-y, x);
	}

	VECTOR_2D<T> Rotate_Counterclockwise_Multiple_90 (const int n) const
	{
		VECTOR_2D<T> r (*this);

		if (n & 2) r = -r;

		return n & 1 ? r.Rotate_Counterclockwise_90() : r;
	}

	T Min_Element() const
	{
		return min (x, y);
	}

	T Max_Element() const
	{
		return max (x, y);
	}

	T Max_Abs_Element() const
	{
		return maxabs (x, y);
	}

	int Dominant_Axis() const
	{
		return (::fabs (x) > ::fabs (y)) ? 1 : 2;
	}

	static T Dot_Product (const VECTOR_2D<T>& v1, const VECTOR_2D<T>& v2)
	{
		return v1.x * v2.x + v1.y * v2.y;
	}

	VECTOR_2D<T> Projected_On_Unit_Direction (const VECTOR_2D<T>& direction) const
	{
		return Dot_Product (*this, direction) * direction;
	}

	VECTOR_2D<T> Projected (const VECTOR_2D<T>& direction) const // un-normalized direction
	{
		return Dot_Product (*this, direction) / Dot_Product (direction, direction) * direction;
	}

	void Project_On_Unit_Direction (const VECTOR_2D<T>& direction)
	{
		*this = Dot_Product (*this, direction) * direction;
	}

	void Project (const VECTOR_2D<T>& direction) // un-normalized direction
	{
		*this = Dot_Product (*this, direction) / Dot_Product (direction, direction) * direction;
	}

	VECTOR_2D<T> Projected_Orthogonal_To_Unit_Direction (const VECTOR_2D<T>& direction) const
	{
		T dot_product = Dot_Product (*this, direction);
		return VECTOR_2D<T> (x - dot_product * direction.x, y - dot_product * direction.y);
	}

	static T Cross_Product (const VECTOR_2D<T>& v1, const VECTOR_2D<T>& v2)
	{
		return v1.x * v2.y - v2.x * v1.y;
	}

	static T Angle_Between (const VECTOR_2D<T>& v1, const VECTOR_2D<T>& v2)
	{
		return acos (max ( (T) - 1, min ( (T) 1, Dot_Product (v1, v2) / (v1.Magnitude() * v2.Magnitude()))));
	}

	void Print() const
	{
		std::cout << "VECTOR_2D<T>: " << x << "," << y << std::endl;
	}

	template<class RW>
	void Read (std::istream &input_stream)
	{
		Read_Binary<RW> (input_stream, x, y);
	}

	template<class RW>
	void Write (std::ostream &output_stream) const
	{
		Write_Binary<RW> (output_stream, x, y);
	}

//#####################################################################
};
template<>
inline VECTOR_2D<int> VECTOR_2D<int>::operator/ (const int a) const
{
	return VECTOR_2D<int> (x / a, y / a);
}

template<>
inline VECTOR_2D<int>& VECTOR_2D<int>::operator/= (const int a)
{
	x /= a;
	y /= a;
	return *this;
}

template<class T>
inline VECTOR_2D<T> operator* (const T a, const VECTOR_2D<T>& v)
{
	return VECTOR_2D<T> (a * v.x, a * v.y);
}

template<class T>
inline std::istream& operator>> (std::istream& input_stream, VECTOR_2D<T>& v)
{
	input_stream >> v.x >> v.y;
	return input_stream;
}

template<class T>
inline std::ostream& operator<< (std::ostream& output_stream, const VECTOR_2D<T>& v)
{
	output_stream << v.x << " " << v.y;
	return output_stream;
}
}
#endif
