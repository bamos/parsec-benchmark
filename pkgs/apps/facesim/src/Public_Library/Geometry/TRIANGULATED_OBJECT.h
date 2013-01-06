//#####################################################################
// Copyright 2004, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class TRIANGLULATED_OBJECT
//#####################################################################
#ifndef __TRIANGULATED_OBJECT__
#define __TRIANGULATED_OBJECT__

#include <iostream>
#include "../Grids/TRIANGLE_MESH.h"
#include "../Particles/SOLIDS_PARTICLE.h"
namespace PhysBAM
{

template<class T, class TV>
class TRIANGULATED_OBJECT
{
public:
	TRIANGLE_MESH& triangle_mesh;
	SOLIDS_PARTICLE<T, TV>& particles;

	TRIANGULATED_OBJECT (TRIANGLE_MESH& triangle_mesh_input, SOLIDS_PARTICLE<T, TV>& particles_input)
		: triangle_mesh (triangle_mesh_input), particles (particles_input)
	{}

	virtual ~TRIANGULATED_OBJECT()
	{}

	void Discard_Valence_Zero_Particles_And_Renumber()
	{
		ARRAY<int> condensation_mapping;
		Discard_Valence_Zero_Particles_And_Renumber (condensation_mapping);
	}

	virtual void Refresh_Auxiliary_Structures()
	{
		triangle_mesh.Refresh_Auxiliary_Structures();
	}

	template<class RW>
	void Read (std::istream& input_stream)
	{
		triangle_mesh.template Read<RW> (input_stream);
		particles.template Read_State<RW> (input_stream);
		particles.X.template Read<RW> (input_stream);
	}

	template<class RW>
	void Write (std::ostream& output_stream) const
	{
		triangle_mesh.template Write<RW> (output_stream);
		particles.template Write_State<RW> (output_stream);
		particles.X.template Write<RW> (output_stream, particles.number);
	}

//#####################################################################
	void Discard_Valence_Zero_Particles_And_Renumber (ARRAY<int>& condensation_mapping);
//#####################################################################
};
}
#endif

