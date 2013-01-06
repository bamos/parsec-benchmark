//#####################################################################
// Copyright 2002-2005, Robert Bridson, Ronald Fedkiw, Eran Guendelman, Eilene Hao, Geoffrey Irving, Neil Molino, Duc Nguyen, Andrew Selle, Eftychios Sifakis, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class PARTICLE
//#####################################################################
#ifndef __PARTICLE__
#define __PARTICLE__

#include "../Read_Write/FILE_UTILITIES.h"
#include "../Arrays/LIST_ARRAY.h"
#include "../Matrices_And_Vectors/VECTOR_2D.h"
#include "PARTICLE_ATTRIBUTE.h"
namespace PhysBAM
{

class PARTICLE_ATTRIBUTE_COLLECTION;
class PARTICLE
{
public:
	int number; // total particles
	int array_size; // includes dead space
	int array_buffer_size; // preferred number of extra cells in the buffer
	PARTICLE_ATTRIBUTE_COLLECTION* attribute_collection;
	ARRAY<VECTOR_2D<int> >* particle_ranges;
private:
	LIST_ARRAY<int>* deletion_list;
public:

	PARTICLE()
		: number (0), array_size (0), array_buffer_size (0), attribute_collection (0), deletion_list (0), particle_ranges (0)
	{}

	PARTICLE (const PARTICLE& particles)
		: attribute_collection (0)
	{
		Initialize_Particles (particles);
	}

	template<template<class> class T_ARRAYS, class T_PARTICLE>
	void Initialize_Particles (const T_ARRAYS<T_PARTICLE*>& particles_per_cell) // Combines particles per cell into a single collection
	{
		Clean_Up_Memory();
		int number_of_particles = 0;

		for (int t = 0; t < particles_per_cell.size; t++) if (particles_per_cell.array[t]) number_of_particles += particles_per_cell.array[t]->number;

		Increase_Array_Size (number_of_particles);

		for (int t = 0; t < particles_per_cell.size; t++) if (particles_per_cell.array[t]) for (int k = 1; k <= particles_per_cell.array[t]->number; k++)
				{
					int index = Add_Particle();
					Copy_Particle (*particles_per_cell.array[t], k, index);
				}
	}

	template<class T_PARTICLE>
	void Initialize_Particles (const ARRAY<T_PARTICLE*>& particles_per_cell) // Combines particles per cell into a single collection
	{
		Clean_Up_Memory();
		int number_of_particles = 0;

		for (int t = 1; t <= particles_per_cell.m; t++) if (particles_per_cell (t)) number_of_particles += particles_per_cell (t)->number;

		Increase_Array_Size (number_of_particles);

		for (int t = 1; t <= particles_per_cell.m; t++) if (particles_per_cell (t)) for (int k = 1; k <= particles_per_cell (t)->number; k++)
				{
					int index = Add_Particle();
					Copy_Particle (*particles_per_cell (t), k, index);
				}
	}

	template<class T1_PARTICLES, class T2_PARTICLES>
	static void Move_Particle (T1_PARTICLES& from_particles, T2_PARTICLES& to_particles, const int from_index)
	{
		int to_index = to_particles.Add_Particle();
		to_particles.T2_PARTICLES::Copy_Particle (from_particles, from_index, to_index);
		from_particles.Delete_Particle (from_index);
	}

	template<class T1_PARTICLES, class T2_PARTICLES>
	static void Move_Particle (T1_PARTICLES& from_particles, T2_PARTICLES& to_particles, const int from_index, const int to_index)
	{
		to_particles.T2_PARTICLES::Copy_Particle (from_particles, from_index, to_index);
		from_particles.Delete_Particle (from_index);
	}

	template<class T1_PARTICLES, class T2_PARTICLES>
	static void Move_All_Particles (T1_PARTICLES& from_particles, T2_PARTICLES& to_particles)
	{
		for (int k = 1; k <= from_particles.number; k++) Move_Particle (from_particles, to_particles, k);
	}

	template<class T_GRID, class TV> // GRID_1D & GRID_2D & GRID_3D & QUADTREE_GRID & OCTREE_GRID
	void Delete_Particles_Outside_Grid (const T_GRID& grid, const ARRAY<TV>& X)
	{
		for (int k = number; k >= 1; k--) if (grid.Outside (X (k))) Delete_Particle (k);
	}

	template<class RW>
	void Read_State (std::istream& input_stream)
	{
		char version;
		Read_Binary<RW> (input_stream, version);
		Read_Binary<RW> (input_stream, number, array_size, array_buffer_size);

		if (version != 1)
		{
			std::cerr << "Unrecognized particle version " << (int) version << std::endl;
			assert (false);
			exit (1);
		}

		assert (number >= 0);
		assert (array_size >= number);
		assert (array_buffer_size >= 0);
	}

	template<class RW>
	void Write_State (std::ostream& output_stream) const
	{
		char version = 1;        // write compacted (array_size=number)
		Write_Binary<RW> (output_stream, version);
		Write_Binary<RW> (output_stream, number, number, array_buffer_size);
	}

	template<class RW>
	void Read_Attributes (std::istream& input_stream)
	{}

	template<class RW>
	void Write_Attributes (std::ostream& output_stream) const
	{}

private:
	PARTICLE &operator= (const PARTICLE& particle_input)
	{
		assert (false);        // Make private to avoid accidentally calling
		return *this;
	}

//#####################################################################
public:
	virtual ~PARTICLE();
	void Set_Array_Buffer_Size (const int array_buffer_size_input);
	void Initialize_Particles (const PARTICLE& particles);
	void Initialize_Attributes_Collection();
	virtual void Increase_Array_Size (const int number_of_new_indices);
	virtual void Clean_Up_Memory();
	virtual void Copy_Particle (const int from, const int to);
	virtual void Copy_Particle (const PARTICLE& from_particles, const int from, const int to);
	int Add_Particle();
	void Add_Particles (const int new_particles);
	void Delete_Particle (const int index);
	void Delete_All_Particles();
	void Add_To_Deletion_List (const int index);
	void Delete_Particles_On_Deletion_List (const bool preserve_order = false, const bool already_sorted = false, const bool use_heap_sort = false);
	virtual void Copy_Particle_State (const PARTICLE& from_particles);
	void Optimize_Storage();
	virtual void Print (std::ostream &output_stream, const int particle_index) const;
	void Default();
//#####################################################################
};
}
#endif
