//#####################################################################
// Copyright 2002-2006, Robert Bridson, Ronald Fedkiw, Geoffrey Irving, Neil Molino, Eftychios Sifakis, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class EXTERNAL_FORCES_AND_VELOCITIES
//#####################################################################
#ifndef __EXTERNAL_FORCES_AND_VELOCITIES__
#define __EXTERNAL_FORCES_AND_VELOCITIES__

#include "../Utilities/DEBUG_UTILITIES.h"

namespace PhysBAM
{

template<class T> class ARRAY;
template<class T> class VECTOR_2D;
template<class T> class VECTOR_3D;
template<class T> class QUATERNION;

template<class T, class TV>
class EXTERNAL_FORCES_AND_VELOCITIES
{
public:
	bool collisions_on;

	EXTERNAL_FORCES_AND_VELOCITIES()
	{
		Activate_Collisions (false);
	}

	virtual ~EXTERNAL_FORCES_AND_VELOCITIES()
	{}

	void Activate_Collisions (const bool collisions_on_input = true)
	{
		collisions_on = collisions_on_input;
	}

	// Deformable objects - multiple partitions
	virtual void Add_External_Forces (ARRAY<TV>& F, const T time, const int id_number, const int partition_id)
	{
		NOT_IMPLEMENTED();
	}
	virtual void Set_External_Velocities (ARRAY<TV>& V, const T time, const int id_number, const int partition_id)
	{
		NOT_IMPLEMENTED();
	}
	virtual void Zero_Out_Enslaved_Velocity_Nodes (ARRAY<TV>& V, const T time, const int id_number, const int partition_id)
	{
		NOT_IMPLEMENTED();
	}
	virtual void Set_External_Positions (ARRAY<TV>& X, const T time, const int id_number, const int partition_id)
	{
		NOT_IMPLEMENTED();
	}
	virtual void Zero_Out_Enslaved_Position_Nodes (ARRAY<TV>& X, const T time, const int id_number, const int partition_id)
	{
		NOT_IMPLEMENTED();
	}
	virtual void Update_Time_Varying_Material_Properties (const T time, const int id_number, const int partition_id)
	{
		NOT_IMPLEMENTED();
	}

//#####################################################################
	// Rigid bodies
	virtual void Add_External_Forces (VECTOR_2D<T>& F, T& torque, const T time, const int id_number = 1) {} // force and torque
	virtual void Add_External_Forces (VECTOR_3D<T>& F, VECTOR_3D<T>& torque, const T time, const int id_number = 1) {} // force and torque
	virtual void Set_External_Velocities (VECTOR_2D<T>& V, T& omega, const T time, const int id_number = 1) {} // velocity and angular velocity
	virtual void Set_External_Velocities (VECTOR_3D<T>& V, VECTOR_3D<T>& omega, const T time, const int id_number = 1) {} // velocity and angular velocity
	virtual void Set_External_Position_And_Orientation (VECTOR_2D<T>& X, T& orientation, const T time, const int id_number = 1) {} // set external position and orientation
	virtual void Set_External_Position_And_Orientation (VECTOR_3D<T>& X, QUATERNION<T>& orientation, const T time, const int id_number = 1) {} // set external position and orientation
	// Deformable objects - single partition
	virtual void Add_External_Forces (ARRAY<TV>& F, const T time, const int id_number = 1) {}
	virtual void Set_External_Velocities (ARRAY<TV>& V, const T time, const int id_number = 1) {}
	virtual void Zero_Out_Enslaved_Velocity_Nodes (ARRAY<TV>& V, const T time, const int id_number = 1) {} // or zero out components of their velocities
	virtual void Set_External_Positions (ARRAY<TV>& X, const T time, const int id_number = 1) {} // set external positions
	virtual void Zero_Out_Enslaved_Position_Nodes (ARRAY<TV>& X, const T time, const int id_number = 1) {} // zero out entries corresponding to external positions
	virtual void Update_Time_Varying_Material_Properties (const T time, const int id_number = 1) {}
	// Asynchronous
	virtual void Add_External_Impulses (ARRAY<TV>& V, const T time, const T dt, const int id_number = 1) {} // adjust velocity with external impulses
	virtual void Add_External_Impulse (ARRAY<TV>& V, const int node, const T time, const T dt, const int id_number = 1) {} // adjust velocity of single node with external impulse
//#####################################################################
};
}
#endif
