//#####################################################################
// Copyright 2004, Ron Fedkiw, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class PARTICLE_ATTRIBUTE_COLLECTION_MAP
//#####################################################################
//
// Records array indices in collection for particle attributes
//
// To add a new attribute type:
//   1. Declare new int map entry
//   2. Add appropriate lines to Read/Write/Allocate functions
//   3. Write Get() function on ATTRIBUTE_COLLECTION
//
//#####################################################################
#ifndef __PARTICLE_ATTRIBUTE_COLLECTION_MAP__
#define __PARTICLE_ATTRIBUTE_COLLECTION_MAP__

#include "PARTICLE_ATTRIBUTE_COLLECTION.h"
namespace PhysBAM
{

class PARTICLE_ATTRIBUTE_COLLECTION;
template<class T, class TV> class PARTICLE_POSITION_ATTRIBUTE;
template<class T, class TV> class PARTICLE_VELOCITY_ATTRIBUTE;
template<class T> class PARTICLE_MASS_ATTRIBUTE;
template<class T> class PARTICLE_ATTRIBUTE;
template<class T> class VECTOR_3D;

template<class T_PARTICLE>
class PARTICLE_ATTRIBUTE_COLLECTION_MAP
{
public:
	int maximum_index;
	int position;
	int velocity;
	int mass;
	int radius;
	int density;
	int temperature;
	int age;
	int id;
	int vorticity;
	int quantized_collision_distance;
	ARRAY<PARTICLE_ATTRIBUTE_UNTEMPLATIZED T_PARTICLE::*> internal_attributes;
	const char current_version;
	bool use_attributes_collection; // If true, particles will Initialize_Attributes_Collection when constructed

	PARTICLE_ATTRIBUTE_COLLECTION_MAP()
		: maximum_index (0), position (0), velocity (0), mass (0), radius (0), density (0), temperature (0), age (0), id (0), vorticity (0), quantized_collision_distance (0), current_version (2)
	{
		T_PARTICLE::Initialize_Attribute_Collection_Map (*this);
		Use_Attribute_Collection (false);
	}

	template<class T_ATTRIBUTE_STATIC_OFFSET>
	void Register_Internal_Attribute (T_ATTRIBUTE_STATIC_OFFSET attribute, int& attribute_slot)
	{
		internal_attributes.Append_Element ( (PARTICLE_ATTRIBUTE_UNTEMPLATIZED T_PARTICLE::*) attribute);
		attribute_slot = -internal_attributes.m;
	}

	// Records map entry for particular particle class (MUST BE CALLED BEFORE ANY PARTICLES ARE INSTANTIATED) e.g. call as
	// SOLIDS_PARTICLE<T,TV>::attribute_map(SOLIDS_PARTICLE<T,TV>::attribute_map.position);
	void Store (int& slot)
	{
		Use_Attribute_Collection (true);

		if (slot == 0) slot = ++maximum_index;
	}

	void Use_Attribute_Collection (const bool use_attributes_collection_input)
	{
		use_attributes_collection = use_attributes_collection_input;
	}

	template<class T, class TV>
	void Allocate_Attributes (PARTICLE_ATTRIBUTE_COLLECTION* attribute_collection)
	{
		attribute_collection->attributes.Resize_Array (maximum_index);
		attribute_collection->template Store<PARTICLE_POSITION_ATTRIBUTE<T, TV> > (position);
		attribute_collection->template Store<PARTICLE_VELOCITY_ATTRIBUTE<T, TV> > (velocity);
		attribute_collection->template Store<PARTICLE_MASS_ATTRIBUTE<T> > (mass);
		attribute_collection->template Store<PARTICLE_ATTRIBUTE<T> > (radius);
		attribute_collection->template Store<PARTICLE_ATTRIBUTE<T> > (density);
		attribute_collection->template Store<PARTICLE_ATTRIBUTE<T> > (temperature);
		attribute_collection->template Store<PARTICLE_ATTRIBUTE<T> > (age);
		attribute_collection->template Store<PARTICLE_ATTRIBUTE<int> > (id);
		attribute_collection->template Store<PARTICLE_ATTRIBUTE<VECTOR_3D<T> > > (vorticity);
		attribute_collection->template Store<PARTICLE_ATTRIBUTE<unsigned short> > (quantized_collision_distance);
	}

	template<class T_ATTRIBUTE>
	T_ATTRIBUTE* Get_Attribute (T_PARTICLE* particles, const PARTICLE_ATTRIBUTE_COLLECTION* attributes, const int entry_index)
	{
		if (entry_index < 0) return & (particles->* ( (T_ATTRIBUTE T_PARTICLE::*) internal_attributes (-entry_index)));
		else if (attributes && entry_index > 0) return (T_ATTRIBUTE*) attributes->attributes (entry_index);
		else return 0;
	}

	template<class T_ATTRIBUTE>
	const T_ATTRIBUTE* Get_Attribute (const T_PARTICLE* particles, const PARTICLE_ATTRIBUTE_COLLECTION* attributes, const int entry_index) const
	{
		if (entry_index < 0) return & (particles->* ( (T_ATTRIBUTE T_PARTICLE::*) internal_attributes (-entry_index)));
		else if (attributes && entry_index > 0) return (const T_ATTRIBUTE*) attributes->attributes (entry_index);
		else return 0;
	}

	template<class ATTRIBUTE_TYPE_TO_READ, class RW>
	void Read_Helper (std::istream& input_stream, T_PARTICLE* particles, const PARTICLE_ATTRIBUTE_COLLECTION* attributes, const int read_map_entry,
			  const int my_map_entry)
	{
		ATTRIBUTE_TYPE_TO_READ* attribute = Get_Attribute<ATTRIBUTE_TYPE_TO_READ> (particles, attributes, my_map_entry);

		if (read_map_entry == 0)
		{
			if (attribute)
			{
				attribute->Resize_Array (particles->number);       // nothing in file, but at least resize to proper size
			}

			return;
		}
		else if (attribute) attribute->template Read<RW> (input_stream);
		else
		{
			ATTRIBUTE_TYPE_TO_READ temporary;        // don't have place to put but need to read it to get to next, so just read and throw away.
			temporary.template Read<RW> (input_stream);
		}
	}

	template<class T, class TV, class RW>
	void Read (std::istream& input_stream, T_PARTICLE* particles, const PARTICLE_ATTRIBUTE_COLLECTION* attributes)
	{
		PARTICLE_ATTRIBUTE_COLLECTION_MAP<T_PARTICLE> read_map; // temporary map that says what is in file... where in file is irrelevant, always same order
		char version;
		Read_Binary<RW> (input_stream, version);

		if (version < 1 || version > 2)
		{
			std::cerr << "Unrecognized particle_map version " << version << std::endl;
			exit (1);
		}

		Read_Binary<RW> (input_stream, read_map.position, read_map.velocity, read_map.mass, read_map.radius, read_map.density, read_map.temperature, read_map.age, read_map.id, read_map.vorticity);

		if (version > 1) Read_Binary<RW> (input_stream, read_map.quantized_collision_distance);
		else read_map.quantized_collision_distance = 0;

		Read_Helper<PARTICLE_POSITION_ATTRIBUTE<T, TV>, RW> (input_stream, particles, attributes, read_map.position, position);
		Read_Helper<PARTICLE_VELOCITY_ATTRIBUTE<T, TV>, RW> (input_stream, particles, attributes, read_map.velocity, velocity);
		Read_Helper<PARTICLE_MASS_ATTRIBUTE<T>, RW> (input_stream, particles, attributes, read_map.mass, mass);
		Read_Helper<PARTICLE_ATTRIBUTE<T>, RW> (input_stream, particles, attributes, read_map.radius, radius);
		Read_Helper<PARTICLE_ATTRIBUTE<T>, RW> (input_stream, particles, attributes, read_map.density, density);
		Read_Helper<PARTICLE_ATTRIBUTE<T>, RW> (input_stream, particles, attributes, read_map.temperature, temperature);
		Read_Helper<PARTICLE_ATTRIBUTE<T>, RW> (input_stream, particles, attributes, read_map.age, age);
		Read_Helper<PARTICLE_ATTRIBUTE<int>, RW> (input_stream, particles, attributes, read_map.id, id);
		Read_Helper<PARTICLE_ATTRIBUTE<VECTOR_3D<T> >, RW> (input_stream, particles, attributes, read_map.vorticity, vorticity);
		Read_Helper<PARTICLE_ATTRIBUTE<unsigned short>, RW> (input_stream, particles, attributes, read_map.quantized_collision_distance, quantized_collision_distance);
	}

	template<class ATTRIBUTE_TYPE_TO_WRITE, class RW>
	void Write_Helper (std::ostream& output_stream, const T_PARTICLE* particles, const PARTICLE_ATTRIBUTE_COLLECTION* attributes, const int map_entry) const
	{
		const ATTRIBUTE_TYPE_TO_WRITE* attribute = Get_Attribute<ATTRIBUTE_TYPE_TO_WRITE> (particles, attributes, map_entry);

		if (attribute) attribute->template Write<RW> (output_stream, particles->number);
	}

	template<class T, class TV, class RW>
	void Write (std::ostream& output_stream, const T_PARTICLE* particles, const PARTICLE_ATTRIBUTE_COLLECTION* attributes) const
	{
		Write_Binary<RW> (output_stream, current_version);
		Write_Binary<RW> (output_stream, position, velocity, mass, radius, density, temperature, age, id, vorticity);
		Write_Binary<RW> (output_stream, quantized_collision_distance);
		Write_Helper<PARTICLE_POSITION_ATTRIBUTE<T, TV>, RW> (output_stream, particles, attributes, position);
		Write_Helper<PARTICLE_VELOCITY_ATTRIBUTE<T, TV>, RW> (output_stream, particles, attributes, velocity);
		Write_Helper<PARTICLE_MASS_ATTRIBUTE<T>, RW> (output_stream, particles, attributes, mass);
		Write_Helper<PARTICLE_ATTRIBUTE<T>, RW> (output_stream, particles, attributes, radius);
		Write_Helper<PARTICLE_ATTRIBUTE<T>, RW> (output_stream, particles, attributes, density);
		Write_Helper<PARTICLE_ATTRIBUTE<T>, RW> (output_stream, particles, attributes, temperature);
		Write_Helper<PARTICLE_ATTRIBUTE<T>, RW> (output_stream, particles, attributes, age);
		Write_Helper<PARTICLE_ATTRIBUTE<int>, RW> (output_stream, particles, attributes, id);
		Write_Helper<PARTICLE_ATTRIBUTE<VECTOR_3D<T> >, RW> (output_stream, particles, attributes, vorticity);
		Write_Helper<PARTICLE_ATTRIBUTE<unsigned short>, RW> (output_stream, particles, attributes, quantized_collision_distance);
	}

	void Print_Helper (std::ostream& output_stream, const T_PARTICLE* particles, const PARTICLE_ATTRIBUTE_COLLECTION* attributes, const int map_entry, const std::string& name, const int particle_index) const
	{
		const PARTICLE_ATTRIBUTE_UNTEMPLATIZED* attribute = Get_Attribute<PARTICLE_ATTRIBUTE_UNTEMPLATIZED> (particles, attributes, map_entry);

		if (attribute) attribute->Print (output_stream, name, particle_index);
	}

	void Print (std::ostream& output_stream, const T_PARTICLE* particles, const PARTICLE_ATTRIBUTE_COLLECTION* attributes, const int particle_index) const
	{
		Print_Helper (output_stream, particles, attributes, position, "Position", particle_index);
		Print_Helper (output_stream, particles, attributes, velocity, "Velocity", particle_index);
		Print_Helper (output_stream, particles, attributes, mass, "Mass", particle_index);
		Print_Helper (output_stream, particles, attributes, radius, "Radius", particle_index);
		Print_Helper (output_stream, particles, attributes, density, "Density", particle_index);
		Print_Helper (output_stream, particles, attributes, temperature, "Temperature", particle_index);
		Print_Helper (output_stream, particles, attributes, age, "Age", particle_index);
		Print_Helper (output_stream, particles, attributes, id, "Id", particle_index);
		Print_Helper (output_stream, particles, attributes, vorticity, "Vorticity", particle_index);
		Print_Helper (output_stream, particles, attributes, quantized_collision_distance, "Quantized collision distance", particle_index);
	}

//#####################################################################
};
}
#endif
