//#####################################################################
// Copyright 2003, Zhaosheng Bao, Ronald Fedkiw.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class EMBEDDED_OBJECT
//#####################################################################
#include "EMBEDDED_OBJECT.h"
#include "../Utilities/DEBUG_UTILITIES.h"

using namespace PhysBAM;
//#####################################################################
// Function Update_Embedded_Particle_Positions
//#####################################################################
template<class T, class TV> void EMBEDDED_OBJECT<T, TV>::
Update_Embedded_Particle_Positions()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Update_Embedded_Particle_Velocities
//#####################################################################
template<class T, class TV> void EMBEDDED_OBJECT<T, TV>::
Update_Embedded_Particle_Velocities()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Update_Embedded_Particle_Masses
//#####################################################################
template<class T, class TV> void EMBEDDED_OBJECT<T, TV>::
Update_Embedded_Particle_Masses()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Embedded_Children
//#####################################################################
template<class T, class TV> void EMBEDDED_OBJECT<T, TV>::
Initialize_Embedded_Children()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_Embedded_Particle_To_Embedded_Chidren
//#####################################################################
template<class T, class TV> void EMBEDDED_OBJECT<T, TV>::
Add_Embedded_Particle_To_Embedded_Chidren (const int embedded_particle)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
template class EMBEDDED_OBJECT<float, VECTOR_3D<float> >;
template class EMBEDDED_OBJECT<double, VECTOR_3D<double> >;
template class EMBEDDED_OBJECT<float, VECTOR_2D<float> >;
template class EMBEDDED_OBJECT<double, VECTOR_2D<double> >;
