//#####################################################################
// Copyright 2002, 2003, 2004, Ronald Fedkiw, Sergey Koltakov, Eran Guendelman, Neil Molino, Rachel Weinstein.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "IMPLICIT_SURFACE.h"
using namespace PhysBAM;
//#####################################################################
// Function Intersection
//#####################################################################
template<class T> bool IMPLICIT_SURFACE<T>::
Intersection (RAY_3D<T>& ray, const T thickness) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
template class IMPLICIT_SURFACE<float>;
template class IMPLICIT_SURFACE<double>;
