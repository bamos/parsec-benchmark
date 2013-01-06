//#####################################################################
// Copyright 2002, 2003, 2004, Robert Bridson, Ronald Fedkiw, Eran Guendelman, Geoffrey Irving, Sergey Koltakov, Neil Molino, Duc Nguyen, Andrew Selle, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "BOX_2D.h"
#include "../Utilities/DEBUG_UTILITIES.h"
#include "../Grids/GRID_2D.h"
#include "../Arrays/ARRAYS_2D.h"
using namespace PhysBAM;
//#####################################################################
// Function Intersection
//#####################################################################
// This does not do an inside test
template<class T> bool BOX_2D<T>::
Intersection (RAY_2D<T>& ray, const T thickness_over_two, const T segment_intersect_epsilon) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Normal
//#####################################################################
template<class T> VECTOR_2D<T> BOX_2D<T>::
Normal (const int aggregate) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Surface
//#####################################################################
template<class T> VECTOR_2D<T> BOX_2D<T>::
Surface (const VECTOR_2D<T>& location) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Calculate_Signed_Distance_Function
//#####################################################################
template<class T> void BOX_2D<T>::
Calculate_Signed_Distance_Function (const GRID_2D<T>& grid, ARRAYS_2D<T>& phi) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
template class BOX_2D<float>;
template class BOX_2D<double>;
