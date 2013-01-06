//#####################################################################
// Copyright 2003-2004, Josh, Bao, Ron Fedkiw, Geoffrey Irving, Neil Molino.
// This file is part of PhysBAM whose distribution is governed by the license  contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE
//#####################################################################
#include "EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE.h"
using namespace PhysBAM;
//#####################################################################
// Function Create_Boundary_Surface_From_Manifold_Embedded_Surface
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Create_Boundary_Surface_From_Manifold_Embedded_Surface (const bool verbose)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Create_Boundary_Surface
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Create_Boundary_Surface (LIST_ARRAY<int>& particle_map, LIST_ARRAY<int>& map_to_old_embedded_particles, const T perturb_amount, const bool restart, const bool verbose)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Create_Boundary_Surface
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Create_Boundary_Surface (bool verbose)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Perturb_Nodes_For_Collision_Freeness
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Perturb_Nodes_For_Collision_Freeness (LIST_ARRAY<int>& map_to_old_embedded_particles, const T perturb_amount)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Perturb_Nodes_For_Collision_Freeness
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Aggressive_Perturb_Nodes_For_Collision_Freeness (LIST_ARRAY<int>& map_to_old_embedded_particles, const T perturb_amount)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Center_Octahedron_In_Material
//#####################################################################
template<class T> bool EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Center_Octahedron_In_Material (const int tetrahedron)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Update_Particle_Positions
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Update_Particle_Positions()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Update_Particle_Velocities
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Update_Particle_Velocities()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Update_Particle_Masses
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Update_Particle_Masses()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_To_Boundary_Surface_Tetrahedrons_Of_Material
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Add_To_Boundary_Surface_Tetrahedrons_Of_Material (const bool verbose)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_To_Boundary_Surface_Tetrahedron
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Add_To_Boundary_Surface_Tetrahedron (const int i, const int j, const int k, const int l)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_To_Boundary_Surface_Tetrahedron_Face
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Add_To_Boundary_Surface_Tetrahedron_Face (const int i, const int j, const int l, const bool is_clockwise)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_To_Boundary_Surface_Triangle
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Add_To_Boundary_Surface_Triangle (int x1, int x2, int x3, const bool is_clockwise)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_To_Boundary_Surface_Quad
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Add_To_Boundary_Surface_Quad_Cut (const int il, const int jl, const int jk, const int ik, const bool is_clockwise)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_To_Boundary_Surface_Quad
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Add_To_Boundary_Surface_Planar_Quad (const int x1, const int x2, const int x3, const int x4, const bool is_clockwise)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_To_Boundary_Surface_Subtetrahedron_And_Subprism
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Add_To_Boundary_Surface_Subtetrahedron_And_Subprism (const int tetrahedron, const int embedded_triangle1, const int i, const int j, const int k, const int l)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_To_Boundary_Surface_Subtetrahedron
//#####################################################################
template<class T>
void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Add_To_Boundary_Surface_Subtetrahedron (const int i, const int j, const int k, const int l, const bool is_clockwise)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_To_Boundary_Surface_Subprism
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Add_To_Boundary_Surface_Subprism (const int i, const int j, const int k, const int l, const bool is_clockwise)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_To_Boundary_Surface_Subtetrahedrons_And_Wrick
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Add_To_Boundary_Surface_Subtetrahedrons_And_Wrick (const int tetrahedron, const int embedded_triangle1, const int embedded_triangle2, const int i, const int j, const int k, const int l)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_To_Boundary_Surface_Wrick
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Add_To_Boundary_Surface_Wrick (const int i, const int j, const int k, const int l, const bool is_clockwise)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_To_Boundary_Surface_Wedge_On_Either_Side
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Add_To_Boundary_Surface_Wedge_On_Either_Side (const int tetrahedron, const int embedded_triangle1, const int embedded_triangle2, const int i, const int j, const int k, const int l)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_To_Boundary_Surface_Subwedge
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Add_To_Boundary_Surface_Subwedge (const int i, const int j, const int k, const int l, const bool is_clockwise)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_To_Boundary_Surface_Subtetrahedron_Tets_And_Oct_Plus_Tet
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Add_To_Boundary_Surface_Subtetrahedron_Tets_And_Oct_Plus_Tet (const int tetrahedron, const int i, const int j, const int k, const int l)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_To_Boundary_Surface_Oct_Plus_Tet
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Add_To_Boundary_Surface_Oct_Plus_Tet (const int i, const int j, const int k, const int l, const bool is_clockwise)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_To_Boundary_Surface_Half_Oct_Plus_Tet
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Add_To_Boundary_Surface_Half_Oct_Plus_Tet (const int i, const int j, const int k, const int l, const bool is_clockwise)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_To_Boundary_Surface_Subtetrahedron_And_Wedge_And_Half_Oct_Plus_Tet
//#####################################################################
template<class T> void EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<T>::
Add_To_Boundary_Surface_Subtetrahedron_And_Wedge_And_Half_Oct_Plus_Tet (const int tetrahedron, const int i, const int j, const int k, const int l)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
template class EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<float>;
template class EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE<double>;
