//#####################################################################
// Copyright 2004, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class DIAGONALIZED_SEMI_IMPLICIT_ELEMENT_3D
//#####################################################################
#ifndef __DIAGONALIZED_SEMI_IMPLICIT_ELEMENT_3D__
#define __DIAGONALIZED_SEMI_IMPLICIT_ELEMENT_3D__

#include "../Matrices_And_Vectors/MATRIX_3X3.h"
#include "../Matrices_And_Vectors/DIAGONAL_MATRIX_3X3.h"
namespace PhysBAM
{

template<class T>
class DIAGONALIZED_SEMI_IMPLICIT_ELEMENT_3D
{
public:
	int nodes[4];
	T Bm_scale;
	DIAGONAL_MATRIX_3X3<T> W;
	MATRIX_3X3<T> Dm_inverse;
	T dt_cfl, time;

//#####################################################################
};
}
#endif
