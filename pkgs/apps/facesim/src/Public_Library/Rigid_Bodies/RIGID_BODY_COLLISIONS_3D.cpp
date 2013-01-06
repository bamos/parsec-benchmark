//#####################################################################
// Copyright 2003-2004, Ronald Fedkiw, Eran Guendelman, Rachel Weinstein
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "RIGID_BODY_COLLISIONS_3D.h"
#include "../Utilities/DEBUG_UTILITIES.h"
using namespace PhysBAM;
//#####################################################################
// Function Check_For_Any_Interpenetration
//#####################################################################
template<class T> bool RIGID_BODY_COLLISIONS_3D<T>::
Check_For_Any_Interpenetration()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Print_Interpenetration_Statistics
//#####################################################################
template<class T> void RIGID_BODY_COLLISIONS_3D<T>::
Print_Interpenetration_Statistics()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
template class RIGID_BODY_COLLISIONS_3D<double>;
template class RIGID_BODY_COLLISIONS_3D<float>;
