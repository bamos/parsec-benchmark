//#####################################################################
// Copyright 2004, Igor Neverov.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class LANDMARK_3D
//#####################################################################
#ifndef __LANDMARK_3D__
#define __LANDMARK_3D__

#include "../../Public_Library/Geometry/TETRAHEDRALIZED_VOLUME.h"

namespace PhysBAM
{

template<class T>
struct LANDMARK_3D
{
	int node1, node2, node3, node4;
	VECTOR_3D<T> barycentric;

	LANDMARK_3D()
	{}

	LANDMARK_3D (const int node1_input, const int node2_input, const int node3_input, const int node4_input, const VECTOR_3D<T>& barycentric_input)
		: node1 (node1_input), node2 (node2_input), node3 (node3_input), node4 (node4_input), barycentric (barycentric_input)
	{}

	void Initialize (const TRIANGULATED_SURFACE<T>& surface, const int triangle, const VECTOR_3D<T>& point)
	{
		surface.triangle_mesh.triangles.Get (triangle, node1, node2, node3);
		node4 = 1;
		const ARRAY<VECTOR_3D<T> >& X = surface.particles.X.array;
		barycentric = TRIANGLE_3D<T>::Barycentric_Coordinates (point, X (node1), X (node2), X (node3));
	}

	static void Initialize_Surface_Markers (TETRAHEDRALIZED_VOLUME<T>& volume, const ARRAY<VECTOR_3D<T> >& points, ARRAY<LANDMARK_3D<T> >& markers)
	{
		markers.Resize_Array (points.m);
		bool boundary_mesh_defined = volume.tetrahedron_mesh.boundary_mesh != 0;

		if (!boundary_mesh_defined) volume.tetrahedron_mesh.Initialize_Boundary_Mesh();

		bool boundary_defined = volume.triangulated_surface != 0;

		if (!boundary_defined) volume.Initialize_Triangulated_Surface();

		TRIANGULATED_SURFACE<T>& surface = *volume.triangulated_surface;
		bool triangle_list_defined = surface.triangle_list != 0;

		if (!triangle_list_defined) surface.Update_Triangle_List();

		bool triangle_hierarchy_defined = surface.triangle_hierarchy != 0;

		if (!triangle_hierarchy_defined) surface.Initialize_Triangle_Hierarchy();

		bool incident_tetrahedrons_defined = volume.tetrahedron_mesh.incident_tetrahedrons != 0;

		if (!incident_tetrahedrons_defined) volume.tetrahedron_mesh.Initialize_Incident_Tetrahedrons();

		const ARRAY<VECTOR_3D<T> >& X = volume.particles.X.array;
		int triangle_index;

		for (int i = 1; i <= points.m; ++i)
		{
			VECTOR_3D<T> projected_point = volume.triangulated_surface->Surface (points (i), 0, 0, &triangle_index);
			int j, node1, node2, node3;
			surface.triangle_mesh.triangles.Get (triangle_index, node1, node2, node3);
			VECTOR_3D<T> b = TRIANGLE_3D<T>::Barycentric_Coordinates (projected_point, X (node1), X (node2), X (node3));
			const LIST_ARRAY<int>& incident_tetrahedrons = (*volume.tetrahedron_mesh.incident_tetrahedrons) (node1);

			for (j = 1; j <= incident_tetrahedrons.m; ++j) if (volume.tetrahedron_mesh.Triangle_In_Tetrahedron (node1, node2, node3, incident_tetrahedrons (j))) break;

			if (j > incident_tetrahedrons.m)
			{
				printf ("not found tet containing marker %d!\n", i);
				exit (1);
			}

			int t = incident_tetrahedrons (j);
			int n1, n2, n3, n4;
			volume.tetrahedron_mesh.tetrahedrons.Get (t, n1, n2, n3, n4);
			int other_node = n1 ^ n2 ^ n3 ^ n4 ^ node1 ^ node2 ^ node3;
			markers (i).Initialize (node1, node2, node3, other_node, b);
		}

		if (!incident_tetrahedrons_defined)
		{
			delete volume.tetrahedron_mesh.incident_tetrahedrons;
			volume.tetrahedron_mesh.incident_tetrahedrons = 0;
		}

		if (!triangle_hierarchy_defined)
		{
			delete surface.triangle_hierarchy;
			surface.triangle_hierarchy = 0;
		}

		if (!triangle_list_defined)
		{
			delete surface.triangle_list;
			surface.triangle_list = 0;
		}

		if (!boundary_defined)
		{
			delete volume.triangulated_surface;
			volume.triangulated_surface = 0;
		}

		if (!boundary_mesh_defined)
		{
			delete volume.tetrahedron_mesh.boundary_mesh;
			volume.tetrahedron_mesh.boundary_mesh = 0;
		}
	}

	VECTOR_3D<T> Evaluate (const ARRAY<VECTOR_3D<T> >& X) const
	{
		assert (X.Valid_Index (node1) && X.Valid_Index (node2) && X.Valid_Index (node3) && X.Valid_Index (node4));
		return barycentric.x * X (node1) + barycentric.y * X (node2) + barycentric.z * X (node3) + (1 - barycentric.x - barycentric.y - barycentric.z) * X (node4);
	}

	static void Evaluate (const ARRAY<VECTOR_3D<T> >& X, const ARRAY<LANDMARK_3D<T> >& markers, ARRAY<VECTOR_3D<T> >& points)
	{
		points.Resize_Array (markers.m);

		for (int i = 1; i <= markers.m; ++i) points (i) = markers (i).Evaluate (X);
	}

	template<class RW>
	void Read (std::istream& input)
	{
		Read_Binary<RW> (input, node1, node2, node3, node4, barycentric);
	}

	template<class RW>
	void Write (std::ostream& output) const
	{
		Write_Binary<RW> (output, node1, node2, node3, node4, barycentric);
	}

//#####################################################################
};
}
#endif
