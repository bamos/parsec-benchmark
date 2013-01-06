//#####################################################################
// Copyright 2004-2005, Ron Fedkiw, Geoffrey Irving, Andrew Selle, Rachel Weinstein.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class QUEUE
//#####################################################################
#ifndef __QUEUE__
#define __QUEUE__

#include "../Arrays/ARRAY.h"
namespace PhysBAM
{

template<class T>
class QUEUE
{
public:
	ARRAY<T> array;
	int front, back;

	QUEUE (const int size)
		: array (size), front (1), back (1)
	{}

	void Enqueue (const T& element)
	{
		array (back) = element;

		if (++back > array.m) back = 1;

		assert (back != front);
	} // dies if you run out of room

	T Dequeue()
	{
		T value = array (front);

		if (++front > array.m) front = 1;

		return value;
	}

	T& Peek() const
	{
		return array (front);
	}

	int Size() const
	{
		if (back < front) return back + array.m - front;
		else return back - front;
	}

	bool Empty() const
	{
		return Size() == 0;
	}

	void Remove_All_Entries()
	{
		front = 1;
		back = 1;
	}

//#####################################################################
};
}
#endif
