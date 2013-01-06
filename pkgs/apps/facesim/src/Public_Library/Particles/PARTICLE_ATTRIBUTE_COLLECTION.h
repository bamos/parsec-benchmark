//#####################################################################
// Copyright 2004, Ron Fedkiw, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class PARTICLE_ATTRIBUTE_COLLECTION
//#####################################################################
//
// An array of the untemplatized version so that we can have multiple types.
//
//#####################################################################
#ifndef __PARTICLE_ATTRIBUTE_COLLECTION__
#define __PARTICLE_ATTRIBUTE_COLLECTION__

#include "PARTICLE.h"
#include "PARTICLE_ATTRIBUTE_UNTEMPLATIZED.h"
#include "PARTICLE_ATTRIBUTE.h"
#include "PARTICLE_POSITION_ATTRIBUTE.h"
#include "PARTICLE_VELOCITY_ATTRIBUTE.h"
#include "PARTICLE_MASS_ATTRIBUTE.h"
namespace PhysBAM
{

class PARTICLE;
template<class T, class TV> class PARTICLE_POSITION_ATTRIBUTE;
template<class T, class TV> class PARTICLE_VELOCITY_ATTRIBUTE;
template<class T> class PARTICLE_MASS_ATTRIBUTE;
template<class T_PARTICLE> class PARTICLE_ATTRIBUTE_COLLECTION_MAP;

class PARTICLE_ATTRIBUTE_COLLECTION
{
public:
	ARRAY<PARTICLE_ATTRIBUTE_UNTEMPLATIZED*> attributes;

	PARTICLE_ATTRIBUTE_COLLECTION()
	{}

	~PARTICLE_ATTRIBUTE_COLLECTION()
	{
		for (int i = 1; i <= attributes.m; i++) delete attributes (i);
	}

	void Clean_Up_Memory()
	{
		for (int i = 1; i <= attributes.m; i++) attributes (i)->Clean_Up_Memory();
	}

	void Resize_Array (const int number_of_particles)
	{
		for (int i = 1; i <= attributes.m; i++) attributes (i)->Resize_Array (number_of_particles);
	}

	void Copy_Particle (const int from, const int to)
	{
		for (int i = 1; i <= attributes.m; i++) attributes (i)->Copy_Particle (from, to);
	}

	void Copy_Particle (const PARTICLE_ATTRIBUTE_COLLECTION* from_attribute_collection, const int from, const int to)
	{
		assert (from_attribute_collection->attributes.m == attributes.m); // ALSO, order and contents of attributes arrays must be the same

		for (int i = 1; i <= attributes.m; i++) attributes (i)->Copy_Particle (from_attribute_collection->attributes (i), from, to);
	}

private:
	template<class TS>
	void Store (const int slot) // only map calls this
	{
		if (slot > 0)
		{
			delete attributes (slot);
			attributes (slot) = new TS;
		}
	}

	template<class T_PARTICLE> friend class PARTICLE_ATTRIBUTE_COLLECTION_MAP;

public:
	template<class TS>
	TS Get (const int map_slot)
	{
		if (map_slot <= 0) return 0;
		else return (TS) attributes (map_slot);
	}

	template<class T, class TV, class T_ATTRIBUTE_MAP>
	PARTICLE_POSITION_ATTRIBUTE<T, TV>* Get_Position (const T_ATTRIBUTE_MAP& attribute_map)
	{
		return Get<PARTICLE_POSITION_ATTRIBUTE<T, TV>*> (attribute_map.position);
	}

	template<class T, class TV, class T_ATTRIBUTE_MAP>
	PARTICLE_VELOCITY_ATTRIBUTE<T, TV>* Get_Velocity (const T_ATTRIBUTE_MAP& attribute_map)
	{
		return Get<PARTICLE_VELOCITY_ATTRIBUTE<T, TV>*> (attribute_map.velocity);
	}

	template<class T, class T_ATTRIBUTE_MAP>
	PARTICLE_MASS_ATTRIBUTE<T>* Get_Mass (const T_ATTRIBUTE_MAP& attribute_map)
	{
		return Get<PARTICLE_MASS_ATTRIBUTE<T>* > (attribute_map.mass);
	}

	template<class T, class T_ATTRIBUTE_MAP>
	PARTICLE_ATTRIBUTE<T>* Get_Radius (const T_ATTRIBUTE_MAP& attribute_map)
	{
		return Get<PARTICLE_ATTRIBUTE<T>*> (attribute_map.radius);
	}

	template<class T, class T_ATTRIBUTE_MAP>
	PARTICLE_ATTRIBUTE<T>* Get_Density (const T_ATTRIBUTE_MAP& attribute_map)
	{
		return Get<PARTICLE_ATTRIBUTE<T>*> (attribute_map.density);
	}

	template<class T, class T_ATTRIBUTE_MAP>
	PARTICLE_ATTRIBUTE<T>* Get_Temperature (const T_ATTRIBUTE_MAP& attribute_map)
	{
		return Get<PARTICLE_ATTRIBUTE<T>*> (attribute_map.temperature);
	}

	template<class T, class T_ATTRIBUTE_MAP>
	PARTICLE_ATTRIBUTE<T>* Get_Age (const T_ATTRIBUTE_MAP& attribute_map)
	{
		return Get<PARTICLE_ATTRIBUTE<T>*> (attribute_map.age);
	}

	template<class T_ATTRIBUTE_MAP>
	PARTICLE_ATTRIBUTE<int>* Get_Id (const T_ATTRIBUTE_MAP& attribute_map)
	{
		return Get<PARTICLE_ATTRIBUTE<int>*> (attribute_map.id);
	}

	template<class TV, class T_ATTRIBUTE_MAP>
	PARTICLE_ATTRIBUTE<TV>* Get_Vorticity (const T_ATTRIBUTE_MAP& attribute_map)
	{
		return Get<PARTICLE_ATTRIBUTE<TV>*> (attribute_map.vorticity);
	}

	template<class TV, class T_ATTRIBUTE_MAP>
	PARTICLE_ATTRIBUTE<unsigned short>* Get_Quantized_Collision_Distance (const T_ATTRIBUTE_MAP& attribute_map)
	{
		return Get<PARTICLE_ATTRIBUTE<unsigned short>*> (attribute_map.quantized_collision_distance);
	}

//#####################################################################
};
}
#endif
