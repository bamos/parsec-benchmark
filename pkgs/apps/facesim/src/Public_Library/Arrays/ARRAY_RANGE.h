//#####################################################################
// Copyright 2006, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class ARRAY_RANGE
//#####################################################################
#ifndef __ARRAY_RANGE__
#define __ARRAY_RANGE__

#include "../Utilities/TYPE_UTILITIES.h"
namespace PhysBAM
{

template<class T_ARRAY>
class ARRAY_RANGE
{
	typedef typename T_ARRAY::ELEMENT T;
public:
	typedef T ELEMENT;

	T_ARRAY& array;
	const int offset;
	const int m;

	ARRAY_RANGE (T_ARRAY& array_input, const VECTOR_2D<int>& range)
		: array (array_input), offset (range.x - 1), m (range.y - range.x + 1)
	{}

	typename IF<IS_CONST<T_ARRAY>::value, const T&, T&>::TYPE operator() (const int i)
	{
		assert (1 <= i && i <= m);
		return array (i + offset);
	}

	const T& operator() (const int i) const
	{
		assert (1 <= i && i <= m);
		return array (i + offset);
	}

	ARRAY_RANGE& operator= (const ARRAY_RANGE& source)
	{
		assert (m == source.m);

		for (int i = 1; i <= m; i++) (*this) (i) = source (i);

		return *this;
	}

//#####################################################################
};
}
#endif
