//#####################################################################
// Copyright 2004-2005, Ron Fedkiw, Geoffrey Irving, Joseph Teran, Rachel Weinstein.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "TETRAHEDRON_COLLISION_BODY.h"
using namespace PhysBAM;
//#####################################################################
// Implicit_Surface_Lazy_Inside_And_Value
//#####################################################################
template<class T> bool TETRAHEDRON_COLLISION_BODY<T>::
Implicit_Surface_Lazy_Inside_And_Value (const VECTOR_3D<T>& location, T& phi, T contour_value) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Implicit_Surface_Lazy_Outside_Extended_Levelset_And_Value
//#####################################################################
template<class T> bool TETRAHEDRON_COLLISION_BODY<T>::
Implicit_Surface_Lazy_Outside_Extended_Levelset_And_Value (const VECTOR_3D<T>& location, T& phi_value, T contour_value) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Implicit_Surface_Normal
//#####################################################################
template<class T> VECTOR_3D<T> TETRAHEDRON_COLLISION_BODY<T>::
Implicit_Surface_Normal (const VECTOR_3D<T>& location, const int aggregate) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Implicit_Surface_Normal
//#####################################################################
template<class T> VECTOR_3D<T> TETRAHEDRON_COLLISION_BODY<T>::
Implicit_Surface_Normal (const VECTOR_3D<T>& location, T& phi_value, const int aggregate, const int location_particle_index) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Implicit_Surface_Extended_Normal
//#####################################################################
template<class T> VECTOR_3D<T> TETRAHEDRON_COLLISION_BODY<T>::
Implicit_Surface_Extended_Normal (const VECTOR_3D<T>& location, T& phi_value, const int aggregate, const int location_particle_index) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Get_Tetrahedron_Near_Point
//#####################################################################
template<class T> int TETRAHEDRON_COLLISION_BODY<T>::
Get_Tetrahedron_Near_Point (const VECTOR_3D<T>& point, VECTOR_3D<T>& weights, const int particle_index) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Get_Surface_Triangle
//#####################################################################
template<class T> int TETRAHEDRON_COLLISION_BODY<T>::
Get_Surface_Triangle (const int tetrahedron_index, const VECTOR_3D<T>& tetrahedron_weights, VECTOR_3D<T>& surface_weights, const bool omit_outside_points, const bool omit_inside_points,
		      bool* inside) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Adjust_Point_Face_Collision_Position_And_Velocity
//#####################################################################
template<class T> void TETRAHEDRON_COLLISION_BODY<T>::
Adjust_Point_Face_Collision_Position_And_Velocity (const int triangle_index, VECTOR_3D<T>& X, VECTOR_3D<T>& V, const T mass, const T dt, const VECTOR_3D<T>& weights,
		VECTOR_3D<T>& position_change)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
template class TETRAHEDRON_COLLISION_BODY<float>;
template class TETRAHEDRON_COLLISION_BODY<double>;
