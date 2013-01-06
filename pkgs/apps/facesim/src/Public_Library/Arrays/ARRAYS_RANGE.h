//#####################################################################
// Copyright 2006, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class ARRAYS_RANGE
//#####################################################################
#ifndef __ARRAYS_RANGE__
#define __ARRAYS_RANGE__

#include "../Utilities/TYPE_UTILITIES.h"
namespace PhysBAM
{

template<class T_ARRAYS>
class ARRAYS_RANGE
{
	typedef typename T_ARRAYS::ELEMENT T;
public:
	typedef T ELEMENT;

	T_ARRAYS& arrays;
	const int offset;
	const int m;

	ARRAYS_RANGE (T_ARRAYS& arrays_input, const VECTOR_2D<int>& range)
		: arrays (arrays_input), offset (range.x - 1), m (range.y - range.x + 1)
	{}

	void Get (const int i, T& element1, T& element2) const
	{
		assert (1 <= i && i <= m);
		arrays.Get (i + offset, element1, element2);
	}

	void Get (const int i, T& element1, T& element2, T& element3) const
	{
		assert (1 <= i && i <= m);
		arrays.Get (i + offset, element1, element2, element3);
	}

	void Get (const int i, T& element1, T& element2, T& element3, T& element4) const
	{
		assert (1 <= i && i <= m);
		arrays.Get (i + offset, element1, element2, element3, element4);
	}

	void Set (const int i, const T& element1, const T& element2)
	{
		assert (1 <= i && i <= m);
		arrays.Set (i + offset, element1, element2);
	}

	void Set (const int i, const T& element1, const T& element2, const T& element3)
	{
		assert (1 <= i && i <= m);
		arrays.Set (i + offset, element1, element2, element3);
	}

	void Set (const int i, const T& element1, const T& element2, const T& element3, const T& element4)
	{
		assert (1 <= i && i <= m);
		arrays.Set (i + offset, element1, element2, element3, element4);
	}

//#####################################################################
};
}
#endif
