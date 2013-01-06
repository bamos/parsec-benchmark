//#####################################################################
// Copyright 2003, Zhaosheng Bao, Neil Molino
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE
//#####################################################################
#ifndef __EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE__
#define __EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE__

#include "../Geometry/EMBEDDED_TETRAHEDRALIZED_VOLUME.h"
#include "../Data_Structures/HASHTABLE_3D.h"
namespace PhysBAM
{

template<class T>
class EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE
{
public:
	SOLIDS_PARTICLE<T, VECTOR_3D<T> > boundary_particles;
	TRIANGLE_MESH boundary_mesh;
	TRIANGULATED_SURFACE<T> boundary_surface;
	LIST_ARRAY<bool> particle_on_surface;
	LIST_ARRAY<bool> previously_perturbed;
	int embedded_particle_number_save;
	int embedded_particle_number_save_previous;
	ARRAY<VECTOR_3D<T> > boundary_particle_X_save, boundary_particle_V_save;
	LIST_ARRAY<int> orientation_index;
private:
	EMBEDDED_TETRAHEDRALIZED_VOLUME<T>& embedded_tetrahedralized_volume;
	TRIANGLE_MESH candidate_boundary_mesh;
	LIST_ARRAY<bool> is_triangle_active;
	HASHTABLE_3D<int> surface_hash_table;
	int surface_hash_table_ratio;

public:
	EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE (EMBEDDED_TETRAHEDRALIZED_VOLUME<T>& embedded_tetrahedralized_volume_input, const int surface_hash_table_ratio_input = 60)
		: boundary_surface (boundary_mesh, boundary_particles), embedded_particle_number_save (0), embedded_particle_number_save_previous (0),
		  embedded_tetrahedralized_volume (embedded_tetrahedralized_volume_input), surface_hash_table_ratio (surface_hash_table_ratio_input)
	{}

	~EMBEDDED_TETRAHEDRALIZED_VOLUME_BOUNDARY_SURFACE()
	{}

	void Initialize_Hash_Table (const bool verbose = true)
	{
		surface_hash_table.Initialize_New_Table (embedded_tetrahedralized_volume.tetrahedralized_volume.particles.number * surface_hash_table_ratio);

		if (verbose) std::cout << "Triangle Hash Table Size: " << embedded_tetrahedralized_volume.tetrahedralized_volume.particles.number*surface_hash_table_ratio << std::endl;
	}

	int Remap_Tetrahedron_Particle_Number (const int tetrahedron_particle_number)
	{
		return tetrahedron_particle_number + embedded_tetrahedralized_volume.embedded_particles.number;
	}

	int Remap_Embedded_Surface_Particle_Number (const int surface_particle_number)
	{
		return surface_particle_number;
	}

	bool Is_Embedded_Particle_On_Boundary_Surface (const int embedded_particle_number)
	{
		return particle_on_surface (Remap_Embedded_Surface_Particle_Number (embedded_particle_number));
	}

	bool Is_Tetrahedron_Particle_On_Boundary_Surface (const int tetrahedron_particle_number)
	{
		return particle_on_surface (Remap_Tetrahedron_Particle_Number (tetrahedron_particle_number));
	}

//#####################################################################
	void Create_Boundary_Surface (bool verbose = true);
	void Create_Boundary_Surface (LIST_ARRAY<int>& particle_map, LIST_ARRAY<int>& map_to_old_embedded_particles, const T perturb_amount = 1e-6, const bool restart = false, const bool verbose = false);
	void Create_Boundary_Surface_From_Manifold_Embedded_Surface (bool verbose = true);
	void Perturb_Nodes_For_Collision_Freeness (LIST_ARRAY<int>& map_to_old_embedded_particles, const T perturb_amount);
	void Aggressive_Perturb_Nodes_For_Collision_Freeness (LIST_ARRAY<int>& map_to_old_embedded_particles, const T perturb_amount);
	bool Center_Octahedron_In_Material (const int tetrahedron);
	void Update_Particle_Positions();
	void Update_Particle_Velocities();
	void Update_Particle_Masses();
	void Add_To_Boundary_Surface_Tetrahedrons_Of_Material (const bool verbose = true);
	void Add_To_Boundary_Surface_Tetrahedron (const int i, const int j, const int k, const int l);
	void Add_To_Boundary_Surface_Tetrahedron_Face (const int i, const int j, const int l, const bool is_clockwise);
	void Add_To_Boundary_Surface_Triangle (int x1, int x2, int x3, const bool is_clockwise);
	void Add_To_Boundary_Surface_Planar_Quad (const int x1, const int x2, const int x3, const int x4, const bool is_clockwise);
	void Add_To_Boundary_Surface_Quad_Cut (const int il, const int jl, const int jk, const int ik, const bool is_clockwise);
	void Add_To_Boundary_Surface_Subtetrahedron_And_Subprism (const int tetrahedron, const int embedded_triangle1, const int i, const int j, const int k, const int l);
	void Add_To_Boundary_Surface_Subtetrahedron (const int i, const int j, const int k, const int l, const bool is_clockwise);
	void Add_To_Boundary_Surface_Subprism (const int i, const int j, const int k, const int l, const bool is_clockwise);
	void Add_To_Boundary_Surface_Subtetrahedrons_And_Wrick (const int tetrahedron, const int embedded_triangle1, const int embedded_triangle2, const int i, const int j, const int k, const int l);
	void Add_To_Boundary_Surface_Wrick (const int i, const int j, const int k, const int l, const bool is_clockwise);
	void Add_To_Boundary_Surface_Wedge_On_Either_Side (const int tetrahedron, const int embedded_triangle1, const int embedded_triangle2, const int i, const int j, const int k, const int l);
	void Add_To_Boundary_Surface_Subwedge (const int i, const int j, const int k, const int l, const bool is_clockwise);
	void Add_To_Boundary_Surface_Subtetrahedron_Tets_And_Oct_Plus_Tet (const int tetrahedron, const int i, const int j, const int k, const int l);
	void Add_To_Boundary_Surface_Oct_Plus_Tet (const int i, const int j, const int k, const int l, const bool is_clockwise);   // j is material
	void Add_To_Boundary_Surface_Half_Oct_Plus_Tet (const int i, const int j, const int k, const int l, const bool is_clockwise); // j is material
	void Add_To_Boundary_Surface_Subtetrahedron_And_Wedge_And_Half_Oct_Plus_Tet (const int tetrahedron, const int i, const int j, const int k, const int l);
//#####################################################################
};
}
#endif
