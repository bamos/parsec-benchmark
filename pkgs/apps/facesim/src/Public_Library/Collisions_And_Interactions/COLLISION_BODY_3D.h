//#####################################################################
// Copyright 2004, Ron Fedkiw, Andrew Selle, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class COLLISION_BODY_3D
//#####################################################################
#ifndef __COLLISION_BODY_3D__
#define __COLLISION_BODY_3D__

#include "../Arrays/ARRAY.h"
#include "../Arrays/ARRAYS_3D.h"
#include "../Matrices_And_Vectors/VECTOR_3D.h"
#include "../Geometry/TRIANGLE_3D.h"
namespace PhysBAM
{

template<class T> class OCTREE_GRID;
template<class T> class LIST_ARRAY;

template<class T>
class COLLISION_BODY_3D
{
public:
	enum BODY_TYPE {ARBITRARY_BODY_TYPE, RIGID_BODY_TYPE, DEFORMABLE_BODY_TYPE, TETRAHEDRON_TYPE};
	enum STATE_INDEX {THIN_SHELLS_NEW_STATE = 1, THIN_SHELLS_OLD_STATE, SOLIDS_EVOLUTION_RIGID_BODY_NEW_STATE, SOLIDS_EVOLUTION_RIGID_BODY_OLD_STATE};
	BODY_TYPE body_type;
	bool active; // when false, body is ignored
	T collision_thickness;

	COLLISION_BODY_3D()
		: body_type (ARBITRARY_BODY_TYPE), active (true)
	{}

	virtual ~COLLISION_BODY_3D()
	{}

	void Default() const
	{
		std::cout << "THIS COLLISION_BODY_3D FUNCTION IS NOT DEFINED!" << std::endl;
		assert (false);
	}

	void Set_Collision_Thickness (const T thickness = (T) 1e-3)
	{
		collision_thickness = thickness;
	}

//#####################################################################
	virtual VECTOR_3D<T> Pointwise_Object_Velocity (const int triangle_id, const VECTOR_3D<T>& location) const
	{
		Default();
		return VECTOR_3D<T>();
	}
	virtual VECTOR_3D<T> Pointwise_Object_Pseudo_Velocity (const int triangle_id, const VECTOR_3D<T>& location, const int state1, const int state2) const
	{
		Default();
		return VECTOR_3D<T>();
	}
	virtual VECTOR_3D<T> Triangulated_Surface_World_Space_Point_From_Barycentric_Coordinates (const int triangle_id, const VECTOR_3D<T>& weights) const
	{
		Default();
		return VECTOR_3D<T>();
	}
	virtual TRIANGLE_3D<T> Triangulated_Surface_World_Space_Triangle (const int triangle_id, const bool use_saved_state = false) const
	{
		Default();
		return TRIANGLE_3D<T>();
	}
	virtual bool Triangulated_Surface_Intersection (RAY_3D<T>& ray) const
	{
		Default();
		return false;
	}
	virtual bool Triangulated_Surface_Closest_Non_Intersecting_Point (RAY_3D<T>& ray) const
	{
		Default();
		return false;
	}
	virtual bool Triangulated_Surface_Inside_Any_Triangle (const VECTOR_3D<T>& location, int& triangle_id) const
	{
		Default();
		return false;
	}
	virtual bool Earliest_Triangle_Crossover (const VECTOR_3D<T>& start_X, const VECTOR_3D<T>& end_X, const T dt, T& hit_time, VECTOR_3D<T>& normal, VECTOR_3D<T>& weights,
			int& triangle_id) const
	{
		Default();
		return false;
	}
	virtual bool Latest_Triangle_Crossover (const VECTOR_3D<T>& start_X, const VECTOR_3D<T>& end_X, const T dt, T& hit_time, VECTOR_3D<T>& normal, VECTOR_3D<T>& weights, int& triangle_id,
						typename TRIANGLE_3D<T>::POINT_TRIANGLE_COLLISION_TYPE& returned_collision_type) const
	{
		Default();
		return false;
	}
	virtual bool Any_Triangle_Crossover (const VECTOR_3D<T>& start_X, const VECTOR_3D<T>& end_X, const T dt) const
	{
		Default();
		return false;
	}
	virtual VECTOR_3D<T> Triangulated_Surface_Surface (const VECTOR_3D<T>& location, const T max_distance, int* triangle_id = 0, T* distance = 0) const
	{
		Default();
		return VECTOR_3D<T>();
	}
	virtual void Compute_Occupied_Cells (const GRID_3D<T>& grid, ARRAYS_3D<bool>& occupied, const bool with_body_motion, const T extra_thickness, const T body_thickness_factor,
					     const bool reset_occupied_to_false = true) const
	{
		Default();
	}
	virtual void Compute_Occupied_Cells (const OCTREE_GRID<T>& grid, ARRAY<bool>& occupied, const bool with_body_motion, const T extra_thickness, const T body_thickness_factor,
					     const bool reset_occupied_to_false = true) const
	{
		Default();
	}
	virtual void Get_Triangle_Bounding_Boxes (LIST_ARRAY<BOX_3D<T> >& bounding_boxes, const bool with_body_motion, const T extra_thickness, const T body_thickness_factor) const
	{
		Default();
	}
	virtual VECTOR_3D<T> Implicit_Surface_Normal (const VECTOR_3D<T>& location, const int aggregate = -1) const
	{
		Default();
		return VECTOR_3D<T>();
	}
	virtual VECTOR_3D<T> Implicit_Surface_Normal (const VECTOR_3D<T>& location, T& phi_value, const int aggregate = -1, const int location_particle_index = 0) const
	{
		Default();
		return VECTOR_3D<T>();
	}
	virtual VECTOR_3D<T> Implicit_Surface_Extended_Normal (const VECTOR_3D<T>& location, T& phi_value, const int aggregate = -1, const int location_particle_index = 0) const
	{
		Default();
		return VECTOR_3D<T>();
	}
	virtual bool Implicit_Surface_Lazy_Inside (const VECTOR_3D<T>& location, T contour_value = 0) const
	{
		Default();
		return false;
	}
	virtual bool Implicit_Surface_Lazy_Inside_And_Value (const VECTOR_3D<T>& location, T& phi, T contour_value = 0) const
	{
		Default();
		return false;
	}
	virtual bool Implicit_Surface_Lazy_Outside_Extended_Levelset_And_Value (const VECTOR_3D<T>& location, T& phi_value, T contour_value = 0) const
	{
		Default();
		return false;
	}
	virtual void Adjust_Point_For_Collision (VECTOR_3D<T>& X, VECTOR_3D<T>& V, const T point_mass, const T penetration_depth, const T one_over_dt)
	{
		Default();
	}
	virtual void Adjust_Point_For_Collision (VECTOR_3D<T>& X, const T penetration_depth)
	{
		Default();
	}
	virtual void Save_State (const int state_index, const T time = 0)
	{
		Default();
	}
	virtual void Restore_State (const int state_index)
	{
		Default();
	}
	virtual void Delete_State (const int state_index)
	{
		Default();
	}
	virtual void Update_Intersection_Acceleration_Structures (const bool use_swept_triangle_hierarchy, const int state1 = 0, const int state2 = 0)
	{
		Default();
	}
	virtual T Thin_Shell_Density() const
	{
		Default();
		return (T) 0;
	}
//#####################################################################
};
}
#endif
