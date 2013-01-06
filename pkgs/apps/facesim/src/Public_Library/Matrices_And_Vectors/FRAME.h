//#####################################################################
// Copyright 2002-2004, Ronald Fedkiw, Geoffrey Irving, Sergey Koltakov, Neil Molino, Igor Neverov, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class FRAME
//#####################################################################
#ifndef __FRAME__
#define __FRAME__

#include <iostream>
#include <math.h>
#include "VECTOR_3D.h"
#include "QUATERNION.h"
namespace PhysBAM
{

template<class T>
class FRAME
{
public:
	VECTOR_3D<T> t; // by default T is (0,0,0)
	QUATERNION<T> r; // by default r is (1,0,0,0)

	FRAME()
	{}

	FRAME (const VECTOR_3D<T>& t_input)
		: t (t_input)
	{}

	FRAME (const QUATERNION<T>& r_input)
		: r (r_input)
	{}

	FRAME (const VECTOR_3D<T>& t_input, const QUATERNION<T>& r_input)
		: t (t_input), r (r_input)
	{}

	bool operator== (const FRAME<T>& f) const
	{
		return t == f.t && r == f.r;
	}

	bool operator!= (const FRAME<T>& f) const
	{
		return t != f.t || r != f.r;
	}

	FRAME<T>& operator*= (const FRAME<T>& f)
	{
		t += r.Rotate (f.t);
		r *= f.r;
		return *this;
	}

	FRAME<T> operator* (const FRAME<T>& f) const
	{
		return FRAME<T> (t + r.Rotate (f.t), r * f.r);
	}

	FRAME<T>& operator+= (const FRAME<T>& f)
	{
		t += f.t;
		r += f.r;
		return *this;
	}

	FRAME<T> operator+ (const FRAME<T>& f) const
	{
		return FRAME<T> (t + f.t, r + f.r);
	}

	FRAME<T> operator- (const FRAME<T>& f) const
	{
		return FRAME<T> (t - f.t, r - f.r);
	}

	VECTOR_3D<T> operator* (const VECTOR_3D<T>& v) const
	{
		return t + r.Rotate (v);
	}

	VECTOR_3D<T> Local_Coordinate (const VECTOR_3D<T>& v) const
	{
		return (r.Inverse()).Rotate (v - t);
	}

	void Invert()
	{
		*this = Inverse();
	}

	FRAME<T> Inverse() const
	{
		QUATERNION<T> r_inverse = r.Inverse();
		return FRAME<T> (-r_inverse.Rotate (t), r_inverse);
	}

	FRAME<T> Left_Multiply_With_Position_And_Orientation (const VECTOR_3D<T>& position, const QUATERNION<T>& orientation) const
	{
		return FRAME<T> (position + t, orientation * r);
	}

	void Print() const
	{
		std::cout << "FRAME:" << std::endl;
		t.Print();
		r.Print();
	}

	template<class RW>
	void Read (std::istream& input_stream)
	{
		t.template Read<RW> (input_stream);
		r.template Read<RW> (input_stream);
	}

	template<class RW>
	void Write (std::ostream& output_stream) const
	{
		t.template Write<RW> (output_stream);
		r.template Write<RW> (output_stream);
	}

//#####################################################################
};
// global functions
template<class T> inline FRAME<T> operator* (const T a, const FRAME<T>& f)
{
	return FRAME<T> (a * f.t, a * f.r);
}

template<class T> inline std::istream& operator>> (std::istream& input_stream, FRAME<T>& f)
{
	input_stream >> f.t >> f.r;
	return input_stream;
}

template<class T> inline std::ostream& operator<< (std::ostream& output_stream, const FRAME<T>& f)
{
	output_stream << f.t << " " << f.r;
	return output_stream;
}
}
#endif

