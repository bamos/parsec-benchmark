//#####################################################################
// Copyright 2002-2004, Ron Fedkiw, Eilene Hao, Geoffrey Irving, Neil Molino, Joseph Teran, Robert Bridson, Andrew Selle, Zhaosheng Bao.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class TETRAHEDRALIZED_VOLUME
//#####################################################################
#ifndef __TETRAHEDRALIZED_VOLUME__
#define __TETRAHEDRALIZED_VOLUME__

#include <iostream>
#include "TETRAHEDRON.h"
#include "TRIANGULATED_SURFACE.h"
#include "IMPLICIT_SURFACE.h"
#include "../Collisions_And_Interactions/TETRAHEDRON_HIERARCHY.h"
#include "../Grids/TETRAHEDRON_MESH.h"
#include "../Particles/SOLIDS_PARTICLE.h"
#include "../Grids/GRID_3D.h"
namespace PhysBAM
{

template<class T>
class TETRAHEDRALIZED_VOLUME
{
public:
	TETRAHEDRON_MESH& tetrahedron_mesh;
	SOLIDS_PARTICLE<T, VECTOR_3D<T> >& particles;
	LIST_ARRAY<TETRAHEDRON<T> >* tetrahedron_list;
	BOX_3D<T>* bounding_box;
	TRIANGULATED_SURFACE<T>* triangulated_surface;
	TETRAHEDRON_HIERARCHY<T>* tetrahedron_hierarchy;
	LIST_ARRAY<T>* tetrahedron_volumes;
	LIST_ARRAY<T>* nodal_volumes;
	T density;

	TETRAHEDRALIZED_VOLUME (TETRAHEDRON_MESH& tetrahedron_mesh_input, SOLIDS_PARTICLE<T, VECTOR_3D<T> >& particles_input)
		: tetrahedron_mesh (tetrahedron_mesh_input), particles (particles_input), tetrahedron_list (0), bounding_box (0),
		  triangulated_surface (0), tetrahedron_hierarchy (0), tetrahedron_volumes (0), nodal_volumes (0), density (0)
	{
		Set_Density();
	}

	virtual ~TETRAHEDRALIZED_VOLUME()
	{
		delete tetrahedron_list;
		delete bounding_box;
		delete triangulated_surface;
		delete tetrahedron_hierarchy;
		delete tetrahedron_volumes;
		delete nodal_volumes;
	}

	static TETRAHEDRALIZED_VOLUME* Create()
	{
		return new TETRAHEDRALIZED_VOLUME (* (new TETRAHEDRON_MESH), * (new SOLIDS_PARTICLE<T, VECTOR_3D<T> >));
	}

	void Destroy_Data() // this is dangerous
	{
		delete &tetrahedron_mesh;
		delete &particles;
	}

	void Clean_Up_Memory()
	{
		delete tetrahedron_list;
		tetrahedron_list = 0;
		delete bounding_box;
		bounding_box = 0;
		delete triangulated_surface;
		triangulated_surface = 0;
		delete tetrahedron_hierarchy;
		tetrahedron_hierarchy = 0;
		delete tetrahedron_volumes;
		tetrahedron_volumes = 0;
		delete nodal_volumes;
		nodal_volumes = 0;
	}

	void Initialize_Tetrahedron_Hierarchy (const bool update_boxes = true) // creates and updates the boxes as well
	{
		delete tetrahedron_hierarchy;

		if (tetrahedron_list) tetrahedron_hierarchy = new TETRAHEDRON_HIERARCHY<T> (tetrahedron_mesh, particles, *tetrahedron_list, update_boxes);
		else tetrahedron_hierarchy = new TETRAHEDRON_HIERARCHY<T> (tetrahedron_mesh, particles, update_boxes);
	}

	void Set_Density (const T density_input = 1000) // default is water
	{
		density = density_input;
	}

	template<class RW>
	void Read (std::istream& input_stream)
	{
		tetrahedron_mesh.template Read<RW> (input_stream);
		particles.template Read_State<RW> (input_stream);
		particles.X.template Read<RW> (input_stream);
	}

	template<class RW>
	void Write (std::ostream& output_stream) const
	{
		tetrahedron_mesh.template Write<RW> (output_stream);
		particles.template Write_State<RW> (output_stream);
		particles.X.template Write<RW> (output_stream, particles.number);
	}

	void Compute_Nodal_And_Tetrahedron_Volumes()
	{
		Compute_Nodal_Volumes (true);
	}

	void Discard_Valence_Zero_Particles_And_Renumber()
	{
		ARRAY<int> condensation_mapping;
		Discard_Valence_Zero_Particles_And_Renumber (condensation_mapping);
	}

//#####################################################################
	void Update_Tetrahedron_List(); // updates the tets assuming the particle positions are already updated
	void Update_Bounding_Box();
	void Initialize_Octahedron_Mesh_And_Particles (const GRID_3D<T> &grid);
	void Initialize_Cube_Mesh_And_Particles (const GRID_3D<T> &grid);
	void Check_Signed_Volumes_And_Make_Consistent (bool verbose = true);
	void Discard_Valence_Zero_Particles_And_Renumber (ARRAY<int>& condensation_mapping);
	void Initialize_Triangulated_Surface();
	T Minimum_Volume (int* index = 0) const;
	T Minimum_Signed_Volume (int* index = 0) const;
	T Total_Volume() const;
	T Minimum_Angle (int* index = 0) const;
	T Maximum_Angle (int* index = 0) const;
	T Minimum_Altitude (int* index = 0) const;
	T Minimum_Edge_Length (int* index = 0) const;
	T Maximum_Edge_Length (int* index = 0) const;
	T Maximum_Aspect_Ratio (int* index = 0) const;
	T Maximum_Interior_Aspect_Ratio (int* index = 0);
	T Maximum_Boundary_Aspect_Ratio (int* index = 0);
	T Average_Aspect_Ratio();
	T Average_Interior_Aspect_Ratio();
	T Average_Boundary_Aspect_Ratio();
	T Minimum_Dihedral_Angle (int* index = 0) const;
	T Maximum_Dihedral_Angle (int* index = 0) const;
	T Maximum_Edge_Length (int* index = 0);
	T Minimum_Edge_Length (int* index = 0);
	void Advance_Interior_Laplacian_Smoothing();
	VECTOR_3D<T> Centroid (const int tetrahedron) const;
	void Rescale (const T scaling_factor);
	void Rescale (const T scaling_x, const T scaling_y, const T scaling_z);
	T Volume (const int tetrahedron) const;
	T Signed_Volume (const int tetrahedron) const;
	VECTOR_3D<T> Centroid_Of_Neighbors (const int node) const;
	bool Completely_Inside_Box (const int tetrahedron, const BOX_3D<T>& box) const;
	void Discard_Spikes_From_Adjacent_Tetrahedrons (LIST_ARRAY<int>* deletion_list = 0);
	void Discard_Spikes (LIST_ARRAY<int>* deletion_list = 0);
	void Interior_Edges_With_Boundary_Nodes (LIST_ARRAYS<int>* deletion_list);
	void Inverted_Tetrahedrons (LIST_ARRAY<int>& inverted_tetrahedrons) const;
	int Inside (const VECTOR_3D<T>& location, const T thickness_over_two = 0) const;
	void Discard_Tetrahedrons_Outside_Implicit_Surface (IMPLICIT_SURFACE<T>& implicit_surface);
	void Set_Mass_Of_Particles (const bool use_constant_mass = false);
	void Increase_Mass_Of_Boundary_Particles (const T multiplicative_fraction = 1);
	T Maximum_Magnitude_Phi_On_Boundary (const IMPLICIT_SURFACE<T>& implicit_surface, int* index = 0);
	T Volume_Incident_On_A_Particle (const int particle_index);
	void Split_Along_Fracture_Plane (const PLANE<T>& plane, LIST_ARRAY<int>& particle_replicated);
	int Split_Node (const int particle_index, const VECTOR_3D<T>& normal);
	int Split_Connected_Component (const int node);
	void Compute_Tetrahedron_Volumes();
	void Compute_Nodal_Volumes (bool save_tetrahedron_volumes = false);
//#####################################################################
};
}
#endif

