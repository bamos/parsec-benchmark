//#####################################################################
// Copyright 2004, Ron Fedkiw, Andrew Selle
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "SOLIDS_PARTICLE.h"
#include "../Read_Write/FILE_UTILITIES.h"
using namespace PhysBAM;
template<class T, class TV> PARTICLE_ATTRIBUTE_COLLECTION_MAP<SOLIDS_PARTICLE<T, TV> > SOLIDS_PARTICLE<T, TV>::attribute_map;
//#####################################################################
template class SOLIDS_PARTICLE<float, VECTOR_1D<float> >;
template class SOLIDS_PARTICLE<float, VECTOR_2D<float> >;
template class SOLIDS_PARTICLE<float, VECTOR_3D<float> >;
template class SOLIDS_PARTICLE<double, VECTOR_1D<double> >;
template class SOLIDS_PARTICLE<double, VECTOR_2D<double> >;
template class SOLIDS_PARTICLE<double, VECTOR_3D<double> >;
