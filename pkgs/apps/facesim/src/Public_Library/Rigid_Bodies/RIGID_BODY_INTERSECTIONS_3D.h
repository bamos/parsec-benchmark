//#####################################################################
// Copyright 2002-2003, Ronald Fedkiw, Eran Guendelman.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class RIGID_BODY_INTERSECTIONS_3D
//#####################################################################
#ifndef __RIGID_BODY_INTERSECTIONS_3D__
#define __RIGID_BODY_INTERSECTIONS_3D__

#include "../Arrays/LIST_ARRAY.h"
#include "RIGID_BODY_3D.h"
#include "RIGID_BODY_BOUNDING_VOLUMES.h"
namespace PhysBAM
{

template<class T>
class RIGID_BODY_INTERSECTIONS_3D
{
public:
	LIST_ARRAY<RIGID_BODY_3D<T>*>& rigid_bodies;
	RIGID_BODY_BOUNDING_VOLUMES<T> bounding_volumes;
	bool use_particle_partition, use_particle_partition_center_phi_test;
	VECTOR_3D<int> particle_partition_size;
	bool use_triangle_hierarchy, use_triangle_hierarchy_center_phi_test;
	bool use_edge_intersection;
private:
	MATRIX_3X3<T> rotation, rotation_reverse;
	VECTOR_3D<T> translation, translation_reverse;

public:
	RIGID_BODY_INTERSECTIONS_3D (LIST_ARRAY<RIGID_BODY_3D<T>*>& rigid_bodies_input)
		: rigid_bodies (rigid_bodies_input), bounding_volumes (rigid_bodies),
		  use_particle_partition (false), use_particle_partition_center_phi_test (false),
		  use_triangle_hierarchy (false), use_triangle_hierarchy_center_phi_test (false),
		  use_edge_intersection (false)
	{}

	void Initialize_Data_Structures()
	{
		bounding_volumes.Initialize_Data_Structures();

		if (use_particle_partition) for (int i = 1; i <= rigid_bodies.m; i++) if (!rigid_bodies (i)->triangulated_surface->particle_partition)
					rigid_bodies (i)->triangulated_surface->Initialize_Particle_Partition (particle_partition_size.x, particle_partition_size.y, particle_partition_size.z);
	}

	void Use_Particle_Partition (const bool use_particle_partition_input = true, const int m = 0, const int n = 0, const int mn = 0)
	{
		use_particle_partition = use_particle_partition_input;
		particle_partition_size = VECTOR_3D<int> (m, n, mn);
	}

	void Use_Particle_Partition_Center_Phi_Test (const bool use_particle_partition_center_phi_test_input = true)
	{
		use_particle_partition_center_phi_test = use_particle_partition_center_phi_test_input;
	}

	void Use_Triangle_Hierarchy (const bool use_triangle_hierarchy_input = true)
	{
		use_triangle_hierarchy = use_triangle_hierarchy_input;
	}

	void Use_Triangle_Hierarchy_Center_Phi_Test (const bool use_triangle_hierarchy_center_phi_test_input = true)
	{
		use_triangle_hierarchy_center_phi_test = use_triangle_hierarchy_center_phi_test_input;
	}

	void Use_Edge_Intersection (const bool use_edge_intersection_input = true)
	{
		use_edge_intersection = use_edge_intersection_input;
	}

private:
	ORIENTED_BOX_3D<T> Oriented_Box2_In_Body1_Coordinates (const RIGID_BODY_3D<T>& body1, const RIGID_BODY_3D<T>& body2)
	{
		ORIENTED_BOX_3D<T> oriented_box = body2.Oriented_Bounding_Box();
		oriented_box.corner = body1.Object_Space_Point (oriented_box.corner);
		oriented_box.edge1 = body1.Object_Space_Vector (oriented_box.edge1);
		oriented_box.edge2 = body1.Object_Space_Vector (oriented_box.edge2);
		oriented_box.edge3 = body1.Object_Space_Vector (oriented_box.edge3);
		return oriented_box;
	}

	void Initialize_Transformation_From_Body1_To_Body2_Coordinates (const RIGID_BODY_3D<T>& body1, const RIGID_BODY_3D<T>& body2)
	{
		QUATERNION<T> body2_orientation_inverse = body2.orientation.Inverse();
		rotation = (body2_orientation_inverse * body1.orientation).Matrix_3X3();
		translation = body2_orientation_inverse.Rotate (body1.position - body2.position);
		rotation_reverse = rotation.Transposed();
		translation_reverse = - (rotation_reverse * translation);
	} // reverse is from body2 to body1 coordinates

	void Flip_Transformation() // then from body2 to body1
	{
		exchange (rotation, rotation_reverse);
		exchange (translation, translation_reverse);
	}

	VECTOR_3D<T> Transform_From_Body1_To_Body2_Coordinates (const VECTOR_3D<T>& body1_location) const
	{
		return rotation * body1_location + translation;
	}

//#####################################################################
public:
	bool Intersection_Check (const int index_1, const int index_2, int& particle_body, int& levelset_body);
	bool Bounding_Volumes_Intersect (const int index_1, const int index_2) const;
	bool Find_Any_Intersection (const int index_1, const int index_2, int& particle_body, int& levelset_body);
	void Append_All_Intersections (const int index_1, const int index_2, LIST_ARRAY<VECTOR_3D<T> >& particle_location, LIST_ARRAY<int>& particle_index,
				       LIST_ARRAY<int>& particle_body, LIST_ARRAY<int>& levelset_body, const T contour_value = 0);
private:
	void Particles_In_Levelset (const int particle_body_index, const int levelset_body_index, LIST_ARRAY<VECTOR_3D<T> >& particle_location, LIST_ARRAY<int>& particle_index,
				    LIST_ARRAY<int>& particle_body, LIST_ARRAY<int>& levelset_body, const T contour_value = 0,
				    const bool exit_early = false);
	void Get_Interfering_Triangles (const RIGID_BODY_3D<T>& body1, const RIGID_BODY_3D<T>& body2, LIST_ARRAY<int>& triangle_list);
	void Intersections_Using_Hierarchy (const int particle_body_index, const int levelset_body_index, LIST_ARRAY<int>& triangle_list,
					    LIST_ARRAY<VECTOR_3D<T> >& particle_location, LIST_ARRAY<int>& particle_index, LIST_ARRAY<int>& particle_body,
					    LIST_ARRAY<int>& levelset_body, const T contour_value = 0, const bool exit_early = false);
	void Intersections_Using_Hierarchy_And_Edges (const int particle_body_index, const int levelset_body_index, LIST_ARRAY<int>& triangle_list1,
			LIST_ARRAY<int>& triangle_list2, LIST_ARRAY<VECTOR_3D<T> >& particle_location, LIST_ARRAY<int>& particle_index,
			LIST_ARRAY<int>& particle_body, LIST_ARRAY<int>& levelset_body,
			const T contour_value = 0, const bool exit_early = false);
//#####################################################################
};
}
#endif
