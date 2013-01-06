//#####################################################################
// Copyright 2002-2004, Chris Allocco, Robert Bridson, Ron Fedkiw, Geoffrey Irving, Eran Guendelman, Sergey Koltakov, Neil Molino, Igor Neverov, Andrew Selle, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class TRIANGULATED_SURFACE
//#####################################################################
#include "TRIANGULATED_SURFACE.h"
#include "../Grids/SEGMENT_MESH.h"
using namespace PhysBAM;
//#####################################################################
// Function Update_Triangle_List
//#####################################################################
template<class T> void TRIANGULATED_SURFACE<T>::
Update_Triangle_List()
{
	Update_Triangle_List (particles.X.array);
}
//#####################################################################
// Function Update_Triangle_List
//#####################################################################
template<class T> void TRIANGULATED_SURFACE<T>::
Update_Triangle_List (const ARRAY<VECTOR_3D<T> >& X)
{
	int number_triangles = triangle_mesh.triangles.m;

	if (!triangle_list) triangle_list = new LIST_ARRAY<TRIANGLE_3D<T> > (number_triangles);
	else triangle_list->Resize_Array (number_triangles);

	for (int k = 1; k <= number_triangles; k++)
	{
		int node1, node2, node3;
		triangle_mesh.triangles.Get (k, node1, node2, node3);
		(*triangle_list) (k).Specify_Three_Clockwise_Points (X (node1), X (node2), X (node3));
	}
}
//#####################################################################
// Function Update_Triangle_List
//#####################################################################
template<class T> void TRIANGULATED_SURFACE<T>::
Update_Triangle_List (const ARRAY<VECTOR_3D<T> >& X, const VECTOR_2D<int>& triangle_range)
{
	int number_triangles = triangle_mesh.triangles.m;

	if (!triangle_list) triangle_list = new LIST_ARRAY<TRIANGLE_3D<T> > (number_triangles);
	else triangle_list->Resize_Array (number_triangles, false, false);

	for (int k = triangle_range.x; k <= triangle_range.y; k++)
	{
		int node1, node2, node3;
		triangle_mesh.triangles.Get (k, node1, node2, node3);
		(*triangle_list) (k).Specify_Three_Clockwise_Points (X (node1), X (node2), X (node3));
	}
}
//#####################################################################
// Function Update_Bounding_Box
//#####################################################################
template<class T> void TRIANGULATED_SURFACE<T>::
Update_Bounding_Box()
{
	if (!bounding_box) bounding_box = new BOX_3D<T>();

	if (triangle_mesh.triangles.m > 0) bounding_box->Reset_Bounds (particles.X (triangle_mesh.triangles (1, 1)));

	for (int k = 1; k <= triangle_mesh.triangles.m; k++) for (int kk = 1; kk <= 3; kk++) bounding_box->Enlarge_To_Include_Point (particles.X (triangle_mesh.triangles (kk, k)));
}
//#####################################################################
// Function Initialize_Particle_Hierarchy
//#####################################################################
template<class T> void TRIANGULATED_SURFACE<T>::
Initialize_Particle_Hierarchy (const bool update_boxes) // creates and updates the boxes as well
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Torus_Mesh_And_Particles
//#####################################################################
template<class T> void TRIANGULATED_SURFACE<T>::
Initialize_Torus_Mesh_And_Particles (const int m, const int n, const T major_radius, const T minor_radius)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Segment_Lengths
//#####################################################################
template<class T> void TRIANGULATED_SURFACE<T>::
Initialize_Segment_Lengths()
{
	assert (triangle_mesh.segment_mesh);
	delete segment_lengths;
	segment_lengths = new ARRAY<T> (triangle_mesh.segment_mesh->segments.m);

	for (int t = 1; t <= triangle_mesh.segment_mesh->segments.m; t++)
		(*segment_lengths) (t) = (particles.X (triangle_mesh.segment_mesh->segments (1, t)) - particles.X (triangle_mesh.segment_mesh->segments (2, t))).Magnitude();
}
//#####################################################################
// Function Update_Vertex_Normals
//#####################################################################
template<class T> void TRIANGULATED_SURFACE<T>::
Update_Vertex_Normals()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Intersection
//#####################################################################
template<class T> bool TRIANGULATED_SURFACE<T>::
Intersection (RAY_3D<T>& ray, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Closest_Non_Intersecting_Point
//#####################################################################
template<class T> bool TRIANGULATED_SURFACE<T>::
Closest_Non_Intersecting_Point (RAY_3D<T>& ray, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Normal
//#####################################################################
template<class T> VECTOR_3D<T> TRIANGULATED_SURFACE<T>::
Normal (const VECTOR_3D<T>& location, const int aggregate) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Inside
//#####################################################################
template<class T> bool TRIANGULATED_SURFACE<T>::
Inside (const VECTOR_3D<T>& location, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Inside_Relative_To_Triangle
//#####################################################################
template<class T> bool TRIANGULATED_SURFACE<T>::
Inside_Relative_To_Triangle (const VECTOR_3D<T>& location, const int triangle_index_for_ray_test, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Inside_Using_Ray_Test
//#####################################################################
template<class T> bool TRIANGULATED_SURFACE<T>::
Inside_Using_Ray_Test (RAY_3D<T>& ray, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Outside
//#####################################################################
template<class T> bool TRIANGULATED_SURFACE<T>::
Outside (const VECTOR_3D<T>& location, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Boundary
//#####################################################################
template<class T> bool TRIANGULATED_SURFACE<T>::
Boundary (const VECTOR_3D<T>& location, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Inside_Any_Triangle
//#####################################################################
template<class T> bool TRIANGULATED_SURFACE<T>::
Inside_Any_Triangle (const VECTOR_3D<T>& location, int& triangle_id, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Surface
//#####################################################################
template<class T> VECTOR_3D<T> TRIANGULATED_SURFACE<T>::
Surface (const VECTOR_3D<T>& location, const T max_depth, const T thickness, int* closest_triangle, T* distance) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Signed_Solid_Angle_Of_Triangle_Web
//#####################################################################
template<class T> T TRIANGULATED_SURFACE<T>::
Signed_Solid_Angle_Of_Triangle_Web (const VECTOR_3D<T>& location, int web_root_node) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Check_For_Self_Intersection
//#####################################################################
template<class T> bool TRIANGULATED_SURFACE<T>::
Check_For_Self_Intersection (const T thickness, const bool update_bounding_boxes, LIST_ARRAYS<int>* intersecting_segment_triangle_pairs)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Find_First_Segment_Triangle_Intersection
//#####################################################################
template<class T> bool TRIANGULATED_SURFACE<T>::
Find_First_Segment_Triangle_Intersection (const SEGMENT_MESH& test_segment_mesh, const ARRAY<VECTOR_3D<T> >& X, const T thickness, const int max_coarsening_attempts,
		const bool update_bounding_boxes)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Segment_Triangle_Intersection
//#####################################################################
template<class T> bool TRIANGULATED_SURFACE<T>::
Segment_Triangle_Intersection (const SEGMENT_MESH& test_segment_mesh, const ARRAY<VECTOR_3D<T> >& X, const T thickness, const bool update_bounding_boxes,
			       LIST_ARRAYS<int>* intersecting_segment_triangle_pairs)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Get_Triangles_Near_Edges
//#####################################################################
template<class T> void TRIANGULATED_SURFACE<T>::
Get_Triangles_Near_Edges (ARRAY<LIST_ARRAY<int> >& triangles_near_edges, const SEGMENT_MESH& test_segment_mesh, const ARRAY<VECTOR_3D<T> >& X, const T thickness,
			  const bool update_bounding_boxes)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Centroid_Of_Neighbors
//#####################################################################
template<class T> VECTOR_3D<T> TRIANGULATED_SURFACE<T>::
Centroid_Of_Neighbors (const int node) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Calculate_Signed_Distance
//#####################################################################
template<class T> T TRIANGULATED_SURFACE<T>::
Calculate_Signed_Distance (const VECTOR_3D<T>& location, T thickness) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Calculate_Signed_Distance_Function
//#####################################################################
template<class T> void TRIANGULATED_SURFACE<T>::
Calculate_Signed_Distance_Function (const GRID_3D<T>& grid, ARRAYS_3D<T>& phi, bool print_progress)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Calculate_Heaviside_Function
//#####################################################################
template<class T> void TRIANGULATED_SURFACE<T>::
Calculate_Heaviside_Function (const GRID_3D<T>& grid, ARRAYS_3D<T>& phi, bool print_progress)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Calculate_Signed_Distance_Function
//#####################################################################
template<class T> void TRIANGULATED_SURFACE<T>::
Calculate_Signed_Distance_Function (const GRID_3D<T>& uniform_grid, OCTREE_GRID<T>& grid, ARRAY<T>& phi, const int maximum_depth, const T half_band_width, const bool verbose)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Linearly_Subdivide
//#####################################################################
template<class T> void TRIANGULATED_SURFACE<T>::
Linearly_Subdivide()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Loop_Subdivide
//#####################################################################
template<class T> void TRIANGULATED_SURFACE<T>::
Loop_Subdivide()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Root_Three_Subdivide
//#####################################################################
template<class T> void TRIANGULATED_SURFACE<T>::
Root_Three_Subdivide()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Total_Area
//#####################################################################
template<class T> T TRIANGULATED_SURFACE<T>::
Total_Area() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Thin_Shell_Mass
//#####################################################################
template<class T> T TRIANGULATED_SURFACE<T>::
Thin_Shell_Mass() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Minimum_Angle
//#####################################################################
template<class T> T TRIANGULATED_SURFACE<T>::
Minimum_Angle (int* index) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Maximum_Angle
//#####################################################################
template<class T> T TRIANGULATED_SURFACE<T>::
Maximum_Angle (int* index) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Average_Minimum_Angle
//#####################################################################
template<class T> T TRIANGULATED_SURFACE<T>::
Average_Minimum_Angle() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Average_Maximum_Angle
//#####################################################################
template<class T> T TRIANGULATED_SURFACE<T>::
Average_Maximum_Angle() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Minimum_Edge_Length
//#####################################################################
template<class T> T TRIANGULATED_SURFACE<T>::
Minimum_Edge_Length (int* index) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Maximum_Edge_Length
//#####################################################################
template<class T> T TRIANGULATED_SURFACE<T>::
Maximum_Edge_Length (int* index) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Average_Edge_Length
//#####################################################################
template<class T> T TRIANGULATED_SURFACE<T>::
Average_Edge_Length() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Maximum_Aspect_Ratio
//#####################################################################
template<class T> T TRIANGULATED_SURFACE<T>::
Maximum_Aspect_Ratio (int* index) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Average_Aspect_Ratio
//#####################################################################
template<class T> T TRIANGULATED_SURFACE<T>::
Average_Aspect_Ratio() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Minimum_Area
//#####################################################################
template<class T> T TRIANGULATED_SURFACE<T>::
Minimum_Area (int* index) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Minimum_Altitude
//#####################################################################
template<class T> T TRIANGULATED_SURFACE<T>::
Minimum_Altitude (int* index) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Set_Mass_Of_Particles
//#####################################################################
template<class T> void TRIANGULATED_SURFACE<T>::
Set_Mass_Of_Particles (const bool use_constant_mass, const bool verbose)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Maximum_Magnitude_Phi
//#####################################################################
template<class T> T TRIANGULATED_SURFACE<T>::
Maximum_Magnitude_Phi (const IMPLICIT_SURFACE<T>& implicit_surface, int* index)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Make_Orientations_Consistent_With_Implicit_Surface
//#####################################################################
template<class T> void TRIANGULATED_SURFACE<T>::
Make_Orientations_Consistent_With_Implicit_Surface (const IMPLICIT_SURFACE<T>& implicit_surface)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Close_Surface
//#####################################################################
template<class T> void TRIANGULATED_SURFACE<T>::
Close_Surface (const bool merge_coincident_vertices, const T merge_coincident_vertices_threshold, const bool fill_holes, const bool verbose)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Remove_Degenerate_Triangles
//#####################################################################
template<class T> void TRIANGULATED_SURFACE<T>::
Remove_Degenerate_Triangles (const T area_threshold)
{
	NOT_IMPLEMENTED();
}

//#####################################################################

template class TRIANGULATED_SURFACE<float>;
template class TRIANGULATED_SURFACE<double>;
