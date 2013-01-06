//#####################################################################
// Copyright 2004, Ronald Fedkiw.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SOLIDS_EVOLUTION_CALLBACKS
//#####################################################################
#ifndef __SOLIDS_EVOLUTION_CALLBACKS__
#define __SOLIDS_EVOLUTION_CALLBACKS__

#include "../Utilities/DEBUG_UTILITIES.h"
namespace PhysBAM
{

class RIGID_BODY_COLLISION_MANAGER;

template<class T, class TV>
class SOLIDS_EVOLUTION_CALLBACKS
{
public:
	SOLIDS_EVOLUTION_CALLBACKS()
	{}

	virtual ~SOLIDS_EVOLUTION_CALLBACKS()
	{}

//#####################################################################
	virtual void Update_Rigid_Body_Parameters (RIGID_BODY_COLLISION_MANAGER& collision_manager, const T time)
	{
		WARN_IF_NOT_OVERRIDDEN();
	}
	virtual void Update_Collision_Body_Positions_And_Velocities (const T time)
	{
		WARN_IF_NOT_OVERRIDDEN();
	}
	virtual void Update_Kinematic_Rigid_Body_States (const T dt, const T time)
	{
		WARN_IF_NOT_OVERRIDDEN();
	}
	virtual void Update_Solids_Topology_For_Melting (const T dt, const T time)
	{
		WARN_IF_NOT_OVERRIDDEN();
	}
	virtual void Postprocess_Solids_Substep (const T time, const int substep)
	{
		WARN_IF_NOT_OVERRIDDEN();
	}
	virtual void Apply_Constraints (const T dt, const T time)
	{
		WARN_IF_NOT_OVERRIDDEN();
	}
//#####################################################################
};
}
#endif
