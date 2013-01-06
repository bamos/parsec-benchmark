//#####################################################################
// Copyright 2002-2004, Ron Fedkiw, Sergey Koltakov, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SEGMENTED_CURVE_3D
//#####################################################################
#include "SEGMENTED_CURVE_3D.h"
#include "../Utilities/DEBUG_UTILITIES.h"
using namespace PhysBAM;
//#####################################################################
// Function Update_Segment_List
//#####################################################################
template<class T> void SEGMENTED_CURVE_3D<T>::
Update_Segment_List() // updates the segments assuming the particle positions are already updated
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Update_Bounding_Box
//#####################################################################
template<class T> void SEGMENTED_CURVE_3D<T>::
Update_Bounding_Box()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Straight_Mesh_And_Particles
//#####################################################################
template<class T> void SEGMENTED_CURVE_3D<T>::
Initialize_Straight_Mesh_And_Particles (const GRID_1D<T>& grid)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Average_Edge_Length
//#####################################################################
template<class T> T SEGMENTED_CURVE_3D<T>::
Average_Edge_Length() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
template class SEGMENTED_CURVE_3D<float>;
template class SEGMENTED_CURVE_3D<double>;
