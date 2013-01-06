//#####################################################################
// Copyright 2003-2004, Ronald Fedkiw, Eran Guendelman
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "RIGID_BODY_EVOLUTION_3D.h"
#include "../Utilities/DEBUG_UTILITIES.h"
using namespace PhysBAM;
//#####################################################################
// Function Advance_One_Time_Step
//#####################################################################
template<class T> void RIGID_BODY_EVOLUTION_3D<T>::
Advance_One_Time_Step (const T dt, const T time)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Update_Positions
//#####################################################################
template<class T> void RIGID_BODY_EVOLUTION_3D<T>::
Update_Positions (const T dt, const T time)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Update_Velocities
//#####################################################################
template<class T> void RIGID_BODY_EVOLUTION_3D<T>::
Update_Velocities (const T dt, const T time)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_Elastic_Collisions
//#####################################################################
template<class T> void RIGID_BODY_EVOLUTION_3D<T>::
Add_Elastic_Collisions (const T dt, const T time)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Apply_Contact_Forces
//#####################################################################
template<class T> void RIGID_BODY_EVOLUTION_3D<T>::
Apply_Contact_Forces (const T dt, const T time)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function CFL
//#####################################################################
template<class T> T RIGID_BODY_EVOLUTION_3D<T>::
CFL (const bool verbose)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function CFL
//#####################################################################
template<class T> void RIGID_BODY_EVOLUTION_3D<T>::
Clamp_Velocities()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
template class RIGID_BODY_EVOLUTION_3D<double>;
template class RIGID_BODY_EVOLUTION_3D<float>;
