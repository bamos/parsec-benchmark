//#####################################################################
// Copyright 2002, Ronald Fedkiw.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Function sqr
//#####################################################################
//
// Finds the square. That is raises is to the second power.
//
//#####################################################################
#ifndef __sqr__
#define __sqr__

namespace PhysBAM
{

template<class T>
inline T sqr (const T a)
{
	return a * a;
}

}
#endif

