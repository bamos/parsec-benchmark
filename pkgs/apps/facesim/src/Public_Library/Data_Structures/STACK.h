//#####################################################################
// Copyright 2003, 2004, Ron Fedkiw, Eran Guendelman, Neil Molino, Andrew Selle
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class STACK
//#####################################################################
#ifndef __STACK__
#define __STACK__

#include "../Arrays/LIST_ARRAY.h"
namespace PhysBAM
{

template<class T>
class STACK
{
public:
	LIST_ARRAY<T> array;

	STACK()
	{}

	void Preallocate (const int max_size)
	{
		array.Preallocate (max_size);
	}

	void Increase_Size (const int size)
	{
		array.Preallocate (size + array.m);
	}

	void Compact()
	{
		array.Compact();
	}

	void Push (const T& element)
	{
		array.Append_Element (element);
	}

	T Pop()
	{
		T value = array (array.m);
		array.Remove_End_Without_Clearing_Value();
		return value;
	}

	const T& Peek() const
	{
		return array (array.m);
	}

	bool Empty() const
	{
		return array.m == 0;
	}

	void Remove_All_Entries()
	{
		array.Reset_Current_Size_To_Zero();
	}

//#####################################################################
};
}
#endif
