//#####################################################################
// Copyright 2003-2005, Zhaosheng Bao, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Function hash_function
//#####################################################################
//
// fast and random hash.  Requires only 11 cycles for 32 bit int.
//
//#####################################################################
#ifndef __hash_function__
#define __hash_function__

#include "../Data_Structures/PAIR.h"
#include "../Matrices_And_Vectors/VECTOR_2D.h"
#include "../Matrices_And_Vectors/VECTOR_3D.h"
namespace PhysBAM
{

inline unsigned int int_hash (unsigned int key)
{
	key += ~ (key << 15);
	key ^= (key >> 10);
	key += (key << 3);
	key ^= (key >> 6);
	key += ~ (key << 11);
	key ^= (key >> 16);
	return key;
}

inline unsigned int triple_int_hash (unsigned int a, unsigned int b, unsigned int c)
{
	a -= b;
	a -= c;
	a ^= (c >> 13);
	b -= c;
	b -= a;
	b ^= (a << 8);
	c -= a;
	c -= b;
	c ^= (b >> 13);
	a -= b;
	a -= c;
	a ^= (c >> 12);
	b -= c;
	b -= a;
	b ^= (a << 16);
	c -= a;
	c -= b;
	c ^= (b >> 5);
	a -= b;
	a -= c;
	a ^= (c >> 3);
	b -= c;
	b -= a;
	b ^= (a << 10);
	c -= a;
	c -= b;
	c ^= (b >> 15);
	return c;
}

inline int Hash_Reduce (const int key)
{
	return key;
}

inline int Hash_Reduce (const VECTOR_2D<int>& key)
{
	return triple_int_hash (32138912, key.x, key.y);
}

inline int Hash_Reduce (const VECTOR_3D<int>& key)
{
	return triple_int_hash (key.x, key.y, key.z);
}

template<class T, class T2>
inline int Hash_Reduce (const PAIR<T, T2>& pair)
{
	return triple_int_hash (33232132, Hash_Reduce (pair.x), Hash_Reduce (pair.y));
}

template<class T>
inline int Hash (const T& key)
{
	return Hash_Reduce (key);
}

template<>
inline int Hash (const int& key)
{
	return int_hash (key);
}

}
#endif

