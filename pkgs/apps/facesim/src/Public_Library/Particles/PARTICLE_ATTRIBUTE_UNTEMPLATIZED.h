//#####################################################################
// Copyright 2004, Ron Fedkiw, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class PARTICLE_ATTRIBUTE_UNTEMPLATIZED
//#####################################################################
//
// This base class is not templatized so that we can have a list of them.
//
//#####################################################################
#ifndef __PARTICLE_ATTRIBUTE_UNTEMPLATIZED__
#define __PARTICLE_ATTRIBUTE_UNTEMPLATIZED__

namespace PhysBAM
{

class PARTICLE_ATTRIBUTE_UNTEMPLATIZED
{
public:
	PARTICLE_ATTRIBUTE_UNTEMPLATIZED()
	{}

	virtual ~PARTICLE_ATTRIBUTE_UNTEMPLATIZED()
	{}

//#####################################################################
	virtual void Clean_Up_Memory() = 0;
	virtual void Resize_Array (const int number_of_particles) = 0;
	virtual void Copy_Particle (const int from, const int to) = 0;
	virtual void Copy_Particle (const PARTICLE_ATTRIBUTE_UNTEMPLATIZED* from_attribute, const int from, const int to) = 0;
	virtual void Print (std::ostream& output_stream, const std::string& name, const int i) const = 0;
//#####################################################################
};
}
#endif
