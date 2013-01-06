//#####################################################################
// Copyright 2002-2004, Ron Fedkiw, Sergey Koltakov, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SEGMENTED_CURVE_3D
//#####################################################################
#ifndef __SEGMENTED_CURVE_3D__
#define __SEGMENTED_CURVE_3D__

#include <iostream>
#include "SEGMENT_3D.h"
#include "BOX_3D.h"
#include "../Grids/GRID_1D.h"
#include "../Grids/SEGMENT_MESH.h"
#include "../Particles/SOLIDS_PARTICLE.h"
#include "../Matrices_And_Vectors/VECTOR_3D.h"
namespace PhysBAM
{

template<class T>
class SEGMENTED_CURVE_3D
{
public:
	SEGMENT_MESH& segment_mesh;
	SOLIDS_PARTICLE<T, VECTOR_3D<T> >& particles;
	BOX_3D<T>* bounding_box;
	LIST_ARRAY<SEGMENT_3D<T> >* segment_list;

	SEGMENTED_CURVE_3D (SEGMENT_MESH& segment_mesh_input, SOLIDS_PARTICLE<T, VECTOR_3D<T> >& particles_input)
		: segment_mesh (segment_mesh_input), particles (particles_input), bounding_box (0), segment_list (0)
	{}

	virtual ~SEGMENTED_CURVE_3D()
	{
		delete bounding_box;
		delete segment_list;
	}

	static SEGMENTED_CURVE_3D* Create()
	{
		return new SEGMENTED_CURVE_3D (* (new SEGMENT_MESH), * (new SOLIDS_PARTICLE<T, VECTOR_3D<T> >));
	}

	void Destroy_Data() // this is dangerous
	{
		delete &segment_mesh;
		delete &particles;
	}

	void Clean_Up_Memory()
	{
		delete bounding_box;
		bounding_box = 0;
		delete segment_list;
		segment_list = 0;
	}

	void Initialize_Segment_Hierarchy (const bool update_boxes = true) // creates and updates the boxes as well
	{}

	template<class RW>
	void Read (std::istream& input_stream)
	{
		segment_mesh.template Read<RW> (input_stream);
		particles.template Read_State<RW> (input_stream);
		particles.X.template Read<RW> (input_stream);
	}

	template<class RW>
	void Write (std::ostream& output_stream) const
	{
		segment_mesh.template Write<RW> (output_stream);
		particles.template Write_State<RW> (output_stream);
		particles.X.template Write<RW> (output_stream, particles.number);
	}

//#####################################################################
	void Update_Segment_List(); // updates the segments assuming the particle positions are already updated
	void Update_Bounding_Box();
	void Initialize_Straight_Mesh_And_Particles (const GRID_1D<T>& grid);
	T Average_Edge_Length() const;
//#####################################################################
};
}
#endif
