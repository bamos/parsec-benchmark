//#####################################################################
// Copyright 2002, 2003, 2004, Robert Bridson, Ronald Fedkiw, Eran Guendelman, Geoffrey Irving, Neil Molino, Igor Neverov.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "BOX_3D.h"
#include "../Utilities/DEBUG_UTILITIES.h"
using namespace PhysBAM;
//#####################################################################
// Function Lazy_Intersection
//#####################################################################
// This is a fast routine to do ray box intersections
// box_enlargement modifies the bounds of the box -- it's not a thickness
template<class T> bool BOX_3D<T>::
Lazy_Intersection (RAY_3D<T>& ray, T box_enlargement) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Intersection
//#####################################################################
template<class T> bool BOX_3D<T>::
Intersection (RAY_3D<T>& ray, const T thickness_over_2) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Get_Intersection_Range
//#####################################################################
template<class T> bool BOX_3D<T>::
Get_Intersection_Range (const RAY_3D<T>& ray, T& start_t, T& end_t) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Lazy_Outside
//#####################################################################
template<class T> bool BOX_3D<T>::
Lazy_Outside (const RAY_3D<T>& ray) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Normal
//#####################################################################
template<class T> VECTOR_3D<T> BOX_3D<T>::
Normal (const int aggregate) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Surface
//#####################################################################
template<class T> VECTOR_3D<T> BOX_3D<T>::
Surface (const VECTOR_3D<T> &location) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Calculate_Signed_Distance_Function
//#####################################################################
template<class T> void BOX_3D<T>::
Calculate_Signed_Distance_Function (const GRID_3D<T>& grid, ARRAYS_3D<T>& phi) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################


template class BOX_3D<float>;
template class BOX_3D<double>;
