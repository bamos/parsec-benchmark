//#####################################################################
// Copyright 2003, 2004, Zhaosheng Bao, Eran Guendelman, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class RAY_2D
//#####################################################################
#ifndef __RAY_2D__
#define __RAY_2D__

#include <math.h>
#include "../Matrices_And_Vectors/VECTOR_2D.h"
#include "../Geometry/SEGMENT_2D.h"
#include "../Geometry/BOX_2D.h"
namespace PhysBAM
{

template<class T> class SEGMENT_2D; // Needed for cyclic dependencies
template<class T> class BOX_2D;

template<class T>
class RAY_2D
{
public:
	VECTOR_2D<T> endpoint; // endpoint of the ray where t=0
	VECTOR_2D<T> direction; // direction the ray sweeps out - unit vector
	bool semi_infinite; // indicates whether the ray is semi_infinite or should stop at t_max
	T t_max; // maximum value of t allowed for the ray
	int aggregate_id; // indicates the piece of an aggregate object that is intersected by t_max
	enum LOCATION {START_POINT, END_POINT, INTERIOR_POINT, LOCATION_UNKNOWN};
	LOCATION intersection_location; // indicates what type of intersection happened, LOCATION_UNKNOWN is used if not computed
	BOX_2D<T> bounding_box;

	RAY_2D()
		: endpoint (0, 0), direction (0, 1), semi_infinite (true), t_max (0), aggregate_id (0), intersection_location (LOCATION_UNKNOWN)
	{}

	RAY_2D (const VECTOR_2D<T>& endpoint_input, const VECTOR_2D<T>& direction_input, const bool already_normalized = false)
		: endpoint (endpoint_input), direction (direction_input), semi_infinite (true), t_max (0), aggregate_id (0), intersection_location (LOCATION_UNKNOWN)
	{
		if (!already_normalized) direction.Normalize();
	}

	RAY_2D (const SEGMENT_2D<T>& segment)
		: endpoint (segment.x1), direction (segment.x2 - segment.x1), semi_infinite (false), aggregate_id (0), intersection_location (LOCATION_UNKNOWN)
	{
		t_max = direction.Normalize();
	}

	void Initialize (const VECTOR_2D<T>& endpoint_input, const VECTOR_2D<T>& direction_input, const bool already_normalized = false)
	{
		endpoint = endpoint_input;
		direction = direction_input;
		semi_infinite = true;
		t_max = 0;
		aggregate_id = 0;
		intersection_location = LOCATION_UNKNOWN;

		if (!already_normalized) direction.Normalize();
	}

	void Save_Intersection_Information (RAY_2D<T>& storage_ray) const
	{
		storage_ray.semi_infinite = semi_infinite;
		storage_ray.t_max = t_max;
		storage_ray.aggregate_id = aggregate_id;
		storage_ray.intersection_location = intersection_location;
	}

	void Restore_Intersection_Information (const RAY_2D<T>& storage_ray)
	{
		semi_infinite = storage_ray.semi_infinite;
		t_max = storage_ray.t_max;
		aggregate_id = storage_ray.aggregate_id;
		intersection_location = storage_ray.intersection_location;
	}

	VECTOR_2D<T> Point (const T t) const // finds the point on the ray, given by the parameter t
	{
		return endpoint + t * direction;
	}

	void Compute_Bounding_Box()
	{
		assert (!semi_infinite);
		VECTOR_2D<T> ray_max_point = Point (t_max);

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
	}

	T Parameter (const VECTOR_2D<T>& point) const // finds the parameter t, given a point that lies on the ray
	{
		int axis = direction.Dominant_Axis();
		return (point[axis] - endpoint[axis]) / direction[axis];
	}

	VECTOR_2D<T> Reflected_Direction (const VECTOR_2D<T>& normal) const
	{
		return 2 * VECTOR_2D<T>::Dot_Product (-direction, normal) * normal + direction;
	}

	static bool Create_Non_Degenerate_Ray (const VECTOR_2D<T>& endpoint, const VECTOR_2D<T>& unnormalized_direction, RAY_2D<T>& ray)
	{
		T length_squared = unnormalized_direction.Magnitude_Squared();

		if (length_squared > 0)
		{
			ray.t_max = sqrt (length_squared);
			ray.endpoint = endpoint;
			ray.direction = unnormalized_direction / ray.t_max;
			ray.semi_infinite = false;
			return true;
		}
		else return false;
	}

//#####################################################################
//#####################################################################
};

template<class T> std::ostream &operator<< (std::ostream &output, const RAY_2D<T> &ray)
{
	output << "endpoint = " << ray.endpoint << ", direction = " << ray.direction << ", ";

	if (ray.semi_infinite) output << "semi infinite";
	else output << "t_max = " << ray.t_max;

	output << std::endl;
	return output;
}

}
#endif

