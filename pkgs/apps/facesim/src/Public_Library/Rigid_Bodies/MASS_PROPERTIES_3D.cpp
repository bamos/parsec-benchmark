//#####################################################################
// Copyright 2003-2005, Zhaosheng Bao, Ron Fedkiw, Eran Guendelman, Geoffrey Irving, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "MASS_PROPERTIES_3D.h"
#include "../Geometry/TRIANGULATED_SURFACE.h"
#include "../Utilities/DEBUG_UTILITIES.h"
using namespace PhysBAM;
//#####################################################################
// Function Compute_Volume_Integrals
//#####################################################################
template<class T> void MASS_PROPERTIES_3D<T>::
Compute_Volume_Integrals()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Compute_Thin_Shell_Integrals
//#####################################################################
template<class T> void MASS_PROPERTIES_3D<T>::
Compute_Thin_Shell_Integrals()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Compute_Face_Integrals
//#####################################################################
template<class T> void MASS_PROPERTIES_3D<T>::
Compute_Face_Integrals (const TRIANGLE_3D<T>& triangle)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Compute_Projection_Integrals
//#####################################################################
template<class T> void MASS_PROPERTIES_3D<T>::
Compute_Projection_Integrals (const TRIANGLE_3D<T> &triangle)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Get_Center_Of_Mass_And_Inertia_Tensor
//#####################################################################
template<class T> void MASS_PROPERTIES_3D<T>::
Get_Center_Of_Mass_And_Inertia_Tensor (VECTOR_3D<T>& center_of_mass, SYMMETRIC_MATRIX_3X3<T>& inertia_tensor) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Transform_To_Object_Frame
//#####################################################################
template<class T> void MASS_PROPERTIES_3D<T>::
Transform_To_Object_Frame (VECTOR_3D<T>& center_of_mass, QUATERNION<T>& orientation, DIAGONAL_MATRIX_3X3<T>& moment_of_inertia, SOLIDS_PARTICLE<T, VECTOR_3D<T> >& particles) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
template class MASS_PROPERTIES_3D<double>;
template class MASS_PROPERTIES_3D<float>;
