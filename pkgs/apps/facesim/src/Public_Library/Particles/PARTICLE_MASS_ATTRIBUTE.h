//#####################################################################
// Copyright 2004, Ron Fedkiw, Andrew Selle, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class PARTICLE_MASS_ATTRIBUTE
//#####################################################################
#ifndef __PARTICLE_MASS_ATTRIBUTE__
#define __PARTICLE_MASS_ATTRIBUTE__

#include "PARTICLE_ATTRIBUTE.h"
#include <float.h> // for FLT_MAX
namespace PhysBAM
{

template<class T>
class PARTICLE_MASS_ATTRIBUTE: public PARTICLE_ATTRIBUTE<T>
{
public:
	using PARTICLE_ATTRIBUTE<T>::array;

	ARRAY<T> one_over_mass;

	PARTICLE_MASS_ATTRIBUTE()
	{}

	void Clean_Up_Memory()
	{
		PARTICLE_ATTRIBUTE<T>::Clean_Up_Memory();
		one_over_mass.Resize_Array (0);
	}

	void Compute_One_Over_Mass()
	{
		ARRAY<T>& mass = array;
		one_over_mass.Resize_Array (mass.m, false, false);

		for (int i = 1; i <= mass.m; i++) one_over_mass (i) = 1 / mass (i);
	}

	template<class TV>
	TV Center_Of_Mass (const PARTICLE& particle, const ARRAY<TV>& X, const bool store_mass = true) const
	{
		ARRAY<T>& mass = array;
		T total_mass = 0;
		TV center_of_mass;

		if (store_mass) for (int i = 1; i <= particle.number; i++)
			{
				total_mass += mass (i);
				center_of_mass += mass (i) * X (i);
			}
		else
		{
			total_mass = (T) particle.number;

			for (int i = 1; i <= particle.number; i++) center_of_mass += X (i);
		}

		assert (total_mass > 0);
		return center_of_mass / total_mass;
	}

	T Total_Mass (const PARTICLE& particle) const
	{
		return ARRAY<T>::sum (array, particle.number);
	}

	T Minimum_Mass (const PARTICLE& particle) const
	{
		const ARRAY<T>& mass = array;
		return (particle.number > 0) ? ARRAY<T>::min (mass, particle.number) : FLT_MAX;
	}

	T Maximum_Mass (const PARTICLE& particle) const
	{
		const ARRAY<T>& mass = array;
		return (particle.number > 0) ? ARRAY<T>::max (mass, particle.number) : 0;
	}

	T Average_Mass (const PARTICLE& particle) const
	{
		return (particle.number > 0) ? Total_Mass (particle) / particle.number : 0;
	}

//#####################################################################
};
}
#endif
