//#####################################################################
// Copyright 2004, Ron Fedkiw, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class PARTICLE_POSITION_ATTRIBUTE
//#####################################################################
#ifndef __PARTICLE_POSITION_ATTRIBUTE__
#define __PARTICLE_POSITION_ATTRIBUTE__

#include "PARTICLE_ATTRIBUTE.h"
#include "../Interpolation/LINEAR_INTERPOLATION.h"
namespace PhysBAM
{

template<class T, class TV>
class PARTICLE_POSITION_ATTRIBUTE: public PARTICLE_ATTRIBUTE<TV>
{
public:
	using PARTICLE_ATTRIBUTE<TV>::array;

	PARTICLE_POSITION_ATTRIBUTE()
	{}

	void Euler_Step (const PARTICLE& particle, const ARRAY<TV>& V, const T dt)
	{
		ARRAY<TV>& X = array;

		for (int k = 1; k <= particle.number; k++) X (k) += dt * V (k);
	}

	template<class T_GRID, template<class> class T_ARRAYS, template<class> class T_VECTOR> // GRID_1D & GRID_2D & GRID_3D
	void Euler_Step (const PARTICLE& particle, const T_GRID& grid, const T_ARRAYS<T_VECTOR<T> >& U, const T dt)
	{
		ARRAY<TV>& X = array;
		LINEAR_INTERPOLATION<T, T_VECTOR<T> > interpolation;
		T_VECTOR<int> index;

		for (int k = 1; k <= particle.number; k++)
		{
			interpolation.Clamped_Index_End_Minus_One (grid, U, X (k), index);
			X (k) += dt * interpolation.From_Base_Node (grid, U, X (k), index);
		}
	}

	template<class T_GRID> // QUADTREE_GRID & OCTREE_GRID
	void Euler_Step (const PARTICLE& particle, const T_GRID& grid, const ARRAY<TV>& U, const T dt)
	{
		ARRAY<TV>& X = array;

		for (int k = 1; k <= particle.number; k++) X (k) += dt * grid.Interpolate_Nodes (U, X (k));
	}

	template<class T_GRID, template<class> class T_ARRAYS, template<class> class T_VECTOR> // GRID_1D & GRID_2D & GRID_3D
	void Second_Order_Runge_Kutta_Step (const PARTICLE& particle, const T_GRID& grid, const T_ARRAYS<T_VECTOR<T> >& U, const T dt)
	{
		ARRAY<TV>& X = array;
		LINEAR_INTERPOLATION<T, T_VECTOR<T> > interpolation;
		TV X_new;
		T_VECTOR<int> index;

		for (int k = 1; k <= particle.number; k++)
		{
			interpolation.Clamped_Index_End_Minus_One (grid, U, X (k), index);
			TV velocity = interpolation.From_Base_Node (grid, U, X (k), index);
			X_new = X (k) + dt * velocity;
			interpolation.Clamped_Index_End_Minus_One (grid, U, X_new, index);
			X (k) += (T).5 * dt * (velocity + interpolation.From_Base_Node (grid, U, X_new, index));
		}
	}

	template<class T_GRID> // QUADTREE_GRID & OCTREE_GRID
	void Second_Order_Runge_Kutta_Step (const PARTICLE& particle, const T_GRID& grid, const ARRAY<TV>& U, const T dt)
	{
		ARRAY<TV>& X = array;
		TV X_new;

		for (int k = 1; k <= particle.number; k++)
		{
			TV velocity = dt * grid.Interpolate_Nodes (U, X (k));
			X_new = X (k) + dt * velocity;
			X (k) += (T).5 * dt * (velocity + dt * grid.Interpolate_Nodes (U, X_new));
		}
	}

//#####################################################################
};
}
#endif
