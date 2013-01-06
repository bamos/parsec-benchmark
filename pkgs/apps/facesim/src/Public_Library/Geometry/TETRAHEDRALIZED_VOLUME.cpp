//#####################################################################
// Copyright 2002-2004, Zhaosheng Bao, Ron Fedkiw, Eilene Hao, Geoffrey Irving, Neil Molino, Joseph Teran, Robert Bridson.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class TETRAHEDRALIZED_VOLUME
//#####################################################################
#include "TETRAHEDRALIZED_VOLUME.h"
using namespace PhysBAM;
//#####################################################################
// Function Update_Tetrahedron_List
//#####################################################################
template<class T> void TETRAHEDRALIZED_VOLUME<T>::
Update_Tetrahedron_List()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Update_Bounding_Box
//#####################################################################
template<class T> void TETRAHEDRALIZED_VOLUME<T>::
Update_Bounding_Box()
{
	if (!bounding_box) bounding_box = new BOX_3D<T>();

	if (tetrahedron_hierarchy) bounding_box->Reset_Bounds (tetrahedron_hierarchy->box_hierarchy (tetrahedron_hierarchy->root));
	else
	{
		if (tetrahedron_mesh.tetrahedrons.m > 0) bounding_box->Reset_Bounds (particles.X (tetrahedron_mesh.tetrahedrons (1, 1)));

		for (int k = 1; k <= tetrahedron_mesh.tetrahedrons.m; k++) for (int kk = 1; kk <= 4; kk++) bounding_box->Enlarge_To_Include_Point (particles.X (tetrahedron_mesh.tetrahedrons (kk, k)));
	}
}
//#####################################################################
// Funcion Initialize_Octahedron_Mesh_And_Particles
//#####################################################################
template<class T> void TETRAHEDRALIZED_VOLUME<T>::
Initialize_Octahedron_Mesh_And_Particles (const GRID_3D<T>& grid)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Initialize_Cube_Mesh_And_Particles
//#####################################################################
template<class T> void TETRAHEDRALIZED_VOLUME<T>::
Initialize_Cube_Mesh_And_Particles (const GRID_3D<T>& grid)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Check_Signed_Volumes_And_Make_Consistent
//#####################################################################
template<class T> void TETRAHEDRALIZED_VOLUME<T>::
Check_Signed_Volumes_And_Make_Consistent (bool verbose)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Discard_Valence_Zero_Particles_And_Renumber
//#####################################################################
template<class T> void TETRAHEDRALIZED_VOLUME<T>::
Discard_Valence_Zero_Particles_And_Renumber (ARRAY<int>& condensation_mapping)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_Triangulated_Surface
//#####################################################################
template<class T> void TETRAHEDRALIZED_VOLUME<T>::
Initialize_Triangulated_Surface()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Minimum_Volume
//#####################################################################
template<class T> T TETRAHEDRALIZED_VOLUME<T>::
Minimum_Volume (int* index) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Minimum_Signed_Volume
//#####################################################################
template<class T> T TETRAHEDRALIZED_VOLUME<T>::
Minimum_Signed_Volume (int* index) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Total_Volume
//#####################################################################
template<class T> T TETRAHEDRALIZED_VOLUME<T>::
Total_Volume() const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Minimum_Angle
//#####################################################################
template<class T> T TETRAHEDRALIZED_VOLUME<T>::
Minimum_Angle (int* index) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Maximum_Angle
//#####################################################################
template<class T> T TETRAHEDRALIZED_VOLUME<T>::
Maximum_Angle (int* index) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Minimum_Altitude
//#####################################################################
template<class T> T TETRAHEDRALIZED_VOLUME<T>::
Minimum_Altitude (int* index) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Minimum_Edge_Length
//#####################################################################
template<class T> T TETRAHEDRALIZED_VOLUME<T>::
Minimum_Edge_Length (int* index) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Maximum_Edge_Length
//#####################################################################
template<class T> T TETRAHEDRALIZED_VOLUME<T>::
Maximum_Edge_Length (int* index) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Maximum_Aspect_Ratio
//#####################################################################
template<class T> T TETRAHEDRALIZED_VOLUME<T>::
Maximum_Aspect_Ratio (int* index) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Maximum_Interior_Aspect_Ratio
//#####################################################################
template<class T> T TETRAHEDRALIZED_VOLUME<T>::
Maximum_Interior_Aspect_Ratio (int* index)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Maximum_Boundary_Aspect_Ratio
//#####################################################################
template<class T> T TETRAHEDRALIZED_VOLUME<T>::
Maximum_Boundary_Aspect_Ratio (int* index)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Average_Aspect_Ratio
//#####################################################################
template<class T> T TETRAHEDRALIZED_VOLUME<T>::
Average_Aspect_Ratio()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Average_Interior_Aspect_Ratio
//#####################################################################
template<class T> T TETRAHEDRALIZED_VOLUME<T>::
Average_Interior_Aspect_Ratio()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Average_Boundary_Aspect_Ratio
//#####################################################################
template<class T> T TETRAHEDRALIZED_VOLUME<T>::
Average_Boundary_Aspect_Ratio()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Minimum_Dihedral_Angle
//#####################################################################
template<class T> T TETRAHEDRALIZED_VOLUME<T>::
Minimum_Dihedral_Angle (int* index) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Maximum_Dihedral_Angle
//#####################################################################
template<class T> T TETRAHEDRALIZED_VOLUME<T>::
Maximum_Dihedral_Angle (int* index) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Maximum_Edge_Length
//#####################################################################
template<class T> T TETRAHEDRALIZED_VOLUME<T>::
Maximum_Edge_Length (int* index)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Funcion Minimum_Edge_Length
//#####################################################################
template<class T> T TETRAHEDRALIZED_VOLUME<T>::
Minimum_Edge_Length (int* index)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Interior_Laplacian_Smoothing
//#####################################################################
// one step of mesh adjustment using Laplacian smoothing
template<class T> void TETRAHEDRALIZED_VOLUME<T>::
Advance_Interior_Laplacian_Smoothing()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Centroid
//#####################################################################
template<class T> VECTOR_3D<T> TETRAHEDRALIZED_VOLUME<T>::
Centroid (const int tetrahedron) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Rescale
//#####################################################################
template<class T> void TETRAHEDRALIZED_VOLUME<T>::
Rescale (const T scaling_factor)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Rescale
//#####################################################################
template<class T> void TETRAHEDRALIZED_VOLUME<T>::
Rescale (const T scaling_x, const T scaling_y, const T scaling_z)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Signed_Volume
//#####################################################################
template<class T> T TETRAHEDRALIZED_VOLUME<T>::
Signed_Volume (const int tetrahedron) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Volume
//#####################################################################
template<class T> T TETRAHEDRALIZED_VOLUME<T>::
Volume (const int tetrahedron) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Centroid_Of_Neighbors
//#####################################################################
template<class T> VECTOR_3D<T> TETRAHEDRALIZED_VOLUME<T>::
Centroid_Of_Neighbors (const int node) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Completely_Inside_Box
//#####################################################################
template<class T> bool TETRAHEDRALIZED_VOLUME<T>::
Completely_Inside_Box (const int tetrahedron, const BOX_3D<T>& box) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Discard_Spikes_From_Adjacent_Tetrahedrons
//#####################################################################
// throws out all tetrahedrons with only one neighbor (i.e. a spike on the boundary)
// returns index of first discarded
template<class T> void TETRAHEDRALIZED_VOLUME<T>::
Discard_Spikes_From_Adjacent_Tetrahedrons (LIST_ARRAY<int>* deletion_list)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Interior_Edges_With_Boundary_Nodes
//#####################################################################
// throws out all tetrahedrons with all four nodes on boundary
template<class T> void TETRAHEDRALIZED_VOLUME<T>::
Interior_Edges_With_Boundary_Nodes (LIST_ARRAYS<int>* deletion_list)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Discard_Spikes
//#####################################################################
// throws out all tetrahedrons with all four nodes on boundary
template<class T> void TETRAHEDRALIZED_VOLUME<T>::
Discard_Spikes (LIST_ARRAY<int>* deletion_list)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Inverted_Tetrahedrons
//#####################################################################
template<class T> void TETRAHEDRALIZED_VOLUME<T>::
Inverted_Tetrahedrons (LIST_ARRAY<int>& inverted_tetrahedrons) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Inside
//#####################################################################
// note: return the first tetrahedron that it is inside of (including boundary), otherwise returns 0
template<class T> int TETRAHEDRALIZED_VOLUME<T>::
Inside (const VECTOR_3D<T>& location, const T thickness_over_two) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Discard_Tetrahedrons_Outside_Implicit_Surface
//#####################################################################
// uses Whitney-like criterion to discard only those tets that are for sure outside levelset (assuming accurate signed distance)
template<class T> void TETRAHEDRALIZED_VOLUME<T>::
Discard_Tetrahedrons_Outside_Implicit_Surface (IMPLICIT_SURFACE<T>& implicit_surface)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Set_Mass_Of_Particles
//#####################################################################
template<class T> void TETRAHEDRALIZED_VOLUME<T>::
Set_Mass_Of_Particles (const bool use_constant_mass)
{
	assert (density != 0);

	if (use_constant_mass)
	{
		T mass_per_node = density * Total_Volume() / particles.number;

		for (int i = 1; i <= particles.number; i++) particles.mass (i) = mass_per_node;
	}
	else
	{
		ARRAY<VECTOR_3D<T> >& Xm = particles.X.array; // TODO: make Xm work again, or delete it
		ARRAY<T>::copy (0, particles.mass.array);

		for (int t = 1; t <= tetrahedron_mesh.tetrahedrons.m; t++)
		{
			int i, j, k, l;
			tetrahedron_mesh.tetrahedrons.Get (t, i, j, k, l);
			T volume = TETRAHEDRON<T>::Volume (Xm (i), Xm (j), Xm (k), Xm (l));
			particles.mass (i) += volume;
			particles.mass (j) += volume;
			particles.mass (k) += volume;
			particles.mass (l) += volume;
		}

		T density_over_four = (T).25 * density;

		for (int i = 1; i <= particles.number; i++) particles.mass (i) *= density_over_four;
	}
}
//#####################################################################
// Function Set_Mass_Of_Particles
//#####################################################################
template<class T> void TETRAHEDRALIZED_VOLUME<T>::
Increase_Mass_Of_Boundary_Particles (const T multiplicative_fraction)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Maximum_Magnitude_Phi_On_Boundary
//#####################################################################
template<class T> T TETRAHEDRALIZED_VOLUME<T>::
Maximum_Magnitude_Phi_On_Boundary (const IMPLICIT_SURFACE<T>& implicit_surface, int* index)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Volume_Incident_On_A_Particle
//#####################################################################
template<class T> T TETRAHEDRALIZED_VOLUME<T>::
Volume_Incident_On_A_Particle (const int particle_index)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Split_Along_Fracture_Plane
//#####################################################################
template<class T> void TETRAHEDRALIZED_VOLUME<T>::
Split_Along_Fracture_Plane (const PLANE<T>& plane, LIST_ARRAY<int>& particle_replicated)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Split_Node
//#####################################################################
// warning: will corrupt any aux structures aside from incident_tetrahedrons
template<class T> int TETRAHEDRALIZED_VOLUME<T>::
Split_Node (const int particle_index, const VECTOR_3D<T>& normal)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Split_Connected_Component
//#####################################################################
// warning: will corrupt any aux structures aside from incident_tetrahedrons & adjacent_tetrahedrons
template<class T> int TETRAHEDRALIZED_VOLUME<T>::
Split_Connected_Component (const int node)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Compute_Tetrahedron_Volumes
//#####################################################################
template<class T> void TETRAHEDRALIZED_VOLUME<T>::
Compute_Tetrahedron_Volumes()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Compute_Nodal_Volumes
//#####################################################################
template<class T> void TETRAHEDRALIZED_VOLUME<T>::
Compute_Nodal_Volumes (bool save_tetrahedron_volumes)
{
	NOT_IMPLEMENTED();
}

//#####################################################################

template class TETRAHEDRALIZED_VOLUME<float>;
template class TETRAHEDRALIZED_VOLUME<double>;
