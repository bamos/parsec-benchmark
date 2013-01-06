//#####################################################################
// Copyright 2003-2004, Zhaosheng Bao, Ronald Fedkiw, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class EMBEDDED_OBJECT
//#####################################################################
#ifndef __EMBEDDED_OBJECT__
#define __EMBEDDED_OBJECT__

#include <iostream>
#include "../Matrices_And_Vectors/VECTOR_3D.h"
#include "../Matrices_And_Vectors/VECTOR_2D.h"
#include "../Particles/SOLIDS_PARTICLE.h"
#include "../Arrays/LIST_ARRAY.h"
#include "../Arrays/LIST_ARRAYS.h"
#include "../Data_Structures/HASHTABLE_2D.h"
#include "../Interpolation/LINEAR_INTERPOLATION.h"
namespace PhysBAM
{

template<class T, class TV>
class EMBEDDED_OBJECT
{
public:
	SOLIDS_PARTICLE<T, TV>& particles; // reference to the particles of the tetrahedron or triangle mesh
	SOLIDS_PARTICLE<T, TV> embedded_particles; // used for the embedded surface
	LIST_ARRAYS<int> parent_particles; // two parent particles for each embedded particle
	LIST_ARRAY<T> interpolation_fraction;
	T interpolation_fraction_threshold;
	LIST_ARRAY<int>* embedded_children_index; // has length number of vertices in the parent tetrahedron or triangle mesh
	LIST_ARRAY<LIST_ARRAY<int> >* embedded_children;
	HASHTABLE_2D<int>* parents_to_embedded_particles_hash_table; // hash table mapping parents to embedded particles
	int hashtable_multiplier;
	LIST_ARRAY<int>* embedded_sub_elements_in_parent_element_index; // length number of parent elements
	LIST_ARRAY<int>* number_of_embedded_sub_elements_in_parent_element; // lower dimensional
	LIST_ARRAYS<int>* embedded_sub_elements_in_parent_element;
	bool average_interpolation_fractions;

	EMBEDDED_OBJECT (SOLIDS_PARTICLE<T, TV>& particles_input)
		: particles (particles_input), parent_particles (2, 0), interpolation_fraction_threshold ( (T) 0.1), embedded_children_index (0), embedded_children (0),
		  parents_to_embedded_particles_hash_table (0), hashtable_multiplier (12),
		  embedded_sub_elements_in_parent_element_index (0), number_of_embedded_sub_elements_in_parent_element (0),
		  embedded_sub_elements_in_parent_element (0), average_interpolation_fractions (false)
	{}

	virtual ~EMBEDDED_OBJECT()
	{
		delete embedded_children_index;
		delete embedded_children;
		delete parents_to_embedded_particles_hash_table;
		delete embedded_sub_elements_in_parent_element_index;
		delete number_of_embedded_sub_elements_in_parent_element;
		delete embedded_sub_elements_in_parent_element;
	}

	virtual void Clean_Up_Memory()
	{
		embedded_particles.Clean_Up_Memory();
		parent_particles.Resize_Array (2, 0);
		interpolation_fraction.Resize_Array (0);
		delete embedded_children_index;
		embedded_children_index = 0;
		delete embedded_children;
		embedded_children = 0;
		delete parents_to_embedded_particles_hash_table;
		parents_to_embedded_particles_hash_table = 0;
		delete embedded_sub_elements_in_parent_element_index;
		embedded_sub_elements_in_parent_element_index = 0;
		delete number_of_embedded_sub_elements_in_parent_element;
		number_of_embedded_sub_elements_in_parent_element = 0;
		delete embedded_sub_elements_in_parent_element;
		embedded_sub_elements_in_parent_element = 0;
	}

	void Set_Interpolation_Fraction_Threshold (const T interpolation_fraction_threshold_input = .1)
	{
		interpolation_fraction_threshold = interpolation_fraction_threshold_input;
	}

	T Clamp_Interpolation_Fraction (const T interpolation_fraction_input) const
	{
		return clamp<T> (interpolation_fraction_input, interpolation_fraction_threshold, (T) 1 - interpolation_fraction_threshold);
	}

	void Reset_Parents_To_Embedded_Particles_Hash_Table()
	{
		if (parents_to_embedded_particles_hash_table) parents_to_embedded_particles_hash_table->Delete_All_Entries();
	}

	void Copy_Then_Reset_Parents_To_Embedded_Particles_Hash_Table (HASHTABLE_2D<int>*& hash_table_copy)
	{
		hash_table_copy = parents_to_embedded_particles_hash_table;
		parents_to_embedded_particles_hash_table = new HASHTABLE_2D<int> (hashtable_multiplier * particles.number);
	}

	void Initialize_Parents_To_Embedded_Particles_Hash_Table (const int new_hashtable_multiplier = 0)
	{
		if (new_hashtable_multiplier) hashtable_multiplier = new_hashtable_multiplier;

		if (parents_to_embedded_particles_hash_table && parents_to_embedded_particles_hash_table->indices->m >= hashtable_multiplier * particles.number) return;

		delete parents_to_embedded_particles_hash_table;
		parents_to_embedded_particles_hash_table = new HASHTABLE_2D<int> (hashtable_multiplier * particles.number);

		for (int p = 1; p <= embedded_particles.number; p++)
		{
			int parent1, parent2;
			parent_particles.Get (p, parent1, parent2);

			if (parent1 < parent2) parents_to_embedded_particles_hash_table->Insert_Entry (parent1, parent2, p);
			else parents_to_embedded_particles_hash_table->Insert_Entry (parent2, parent1, p);
		}
	}

	int Embedded_Particle_On_Segment (const int endpoint1, const int endpoint2)
	{
		Initialize_Parents_To_Embedded_Particles_Hash_Table();
		int embedded_particle = 0;

		if (endpoint1 < endpoint2) parents_to_embedded_particles_hash_table->Get_Entry (endpoint1, endpoint2, embedded_particle);
		else parents_to_embedded_particles_hash_table->Get_Entry (endpoint2, endpoint1, embedded_particle);

		return embedded_particle;
	}

	static int Embedded_Particle_On_Segment (HASHTABLE_2D<int>* parents_to_embedded_particles_hash_table_input, const int endpoint1, const int endpoint2)
	{
		int embedded_particle = 0;

		if (endpoint1 < endpoint2) parents_to_embedded_particles_hash_table_input->Get_Entry (endpoint1, endpoint2, embedded_particle);
		else parents_to_embedded_particles_hash_table_input->Get_Entry (endpoint2, endpoint1, embedded_particle);

		return embedded_particle;
	}

	bool Is_Parent (const int parent_node, const int embedded_node) const
	{
		return (parent_particles (1, embedded_node) == parent_node) || (parent_particles (2, embedded_node) == parent_node);
	}

	int Which_Parent (const int parent_node, const int embedded_node) const
	{
		if (parent_particles (1, embedded_node) == parent_node) return 1;
		else if (parent_particles (2, embedded_node) == parent_node) return 2;
		else return 0;
	}

	bool Are_Parents (const int parent_node_1, const int parent_node_2, const int embedded_node) const
	{
		return Is_Parent (parent_node_1, embedded_node) && Is_Parent (parent_node_2, embedded_node);
	}

	int Other_Parent (const int parent, const int embedded_node) const
	{
		int index = Which_Parent (parent, embedded_node);

		if (index == 1) return parent_particles (2, embedded_node);
		else if (index == 2) return parent_particles (1, embedded_node);
		else return 0;
	}

	void Replace_Parent_Particle (const int embedded_particle, const int old_parent_particle, const int new_parent_particle)
	{
		if (parent_particles (1, embedded_particle) == old_parent_particle) parent_particles (1, embedded_particle) = new_parent_particle;
		else
		{
			assert (parent_particles (2, embedded_particle) == old_parent_particle);
			parent_particles (2, embedded_particle) = new_parent_particle;
		}
	}

	int Number_Of_Children (const int parent_node) const
	{
		if (! (*embedded_children_index) (parent_node)) return 0;
		else return (*embedded_children) ( (*embedded_children_index) (parent_node)).m;
	}

	int Child (const int parent_node, const int child_index) const
	{
		return (*embedded_children) ( (*embedded_children_index) (parent_node)) (child_index);
	}

	T Fraction_Of_Elements_With_Embedded_Sub_Elements()
	{
		if (!embedded_sub_elements_in_parent_element_index) return 0;

		int count = 0;

		for (int t = 1; t <= embedded_sub_elements_in_parent_element_index->m; t++) if ( (*embedded_sub_elements_in_parent_element_index) (t)) count++;

		return (T) count / (T) embedded_sub_elements_in_parent_element_index->m;
	}

	TV Calculated_Position (const int embedded_particle)
	{
		int parent1, parent2;
		parent_particles.Get (embedded_particle, parent1, parent2);
		return LINEAR_INTERPOLATION<T, TV>::Linear (particles.X (parent1), particles.X (parent2), interpolation_fraction (embedded_particle));
	}

	template<class RW>
	void Read (std::istream& input_stream)
	{
		parent_particles.template Read<RW> (input_stream);
		interpolation_fraction.template Read<RW> (input_stream);
	}

	template<class RW>
	void Write (std::ostream& output_stream) const
	{
		parent_particles.template Write<RW> (output_stream);
		interpolation_fraction.template Write<RW> (output_stream);
	}

//#####################################################################
	void Update_Embedded_Particle_Positions();
	void Update_Embedded_Particle_Velocities();
	void Update_Embedded_Particle_Masses();
	void Initialize_Embedded_Children();
	void Add_Embedded_Particle_To_Embedded_Chidren (const int embedded_particle);
//#####################################################################
};
}
#endif
