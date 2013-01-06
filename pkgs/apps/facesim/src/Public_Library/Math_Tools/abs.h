//#####################################################################
// Copyright 2005, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Function abs
//#####################################################################
#ifndef __abs__
#define __abs__

namespace PhysBAM
{

template<class T>
inline T abs (const T a)
{
	return a > 0 ? a : -a;
}

}
#endif

