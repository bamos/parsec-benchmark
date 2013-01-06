//#####################################################################
// Copyright 2002-2003, Geoffrey Irving
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class VECTOR_1D
//#####################################################################
#ifndef __VECTOR_1D__
#define __VECTOR_1D__

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
class VECTOR_1D
{
public:
	T x;

	explicit VECTOR_1D (const T x_input = 0)
		: x (x_input)
	{}

	template<class T2> VECTOR_1D (const VECTOR_1D<T2>& vector_input)
		: x ( (T) vector_input.x)
	{}

	bool operator== (const VECTOR_1D<T>& v) const
	{
		return x == v.x;
	}

	bool operator!= (const VECTOR_1D<T>& v) const
	{
		return x != v.x;
	}

	VECTOR_1D<T> operator-() const
	{
		return VECTOR_1D<T> (-x);
	}

	VECTOR_1D<T>& operator+= (const VECTOR_1D<T>& v)
	{
		x += v.x;
		return *this;
	}

	VECTOR_1D<T>& operator-= (const VECTOR_1D<T>& v)
	{
		x -= v.x;
		return *this;
	}

	VECTOR_1D<T>& operator*= (const VECTOR_1D<T>& v)
	{
		x *= v.x;
		return *this;
	}

	VECTOR_1D<T>& operator*= (const T a)
	{
		x *= a;
		return *this;
	}

	VECTOR_1D<T>& operator/= (const T a)
	{
		assert (a != 0);
		T one_over_a = 1 / a;
		x *= one_over_a;
		return *this;
	}

	VECTOR_1D<T>& operator/= (const VECTOR_1D<T>& v)
	{
		x /= v.x;
		return *this;
	}

	VECTOR_1D<T> operator+ (const VECTOR_1D<T>& v) const
	{
		return VECTOR_1D<T> (x + v.x);
	}

	VECTOR_1D<T> operator- (const VECTOR_1D<T>& v) const
	{
		return VECTOR_1D<T> (x - v.x);
	}

	VECTOR_1D<T> operator* (const VECTOR_1D<T>& v) const
	{
		return VECTOR_1D<T> (x * v.x);
	}

	VECTOR_1D<T> operator/ (const VECTOR_1D<T>& v) const
	{
		return VECTOR_1D<T> (x / v.x);
	}

	VECTOR_1D<T> operator* (const T a) const
	{
		return VECTOR_1D<T> (a * x);
	}

	VECTOR_1D<T> operator/ (const T a) const
	{
		assert (a != 0);
		T s = 1 / a;
		return VECTOR_1D<T> (s * x);
	}

	VECTOR_1D<T> fabs() const
	{
		return VECTOR_1D<T> (fabs (x));
	}

	T Magnitude_Squared() const
	{
		return sqr (x);
	}

	T Magnitude() const
	{
		return sqrt (sqr (x));
	}

	T Normalize()
	{
		T magnitude = Magnitude();
		assert (magnitude != 0);
		T s = 1 / magnitude;
		x *= s;
		return magnitude;
	}

	VECTOR_1D Normalized() const
	{
		T magnitude = Magnitude();
		assert (magnitude != 0);
		T s = 1 / magnitude;
		return VECTOR_1D (x * s);
	}

	T Min_Element() const
	{
		return x;
	}

	T Max_Element() const
	{
		return x;
	}

	static T Dot_Product (const VECTOR_1D<T>& v1, const VECTOR_1D<T>& v2)
	{
		return v1.x * v2.x;
	}

	VECTOR_1D<T> Projected_On_Unit_Direction (const VECTOR_1D<T>& direction) const
	{
		return Dot_Product (*this, direction) * direction;
	}

	VECTOR_1D<T> Projected (const VECTOR_1D<T>& direction) const // un-normalized direction
	{
		return Dot_Product (*this, direction) / Dot_Product (direction, direction) * direction;
	}

	void Project_On_Unit_Direction (const VECTOR_1D<T>& direction)
	{
		*this = Dot_Product (*this, direction) * direction;
	}

	void Project (const VECTOR_1D<T>& direction) // un-normalized direction
	{
		*this = Dot_Product (*this, direction) / Dot_Product (direction, direction) * direction;
	}

	void Print() const
	{
		std::cout << "VECTOR_1D: " << x << std::endl;
	}

	template<class RW>
	void Read (std::istream& input_stream)
	{
		Read_Binary<RW> (input_stream, x);
	}

	template<class RW>
	void Write (std::ostream& output_stream) const
	{
		Write_Binary<RW> (output_stream, x);
	}

//#####################################################################
};
template<>
inline VECTOR_1D<int> VECTOR_1D<int>::operator/ (const int a) const
{
	return VECTOR_1D<int> (x / a);
}

template<class T>
inline VECTOR_1D<T> operator* (const T a, const VECTOR_1D<T>& v)
{
	return VECTOR_1D<T> (a * v.x);
}

template<class T>
inline std::istream& operator>> (std::istream& input_stream, VECTOR_1D<T>& v)
{
	input_stream >> v.x;
	return input_stream;
}

template<class T>
inline std::ostream& operator<< (std::ostream& output_stream, const VECTOR_1D<T>& v)
{
	output_stream << v.x;
	return output_stream;
}
}
#endif
