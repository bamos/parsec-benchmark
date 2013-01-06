//#####################################################################
// Copyright 2004, Zhaosheng Bao, Ron Fedkiw, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class TETRAHEDRON_HIERARCHY
//#####################################################################
#ifndef __TETRAHEDRON_HIERARCHY__
#define __TETRAHEDRON_HIERARCHY__

#include "BOX_HIERARCHY.h"
#include "../Grids/TETRAHEDRON_MESH.h"
#include "../Particles/SOLIDS_PARTICLE.h"
#include "../Geometry/TETRAHEDRON.h"
namespace PhysBAM
{

template<class T> class RIGID_BODY_STATE_3D;

template<class T>
class TETRAHEDRON_HIERARCHY: public BOX_HIERARCHY<T>
{
public:
	using BOX_HIERARCHY<T>::leaves;
	using BOX_HIERARCHY<T>::root;
	using BOX_HIERARCHY<T>::parents;
	using BOX_HIERARCHY<T>::children;
	using BOX_HIERARCHY<T>::box_hierarchy;
	using BOX_HIERARCHY<T>::box_radius;
	using BOX_HIERARCHY<T>::Update_Nonleaf_Boxes;

	TETRAHEDRON_MESH& tetrahedron_mesh;
	SOLIDS_PARTICLE<T, VECTOR_3D<T> >& particles;
	LIST_ARRAY<TETRAHEDRON<T> >* tetrahedron_list;

	TETRAHEDRON_HIERARCHY (TETRAHEDRON_MESH& tetrahedron_mesh_input, SOLIDS_PARTICLE<T, VECTOR_3D<T> >& particles_input, const bool update_boxes = true)
		: tetrahedron_mesh (tetrahedron_mesh_input), particles (particles_input), tetrahedron_list (0)
	{
		if (tetrahedron_mesh.tetrahedrons.m)
		{
			Initialize_Hierarchy_Using_KD_Tree();

			if (update_boxes) Update_Boxes();
		}
		else
		{
			leaves = 0;
			root = 0;
		}
	}

	TETRAHEDRON_HIERARCHY (TETRAHEDRON_MESH& tetrahedron_mesh_input, SOLIDS_PARTICLE<T, VECTOR_3D<T> >& particles_input, LIST_ARRAY<TETRAHEDRON<T> >& tetrahedron_list_input,
			       const bool update_boxes = true)
		: tetrahedron_mesh (tetrahedron_mesh_input), particles (particles_input), tetrahedron_list (&tetrahedron_list_input)
	{
		if (tetrahedron_mesh.tetrahedrons.m)
		{
			Initialize_Hierarchy_Using_KD_Tree();

			if (update_boxes) Update_Boxes();
		}
		else
		{
			leaves = 0;
			root = 0;
		}
	}

	virtual ~TETRAHEDRON_HIERARCHY()
	{}

	void Update_Boxes (const T extra_thickness = 0)
	{
		Update_Leaf_Boxes (extra_thickness);
		Update_Nonleaf_Boxes();
	}

	void Update_Boxes (const ARRAY<VECTOR_3D<T> >& X, const T extra_thickness = 0) // use X instead of the current particle positions
	{
		Update_Leaf_Boxes (X, extra_thickness);
		Update_Nonleaf_Boxes();
	}

	void Update_Boxes (const ARRAY<VECTOR_3D<T> >& start_X, const ARRAY<VECTOR_3D<T> >& end_X, const T extra_thickness = 0) // bound tetrahedrons moving from start_X to end_X
	{
		Update_Leaf_Boxes (start_X, end_X, extra_thickness);
		Update_Nonleaf_Boxes();
	}

	void Update_Boxes (const RIGID_BODY_STATE_3D<T>& start_state, const RIGID_BODY_STATE_3D<T>& end_state, const T extra_thickness = 0) // for a moving rigid body
	{
		Update_Leaf_Boxes (start_state, end_state, extra_thickness);
		Update_Nonleaf_Boxes();
	}

	void Update_Leaf_Boxes (const T extra_thickness = 0)
	{
		Calculate_Bounding_Boxes (box_hierarchy);

		if (extra_thickness) Thicken_Leaf_Boxes (extra_thickness);
	}

	void Update_Leaf_Boxes (const ARRAY<VECTOR_3D<T> >& X, const T extra_thickness = 0) // use X instead of the current particle positions
	{
		Calculate_Bounding_Boxes (box_hierarchy, X);

		if (extra_thickness) Thicken_Leaf_Boxes (extra_thickness);
	}

	void Update_Leaf_Boxes (const ARRAY<VECTOR_3D<T> >& start_X, const ARRAY<VECTOR_3D<T> >& end_X, const T extra_thickness = 0) // bound tetrahedrons moving from start_X to end_X
	{
		Calculate_Bounding_Boxes (box_hierarchy, start_X, end_X);

		if (extra_thickness) Thicken_Leaf_Boxes (extra_thickness);
	}

	void Update_Leaf_Boxes (const RIGID_BODY_STATE_3D<T>& start_state, const RIGID_BODY_STATE_3D<T>& end_state, const T extra_thickness = 0) // for a moving rigid body
	{
		Calculate_Bounding_Boxes (box_hierarchy, start_state, end_state);

		if (extra_thickness) Thicken_Leaf_Boxes (extra_thickness);
	}

	void Calculate_Bounding_Boxes (ARRAY<BOX_3D<T> >& bounding_boxes)
	{
		Calculate_Bounding_Boxes (bounding_boxes, particles.X.array);
	}

//#####################################################################
	void Initialize_Hierarchy_Using_KD_Tree();
	void Calculate_Bounding_Boxes (ARRAY<BOX_3D<T> >& bounding_boxes, const ARRAY<VECTOR_3D<T> >& X);
	void Calculate_Bounding_Boxes (ARRAY<BOX_3D<T> >& bounding_boxes, const ARRAY<VECTOR_3D<T> >& start_X, const ARRAY<VECTOR_3D<T> >& end_X);
	void Calculate_Bounding_Boxes (ARRAY<BOX_3D<T> >& bounding_boxes, const RIGID_BODY_STATE_3D<T>& start_state, const RIGID_BODY_STATE_3D<T>& end_state);
	void Calculate_Bounding_Box_Radii (const ARRAY<BOX_3D<T> >& bounding_boxes, ARRAY<T>& radius);
//#####################################################################
};
}
#endif

