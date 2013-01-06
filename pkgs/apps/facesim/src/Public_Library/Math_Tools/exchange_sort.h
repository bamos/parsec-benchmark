//#####################################################################
// Copyright 2002, Robert Bridson.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Function exchange_sort
//#####################################################################
//
// exchanges the values passed to be in sorted (ascending) order
//
//#####################################################################
#ifndef __exchange_sort__
#define __exchange_sort__

#include "exchange.h"
namespace PhysBAM
{

template<class T>
inline void exchange_sort (T& a, T& b)
{
	if (a > b) exchange (a, b);
}

template<class T>
inline void exchange_sort (T& a, T& b, T& c)
{
	if (a > b) exchange (a, b);

	if (b > c) exchange (b, c);

	if (a > b) exchange (a, b);
}

template<class T>
inline void exchange_sort (T& a, T& b, T& c, T& d)
{
	if (a > b) exchange (a, b);

	if (c > d) exchange (c, d);

	if (a > c) exchange (a, c);

	if (b > d) exchange (b, d);

	if (b > c) exchange (b, c);
}

}
#endif

