//#####################################################################
// Copyright 2003, Eran Guendelman.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class RIGID_BODY_STATE_3D
//#####################################################################
#ifndef __RIGID_BODY_STATE_3D__
#define __RIGID_BODY_STATE_3D__

#include "../Matrices_And_Vectors/VECTOR_3D.h"
#include "../Read_Write/READ_WRITE_FUNCTIONS.h"
namespace PhysBAM
{

template<class T>
class RIGID_BODY_STATE_3D
{
public:
	T time;
	VECTOR_3D<T> position;
	QUATERNION<T> orientation;
	VECTOR_3D<T> velocity;
	VECTOR_3D<T> angular_momentum;
	VECTOR_3D<T> angular_velocity;

	RIGID_BODY_STATE_3D()
	{}

	template<class RW> void Read (std::istream& input_stream)
	{
		char version;
		Read_Binary<RW> (input_stream, version, time, position, orientation, velocity, angular_momentum, angular_velocity);
		assert (version == 1);
	}

	template<class RW> void Write (std::ostream& output_stream) const
	{
		char version = 1;
		Write_Binary<RW> (output_stream, version, time, position, orientation, velocity, angular_momentum, angular_velocity);
	}

//#####################################################################
};
}
#endif
