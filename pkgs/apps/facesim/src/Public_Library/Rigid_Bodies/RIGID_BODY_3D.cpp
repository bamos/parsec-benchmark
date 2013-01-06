//#####################################################################
// Copyright 2003-2004, Zhaosheng Bao, Ronald Fedkiw, Eran Guendelman, Sergey Koltakov, Frank Losasso, Andrew Selle, Rachel Weinstein.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "RIGID_BODY_3D.h"
#include "../Utilities/DEBUG_UTILITIES.h"
using namespace PhysBAM;

//#####################################################################
// Euler_Step
//#####################################################################
template<class T> void RIGID_BODY_3D<T>::
Euler_Step (const T dt, const T time, const T small_number)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Euler_Step_Position
//#####################################################################
// position and orientation
template<class T> void RIGID_BODY_3D<T>::
Euler_Step_Position (const T dt, const T time, const T small_number)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Euler_Step_Velocity
//#####################################################################
// velocity and angular momentum
template<class T> void RIGID_BODY_3D<T>::
Euler_Step_Velocity (const T dt, const T time)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function And_Velocity_Independent_Forces
//#####################################################################
template<class T> void RIGID_BODY_3D<T>::
Add_Velocity_Independent_Forces (VECTOR_3D<T>& F, VECTOR_3D<T>& torque) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Add_Velocity_Dependent_Forces
//#####################################################################
template<class T> void RIGID_BODY_3D<T>::
Add_Velocity_Dependent_Forces (VECTOR_3D<T>& F, VECTOR_3D<T>& torque) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Initialize_CFL
//#####################################################################
template<class T> void RIGID_BODY_3D<T>::
Initialize_CFL()
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function CFL
//#####################################################################
template<class T> T RIGID_BODY_3D<T>::
CFL (const T max_distance_per_time_step, const T max_rotation_per_time_step, const bool verbose)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Interpolate_Between_States
//#####################################################################
template<class T> void RIGID_BODY_3D<T>::
Interpolate_Between_States (const RIGID_BODY_STATE_3D<T>& state1, const RIGID_BODY_STATE_3D<T>& state2, const T time, RIGID_BODY_STATE_3D<T>& interpolated_state)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Compute_Velocity_Between_States
//#####################################################################
template<class T> void RIGID_BODY_3D<T>::
Compute_Velocity_Between_States (const RIGID_BODY_STATE_3D<T>& state1, const RIGID_BODY_STATE_3D<T>& state2, RIGID_BODY_STATE_3D<T>& result_state)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Apply_Impulse
//#####################################################################
template<class T> void RIGID_BODY_3D<T>::
Apply_Impulse (RIGID_BODY_3D<T>& body1, RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& location, const VECTOR_3D<T>& impulse)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Apply_Impulse_And_Angular_Impulse
//#####################################################################
template<class T> void RIGID_BODY_3D<T>::
Apply_Impulse_And_Angular_Impulse (RIGID_BODY_3D<T>& body1, RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& impulse, const VECTOR_3D<T>& angular_impulse)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Apply_Angular_Impulse
//#####################################################################
template<class T> void RIGID_BODY_3D<T>::
Apply_Angular_Impulse (RIGID_BODY_3D<T>& body1, RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& angular_impulse)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Apply_Collision_Impulse
//#####################################################################
// kept for backwards compatibility; eventually should call other version instead
template<class T> void RIGID_BODY_3D<T>::
Apply_Collision_Impulse (RIGID_BODY_3D<T>& body1, RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& location, const VECTOR_3D<T>& normal,
			 const T coefficient_of_restitution, const T coefficient_of_friction, const bool clamp_friction_magnitude, const bool rolling_friction)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Apply_Collision_Impulse
//#####################################################################
// clamp friction magnitude: should be (true) in the elastic collision case, (false) in the inelastic collision case
template<class T> void RIGID_BODY_3D<T>::
Apply_Collision_Impulse (RIGID_BODY_3D<T>& body1, RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& location, const VECTOR_3D<T>& normal, const VECTOR_3D<T>& relative_velocity,
			 const T coefficient_of_restitution, const T coefficient_of_friction, const bool clamp_friction_magnitude, const bool rolling_friction)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Apply_Sticking_Impulse
//#####################################################################
// maks the bodies stick together
template<class T> void RIGID_BODY_3D<T>::
Apply_Sticking_Impulse (RIGID_BODY_3D<T>& body1, RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& location)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Apply_Angular_Sticking_Impulse
//#####################################################################
template<class T> void RIGID_BODY_3D<T>::
Apply_Angular_Sticking_Impulse (RIGID_BODY_3D<T>& body1, RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& location)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Apply_Sticking_And_Angular_Sticking_Impulse
//#####################################################################
template<class T> void RIGID_BODY_3D<T>::
Apply_Sticking_And_Angular_Sticking_Impulse (RIGID_BODY_3D<T>& body1, RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& location, const VECTOR_3D<T>& delta_relative_angular_velocity,
		const VECTOR_3D<T>& delta_relative_linear_velocity, const T epsilon_scale)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Apply_Rolling_Friction
//#####################################################################
// location is a point in world space about which body is rolling
template<class T> void RIGID_BODY_3D<T>::
Apply_Rolling_Friction (RIGID_BODY_3D<T>& body1, RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& location, const VECTOR_3D<T>& normal, const T normal_impulse)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Find_Impulse_And_Angular_Impulse
//#####################################################################
template<class T> void RIGID_BODY_3D<T>::
Find_Impulse_And_Angular_Impulse (const RIGID_BODY_3D<T>& body1, const RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& location, const VECTOR_3D<T>& delta_relative_velocity_at_location,
				  const VECTOR_3D<T>& delta_relative_angular_velocity, VECTOR_3D<T>& impulse, VECTOR_3D<T>& angular_impulse)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Apply_Push
//#####################################################################
template<class T> void RIGID_BODY_3D<T>::
Apply_Push (RIGID_BODY_3D<T>& body1, RIGID_BODY_3D<T>& body2, const VECTOR_3D<T>& location, const VECTOR_3D<T>& normal, const T distance)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Adjust_Point_For_Collision
//#####################################################################
template<class T> void RIGID_BODY_3D<T>::
Adjust_Point_For_Collision (VECTOR_3D<T>& X, VECTOR_3D<T>& V, const T point_mass, const T penetration_depth, const T one_over_dt)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Adjust_Point_For_Collision
//#####################################################################
template<class T> void RIGID_BODY_3D<T>::
Adjust_Point_For_Collision (VECTOR_3D<T>& X, const T penetration_depth)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Earliest_Triangle_Crossover
//#####################################################################
template<class T> bool RIGID_BODY_3D<T>::
Earliest_Triangle_Crossover (const VECTOR_3D<T>& start_X, const VECTOR_3D<T>& end_X, const T dt, T& hit_time, VECTOR_3D<T>& normal, VECTOR_3D<T>& weights, int& triangle_id) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Latest_Triangle_Crossover
//#####################################################################
template<class T> bool RIGID_BODY_3D<T>::
Latest_Triangle_Crossover (const VECTOR_3D<T>& start_X, const VECTOR_3D<T>& end_X, const T dt, T& hit_time, VECTOR_3D<T>& normal, VECTOR_3D<T>& weights, int& triangle_id, typename TRIANGLE_3D<T>::POINT_TRIANGLE_COLLISION_TYPE& returned_collision_type) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Any_Triangle_Crossover
//#####################################################################
template<class T> bool RIGID_BODY_3D<T>::
Any_Triangle_Crossover (const VECTOR_3D<T>& start_X, const VECTOR_3D<T>& end_X, const T dt) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Compute_Occupied_Cells
//#####################################################################
template<class T> void RIGID_BODY_3D<T>::
Compute_Occupied_Cells (const GRID_3D<T>& grid, ARRAYS_3D<bool>& occupied, const bool with_body_motion, const T extra_thickness, const T body_thickness_factor, const bool reset_occupied_to_false) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Get_Triangle_Bounding_Boxes
//#####################################################################
template<class T> void RIGID_BODY_3D<T>::
Get_Triangle_Bounding_Boxes (LIST_ARRAY<BOX_3D<T> >& bounding_boxes, const bool with_body_motion, const T extra_thickness, const T body_thickness_factor) const
{
	NOT_IMPLEMENTED();
}

//#####################################################################
// Function Update_Intersection_Acceleration_Structures
//#####################################################################
template<class T> void RIGID_BODY_3D<T>::
Update_Intersection_Acceleration_Structures (const bool use_swept_triangle_hierarchy, const int state1, const int state2)
{
	NOT_IMPLEMENTED();
}

//#####################################################################
template class RIGID_BODY_3D<float>;
template class RIGID_BODY_3D<double>;
