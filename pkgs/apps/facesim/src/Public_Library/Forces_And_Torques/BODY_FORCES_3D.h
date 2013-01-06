//#####################################################################
// Copyright 2002-2004, Robert Bridson, Ronald Fedkiw, Geoffrey Irving, Sergey Koltakov, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class BODY_FORCES_3D
//#####################################################################
#ifndef __BODY_FORCES_3D__
#define __BODY_FORCES_3D__

#include "SOLIDS_FORCES.h"
#include "../Geometry/SEGMENTED_CURVE_3D.h"
#include "../Geometry/TRIANGULATED_SURFACE.h"
#include "../Geometry/TETRAHEDRALIZED_VOLUME.h"
namespace PhysBAM
{

template<class T>
class BODY_FORCES_3D: public SOLIDS_FORCES<T, VECTOR_3D<T> >
{
public:
	using SOLIDS_FORCES<T, VECTOR_3D<T> >::particles;

	SEGMENTED_CURVE_3D<T>* segmented_curve;
	TRIANGULATED_SURFACE<T>* triangulated_surface;
	TETRAHEDRALIZED_VOLUME<T>* tetrahedralized_volume;
	T gravity;
	VECTOR_3D<T> downward_direction;
	bool ether_drag; // ignores geometry
	bool wind_drag; // uses geometry
	bool use_constant_wind;
	T constant_wind_viscosity;
	VECTOR_3D<T> constant_wind;
	bool use_spatially_varying_wind;
	T spatially_varying_wind_viscosity;
	BOX_3D<T> spatially_varying_wind_domain;
	GRID_3D<T> V_grid;
	ARRAYS_3D<VECTOR_3D<T> >* spatially_varying_wind;
	T wind_density;
	ARRAYS_3D<T> *spatially_varying_wind_density, *spatially_varying_wind_pressure;
	LINEAR_INTERPOLATION<T, T> interpolation;
	LINEAR_INTERPOLATION<T, VECTOR_3D<T> > vector_interpolation;

	BODY_FORCES_3D (SOLIDS_PARTICLE<T, VECTOR_3D<T> >& particles_input)
		: SOLIDS_FORCES<T, VECTOR_3D<T> > (particles_input), segmented_curve (0), triangulated_surface (0), tetrahedralized_volume (0),
		  ether_drag (false), wind_drag (false),
		  use_constant_wind (false), use_spatially_varying_wind (false),
		  wind_density (0), spatially_varying_wind_density (0), spatially_varying_wind_pressure (0)
	{
		Set_Gravity (0);
	}

	BODY_FORCES_3D (SEGMENTED_CURVE_3D<T>& segmented_curve_input)
		: SOLIDS_FORCES<T, VECTOR_3D<T> > (segmented_curve_input.particles), segmented_curve (&segmented_curve_input), triangulated_surface (0), tetrahedralized_volume (0),
		  ether_drag (false), wind_drag (false),
		  use_constant_wind (false), use_spatially_varying_wind (false),
		  wind_density (0), spatially_varying_wind_density (0), spatially_varying_wind_pressure (0)
	{
		Set_Gravity (0);
	}

	BODY_FORCES_3D (TRIANGULATED_SURFACE<T>& triangulated_surface_input)
		: SOLIDS_FORCES<T, VECTOR_3D<T> > (triangulated_surface_input.particles), segmented_curve (0), triangulated_surface (&triangulated_surface_input), tetrahedralized_volume (0),
		  ether_drag (false), wind_drag (false),
		  use_constant_wind (false), use_spatially_varying_wind (false),
		  wind_density (0), spatially_varying_wind_density (0), spatially_varying_wind_pressure (0)
	{
		Set_Gravity (0);
	}

	BODY_FORCES_3D (TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume_input)
		: SOLIDS_FORCES<T, VECTOR_3D<T> > (tetrahedralized_volume_input.particles), segmented_curve (0), triangulated_surface (0), tetrahedralized_volume (&tetrahedralized_volume_input),
		  ether_drag (false), wind_drag (false),
		  use_constant_wind (false), use_spatially_varying_wind (false),
		  wind_density (0), spatially_varying_wind_density (0), spatially_varying_wind_pressure (0)
	{
		Set_Gravity (0);
	}

	virtual ~BODY_FORCES_3D()
	{}

	void Set_Gravity (const T gravity_input = 9.8, const VECTOR_3D<T>& downward_direction_input = VECTOR_3D<T> (0, -1, 0))
	{
		gravity = gravity_input;
		downward_direction = downward_direction_input;
	}

	void Use_No_Drag()
	{
		ether_drag = wind_drag = false;
	}

	void Use_Ether_Drag()
	{
		ether_drag = true;
		wind_drag = false;
	}

	void Use_Wind_Drag()
	{
		ether_drag = false;
		wind_drag = true;
	}

	void Use_Constant_Wind (const T viscosity_input, const VECTOR_3D<T>& wind_input = VECTOR_3D<T> (0, 0, 0))
	{
		use_constant_wind = true;
		constant_wind_viscosity = viscosity_input;
		constant_wind = wind_input;
	}

	void Use_Spatially_Varying_Wind (const T viscosity_input, const BOX_3D<T>& domain_input, const GRID_3D<T>& grid_input, ARRAYS_3D<VECTOR_3D<T> >& V_input)
	{
		use_spatially_varying_wind = true;
		spatially_varying_wind_viscosity = viscosity_input;
		spatially_varying_wind_domain.Reset_Bounds (domain_input);
		V_grid = grid_input;
		spatially_varying_wind = &V_input;
	}

	void Set_Wind_Density (const T wind_density_input)
	{
		wind_density = wind_density_input;
	}

	void Set_Wind_Density (ARRAYS_3D<T>& density_input)
	{
		spatially_varying_wind_density = &density_input;
	}

	void Set_Wind_Pressure (ARRAYS_3D<T>& pressure_input)
	{
		spatially_varying_wind_pressure = &pressure_input;
	}

	VECTOR_3D<T> Spatially_Varying_Wind_Velocity (const VECTOR_3D<T>& X) const
	{
		return vector_interpolation.Clamped_To_Array (V_grid, *spatially_varying_wind, X);
	}

	T Spatially_Varying_Wind_Density (const VECTOR_3D<T>& X) const
	{
		return interpolation.Clamped_To_Array (V_grid, *spatially_varying_wind_density, X);
	}

	T Spatially_Varying_Wind_Pressure (const VECTOR_3D<T>& X) const
	{
		return interpolation.Clamped_To_Array (V_grid, *spatially_varying_wind_pressure, X);
	}

	void Add_Gravity (ARRAY<VECTOR_3D<T> >& F) const
	{
		VECTOR_3D<T> acceleration = gravity * downward_direction;

		for (int k = 1; k <= F.m; k++) F (k) += particles.mass (k) * acceleration;
	}

	void Add_Velocity_Dependent_Forces (ARRAY<VECTOR_3D<T> >& F) const
	{
		if (ether_drag) Add_Ether_Drag_Velocity_Dependent_Term (F);
	}

	void Add_Velocity_Dependent_Forces (ARRAY<VECTOR_3D<T> >& F, const int partition_id) const
	{
		assert (!ether_drag);
	}

	void Add_Force_Differential (const ARRAY<VECTOR_3D<T> >& dX, ARRAY<VECTOR_3D<T> >& dF) const
	{}

	void Enforce_Definiteness (const bool enforce_definiteness_input)
	{}

//#####################################################################
	void Add_Velocity_Independent_Forces (ARRAY<VECTOR_3D<T> >& F) const;
	void Add_Ether_Drag_Velocity_Independent_Term (ARRAY<VECTOR_3D<T> >& F) const;
	void Add_Ether_Drag_Velocity_Dependent_Term (ARRAY<VECTOR_3D<T> >& F) const;
	void Add_Wind_Drag (const TRIANGULATED_SURFACE<T>& surface, ARRAY<VECTOR_3D<T> >& F) const;
//#####################################################################
};
}
#endif
