//#####################################################################
// Copyright 2004, Zhaosheng Bao, Ron Fedkiw, Andrew Selle, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "BOX_HIERARCHY.h"
using namespace PhysBAM;
extern bool PHYSBAM_THREADED_RUN;
//#####################################################################
// Function Set_Leaf_Boxes
//#####################################################################
template<class T> void BOX_HIERARCHY<T>::
Set_Leaf_Boxes (const ARRAY<BOX_3D<T> >& boxes, const bool reinitialize)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Hierarchy_Using_KD_Tree
//#####################################################################
template<class T> void BOX_HIERARCHY<T>::
Initialize_Hierarchy_Using_KD_Tree()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Update_Nonleaf_Boxes
//#####################################################################
template<class T> void BOX_HIERARCHY<T>::
Update_Nonleaf_Boxes()
{
	for (int k = leaves + 1; k <= box_hierarchy.m; k++)
		box_hierarchy (k) = BOX_3D<T>::Combine (box_hierarchy (children (1, k - leaves)), box_hierarchy (children (2, k - leaves)));
}

//#####################################################################
// Function Calculate_Bounding_Box_Radii
//#####################################################################
// at the boxes center
template<class T> void BOX_HIERARCHY<T>::
Calculate_Bounding_Box_Radii (const ARRAY<BOX_3D<T> >& bounding_boxes, ARRAY<T>& radius)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Update_Nonleaf_Box_Radii
//#####################################################################
template<class T> void BOX_HIERARCHY<T>::
Update_Nonleaf_Box_Radii()
{
	for (int k = leaves + 1; k <= box_radius.m; k++)
	{
		int box1, box2;
		children.Get (k - leaves, box1, box2);
		VECTOR_3D<T> center = box_hierarchy (k).Center();
		box_radius (k) = max ( (box_hierarchy (box1).Center() - center).Magnitude() + box_radius (box1), (box_hierarchy (box2).Center() - center).Magnitude() + box_radius (box2));
	}
}

//#####################################################################
// Function Dual_Intersection_List_Against_Self
//#####################################################################
template<class T> void BOX_HIERARCHY<T>::
Dual_Intersection_List_Against_Self (const int box, LIST_ARRAY<VECTOR_2D<int> >& dual_intersection_list, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

template<class T> void BOX_HIERARCHY<T>::
Dual_Intersection_List_Against_Self_Helper (long thread_id, void* helper_raw)
{
	NOT_IMPLEMENTED();
}

template<class T> void BOX_HIERARCHY<T>::
Dual_Intersection_List_Against_Self_Parallel (const int box, LIST_ARRAY<VECTOR_2D<int> >& dual_intersection_list, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

template<class T> void BOX_HIERARCHY<T>::
Dual_Intersection_List_Against_Self_Serial (const int box, LIST_ARRAY<VECTOR_2D<int> >& dual_intersection_list, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Dual_Intersection_List_Against_Other
//#####################################################################
template<class T> void BOX_HIERARCHY<T>::
Dual_Intersection_List_Against_Other (const int box, BOX_HIERARCHY<T> *other_hierarchy, LIST_ARRAY<VECTOR_2D<int> >& dual_intersection_list, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

template<class T> void BOX_HIERARCHY<T>::
Dual_Intersection_List_Against_Other_Helper (long thread_id, void* helper_raw)
{
	NOT_IMPLEMENTED();
}

template<class T> void BOX_HIERARCHY<T>::
Dual_Intersection_List_Against_Other_Parallel (const int box, BOX_HIERARCHY<T> *other_hierarchy, LIST_ARRAY<VECTOR_2D<int> >& dual_intersection_list, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

template<class T> void BOX_HIERARCHY<T>::
Dual_Intersection_List_Against_Other_Serial (const int box, BOX_HIERARCHY<T> *other_hierarchy, LIST_ARRAY<VECTOR_2D<int> >& dual_intersection_list, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Intersection_List
//#####################################################################
template<class T> void BOX_HIERARCHY<T>::
Intersection_List (const int box, const VECTOR_3D<T>& point, LIST_ARRAY<int>& intersection_list, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Intersection_List
//#####################################################################
template<class T> void BOX_HIERARCHY<T>::
Intersection_List (const int box, const BOX_3D<T>& test_box, LIST_ARRAY<int>& intersection_list, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Intersection_List
//#####################################################################
template<class T> void BOX_HIERARCHY<T>::
Intersection_List (const int box, const ORIENTED_BOX_3D<T>& test_box, LIST_ARRAY<int>& intersection_list) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Intersection_List
//#####################################################################
template<class T> void BOX_HIERARCHY<T>::
Intersection_List (const int box, const PLANE<T>& test_plane, LIST_ARRAY<int>& intersection_list, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Intersection_List
//#####################################################################
template<class T> void BOX_HIERARCHY<T>::
Intersection_List (const int box, const IMPLICIT_SURFACE<T>& implicit_surface, const MATRIX_3X3<T>& rotation, const VECTOR_3D<T>& translation, LIST_ARRAY<int>& intersection_list, const T contour_value) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################


template class BOX_HIERARCHY<float>;
template class BOX_HIERARCHY<double>;
