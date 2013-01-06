//#####################################################################
// Copyright 2004, Zhaosheng Bao, Ron Fedkiw.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "TETRAHEDRON_HIERARCHY.h"
#include "../Rigid_Bodies/RIGID_BODY_STATE_3D.h"
using namespace PhysBAM;
//#####################################################################
// Function Initialize_Hierarchy_Using_KD_Tree
//#####################################################################
template<class T> void TETRAHEDRON_HIERARCHY<T>::
Initialize_Hierarchy_Using_KD_Tree()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Calculate_Bounding_Boxes
//#####################################################################
template<class T> void TETRAHEDRON_HIERARCHY<T>::
Calculate_Bounding_Boxes (ARRAY<BOX_3D<T> >& bounding_boxes, const ARRAY<VECTOR_3D<T> >& X)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Calculate_Bounding_Boxes
//#####################################################################
template<class T> void TETRAHEDRON_HIERARCHY<T>::
Calculate_Bounding_Boxes (ARRAY<BOX_3D<T> >& bounding_boxes, const ARRAY<VECTOR_3D<T> >& start_X, const ARRAY<VECTOR_3D<T> >& end_X)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Calculate_Bounding_Boxes
//#####################################################################
template<class T> void TETRAHEDRON_HIERARCHY<T>::
Calculate_Bounding_Boxes (ARRAY<BOX_3D<T> >& bounding_boxes, const RIGID_BODY_STATE_3D<T>& start_state, const RIGID_BODY_STATE_3D<T>& end_state)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Calculate_Bounding_Box_Radii
//#####################################################################
// at the boxes center, but may be a tighter bound on the tetrahedron than the box
template<class T> void TETRAHEDRON_HIERARCHY<T>::
Calculate_Bounding_Box_Radii (const ARRAY<BOX_3D<T> >& bounding_boxes, ARRAY<T>& radius)
{
	NOT_IMPLEMENTED();
}

//#####################################################################

template class TETRAHEDRON_HIERARCHY<float>;
template class TETRAHEDRON_HIERARCHY<double>;
