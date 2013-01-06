//#####################################################################
// Copyright 2004, Ron Fedkiw, Andrew Selle, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class BOX_HIERARCHY
//#####################################################################
#ifndef __BOX_HIERARCHY__
#define __BOX_HIERARCHY__

#include "../Arrays/ARRAY.h"
#include "../Arrays/LIST_ARRAY.h"
#include "../Arrays/LIST_ARRAYS.h"
#include "../Geometry/BOX_3D.h"
#include "../Geometry/ORIENTED_BOX_3D.h"
#include "../Geometry/IMPLICIT_SURFACE.h"
#include "../Geometry/PLANE.h"
#include "../Data_Structures/STACK.h"
namespace PhysBAM
{

template<class T> class VECTOR_2D;
template<class T> class KD_TREE_NODE_3D;

template<class T>
class BOX_HIERARCHY
{
public:
	int leaves, root;
	LIST_ARRAY<int> parents;
	LIST_ARRAYS<int> children;
	ARRAY<BOX_3D<T> > box_hierarchy;
	mutable STACK<int> traversal_stack;
	mutable STACK<VECTOR_2D<int> > dual_traversal_stack;
	ARRAY<T> box_radius;

	BOX_HIERARCHY()
		: children (2, 0)
	{}

	virtual ~BOX_HIERARCHY()
	{}

	bool Leaf (const int box) const
	{
		return box <= leaves;
	}

	void Thicken_Leaf_Boxes (const T extra_thickness)
	{
		for (int k = 1; k <= leaves; k++) box_hierarchy (k).Change_Size (extra_thickness);
	}

	void Thicken_Leaf_Boxes (const T extra_thickness, const VECTOR_2D<int>& range)
	{
		for (int k = range.x; k <= range.y; k++) box_hierarchy (k).Change_Size (extra_thickness);
	}

	void Update_Box_Radii()
	{
		Update_Leaf_Box_Radii();
		Update_Nonleaf_Box_Radii();
	}

	void Update_Leaf_Box_Radii()
	{
		Calculate_Bounding_Box_Radii (box_hierarchy, box_radius);
	}

	void Dual_Intersection_List_Against_Self (LIST_ARRAY<VECTOR_2D<int> >& dual_intersection_list, const T thickness_over_two = 0) const
	{
		Dual_Intersection_List_Against_Self (root, dual_intersection_list, thickness_over_two);
	}

	void Dual_Intersection_List_Against_Other (BOX_HIERARCHY<T>* other_hierarchy, LIST_ARRAY<VECTOR_2D<int> >& dual_intersection_list, const T thickness_over_two = 0) const
	{
		Dual_Intersection_List_Against_Other (root, other_hierarchy, dual_intersection_list, thickness_over_two);
	}

	void Intersection_List (const VECTOR_3D<T>& point, LIST_ARRAY<int>& intersection_list, const T thickness_over_two = 0) const
	{
		Intersection_List (root, point, intersection_list, thickness_over_two);
	}

	void Intersection_List (const BOX_3D<T>& test_box, LIST_ARRAY<int>& intersection_list, const T thickness_over_two = 0) const
	{
		Intersection_List (root, test_box, intersection_list, thickness_over_two);
	}

	void Intersection_List (const ORIENTED_BOX_3D<T>& test_box, LIST_ARRAY<int>& intersection_list) const
	{
		Intersection_List (root, test_box, intersection_list);
	}

	void Intersection_List (const PLANE<T>& test_plane, LIST_ARRAY<int>& intersection_list, const T thickness_over_two = 0) const
	{
		Intersection_List (root, test_plane, intersection_list, thickness_over_two);
	}

	void Intersection_List (const IMPLICIT_SURFACE<T>& implicit_surface, const MATRIX_3X3<T>& rotation, const VECTOR_3D<T>& translation, LIST_ARRAY<int>& intersection_list, const T contour_value = 0) const
	{
		Intersection_List (root, implicit_surface, rotation, translation, intersection_list, contour_value);
	}

//#####################################################################
public:
	virtual void Initialize_Hierarchy_Using_KD_Tree();
	void Set_Leaf_Boxes (const ARRAY<BOX_3D<T> >& boxes, const bool reinitialize = false);
	void Update_Nonleaf_Boxes();
	virtual void Calculate_Bounding_Box_Radii (const ARRAY<BOX_3D<T> >& bounding_boxes, ARRAY<T>& radius);
	void Update_Nonleaf_Box_Radii();
	// for internal use - but octrees use them as well so they're not private
	void Dual_Intersection_List_Against_Self (const int box, LIST_ARRAY<VECTOR_2D<int> >& dual_intersection_list, const T thickness_over_two) const;
	static void Dual_Intersection_List_Against_Self_Helper (long thread_id, void* helper_raw);
	void Dual_Intersection_List_Against_Self_Parallel (const int box, LIST_ARRAY<VECTOR_2D<int> >& dual_intersection_list, const T thickness_over_two) const;
	void Dual_Intersection_List_Against_Self_Serial (const int box, LIST_ARRAY<VECTOR_2D<int> >& dual_intersection_list, const T thickness_over_two) const;
	void Dual_Intersection_List_Against_Other (const int box, BOX_HIERARCHY<T>* other_hierarchy, LIST_ARRAY<VECTOR_2D<int> >& dual_intersection_list, const T thickness_over_two) const;
	static void Dual_Intersection_List_Against_Other_Helper (long thread_id, void* helper_raw);
	void Dual_Intersection_List_Against_Other_Parallel (const int box, BOX_HIERARCHY<T>* other_hierarchy, LIST_ARRAY<VECTOR_2D<int> >& dual_intersection_list, const T thickness_over_two) const;
	void Dual_Intersection_List_Against_Other_Serial (const int box, BOX_HIERARCHY<T>* other_hierarchy, LIST_ARRAY<VECTOR_2D<int> >& dual_intersection_list, const T thickness_over_two) const;
	void Intersection_List (const int box, const VECTOR_3D<T>& point, LIST_ARRAY<int>& intersection_list, const T thickness_over_two) const;
	void Intersection_List (const int box, const BOX_3D<T>& test_box, LIST_ARRAY<int>& intersection_list, const T thickness_over_two) const;
	void Intersection_List (const int box, const ORIENTED_BOX_3D<T>& test_box, LIST_ARRAY<int>& intersection_list) const;
	void Intersection_List (const int box, const PLANE<T>& test_plane, LIST_ARRAY<int>& intersection_list, const T thickness_over_two) const;
	void Intersection_List (const int box, const IMPLICIT_SURFACE<T>& implicit_surface, const MATRIX_3X3<T>& rotation, const VECTOR_3D<T>& translation, LIST_ARRAY<int>& intersection_list,
				const T contour_value) const;
//#####################################################################
};
}
#endif

