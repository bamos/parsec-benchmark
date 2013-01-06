//#####################################################################
// Copyright 2004, Ronald Fedkiw, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class RIGID_BODY_PARAMETERS_CALLBACKS
//#####################################################################
#ifndef __RIGID_BODY_PARAMETERS_CALLBACKS__
#define __RIGID_BODY_PARAMETERS_CALLBACKS__

#include "../Utilities/DEBUG_UTILITIES.h"
namespace PhysBAM
{

template<class T> class RIGID_BODY_STATE_2D;
template<class T> class RIGID_BODY_STATE_3D;

template<class T>
class RIGID_BODY_PARAMETERS_CALLBACKS
{
public:
	RIGID_BODY_PARAMETERS_CALLBACKS()
	{}

	virtual ~RIGID_BODY_PARAMETERS_CALLBACKS()
	{}

//#####################################################################
	virtual void Set_Kinematic_Rigid_Body_State (RIGID_BODY_STATE_2D<T>& state, const T time, const int id_number, bool& velocity_set)
	{
		WARN_IF_NOT_OVERRIDDEN();
	}
	virtual void Set_Kinematic_Rigid_Body_State (RIGID_BODY_STATE_3D<T>& state, const T time, const int id_number, bool& velocity_set)
	{
		WARN_IF_NOT_OVERRIDDEN();
	}
//#####################################################################
};
}
#endif
