//#####################################################################
// Copyright 2004, Ron Fedkiw, Eran Guendelman, Geoffrey Irving, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SOLIDS_FLUIDS_EXAMPLE_3D
//#####################################################################
#ifndef __SOLIDS_FLUIDS_EXAMPLE_3D__
#define __SOLIDS_FLUIDS_EXAMPLE_3D__

#include "SOLIDS_FLUIDS_EXAMPLE.h"
#include "SOLIDS_PARAMETERS_3D.h"
#include "SOLIDS_EVOLUTION_CALLBACKS.h"
#include "../Data_Structures/PAIR.h"
#include "../Forces_And_Torques/EXTERNAL_FORCES_AND_VELOCITIES.h"
namespace PhysBAM
{
template<class T, class RW>
class SOLIDS_FLUIDS_EXAMPLE_3D: public SOLIDS_FLUIDS_EXAMPLE<T>, public EXTERNAL_FORCES_AND_VELOCITIES<T, VECTOR_3D<T> >, public SOLIDS_EVOLUTION_CALLBACKS<T, VECTOR_3D<T> >
{
public:
	using SOLIDS_FLUIDS_EXAMPLE<T>::output_directory;
	using SOLIDS_FLUIDS_EXAMPLE<T>::first_frame;
	using SOLIDS_FLUIDS_EXAMPLE<T>::restart;
	using EXTERNAL_FORCES_AND_VELOCITIES<T, VECTOR_3D<T> >::collisions_on;
	using SOLIDS_FLUIDS_EXAMPLE<T>::Write_Frame_Title;
private:
	using SOLIDS_FLUIDS_EXAMPLE<T>::minimum_surface_roughness;
public:

	SOLIDS_PARAMETERS_3D<T> solids_parameters;

	SOLIDS_FLUIDS_EXAMPLE_3D()
	{}

	virtual void Initialize_Bodies()
	{
		solids_parameters.deformable_body_parameters.Initialize_Bodies (solids_parameters.cfl);
		solids_parameters.deformable_body_parameters.list.Set_Collision_Body_List (solids_parameters.collision_body_list);
	}

	void Update_Collision_Body_Positions_And_Velocities (const T time)
	{}

	void Update_Kinematic_Rigid_Body_States (const T dt, const T time)
	{}

	void Set_External_Velocities (ARRAY<VECTOR_3D<T> >& V, const T time, const int id_number)
	{
		if (collisions_on) solids_parameters.deformable_body_parameters.list (id_number).collisions.Set_Collision_Velocities (V);
	}

	void Zero_Out_Enslaved_Velocity_Nodes (ARRAY<VECTOR_3D<T> >& V, const T time, const int id_number)
	{
		if (collisions_on) solids_parameters.deformable_body_parameters.list (id_number).collisions.Zero_Out_Collision_Velocities (V);
	}

	void Set_External_Velocities (VECTOR_3D<T>& V, VECTOR_3D<T>& omega, const T time, const int id_number)
	{}

	void Set_External_Position_And_Orientation (VECTOR_3D<T>& X, QUATERNION<T>& orientation, const T time, const int id_number)
	{}

	virtual void Read_Output_Files_Solids (const int frame)
	{
		solids_parameters.template Read_Output_Files<RW> (output_directory, frame);
	}

//####################################################################
	virtual void Write_Output_Files (const int frame) const;
//#####################################################################
};
}
#endif
