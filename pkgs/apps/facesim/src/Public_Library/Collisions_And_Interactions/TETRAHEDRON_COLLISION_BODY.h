//#####################################################################
// Copyright 2004-2005, Ron Fedkiw, Geoffrey Irving, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class TETRAHEDRON_COLLISION_BODY
//#####################################################################
#ifndef __TETRAHEDRON_COLLISION_BODY__
#define __TETRAHEDRON_COLLISION_BODY__

#include "COLLISION_BODY_3D.h"
#include "../Particles/SOLIDS_PARTICLE.h"
#include "../Geometry/TETRAHEDRALIZED_VOLUME.h"
#include "../Geometry/LEVELSET_IMPLICIT_SURFACE.h"
namespace PhysBAM
{

template<class T> class DEFORMABLE_OBJECT_3D;

template<class T>
class TETRAHEDRON_COLLISION_BODY: public COLLISION_BODY_3D<T>
{
public:
	using COLLISION_BODY_3D<T>::body_type;
	using COLLISION_BODY_3D<T>::Set_Collision_Thickness;
	using COLLISION_BODY_3D<T>::collision_thickness;

	TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume;
	TRIANGULATED_SURFACE<T>* triangulated_surface;
	LEVELSET_IMPLICIT_SURFACE<T>* implicit_surface;
	SOLIDS_PARTICLE<T, VECTOR_3D<T> >* undeformed_tetrahedron_particles;
	TRIANGULATED_SURFACE<T>* undeformed_triangulated_surface;
	DEFORMABLE_OBJECT_3D<T>* deformable_object; // for use in embedded/embedded tetrahedron collisions
	T max_min_barycentric_weight_tolerance;
	T friction_coefficient;
	T relaxation_factor;
	T normal_interpolation_scale_factor;
	T self_collision_normal_angle_tolerance;
	T min_tet_volume_tolerance;

	TETRAHEDRON_COLLISION_BODY (TETRAHEDRALIZED_VOLUME<T>& tetrahedralized_volume_input, TRIANGULATED_SURFACE<T>* triangulated_surface_input = 0)
		: tetrahedralized_volume (tetrahedralized_volume_input), triangulated_surface (triangulated_surface_input), implicit_surface (0),
		  undeformed_tetrahedron_particles (0), undeformed_triangulated_surface (0), deformable_object (0)
	{
		body_type = COLLISION_BODY_3D<T>::TETRAHEDRON_TYPE;
		Set_Collision_Thickness();
		Set_Max_Min_Barycentric_Weight_Tolerance();
		Set_Min_Tet_Volume_Tolerance();
		Set_Relaxation_Factor();
		Set_Friction_Coefficient();
		Set_Normal_Interpolation_Scale_Factor();
		Set_Self_Collision_Normal_Angle_Tolerance();

		if (!triangulated_surface)
		{
			if (!tetrahedralized_volume.triangulated_surface) tetrahedralized_volume.Initialize_Triangulated_Surface();

			triangulated_surface = tetrahedralized_volume.triangulated_surface;
		}

		tetrahedralized_volume.Initialize_Tetrahedron_Hierarchy();
		triangulated_surface->Initialize_Triangle_Hierarchy();
		triangulated_surface->avoid_normal_interpolation_across_sharp_edges = false;
	}

	VECTOR_3D<T> Surface_Normal (const int triangle, const VECTOR_3D<T>& weights) const
	{
		int i, j, k;
		triangulated_surface->triangle_mesh.triangles.Get (triangle, i, j, k);
		VECTOR_3D<T> n1 = (*triangulated_surface->vertex_normals) (1, i), n2 = (*triangulated_surface->vertex_normals) (1, j), n3 = (*triangulated_surface->vertex_normals) (1, k);
		return (weights.x * n1 + weights.y * n2 + weights.z * n3).Robust_Normalized();
	}

	VECTOR_3D<T> Depth_Interpolated_Normal (const T unsigned_depth, const VECTOR_3D<T>& outward_direction, const VECTOR_3D<T>& surface_normal) const
	{
		T lambda = min (unsigned_depth / normal_interpolation_scale_factor, (T) 1);
		return (lambda * outward_direction + (1 - lambda) * surface_normal).Robust_Normalized();
	}

	void Set_Min_Tet_Volume_Tolerance (const T min_tet_volume_tolerance_input = - (T) 1e-6)
	{
		min_tet_volume_tolerance = min_tet_volume_tolerance_input;
	}

	void Set_Normal_Interpolation_Scale_Factor (const T collision_thickness_scaling = (T) 2)
	{
		normal_interpolation_scale_factor = collision_thickness_scaling * collision_thickness;
	}

	void Set_Implicit_Surface (LEVELSET_IMPLICIT_SURFACE<T>* implicit_surface_input)
	{
		implicit_surface = implicit_surface_input;
	}

	void Set_Undeformed_Triangulated_Surface (TRIANGULATED_SURFACE<T>* undeformed_triangulated_surface_input)
	{
		undeformed_triangulated_surface = undeformed_triangulated_surface_input;
		undeformed_triangulated_surface->Update_Triangle_List();
		undeformed_triangulated_surface->Initialize_Triangle_Hierarchy();

		if (!undeformed_tetrahedron_particles) undeformed_tetrahedron_particles = &undeformed_triangulated_surface->particles;
	}

	void Set_Undeformed_Tetrahedron_Particles (SOLIDS_PARTICLE<T, VECTOR_3D<T> >* undeformed_tetrahedron_particles_input)
	{
		undeformed_tetrahedron_particles = undeformed_tetrahedron_particles_input;
	}

	void Set_Deformable_Object (DEFORMABLE_OBJECT_3D<T>* deformable_object_input = 0)
	{
		deformable_object = deformable_object_input;
	}

	void Set_Self_Collision_Normal_Angle_Tolerance (const T self_collision_normal_angle_tolerance_input = - (T) 1e-5)
	{
		self_collision_normal_angle_tolerance = self_collision_normal_angle_tolerance_input;
	}

	void Set_Friction_Coefficient (const T friction_coefficient_input = (T).1)
	{
		friction_coefficient = friction_coefficient_input;
	}

	void Set_Max_Min_Barycentric_Weight_Tolerance (const T max_min_barycentric_weight_tolerance_input = (T) - 1e-6)
	{
		max_min_barycentric_weight_tolerance = max_min_barycentric_weight_tolerance_input;
	}

	void Set_Relaxation_Factor (const T relaxation_factor_input = (T).25)
	{
		relaxation_factor = relaxation_factor_input;
	}

//#####################################################################
	bool Implicit_Surface_Lazy_Inside_And_Value (const VECTOR_3D<T>& location, T& phi, T contour_value = 0) const;
	bool Implicit_Surface_Lazy_Outside_Extended_Levelset_And_Value (const VECTOR_3D<T>& location, T& phi_value, T contour_value = 0) const;
	VECTOR_3D<T> Implicit_Surface_Normal (const VECTOR_3D<T>& location, const int aggregate = -1) const;
	VECTOR_3D<T> Implicit_Surface_Normal (const VECTOR_3D<T>& location, T& phi_value, const int aggregate = -1, const int location_particle_index = 0) const;
	VECTOR_3D<T> Implicit_Surface_Extended_Normal (const VECTOR_3D<T>& location, T& phi_value, const int aggregate = -1, const int location_particle_index = 0) const;
	int Get_Tetrahedron_Near_Point (const VECTOR_3D<T>& point, VECTOR_3D<T>& weights, const int particle_index = 0) const;
	int Get_Surface_Triangle (const int tetrahedron_index, const VECTOR_3D<T>& tetrahedron_weights, VECTOR_3D<T>& surface_weights, const bool omit_outside_points = false,
				  const bool omit_inside_points = false, bool* inside = 0) const;
	void Adjust_Point_Face_Collision_Position_And_Velocity (const int triangle_index, VECTOR_3D<T>& X, VECTOR_3D<T>& V, const T mass, const T dt, const VECTOR_3D<T>& weights,
			VECTOR_3D<T>& position_change);
//#####################################################################
};
}
#endif
