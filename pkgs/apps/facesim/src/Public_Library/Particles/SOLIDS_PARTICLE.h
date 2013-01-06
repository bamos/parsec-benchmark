//#####################################################################
// Copyright 2004, Ronald Fedkiw, Geoffrey Irving, Andrew Selle, Eftychios Sifakis, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SOLIDS_PARTICLE
//#####################################################################
#ifndef __SOLIDS_PARTICLE__
#define __SOLIDS_PARTICLE__

#include "PARTICLE.h"
#include "PARTICLE_POSITION_ATTRIBUTE.h"
#include "PARTICLE_VELOCITY_ATTRIBUTE.h"
#include "PARTICLE_MASS_ATTRIBUTE.h"
#include "PARTICLE_ATTRIBUTE_COLLECTION_MAP.h"
namespace PhysBAM
{

template<class T, class TV>
class SOLIDS_PARTICLE: public PARTICLE
{
public:
	using PARTICLE::attribute_collection;

	static PARTICLE_ATTRIBUTE_COLLECTION_MAP<SOLIDS_PARTICLE> attribute_map;
	PARTICLE_POSITION_ATTRIBUTE<T, TV> X;
	PARTICLE_VELOCITY_ATTRIBUTE<T, TV> V;
	PARTICLE_MASS_ATTRIBUTE<T> mass;
	bool store_velocity, store_mass, update_velocity;

	SOLIDS_PARTICLE()
		: store_velocity (false), store_mass (false), update_velocity (false)
	{
		if (attribute_map.use_attributes_collection) Initialize_Attributes_Collection();
	}

	SOLIDS_PARTICLE (const SOLIDS_PARTICLE<T, TV>& particles)
	{
		Initialize_Particles (particles);
	}

	virtual ~SOLIDS_PARTICLE()
	{}

	void Initialize_Attributes_Collection()
	{
		PARTICLE::Initialize_Attributes_Collection();
		attribute_map.template Allocate_Attributes<T, TV> (attribute_collection);
	}

	static void Initialize_Attribute_Collection_Map (PARTICLE_ATTRIBUTE_COLLECTION_MAP<SOLIDS_PARTICLE>& attribute_map)
	{
		attribute_map.Register_Internal_Attribute (&SOLIDS_PARTICLE::X, attribute_map.position);
		attribute_map.Register_Internal_Attribute (&SOLIDS_PARTICLE::V, attribute_map.velocity);
		attribute_map.Register_Internal_Attribute (&SOLIDS_PARTICLE::mass, attribute_map.mass);
	}

	void Clean_Up_Memory()
	{
		PARTICLE::Clean_Up_Memory();
		X.Clean_Up_Memory();
		V.Clean_Up_Memory();
		mass.Clean_Up_Memory();
		Store_Velocity (false);
		Store_Mass (false);
	}

	void Store_Velocity (const bool store = true)
	{
		if (store)
		{
			if (!store_velocity) V.Resize_Array (array_size);

			store_velocity = true;
		}
		else
		{
			V.Resize_Array (0);
			store_velocity = false;
			update_velocity = false;
		}
	}

	void Update_Velocity (const bool update = true)
	{
		update_velocity = update;

		if (update_velocity) Store_Velocity();
	}

	void Store_Mass (const bool store = true)
	{
		if (store)
		{
			if (!store_mass) mass.Resize_Array (array_size);

			store_mass = true;
		}
		else
		{
			mass.Resize_Array (0);
			store_mass = false;
		}
	}

	void Increase_Array_Size (const int number_of_new_indices)
	{
		PARTICLE::Increase_Array_Size (number_of_new_indices);
		X.Resize_Array (array_size);

		if (store_velocity) V.Resize_Array (array_size);

		if (store_mass) mass.Resize_Array (array_size);
	}

	void Copy_Particle_State (const PARTICLE& from_particles) // overloaded virtual -- assumes particles same type... dangerous!!
	{
		Copy_Particle_State ( (const SOLIDS_PARTICLE<T, TV>&) from_particles);
	}

	void Copy_Particle_State (const SOLIDS_PARTICLE<T, TV>& from_particles)
	{
		PARTICLE::Copy_Particle_State (from_particles);
		store_velocity = from_particles.store_velocity;
		store_mass = from_particles.store_mass;
		update_velocity = from_particles.update_velocity;
	}

	void Copy_Particle (const int from, const int to)
	{
		PARTICLE::Copy_Particle (from, to);
		X.Copy_Particle (from, to);

		if (store_velocity) V.Copy_Particle (from, to);

		if (store_mass) mass.Copy_Particle (from, to);
	}

	void Copy_Particle (const PARTICLE& from_particles, const int from, const int to) // overloaded virtual -- assumes particles same type... dangerous!!
	{
		Copy_Particle ( (const SOLIDS_PARTICLE<T, TV>&) from_particles, from, to);
	}

	void Copy_Particle (const SOLIDS_PARTICLE<T, TV>& from_particles, const int from, const int to)
	{
		PARTICLE::Copy_Particle (from_particles, from, to);
		X.Copy_Particle (&from_particles.X, from, to);

		if (store_velocity) V.Copy_Particle (&from_particles.V, from, to);

		if (store_mass) mass.Copy_Particle (&from_particles.mass, from, to);
	}

	void Euler_Step (const ARRAY<TV>& F, const T dt)
	{
		X.Euler_Step (*this, V.array, dt);

		if (update_velocity) V.Euler_Step (*this, F, mass.array, dt);
	}

	void Euler_Step_Position (const T dt)
	{
		assert (store_velocity);
		X.Euler_Step (*this, V.array, dt);
	}

	T Maximum_Speed (int* index = 0)
	{
		return V.Maximum_Speed (*this, index);
	}

	void Default()
	{
		std::cout << "THIS SOLIDS_PARTICLE FUNCTION IS NOT DEFINED!" << std::endl;
	}

	template<class RW>
	void Read_State (std::istream& input_stream)
	{
		PARTICLE::template Read_State<RW> (input_stream);
		Read_Binary<RW> (input_stream, store_velocity, store_mass, update_velocity);
	}

	template<class RW>
	void Write_State (std::ostream& output_stream) const
	{
		PARTICLE::template Write_State<RW> (output_stream);
		Write_Binary<RW> (output_stream, store_velocity, store_mass, update_velocity);
	}

	template<class RW>
	void Read_Attributes (std::istream& input_stream)
	{
		PARTICLE::template Read_Attributes<RW> (input_stream);

		if (array_size > 0)
		{
			attribute_map.template Read<T, TV, RW> (input_stream, this, attribute_collection);
		}
		else
		{
			X.Resize_Array (0);
			V.Resize_Array (0);
			mass.Resize_Array (0);

			if (attribute_collection) attribute_collection->Resize_Array (0);
		}
	}

	template<class RW>
	void Write_Attributes (std::ostream& output_stream) const
	{
		PARTICLE::template Write_Attributes<RW> (output_stream);

		if (number > 0) attribute_map.template Write<T, TV, RW> (output_stream, this, attribute_collection);
	}

	template<class RW>
	void Read (std::istream& input_stream)
	{
		Read_State<RW> (input_stream);
		Read_Attributes<RW> (input_stream);
	}

	template<class RW>
	void Write (std::ostream& output_stream) const
	{
		Write_State<RW> (output_stream);
		Write_Attributes<RW> (output_stream);
	}

	template<class RW>
	void Read (const std::string& prefix, const int frame_number)
	{
		std::istream* input;
		std::string f = STRING_UTILITIES::string_sprintf (".%d", frame_number);
		input = FILE_UTILITIES::Safe_Open_Input (prefix + "particle_class_state" + f);
		Read_State<RW> (*input);
		delete input;

		if (store_mass)
		{
			input = FILE_UTILITIES::Safe_Open_Input (prefix + "particle_mass" + f);
			mass.template Read<RW> (*input);
			delete input;
		}

		input = FILE_UTILITIES::Safe_Open_Input (prefix + "position" + f);
		X.template Read<RW> (*input);
		delete input;

		if (store_velocity)
		{
			input = FILE_UTILITIES::Safe_Open_Input (prefix + "velocity" + f);
			V.template Read<RW> (*input);
			delete input;
		}
	}

	template<class RW>
	void Write (const std::string& prefix, const int frame_number) const
	{
		std::ostream* output;
		std::string f = STRING_UTILITIES::string_sprintf (".%d", frame_number);
		output = FILE_UTILITIES::Safe_Open_Output (prefix + "particle_class_state" + f);
		Write_State<RW> (*output);
		delete output;

		if (store_mass)
		{
			output = FILE_UTILITIES::Safe_Open_Output (prefix + "particle_mass" + f);
			mass.template Write<RW> (*output, number);
			delete output;
		}

		output = FILE_UTILITIES::Safe_Open_Output (prefix + "position" + f);
		X.template Write<RW> (*output, number);
		delete output;

		if (store_velocity)
		{
			output = FILE_UTILITIES::Safe_Open_Output (prefix + "velocity" + f);
			V.template Write<RW> (*output, number);
			delete output;
		}
	}

	void Print (std::ostream &output_stream, const int particle_index) const
	{
		attribute_map.Print (output_stream, this, attribute_collection, particle_index);
	}

//#####################################################################
};
}
#endif
