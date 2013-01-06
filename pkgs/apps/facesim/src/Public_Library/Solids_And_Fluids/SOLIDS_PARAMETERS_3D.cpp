//#####################################################################
// Copyright 2004-2005, Ron Fedkiw, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SOLIDS_PARAMETERS_3D
//#####################################################################
#include "SOLIDS_PARAMETERS_3D.h"
using namespace PhysBAM;
//#####################################################################
// Function Initialize_Triangle_Collisions
//#####################################################################
template<class T> void SOLIDS_PARAMETERS_3D<T>::
Initialize_Triangle_Collisions (const bool clamp_repulsion_thickness)
{
}
//#####################################################################
template class SOLIDS_PARAMETERS_3D<float>;
template class SOLIDS_PARAMETERS_3D<double>;
