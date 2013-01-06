//#####################################################################
// Copyright 2004, Ron Fedkiw,  Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class PARTICLE_ATTRIBUTE
//#####################################################################
//
// True base class for particle attributes. Inherits from the untemplatized version so that we can have a list of them.
//
//#####################################################################
#ifndef __PARTICLE_ATTRIBUTE__
#define __PARTICLE_ATTRIBUTE__

#include "PARTICLE_ATTRIBUTE_UNTEMPLATIZED.h"
#include "../Arrays/ARRAY.h"
namespace PhysBAM
{

class PARTICLE;

template<class T>
class PARTICLE_ATTRIBUTE: public PARTICLE_ATTRIBUTE_UNTEMPLATIZED
{
public:
	ARRAY<T> array;

	PARTICLE_ATTRIBUTE()
	{}

	virtual void Clean_Up_Memory()
	{
		array.Resize_Array (0);
	}

	void Resize_Array (const int number_of_particles)
	{
		array.Resize_Array (number_of_particles);
	}

	T& operator() (const int i)
	{
		return array (i);
	}

	const T& operator() (const int i) const
	{
		return array (i);
	}

	void Copy_Particle (const int from, const int to)
	{
		array (to) = array (from);
	}

	void Copy_Particle (const PARTICLE_ATTRIBUTE_UNTEMPLATIZED* from_attribute, const int from, const int to)
	{
		array (to) = (* ( (PARTICLE_ATTRIBUTE<T>*) from_attribute)) (from);
	}

	void Print (std::ostream& output_stream, const std::string& name, const int i) const
	{
		if (array.Valid_Index (i)) output_stream << name << " = " << array (i) << std::endl;
	}

	template<class RW>
	void Read (std::istream& input_stream)
	{
		array.template Read<RW> (input_stream);
	}

	template<class RW>
	void Write (std::ostream& output_stream, const int number_of_particles) const
	{
		array.template Write_Prefix<RW> (output_stream, number_of_particles);       // write compacted
	}

//#####################################################################
};
}
#endif
