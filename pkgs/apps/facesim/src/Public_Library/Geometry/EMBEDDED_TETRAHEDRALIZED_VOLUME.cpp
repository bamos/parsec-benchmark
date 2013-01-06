//#####################################################################
// Copyright 2003-2004, Zhaosheng Bao, Ronald Fedkiw, Geoffrey Irving, Neil Molino.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class EMBEDDED_TETRAHEDRALIZED_VOLUME
//#####################################################################
#include "EMBEDDED_TETRAHEDRALIZED_VOLUME.h"
using namespace PhysBAM;
//#####################################################################
// Tetrahedron_Containing_Triangle
//#####################################################################
template<class T> int EMBEDDED_TETRAHEDRALIZED_VOLUME<T>::
Tetrahedron_Containing_Triangle (const int embedded_triangle) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Embedded_Triangles_In_Tetrahedron
//#####################################################################
template<class T> int EMBEDDED_TETRAHEDRALIZED_VOLUME<T>::
Embedded_Triangles_In_Tetrahedron (const int tetrahedron, int& first_embedded_triangle, int& second_embedded_triangle, int& third_embedded_triangle, int& fourth_embedded_triangle)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_Embedded_Triangle_To_Embedded_Triangles_In_Tetrahedron
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME<T>::
Add_Embedded_Triangle_To_Embedded_Triangles_In_Tetrahedron (const int triangle)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_Emdedded_Particle_If_Not_Already_There
//#####################################################################
template<class T> int EMBEDDED_TETRAHEDRALIZED_VOLUME<T>::
Add_Embedded_Particle_If_Not_Already_There (const int node1, const int node2, T interpolation_fraction_input)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_Embedded_Triangle_If_Not_Already_There
//#####################################################################
// only updates embedded_triangle_mesh.incident_triangles and embedded_sub_elements_in_parent_element
template<class T> int EMBEDDED_TETRAHEDRALIZED_VOLUME<T>::
Add_Embedded_Triangle_If_Not_Already_There (const int embedded_particle1, const int embedded_particle2, const int embedded_particle3)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Embedded_Sub_Elements_In_Parent_Element
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME<T>::
Initialize_Embedded_Sub_Elements_In_Parent_Element()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Calculate_Triangulated_Surface_From_Levelset_On_Tetrahedron_Nodes
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME<T>::
Calculate_Triangulated_Surface_From_Levelset_On_Tetrahedron_Nodes (ARRAY<T>& phi, const bool discard_tetrahedra_outside_levelset, const bool verbose)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Write_Percentage_Of_Tetrahedra_With_Embedded_Triangles
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME<T>::
Write_Percentage_Of_Tetrahedra_With_Embedded_Triangles (const std::string& output_directory)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
template class EMBEDDED_TETRAHEDRALIZED_VOLUME<float>;
template class EMBEDDED_TETRAHEDRALIZED_VOLUME<double>;
