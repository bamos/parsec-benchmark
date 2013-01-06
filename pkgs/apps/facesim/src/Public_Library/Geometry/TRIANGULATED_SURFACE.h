//#####################################################################
// Copyright 2002-2004, Chris Allocco, Robert Bridson, Ron Fedkiw, Geoffrey Irving, Eran Guendelman, Sergey Koltakov, Neil Molino, Igor Neverov, Andrew Selle, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class TRIANGULATED_SURFACE
//#####################################################################
#ifndef __TRIANGULATED_SURFACE__
#define __TRIANGULATED_SURFACE__

#include <fstream>
#include <iostream>
#include <stdlib.h>
#include "TRIANGULATED_OBJECT.h"
#include "TRIANGLE_3D.h"
#include "../Collisions_And_Interactions/PARTICLE_PARTITION.h"
#include "../Grids/TRIANGLE_MESH.h"
#include "../Particles/SOLIDS_PARTICLE.h"
#include "../Matrices_And_Vectors/VECTOR_3D.h"
#include "../Arrays/ARRAY.h"
#include "../Arrays/ARRAYS.h"
#include "../Arrays/ARRAYS_3D.h"
namespace PhysBAM
{

template<class T> class GRID_3D;
template<class T> class OCTREE_GRID;
template<class T> class IMPLICIT_SURFACE;

template<class T>
class TRIANGULATED_SURFACE: public TRIANGULATED_OBJECT<T, VECTOR_3D<T> >
{
public:
	using TRIANGULATED_OBJECT<T, VECTOR_3D<T> >::triangle_mesh;
	using TRIANGULATED_OBJECT<T, VECTOR_3D<T> >::particles;
	using TRIANGULATED_OBJECT<T, VECTOR_3D<T> >::Discard_Valence_Zero_Particles_And_Renumber;

	LIST_ARRAY<TRIANGLE_3D<T> >* triangle_list;
	BOX_3D<T>* bounding_box;
	ARRAY<T>* segment_lengths;
	PARTICLE_PARTITION<T>* particle_partition;
	int desired_particle_partition_size_m, desired_particle_partition_size_n, desired_particle_partition_size_mn;
	bool use_vertex_normals, avoid_normal_interpolation_across_sharp_edges;
	T normal_variance_threshold;
	ARRAYS<VECTOR_3D<T> >* vertex_normals;
	T density;

	TRIANGULATED_SURFACE (TRIANGLE_MESH& triangle_mesh_input, SOLIDS_PARTICLE<T, VECTOR_3D<T> >& particles_input)
		: TRIANGULATED_OBJECT<T, VECTOR_3D<T> > (triangle_mesh_input, particles_input),
		  triangle_list (0), bounding_box (0), segment_lengths (0), particle_partition (0),
		  desired_particle_partition_size_m (0), desired_particle_partition_size_n (0), desired_particle_partition_size_mn (0),
		  avoid_normal_interpolation_across_sharp_edges (false), normal_variance_threshold ( (T).1),
		  vertex_normals (0)
	{
		Use_Face_Normals();
		Set_Density();
	}

	virtual ~TRIANGULATED_SURFACE()
	{
		delete triangle_list;
		delete bounding_box;
		delete segment_lengths;
		delete particle_partition;
		delete vertex_normals;
	}

	static TRIANGULATED_SURFACE* Create()
	{
		return new TRIANGULATED_SURFACE (* (new TRIANGLE_MESH), * (new SOLIDS_PARTICLE<T, VECTOR_3D<T> >));
	}

	void Destroy_Data() // this is dangerous
	{
		delete &triangle_mesh;
		delete &particles;
	}

	void Clean_Up_Memory()
	{
		delete triangle_list;
		triangle_list = 0;
		delete bounding_box;
		bounding_box = 0;
		delete segment_lengths;
		segment_lengths = 0;
		delete particle_partition;
		particle_partition = 0;
		delete vertex_normals;
		vertex_normals = 0;
		Use_Face_Normals();
	}

	void Refresh_Auxiliary_Structures()
	{
		TRIANGULATED_OBJECT<T, VECTOR_3D<T> >::Refresh_Auxiliary_Structures();

		if (triangle_list) Update_Triangle_List();

		if (vertex_normals) Update_Vertex_Normals();

		if (bounding_box) Update_Bounding_Box();

		if (segment_lengths) Initialize_Segment_Lengths();
	}

	void Initialize_Triangle_Hierarchy (const bool update_boxes = true) // creates and updates the boxes as well
	{}

	void Initialize_Particle_Partition (const int m = 0, const int n = 0, const int mn = 0)
	{
		assert (bounding_box);
		int m_new = m, n_new = n, mn_new = mn;

		if (desired_particle_partition_size_m) m_new = desired_particle_partition_size_m;

		if (desired_particle_partition_size_n) n_new = desired_particle_partition_size_n;

		if (desired_particle_partition_size_mn) mn_new = desired_particle_partition_size_mn;

		assert (m_new && n_new && mn_new);
		delete particle_partition;
		particle_partition = new PARTICLE_PARTITION<T> (*bounding_box, m_new, n_new, mn_new, particles);
	}

	void Initialize_Particle_Hierarchy (const bool update_boxes = true);

	void Set_Desired_Particle_Partition_Size (const int m, const int n, const int mn)
	{
		desired_particle_partition_size_m = m;
		desired_particle_partition_size_n = n;
		desired_particle_partition_size_mn = mn;
	}

	void Use_Vertex_Normals() // averaged on the faces
	{
		use_vertex_normals = true;
	}

	void Use_Face_Normals() // i.e. flat normals
	{
		use_vertex_normals = false;
	}

	void Set_Density (const T density_input = 100) // default is water
	{
		density = density_input;
	}

	TRIANGLE_3D<T> Get_Triangle (const int aggregate_id) const
	{
		return TRIANGLE_3D<T> (particles.X (triangle_mesh.triangles (1, aggregate_id)), particles.X (triangle_mesh.triangles (2, aggregate_id)), particles.X (triangle_mesh.triangles (3, aggregate_id)));
	}

	VECTOR_3D<T> Face_Normal (const int index) const
	{
		if (triangle_list) return (*triangle_list) (index).normal;

		int i, j, k;
		triangle_mesh.triangles.Get (index, i, j, k);
		return TRIANGLE_3D<T>::Clockwise_Normal (particles.X (i), particles.X (j), particles.X (k));
	}

	VECTOR_3D<T> Centroid (const int triangle) const
	{
		int i, j, k;
		triangle_mesh.triangles.Get (triangle, i, j, k);
		return (T) one_third * (particles.X (i) + particles.X (j) + particles.X (k));
	}

	T Area (const int triangle) const
	{
		int i, j, k;
		triangle_mesh.triangles.Get (triangle, i, j, k);
		return TRIANGLE_3D<T>::Area (particles.X (i), particles.X (j), particles.X (k));
	}

	void Rescale (const T scaling_factor)
	{
		Rescale (scaling_factor, scaling_factor, scaling_factor);
	}

	void Rescale (const T scaling_x, const T scaling_y, const T scaling_z)
	{
		for (int k = 1; k <= particles.number; k++) particles.X (k) *= VECTOR_3D<T> (scaling_x, scaling_y, scaling_z);

		if (triangle_list) Update_Triangle_List();

		if (bounding_box) Update_Bounding_Box();
	}

	void Print_Mesh_Diagnostics()
	{
		std::cout << "MESH DIAGNOSTICS" << std::endl;

		if (Check_For_Self_Intersection ( (T) 1e-8)) std::cout << "found self intersections!" << std::endl;
		else std::cout << "no self intersections" << std::endl;

		std::cout << "total Triangles = " << triangle_mesh.triangles.m << std::endl;
		std::cout << "total Mass = " << particles.mass.Total_Mass (particles) << std::endl;
		std::cout << "min Mass Particle = " << particles.mass.Minimum_Mass (particles) << std::endl;
		std::cout << "max Mass Particle = " << particles.mass.Maximum_Mass (particles) << std::endl;
		std::cout << "total Area = " << Total_Area() << std::endl;
		std::cout << "min Area = " << Minimum_Area() << std::endl;
		std::cout << "Density = " << density << std::endl;
		std::cout << "min Altitude = " <<  Minimum_Altitude() << std::endl;
		std::cout << "min Edge Length = " << Minimum_Edge_Length() << std::endl;
		std::cout << "max Edge Length = " << Maximum_Edge_Length() << std::endl;
		std::cout << "min Angle = " << Minimum_Angle() << std::endl;
		std::cout << "max Angle = " << Maximum_Angle() << std::endl;
		std::cout << "ave min Angle = " << Average_Minimum_Angle() << std::endl;
		std::cout << "ave max Angle = " << Average_Maximum_Angle() << std::endl;
		std::cout <<  "max Aspect Ratio = " << Maximum_Aspect_Ratio() << std::endl;
		std::cout << "ave Aspect Ratio = " << Average_Aspect_Ratio() << std::endl;
	}

//#####################################################################
	void Update_Triangle_List(); // updates the triangles assuming the particle particles.Xs are already updated
	void Update_Triangle_List (const ARRAY<VECTOR_3D<T> >& X);
	void Update_Triangle_List (const ARRAY<VECTOR_3D<T> >& X, const VECTOR_2D<int>& triangle_range);
	void Update_Bounding_Box();
	void Initialize_Torus_Mesh_And_Particles (const int m, const int n, const T major_radius, const T minor_radius);
	void Initialize_Segment_Lengths();
	void Update_Vertex_Normals();
	bool Intersection (RAY_3D<T>& ray, const T thickness_over_two = 0) const;
	bool Closest_Non_Intersecting_Point (RAY_3D<T>& ray, const T thickness_over_two = 0) const;
	VECTOR_3D<T> Normal (const VECTOR_3D<T>& location, const int aggregate = 0) const;
	bool Inside (const VECTOR_3D<T>& location, const T thickness_over_two = 0) const;
	bool Inside_Relative_To_Triangle (const VECTOR_3D<T>& location, const int triangle_index_for_ray_test, const T thickness_over_two = 0) const;
	bool Inside_Using_Ray_Test (RAY_3D<T>& ray, const T thickness_over_two = 0) const;
	bool Outside (const VECTOR_3D<T>& location, const T thickness_over_two = 0) const;
	bool Boundary (const VECTOR_3D<T>& location, const T thickness_over_two = 0) const;
	bool Inside_Any_Triangle (const VECTOR_3D<T>& location, int& triangle_id, const T thickness_over_two = 0) const;
	VECTOR_3D<T> Surface (const VECTOR_3D<T>& location, const T max_depth = 0, const T thickness = 0, int* closest_triangle = 0, T* distance = 0) const; // without max_depth, this is slow
	T Signed_Solid_Angle_Of_Triangle_Web (const VECTOR_3D<T>& location, int web_root_node) const;
	bool Check_For_Self_Intersection (const T thickness = 0, const bool update_bounding_boxes = true, LIST_ARRAYS<int>* intersecting_segment_triangle_pairs = 0);
	bool Find_First_Segment_Triangle_Intersection (const SEGMENT_MESH& test_segment_mesh, const ARRAY<VECTOR_3D<T> >& X, const T thickness, const int max_coarsening_attempts = 5,
			const bool update_bounding_boxes = true);
	bool Segment_Triangle_Intersection (const SEGMENT_MESH& test_segment_mesh, const ARRAY<VECTOR_3D<T> >& X, const T thickness = 0, const bool update_bounding_boxes = true,
					    LIST_ARRAYS<int>* intersecting_segment_triangle_pairs = 0);
	void Get_Triangles_Near_Edges (ARRAY<LIST_ARRAY<int> >& triangles_near_edges, const SEGMENT_MESH& test_segment_mesh, const ARRAY<VECTOR_3D<T> >& X, const T thickness = 0,
				       const bool update_bounding_boxes = true);
	VECTOR_3D<T> Centroid_Of_Neighbors (const int node) const;
	T Calculate_Signed_Distance (const VECTOR_3D<T>& location, T thickness = 0) const;
	void Calculate_Signed_Distance_Function (const GRID_3D<T>& grid, ARRAYS_3D<T>& phi, bool print_progress = false);
	void Calculate_Heaviside_Function (const GRID_3D<T>& grid, ARRAYS_3D<T>& phi, bool print_progress = false);
	void Calculate_Signed_Distance_Function (const GRID_3D<T>& uniform_grid, OCTREE_GRID<T>& grid, ARRAY<T>& phi, const int maximum_depth, const T half_band_width = 0, const bool verbose = false);
	void Linearly_Subdivide();
	void Loop_Subdivide();
	void Root_Three_Subdivide();
	T Total_Area() const;
	T Thin_Shell_Mass() const;
	T Minimum_Angle (int* index = 0) const;
	T Maximum_Angle (int* index = 0) const;
	T Average_Minimum_Angle() const;
	T Average_Maximum_Angle() const;
	T Minimum_Edge_Length (int* index = 0) const;
	T Maximum_Edge_Length (int* index = 0) const;
	T Average_Edge_Length() const;
	T Maximum_Aspect_Ratio (int* index = 0) const;
	T Average_Aspect_Ratio() const;
	T Minimum_Area (int* index = 0) const;
	T Minimum_Altitude (int* index = 0) const;
	void Set_Mass_Of_Particles (const bool use_constant_mass = false, const bool verbose = true);
	T Maximum_Magnitude_Phi (const IMPLICIT_SURFACE<T>& implicit_surface, int* index = 0);
	void Make_Orientations_Consistent_With_Implicit_Surface (const IMPLICIT_SURFACE<T>& implicit_surface);
	void Close_Surface (const bool merge_coincident_vertices, const T merge_coincident_vertices_threshold, const bool fill_holes, const bool verbose = false);
	void Remove_Degenerate_Triangles (const T area_threshold = (T) 1e-8);
//#####################################################################
};
}
#endif
