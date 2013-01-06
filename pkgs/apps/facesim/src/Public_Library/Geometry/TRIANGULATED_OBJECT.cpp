//#####################################################################
// Copyright 2004, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "TRIANGULATED_OBJECT.h"
#include "../Utilities/DEBUG_UTILITIES.h"
using namespace PhysBAM;
//#####################################################################
// Function Discard_Valence_Zero_Particles_And_Renumber
//#####################################################################
template<class T, class TV> void TRIANGULATED_OBJECT<T, TV>::
Discard_Valence_Zero_Particles_And_Renumber (ARRAY<int>& condensation_mapping)
{
	NOT_IMPLEMENTED();
}

//#####################################################################


template class TRIANGULATED_OBJECT<float, VECTOR_2D<float> >;
template class TRIANGULATED_OBJECT<float, VECTOR_3D<float> >;
template class TRIANGULATED_OBJECT<double, VECTOR_2D<double> >;
template class TRIANGULATED_OBJECT<double, VECTOR_3D<double> >;
