//#####################################################################
// Copyright 2004, Ron Fedkiw, Geoffrey Irving, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class RIGID_BODY_PARAMETERS_3D
//#####################################################################
#ifndef __RIGID_BODY_PARAMETERS_3D__
#define __RIGID_BODY_PARAMETERS_3D__

#include "RIGID_BODY_PARAMETERS.h"
#include "../Rigid_Bodies/RIGID_BODY_LIST_3D.h"
namespace PhysBAM
{

template <class T>
class RIGID_BODY_PARAMETERS_3D: public RIGID_BODY_PARAMETERS<T>
{
public:
	using RIGID_BODY_PARAMETERS<T>::simulate;
	using RIGID_BODY_PARAMETERS<T>::callbacks;

	RIGID_BODY_LIST_3D<T> list;
	ARRAY<RIGID_BODY_STATE_3D<T> > kinematic_current_state;
	ARRAY<RIGID_BODY_STATE_3D<T> > kinematic_next_state;

	RIGID_BODY_PARAMETERS_3D()
	{}

	virtual ~RIGID_BODY_PARAMETERS_3D()
	{}

//#####################################################################
};
}
#endif
