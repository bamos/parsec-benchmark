//#####################################################################
// Copyright 2004, Ron Fedkiw, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class PARTICLE_VELOCITY_ATTRIBUTE
//#####################################################################
#ifndef __PARTICLE_VELOCITY_ATTRIBUTE__
#define __PARTICLE_VELOCITY_ATTRIBUTE__

#include "PARTICLE_ATTRIBUTE.h"
namespace PhysBAM
{

template<class T, class TV>
class PARTICLE_VELOCITY_ATTRIBUTE: public PARTICLE_ATTRIBUTE<TV>
{
public:
	using PARTICLE_ATTRIBUTE<TV>::array;

	PARTICLE_VELOCITY_ATTRIBUTE()
	{}

	void Euler_Step (const PARTICLE& particle, const ARRAY<TV>& A, const T dt)
	{
		ARRAY<TV>& V = array;

		for (int k = 1; k <= particle.number; k++) V (k) += dt * A (k);
	}

	void Euler_Step (const PARTICLE& particle, const ARRAY<TV>& F, const ARRAY<T>& mass, const T dt)
	{
		ARRAY<TV>& V = array;

		for (int k = 1; k <= particle.number; k++) V (k) += dt / mass (k) * F (k);
	}

	T Maximum_Speed (const PARTICLE& particle, int* index = 0) const
	{
		const ARRAY<TV>& V = array;
		T max_speed_sqr = 0;
		int max_speed_index = 0;

		for (int k = 1; k <= particle.number; k++)
		{
			T speed_sqr = V (k).Magnitude_Squared();

			if (speed_sqr > max_speed_sqr)
			{
				max_speed_sqr = speed_sqr;
				max_speed_index = k;
			}
		}

		if (index) *index = max_speed_index;

		return sqrt (max_speed_sqr);
	}

//#####################################################################
};
}
#endif
