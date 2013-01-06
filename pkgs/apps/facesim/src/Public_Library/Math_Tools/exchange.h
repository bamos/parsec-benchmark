//#####################################################################
// Copyright 2002, Ronald Fedkiw.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Function exchange
//#####################################################################
//
// exchanges the values of a and b
//
//#####################################################################
#ifndef __exchange__
#define __exchange__

namespace PhysBAM
{

template<class T>
inline void exchange (T& a, T& b)
{
	T c = a;
	a = b;
	b = c;
}

}
#endif

