//#####################################################################
// Copyright 2003-2005, Zhaosheng Bao, Ronald Fedkiw, Geoffrey Irving, Sergey Koltakov, Neil Molino, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class EMBEDDED_TETRAHEDRALIZED_VOLUME
//#####################################################################
#ifndef __EMBEDDED_TETRAHEDRALIZED_VOLUME__
#define __EMBEDDED_TETRAHEDRALIZED_VOLUME__

#include "EMBEDDED_OBJECT.h"
#include "TETRAHEDRALIZED_VOLUME.h"
namespace PhysBAM
{

template<class T>
class EMBEDDED_TETRAHEDRALIZED_VOLUME: public EMBEDDED_OBJECT<T, VECTOR_3D<T> >
{
public:
	using EMBEDDED_OBJECT<T, VECTOR_3D<T> >::embedded_particles;
	using EMBEDDED_OBJECT<T, VECTOR_3D<T> >::parent_particles;
	using EMBEDDED_OBJECT<T, VECTOR_3D<T> >::embedded_sub_elements_in_parent_element;
	using EMBEDDED_OBJECT<T, VECTOR_3D<T> >::embedded_sub_elements_in_parent_element_index;
	using EMBEDDED_OBJECT<T, VECTOR_3D<T> >::interpolation_fraction;
	using EMBEDDED_OBJECT<T, VECTOR_3D<T> >::number_of_embedded_sub_elements_in_parent_element;
	using EMBEDDED_OBJECT<T, VECTOR_3D<T> >::Is_Parent;
	using EMBEDDED_OBJECT<T, VECTOR_3D<T> >::Other_Parent;
	using EMBEDDED_OBJECT<T, VECTOR_3D<T> >::Embedded_Particle_On_Segment;
	using EMBEDDED_OBJECT<T, VECTOR_3D<T> >::particles;
	using EMBEDDED_OBJECT<T, VECTOR_3D<T> >::parents_to_embedded_particles_hash_table;
	using EMBEDDED_OBJECT<T, VECTOR_3D<T> >::embedded_children_index;
	using EMBEDDED_OBJECT<T, VECTOR_3D<T> >::Add_Embedded_Particle_To_Embedded_Chidren;
	using EMBEDDED_OBJECT<T, VECTOR_3D<T> >::Fraction_Of_Elements_With_Embedded_Sub_Elements;

	TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume;
	TRIANGLE_MESH embedded_triangle_mesh; // used for the embedded surface
	TRIANGULATED_SURFACE<T> embedded_surface;
	LIST_ARRAYS<bool> node_in_tetrahedron_is_material;
	BOX_3D<T>* bounding_box;

	EMBEDDED_TETRAHEDRALIZED_VOLUME (TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume_input)
		: EMBEDDED_OBJECT<T, VECTOR_3D<T> > (tetrahedralized_volume_input.particles),
		  tetrahedralized_volume (tetrahedralized_volume_input), embedded_surface (embedded_triangle_mesh, embedded_particles),
		  node_in_tetrahedron_is_material (4, 0), bounding_box (0)
	{}

	virtual ~EMBEDDED_TETRAHEDRALIZED_VOLUME()
	{
		delete bounding_box;
	}

	static EMBEDDED_TETRAHEDRALIZED_VOLUME* Create()
	{
		return new EMBEDDED_TETRAHEDRALIZED_VOLUME (*TETRAHEDRALIZED_VOLUME<T>::Create());
	}

	void Destroy_Data() // this is dangerous
	{
		tetrahedralized_volume.Destroy_Data();
		delete &tetrahedralized_volume;
	}

	void Clean_Up_Memory()
	{
		EMBEDDED_OBJECT<T, VECTOR_3D<T> >::Clean_Up_Memory();
		embedded_surface.Clean_Up_Memory();
		embedded_triangle_mesh.Clean_Up_Memory();
		node_in_tetrahedron_is_material.Resize_Array (4, 0);
		delete bounding_box;
		bounding_box = 0;
	}

	void Initialize_Node_In_Tetrahedron_Is_Material()
	{
		if (node_in_tetrahedron_is_material.m == tetrahedralized_volume.tetrahedron_mesh.tetrahedrons.m) return; // don't overwrite existing values if they exist

		node_in_tetrahedron_is_material.Resize_Array (4, tetrahedralized_volume.tetrahedron_mesh.tetrahedrons.m, false, false);
		LIST_ARRAYS<bool>::copy (true, node_in_tetrahedron_is_material);
	}

	bool Node_In_Tetrahedron_Is_Material (const int node, const int tetrahedron) const
	{
		int i, j, k, l;
		tetrahedralized_volume.tetrahedron_mesh.tetrahedrons.Get (tetrahedron, i, j, k, l);

		if (node == i) return node_in_tetrahedron_is_material (1, tetrahedron);
		else if (node == j) return node_in_tetrahedron_is_material (2, tetrahedron);
		else if (node == k) return node_in_tetrahedron_is_material (3, tetrahedron);
		else
		{
			assert (node == l);
			return node_in_tetrahedron_is_material (4, tetrahedron);
		}
	}

	int Node_Separated_By_Embedded_Triangle (const int embedded_triangle) const
	{
		int a, b, c;
		embedded_triangle_mesh.triangles.Get (embedded_triangle, a, b, c);
		int ppa1, ppa2, ppb1, ppb2, ppc1, ppc2;
		parent_particles.Get (a, ppa1, ppa2);
		parent_particles.Get (b, ppb1, ppb2);
		parent_particles.Get (c, ppc1, ppc2);

		if (Is_Parent (ppa1, b) && Is_Parent (ppa1, c)) return ppa1;
		else if (Is_Parent (ppa2, b) && Is_Parent (ppa2, c)) return ppa2;
		else return 0;
	}

	bool Nodes_Are_Separated_In_Tetrahedron (const int node1, const int node2, const int tetrahedron)
	{
		int embedded_node = Embedded_Particle_On_Segment (node1, node2);

		if (!embedded_node) return false;

		int triangle_1, triangle_2, triangle_3, triangle_4;
		Embedded_Triangles_In_Tetrahedron (tetrahedron, triangle_1, triangle_2, triangle_3, triangle_4);

		if (!triangle_1) return false;
		else if (embedded_triangle_mesh.Node_In_Triangle (embedded_node, triangle_1)) return true;

		if (!triangle_2) return false;
		else if (embedded_triangle_mesh.Node_In_Triangle (embedded_node, triangle_2)) return true;

		if (!triangle_3) return false;
		else if (embedded_triangle_mesh.Node_In_Triangle (embedded_node, triangle_3)) return true;

		if (!triangle_4) return false;
		else if (embedded_triangle_mesh.Node_In_Triangle (embedded_node, triangle_4)) return true;

		return false;
	}

	int Number_Of_Embedded_Cuts (const int tetrahedron) // a quad only counts as one cut
	{
		int triangle_1, triangle_2, triangle_3, triangle_4, triangle_count = Embedded_Triangles_In_Tetrahedron (tetrahedron, triangle_1, triangle_2, triangle_3, triangle_4);

		if (Cut_By_Quad (tetrahedron)) return triangle_count - 1;
		else return triangle_count;
	}

	bool Cut_By_Quad (const int tetrahedron)
	{
		int triangle_1, triangle_2, triangle_3, triangle_4, triangle_count = Embedded_Triangles_In_Tetrahedron (tetrahedron, triangle_1, triangle_2, triangle_3, triangle_4);

		if (triangle_count <= 1) return false;
		else if (triangle_count == 2)
		{
			if (Node_Separated_By_Embedded_Triangle (triangle_1)) return false;
			else return true;
		}
		else if (triangle_count == 3)
		{
			if (!Node_Separated_By_Embedded_Triangle (triangle_1) || !Node_Separated_By_Embedded_Triangle (triangle_2) || !Node_Separated_By_Embedded_Triangle (triangle_3)) return true;
			else return false;
		}
		else return true;
	} // if four embedded triangles, there must be a quad

	bool Segment_Is_Broken (const int node1, const int node2)
	{
		int particle_on_segment = Embedded_Particle_On_Segment (node1, node2);

		if (!particle_on_segment) return false;

		LIST_ARRAY<int> tetrahedrons_on_edge;
		tetrahedralized_volume.tetrahedron_mesh.Tetrahedrons_On_Edge (node1, node2, &tetrahedrons_on_edge);

		for (int t = 1; t <= tetrahedrons_on_edge.m; t++) if (!Nodes_Are_Separated_In_Tetrahedron (node1, node2, tetrahedrons_on_edge (t))) return false;

		return true;
	}

	int Number_Of_Edges_With_Embedded_Particles (const int tetrahedron)
	{
		int i, j, k, l;
		tetrahedralized_volume.tetrahedron_mesh.tetrahedrons.Get (tetrahedron, i, j, k, l);
		int ij = Embedded_Particle_On_Segment (i, j), ik = Embedded_Particle_On_Segment (i, k), il = Embedded_Particle_On_Segment (i, l),
		    jk = Embedded_Particle_On_Segment (j, k), jl = Embedded_Particle_On_Segment (j, l), kl = Embedded_Particle_On_Segment (k, l);
		return (ij > 0) + (ik > 0) + (il > 0) + (jk > 0) + (jl > 0) + (kl > 0);
	}

	void Update_Bounding_Box()
	{
		tetrahedralized_volume.Update_Bounding_Box();
		bounding_box->Reset_Bounds (*tetrahedralized_volume.bounding_box);
	}

	T Fraction_Of_Tetrahedra_With_One_Cut()
	{
		int count = 0;

		for (int t = 1; t <= tetrahedralized_volume.tetrahedron_mesh.tetrahedrons.m; t++) if (Number_Of_Embedded_Cuts (t) == 1) count++;

		return (T) count / (T) tetrahedralized_volume.tetrahedron_mesh.tetrahedrons.m;
	}

	T Fraction_Of_Tetrahedra_With_Two_Cuts()
	{
		int count = 0;

		for (int t = 1; t <= tetrahedralized_volume.tetrahedron_mesh.tetrahedrons.m; t++) if (Number_Of_Embedded_Cuts (t) == 2) count++;

		return (T) count / (T) tetrahedralized_volume.tetrahedron_mesh.tetrahedrons.m;
	}

	T Fraction_Of_Tetrahedra_With_Three_Cuts()
	{
		int count = 0;

		for (int t = 1; t <= tetrahedralized_volume.tetrahedron_mesh.tetrahedrons.m; t++) if (Number_Of_Embedded_Cuts (t) == 3) count++;

		return (T) count / (T) tetrahedralized_volume.tetrahedron_mesh.tetrahedrons.m;
	}

	int Number_Of_Tetrahedra_With_Cuts()
	{
		int count = 0;

		for (int t = 1; t <= tetrahedralized_volume.tetrahedron_mesh.tetrahedrons.m; t++) if (Number_Of_Embedded_Cuts (t) > 0) count++;

		return count;
	}

	int Number_Of_Tetrahedra_With_One_Cut()
	{
		int count = 0;

		for (int t = 1; t <= tetrahedralized_volume.tetrahedron_mesh.tetrahedrons.m; t++) if (Number_Of_Embedded_Cuts (t) == 1) count++;

		return count;
	}

	int Number_Of_Tetrahedra_With_Two_Cuts()
	{
		int count = 0;

		for (int t = 1; t <= tetrahedralized_volume.tetrahedron_mesh.tetrahedrons.m; t++) if (Number_Of_Embedded_Cuts (t) == 2) count++;

		return count;
	}

	int Number_Of_Tetrahedra_With_Three_Cuts()
	{
		int count = 0;

		for (int t = 1; t <= tetrahedralized_volume.tetrahedron_mesh.tetrahedrons.m; t++) if (Number_Of_Embedded_Cuts (t) == 3) count++;

		return count;
	}

	int Add_Embedded_Triangle_If_Not_Already_There (const int embedded_particle1_parent1, const int embedded_particle1_parent2, const int embedded_particle2_parent1,
			const int embedded_particle2_parent2, const int embedded_particle3_parent1, const int embedded_particle3_parent2)
	{
		return Add_Embedded_Triangle_If_Not_Already_There (Embedded_Particle_On_Segment (embedded_particle1_parent1, embedded_particle1_parent2),
				Embedded_Particle_On_Segment (embedded_particle2_parent1, embedded_particle2_parent2), Embedded_Particle_On_Segment (embedded_particle3_parent1, embedded_particle3_parent2));
	}

	template<class RW>
	void Read (std::istream& input_stream)
	{
		Clean_Up_Memory();
		EMBEDDED_OBJECT<T, VECTOR_3D<T> >::template Read<RW> (input_stream);
		tetrahedralized_volume.template Read<RW> (input_stream);
		embedded_surface.template Read<RW> (input_stream);
		node_in_tetrahedron_is_material.template Read<RW> (input_stream);
	}

	template<class RW>
	void Write (std::ostream& output_stream) const
	{
		EMBEDDED_OBJECT<T, VECTOR_3D<T> >::template Write<RW> (output_stream);
		tetrahedralized_volume.template Write<RW> (output_stream);
		embedded_surface.template Write<RW> (output_stream);
		node_in_tetrahedron_is_material.template Write<RW> (output_stream);
	}

//#####################################################################
	int Tetrahedron_Containing_Triangle (const int embedded_triangle) const;
	int Embedded_Triangles_In_Tetrahedron (const int tetrahedron, int& first_embedded_triangle, int& second_embedded_triangle, int& third_embedded_triangle, int& fourth_embedded_triangle);
private:
	void Add_Embedded_Triangle_To_Embedded_Triangles_In_Tetrahedron (const int triangle); // incrementally update embedded_sub_elements_in_parent_element
public:
	int Add_Embedded_Particle_If_Not_Already_There (const int node1, const int node2, T interpolation_fraction_input);
	int Add_Embedded_Triangle_If_Not_Already_There (const int embedded_particle1, const int embedded_particle2, const int embedded_particle3);
	void Initialize_Embedded_Sub_Elements_In_Parent_Element();
	void Calculate_Triangulated_Surface_From_Levelset_On_Tetrahedron_Nodes (ARRAY<T>& phi, const bool discard_tetrahedra_outside_levelset = true, const bool verbose = true);
	void Write_Percentage_Of_Tetrahedra_With_Embedded_Triangles (const std::string& output_directory);
//#####################################################################
};
}
#endif
