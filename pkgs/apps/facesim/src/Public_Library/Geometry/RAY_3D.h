//#####################################################################
// Copyright 2002, 2003, 2004, Robert Bridson, Ronald Fedkiw, Eran Guendelman.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class RAY_3D
//#####################################################################
#ifndef __RAY_3D__
#define __RAY_3D__

#include <ostream>
#include <math.h>
#include "../Matrices_And_Vectors/VECTOR_3D.h"
#include "../Geometry/SEGMENT_3D.h"
#include "../Geometry/BOX_3D.h"
#include "../Math_Tools/sign.h"
namespace PhysBAM
{

template<class T> class BOX_3D;

template<class T>
class RAY_3D
{
public:
	VECTOR_3D<T> endpoint; // endpoint of the ray where t=0
	VECTOR_3D<T> direction; // direction the ray sweeps out - unit vector
	bool semi_infinite; // indicates whether the ray is semi_infinite or should stop at t_max
	T t_max; // maximum value of t allowed for the ray
	int aggregate_id; // indicates the piece of an aggregate object that is intersected by t_max
	enum LOCATION {START_POINT, END_POINT, INTERIOR_POINT, LOCATION_UNKNOWN};
	LOCATION intersection_location; // indicates what type of intersection happened, LOCATION_UNKNOWN is used if not computed
	// used for triangle hierarchy fast lazy_box_intersection
	bool computed_lazy_box_intersection_acceleration_data;
	VECTOR_3D<T> inverse_direction;
	VECTOR_3D<int> direction_is_negative;
	BOX_3D<T> bounding_box;

	RAY_3D()
		: endpoint (0, 0, 0), direction (0, 0, 1), semi_infinite (true), t_max (0), aggregate_id (0), intersection_location (LOCATION_UNKNOWN), computed_lazy_box_intersection_acceleration_data (false)
	{}

	RAY_3D (const VECTOR_3D<T>& endpoint_input, const VECTOR_3D<T>& direction_input, const bool already_normalized = false)
		: endpoint (endpoint_input), direction (direction_input), semi_infinite (true), t_max (0), aggregate_id (0), intersection_location (LOCATION_UNKNOWN), computed_lazy_box_intersection_acceleration_data (false)
	{
		if (!already_normalized) direction.Normalize();
	}

	RAY_3D (const SEGMENT_3D<T>& segment)
		: endpoint (segment.x1), direction (segment.x2 - segment.x1), semi_infinite (false), aggregate_id (0), intersection_location (LOCATION_UNKNOWN), computed_lazy_box_intersection_acceleration_data (false)
	{
		t_max = direction.Normalize();
	}

	void Initialize (const VECTOR_3D<T>& endpoint_input, const VECTOR_3D<T>& direction_input, const bool already_normalized = false)
	{
		endpoint = endpoint_input;
		direction = direction_input;
		semi_infinite = true;
		t_max = 0;
		aggregate_id = 0;
		intersection_location = LOCATION_UNKNOWN;
		computed_lazy_box_intersection_acceleration_data = false;

		if (!already_normalized) direction.Normalize();
	}

	void Save_Intersection_Information (RAY_3D<T>& storage_ray) const
	{
		storage_ray.semi_infinite = semi_infinite;
		storage_ray.t_max = t_max;
		storage_ray.aggregate_id = aggregate_id;
		storage_ray.intersection_location = intersection_location;
	}

	void Restore_Intersection_Information (const RAY_3D<T>& storage_ray)
	{
		semi_infinite = storage_ray.semi_infinite;
		t_max = storage_ray.t_max;
		aggregate_id = storage_ray.aggregate_id;
		intersection_location = storage_ray.intersection_location;
	}

	VECTOR_3D<T> Point (const T t) const // finds the point on the ray, given by the parameter t
	{
		return endpoint + t * direction;
	}

	void Compute_Lazy_Box_Intersection_Acceleration_Data()
	{
		inverse_direction.x = T (1) / direction.x;
		inverse_direction.y = T (1) / direction.y;
		inverse_direction.z = T (1) / direction.z;
		direction_is_negative.x = int (inverse_direction.x < 0);
		direction_is_negative.y = int (inverse_direction.y < 0);
		direction_is_negative.z = int (inverse_direction.z < 0);
		computed_lazy_box_intersection_acceleration_data = true;
	}

	void Compute_Bounding_Box()
	{
		assert (!semi_infinite);
		VECTOR_3D<T> ray_max_point = Point (t_max);

		if (ray_max_point.x < endpoint.x)
		{
			bounding_box.xmin = ray_max_point.x;
			bounding_box.xmax = endpoint.x;
		}
		else
		{
			bounding_box.xmin = endpoint.x;
			bounding_box.xmax = ray_max_point.x;
		}

		if (ray_max_point.y < endpoint.y)
		{
			bounding_box.ymin = ray_max_point.y;
			bounding_box.ymax = endpoint.y;
		}
		else
		{
			bounding_box.ymin = endpoint.y;
			bounding_box.ymax = ray_max_point.y;
		}

		if (ray_max_point.z < endpoint.z)
		{
			bounding_box.zmin = ray_max_point.z;
			bounding_box.zmax = endpoint.z;
		}
		else
		{
			bounding_box.zmin = endpoint.z;
			bounding_box.zmax = ray_max_point.z;
		}
	}

	T Parameter (const VECTOR_3D<T>& point) const // finds the parameter t, given a point that lies on the ray
	{
		int axis = direction.Dominant_Axis();
		return (point[axis] - endpoint[axis]) / direction[axis];
	}

	VECTOR_3D<T> Reflected_Direction (const VECTOR_3D<T>& normal) const
	{
		return 2 * VECTOR_3D<T>::Dot_Product (-direction, normal) * normal + direction;
	}

	static bool Create_Non_Degenerate_Ray (const VECTOR_3D<T>& endpoint, const VECTOR_3D<T>& length_and_direction, RAY_3D<T>& ray)
	{
		T length_squared = length_and_direction.Magnitude_Squared();

		if (length_squared > 0)
		{
			ray.t_max = sqrt (length_squared);
			ray.endpoint = endpoint;
			ray.direction = length_and_direction / ray.t_max;
			ray.semi_infinite = false;
			return true;
		}
		else return false;
	}

//#####################################################################
};

template<class T> std::ostream &operator<< (std::ostream &output, const RAY_3D<T> &ray)
{
	output << "endpoint = " << ray.endpoint << ", direction = " << ray.direction << ", ";

	if (ray.semi_infinite) output << "semi infinite";
	else output << "t_max = " << ray.t_max;

	output << std::endl;
	return output;
}

}
#endif

