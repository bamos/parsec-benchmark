//#####################################################################
// Copyright 2002, 2003 Ronald Fedkiw, Frederic Gibou, Neil Molino.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class LEVELSET
//#####################################################################
#include "LEVELSET.h"
#include "../Utilities/DEBUG_UTILITIES.h"
using namespace PhysBAM;
//#####################################################################
// Function Negative_Cell_Fraction
//#####################################################################
// finds the fraction of the 1D cell that has phi <= 0
template<class T, class TV> T LEVELSET<T, TV>::
Negative_Cell_Fraction (const T phi_left, const T phi_right) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Positive_Cell_Fraction
//#####################################################################
// finds the fraction of the 1D cell that has phi > 0
template<class T, class TV> T LEVELSET<T, TV>::
Positive_Cell_Fraction (const T phi_left, const T phi_right) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Negative_Cell_Fraction
//#####################################################################
// finds the fraction of the 2D cell that has phi <= 0 - aspect ratio is height/width, i.e. y/x
template<class T, class TV> T LEVELSET<T, TV>::
Negative_Cell_Fraction (const T phi_lower_left, const T phi_lower_right, const T phi_upper_left, const T phi_upper_right, const T aspect_ratio) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Positive_Cell_Fraction
//#####################################################################
// finds the fraction of the 2D cell that has phi > 0 - aspect ratio is height/width, i.e. y/x
template<class T, class TV> T LEVELSET<T, TV>::
Positive_Cell_Fraction (const T phi_lower_left, const T phi_lower_right, const T phi_upper_left, const T phi_upper_right, const T aspect_ratio) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function HJ_WENO
//#####################################################################
// phi is (-2,m_3), phix_minus and phix_plus are (1,m)
template<class T, class TV> void LEVELSET<T, TV>::
HJ_WENO (const int m, const T dx, const ARRAYS_1D<T>& phi, ARRAYS_1D<T>& phix_minus, ARRAYS_1D<T>& phix_plus) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function HJ_ENO
//#####################################################################
// order = 1, 2 or 3, phi is (-2,m_3), phix_minus and phix_plus are (1,m)
template<class T, class TV> void LEVELSET<T, TV>::
HJ_ENO (const int order, const int m, const T dx, const ARRAYS_1D<T>& phi, ARRAYS_1D<T>& phix_minus, ARRAYS_1D<T>& phix_plus) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################

template class LEVELSET<float, VECTOR_1D<float> >;
template class LEVELSET<double, VECTOR_1D<double> >;
template class LEVELSET<float, VECTOR_2D<float> >;
template class LEVELSET<double, VECTOR_2D<double> >;
template class LEVELSET<float, VECTOR_3D<float> >;
template class LEVELSET<double, VECTOR_3D<double> >;
