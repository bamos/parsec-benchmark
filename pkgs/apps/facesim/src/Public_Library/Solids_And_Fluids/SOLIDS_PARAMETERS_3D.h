//#####################################################################
// Copyright 2004-2005, Ron Fedkiw, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SOLIDS_PARAMETERS_3D
//#####################################################################
#ifndef __SOLIDS_PARAMETERS_3D__
#define __SOLIDS_PARAMETERS_3D__

#include "SOLIDS_PARAMETERS.h"
#include "RIGID_BODY_PARAMETERS_3D.h"
#include "DEFORMABLE_BODY_PARAMETERS_3D.h"
#include "../Collisions_And_Interactions/COLLISION_BODY_LIST_3D.h"
namespace PhysBAM
{

template <class T>
class SOLIDS_PARAMETERS_3D: public SOLIDS_PARAMETERS<T>
{
public:
	using SOLIDS_PARAMETERS<T>::write_static_variables_every_frame;

	RIGID_BODY_PARAMETERS_3D<T> rigid_body_parameters;
	DEFORMABLE_BODY_PARAMETERS_3D<T> deformable_body_parameters;
	COLLISION_BODY_LIST_3D<T> collision_body_list;

	SOLIDS_PARAMETERS_3D()
	{}

	virtual ~SOLIDS_PARAMETERS_3D()
	{}

//#####################################################################
	virtual void Initialize_Triangle_Collisions (const bool clamp_repulsion_thickness = true);
//#####################################################################

//#####################################################################
// Function Read_Output_Files
//#####################################################################
	template<class RW>
	void Read_Output_Files (const std::string& output_directory, const int frame)
	{
		std::string prefix = output_directory + "/";

		if (deformable_body_parameters.write)
		{
			if (write_static_variables_every_frame) deformable_body_parameters.list.template Read_Static_Variables<RW> (prefix, frame);

			deformable_body_parameters.list.template Read_Dynamic_Variables<RW> (prefix, frame);
		}

		if (rigid_body_parameters.write) rigid_body_parameters.list.template Read<RW> (output_directory, frame);
	}
//#####################################################################
// Function Write_Output_Files
//#####################################################################
	template<class RW>
	void Write_Output_Files (const std::string& output_directory, const int first_frame, const int frame) const
	{
		std::string prefix = output_directory + "/";

		if (deformable_body_parameters.write)
		{
			if (write_static_variables_every_frame) deformable_body_parameters.list.template Write_Static_Variables<RW> (prefix, frame);
			else if (frame == first_frame) deformable_body_parameters.list.template Write_Static_Variables<RW> (prefix);

			deformable_body_parameters.list.template Write_Dynamic_Variables<RW> (prefix, frame);
		}

		if (rigid_body_parameters.write) rigid_body_parameters.list.template Write<RW> (output_directory, frame);
	}
//#####################################################################
};
}
#endif
