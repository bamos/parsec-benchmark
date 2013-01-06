//#####################################################################
// Copyright 2003-2004, Geoffrey Irving, Frank Losasso.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class PAIR
//#####################################################################
#ifndef __PAIR__
#define __PAIR__

#include "../Read_Write/READ_WRITE_FUNCTIONS.h"
namespace PhysBAM
{

template<class T, class T2>
class PAIR
{
public:
	T x;
	T2 y;

	PAIR()
		: x (T()), y (T2())
	{}

	PAIR (const T& x_input, const T2& y_input)
		: x (x_input), y (y_input)
	{}

	~PAIR()
	{}

	template<class RW>
	void Read (std::istream &input_stream)
	{
		Read_Binary<RW> (input_stream, x);
		Read_Binary<RW> (input_stream, y);
	}

	template<class RW>
	void Write (std::ostream &output_stream) const
	{
		Write_Binary<RW> (output_stream, x);
		Write_Binary<RW> (output_stream, y);
	}

//#####################################################################
};
}
#endif
