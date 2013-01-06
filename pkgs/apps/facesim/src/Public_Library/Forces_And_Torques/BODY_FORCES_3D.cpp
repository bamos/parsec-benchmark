//#####################################################################
// Copyright 2002-2004, Robert Bridson, Ronald Fedkiw, Geoffrey Irving, Sergey Koltakov.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "BODY_FORCES_3D.h"
using namespace PhysBAM;

//#####################################################################
// Function Add_Velocity_Independent_Forces
//#####################################################################
template<class T> void BODY_FORCES_3D<T>::
Add_Velocity_Independent_Forces (ARRAY<VECTOR_3D<T> >& F) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_Ether_Drag_Velocity_Independent_Term
//#####################################################################
template<class T> void BODY_FORCES_3D<T>::
Add_Ether_Drag_Velocity_Independent_Term (ARRAY<VECTOR_3D<T> >& F) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_Ether_Drag_Velocity_Dependent_Term
//#####################################################################
template<class T> void BODY_FORCES_3D<T>::
Add_Ether_Drag_Velocity_Dependent_Term (ARRAY<VECTOR_3D<T> >& F) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_Wind_Drag
//#####################################################################
template<class T> void BODY_FORCES_3D<T>::
Add_Wind_Drag (const TRIANGULATED_SURFACE<T>& surface, ARRAY<VECTOR_3D<T> >& F) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
template class BODY_FORCES_3D<float>;
template class BODY_FORCES_3D<double>;
