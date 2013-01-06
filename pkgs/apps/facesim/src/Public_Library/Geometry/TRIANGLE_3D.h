//#####################################################################
// Copyright 2002-2004, Robert Bridson, Ronald Fedkiw, Eran Guendelman, Geoffrey Irving, Neil Molino, Andrew Selle, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class TRIANGLE_3D
//#####################################################################
#ifndef __TRIANGLE_3D__
#define __TRIANGLE_3D__

#include "PLANE.h"
#include "BOX_3D.h"
#include "SEGMENT_3D.h"
#include "../Math_Tools/constants.h"
namespace PhysBAM
{

template<class T>
class TRIANGLE_3D: public PLANE<T>
{
public:
	using PLANE<T>::x1;
	using PLANE<T>::normal;

	VECTOR_3D<T> x2, x3; // x1 (in PLANE), x2 and x3 - clockwise order when looking at the plane
	enum POINT_TRIANGLE_COLLISION_TYPE {POINT_TRIANGLE_NO_COLLISION, POINT_TRIANGLE_COLLISION_ENDS_OUTSIDE, POINT_TRIANGLE_COLLISION_ENDS_INSIDE,
					    POINT_TRIANGLE_UNKNOWN_COLLISION
					   };

	TRIANGLE_3D()
	{
		Specify_Three_Clockwise_Points (VECTOR_3D<T> (0, 0, 0), VECTOR_3D<T> (0, 1, 0), VECTOR_3D<T> (1, 0, 0));
	}

	TRIANGLE_3D (const VECTOR_3D<T>& x1_input, const VECTOR_3D<T>& x2_input, const VECTOR_3D<T>& x3_input)
	{
		Specify_Three_Clockwise_Points (x1_input, x2_input, x3_input);
	}


	virtual ~TRIANGLE_3D()
	{}

	void Specify_Three_Clockwise_Points (const VECTOR_3D<T>& x1_input, const VECTOR_3D<T>& x2_input, const VECTOR_3D<T>& x3_input)
	{
		PLANE<T>::Specify_Three_Clockwise_Points (x1_input, x2_input, x3_input);
		x2 = x2_input;
		x3 = x3_input;
	}

	static VECTOR_3D<T> Clockwise_Normal (const VECTOR_3D<T>& x1_input, const VECTOR_3D<T>& x2_input, const VECTOR_3D<T>& x3_input)
	{
		return VECTOR_3D<T>::Cross_Product (x2_input - x1_input, x3_input - x1_input).Normalized();
	}

	static VECTOR_3D<T> Clockwise_Normal_Direction (const VECTOR_3D<T>& x1_input, const VECTOR_3D<T>& x2_input, const VECTOR_3D<T>& x3_input)
	{
		return VECTOR_3D<T>::Cross_Product (x2_input - x1_input, x3_input - x1_input);       // can have any magnitude
	}

	static VECTOR_3D<T> Robust_Clockwise_Normal (const VECTOR_3D<T>& x1_input, const VECTOR_3D<T>& x2_input, const VECTOR_3D<T>& x3_input, const T tolerance = 1e-8)
	{
		return VECTOR_3D<T>::Cross_Product (x2_input - x1_input, x3_input - x1_input).Robust_Normalized (tolerance);       // can have any magnitude
	}

	T Area() const
	{
		return Area (x1, x2, x3);
	}

	static T Area (const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2, const VECTOR_3D<T>& x3) // always positive for clockwise vertices: x1, x2, x3
	{
		return (T).5 * VECTOR_3D<T>::Cross_Product (x2 - x1, x3 - x1).Magnitude();
	}

	static T Area_Squared (const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2, const VECTOR_3D<T>& x3) // always positive for clockwise vertices: x1, x2, x3
	{
		return (T).25 * VECTOR_3D<T>::Cross_Product (x2 - x1, x3 - x1).Magnitude_Squared();
	}

	T Aspect_Ratio() const
	{
		return Aspect_Ratio (x1, x2, x3);
	}

	static T Aspect_Ratio (const VECTOR_3D<T>& x1_input, const VECTOR_3D<T>& x2_input, const VECTOR_3D<T>& x3_input)
	{
		VECTOR_3D<T> u = x1_input - x2_input, v = x2_input - x3_input, w = x3_input - x1_input;
		T u2 = VECTOR_3D<T>::Dot_Product (u, u), v2 = VECTOR_3D<T>::Dot_Product (v, v), w2 = VECTOR_3D<T>::Dot_Product (w, w);
		return max (u2, v2, w2) / (T) sqrt (VECTOR_3D<T>::Cross_Product (u, v).Magnitude_Squared());
	}

	static T Minimum_Edge_Length (const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2, const VECTOR_3D<T>& x3)
	{
		return sqrt (min ( (x2 - x1).Magnitude_Squared(), (x3 - x1).Magnitude_Squared(), (x3 - x2).Magnitude_Squared()));
	}

	static T Maximum_Edge_Length (const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2, const VECTOR_3D<T>& x3)
	{
		return sqrt (max ( (x2 - x1).Magnitude_Squared(), (x3 - x1).Magnitude_Squared(), (x3 - x2).Magnitude_Squared()));
	}

	static T Minimum_Altitude (const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2, const VECTOR_3D<T>& x3)
	{
		return 2 * Area (x1, x2, x3) / Maximum_Edge_Length (x1, x2, x3);
	}

	static VECTOR_3D<T> Barycentric_Coordinates (const VECTOR_3D<T>& location, const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2, const VECTOR_3D<T>& x3) // clockwise vertices
	{
		VECTOR_3D<T> u = x2 - x1, v = x3 - x1, w = location - x1;
		T u_dot_u = VECTOR_3D<T>::Dot_Product (u, u), v_dot_v = VECTOR_3D<T>::Dot_Product (v, v), u_dot_v = VECTOR_3D<T>::Dot_Product (u, v),
		  u_dot_w = VECTOR_3D<T>::Dot_Product (u, w), v_dot_w = VECTOR_3D<T>::Dot_Product (v, w);
		T one_over_denominator = 1 / (u_dot_u * v_dot_v - sqr (u_dot_v));
		T a = (v_dot_v * u_dot_w - u_dot_v * v_dot_w) * one_over_denominator, b = (u_dot_u * v_dot_w - u_dot_v * u_dot_w) * one_over_denominator;
		return VECTOR_3D<T> (1 - a - b, a, b);
	}

	static VECTOR_3D<T> Clamped_Barycentric_Coordinates (const VECTOR_3D<T>& location, const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2, const VECTOR_3D<T>& x3, const T tolerance = 1e-7) // clockwise vertices
	{
		VECTOR_3D<T> u = x2 - x1, v = x3 - x1, w = location - x1;
		T u_dot_u = VECTOR_3D<T>::Dot_Product (u, u), v_dot_v = VECTOR_3D<T>::Dot_Product (v, v), u_dot_v = VECTOR_3D<T>::Dot_Product (u, v),
		  u_dot_w = VECTOR_3D<T>::Dot_Product (u, w), v_dot_w = VECTOR_3D<T>::Dot_Product (v, w);

		if (fabs (u_dot_u) < tolerance)
		{
			if (fabs (v_dot_v) < tolerance) return VECTOR_3D<T> ( (T) one_third, (T) one_third, (T) one_third); // single point

			T c = clamp (v_dot_w / v_dot_v, (T) 0, (T) 1);
			T a_and_b = (T).5 * (1 - c);
			return VECTOR_3D<T> (a_and_b, a_and_b, c);
		} // x1 and x2 are a single point
		else if (fabs (v_dot_v) < tolerance)
		{
			T b = clamp (u_dot_w / u_dot_u, (T) 0, (T) 1);
			T a_and_c = (T).5 * (1 - b);
			return VECTOR_3D<T> (a_and_c, b, a_and_c);
		} // x1 and x3 are a single point
		else
		{
			T denominator = u_dot_u * v_dot_v - sqr (u_dot_v);

			if (fabs (denominator) < tolerance)
			{
				if (u_dot_v > 0) // u and v point in the same direction
				{
					if (u_dot_u > u_dot_v)
					{
						T b = clamp (u_dot_w / u_dot_u, (T) 0, (T) 1);
						return VECTOR_3D<T> (1 - b, b, 0);
					}
					else
					{
						T c = clamp (v_dot_w / v_dot_v, (T) 0, (T) 1);
						return VECTOR_3D<T> (1 - c, 0, c);
					}
				}
				else if (u_dot_w > 0)
				{
					T b = clamp (u_dot_w / u_dot_u, (T) 0, (T) 1);        // u and v point in opposite directions, and w is on the u segment
					return VECTOR_3D<T> (1 - b, b, 0);
				}
				else
				{
					T c = clamp (v_dot_w / v_dot_v, (T) 0, (T) 1);        // u and v point in opposite directions, and w is on the v segment
					return VECTOR_3D<T> (1 - c, 0, c);
				}
			}

			T one_over_denominator = 1 / denominator;
			T a = clamp ( (v_dot_v * u_dot_w - u_dot_v * v_dot_w) * one_over_denominator, (T) 0, (T) 1), b = clamp ( (u_dot_u * v_dot_w - u_dot_v * u_dot_w) * one_over_denominator, (T) 0, (T) 1);
			return VECTOR_3D<T> (1 - a - b, a, b);
		}
	}

	VECTOR_3D<T> Barycentric_Coordinates (const VECTOR_3D<T>& location) const
	{
		return Barycentric_Coordinates (location, x1, x2, x3);
	}

	static VECTOR_3D<T> Point_From_Barycentric_Coordinates (const VECTOR_3D<T>& weights, const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2, const VECTOR_3D<T>& x3) // clockwise vertices
	{
		return weights.x * x1 + weights.y * x2 + weights.z * x3;
	}

	VECTOR_3D<T> Point_From_Barycentric_Coordinates (const VECTOR_3D<T>& weights) const
	{
		return Point_From_Barycentric_Coordinates (weights, x1, x2, x3);
	}

	static VECTOR_3D<T> Center (const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2, const VECTOR_3D<T>& x3) // centroid
	{
		return (T) one_third * (x1 + x2 + x3);
	}

	VECTOR_3D<T> Center() const // centroid
	{
		return Center (x1, x2, x3);
	}

	VECTOR_3D<T> Incenter() const // intersection of angle bisectors
	{
		VECTOR_3D<T> edge_lengths ( (x3 - x2).Magnitude(), (x1 - x3).Magnitude(), (x2 - x1).Magnitude());
		T perimeter = edge_lengths.x + edge_lengths.y + edge_lengths.z;
		assert (perimeter > 0);
		return Point_From_Barycentric_Coordinates (edge_lengths / perimeter);
	}

	template<class RW>
	void Read (std::istream &input_stream)
	{
		x1.template Read<RW> (input_stream);
		x2.template Read<RW> (input_stream);
		x3.template Read<RW> (input_stream);
	}

	template<class RW>
	void Write (std::ostream &output_stream) const
	{
		x1.template Write<RW> (output_stream);
		x2.template Write<RW> (output_stream);
		x3.template Write<RW> (output_stream);
	}

//#####################################################################
	void Change_Size (const T delta);
	bool Intersection (RAY_3D<T>& ray, const T thickness_over_2 = 0) const;
	bool Lazy_Intersection (RAY_3D<T>& ray) const;
	bool Closest_Non_Intersecting_Point (RAY_3D<T>& ray, const T thickness_over_2 = 0) const;
	bool Point_Inside_Triangle (const VECTOR_3D<T>& point, const T thickness_over_2 = 0) const;
	bool Planar_Point_Inside_Triangle (const VECTOR_3D<T>& point, const T thickness_over_2 = 0) const;
	bool Lazy_Planar_Point_Inside_Triangle (const VECTOR_3D<T>& point) const;
	T Minimum_Edge_Length() const;
	T Maximum_Edge_Length() const;
	BOX_3D<T> Bounding_Box() const;
	int Region (const VECTOR_3D<T>& location, int& region_id, const T tolerance) const;
	VECTOR_3D<T> Closest_Point (const VECTOR_3D<T>& location, VECTOR_3D<T>& weights) const;
	T Distance_To_Triangle (const VECTOR_3D<T>& location) const;
	T Minimum_Angle() const;
	T Maximum_Angle() const;
	T Signed_Solid_Angle (const VECTOR_3D<T>& center) const;
	bool Segment_Triangle_Intersection (const SEGMENT_3D<T>& segment, const T thickness_over_2 = 0) const;
	bool Segment_Triangle_Intersection (const SEGMENT_3D<T>& segment, T& a, VECTOR_3D<T>& weights, const T thickness_over_2 = 0) const;
	bool Point_Triangle_Interaction (const VECTOR_3D<T>& x, const T interaction_distance, T& distance) const;
	void Point_Triangle_Interaction_Data (const VECTOR_3D<T>& x, T& distance, VECTOR_3D<T>& interaction_normal, VECTOR_3D<T>& weights) const;
	bool Point_Triangle_Interaction (const VECTOR_3D<T>& x, const VECTOR_3D<T>& v, const VECTOR_3D<T>& v1, const VECTOR_3D<T>& v2, const VECTOR_3D<T>& v3, const T interaction_distance,
					 T& distance, VECTOR_3D<T>& interaction_normal, VECTOR_3D<T>& weights, T& relative_speed, const bool exit_early = false) const;
	bool Point_Triangle_Interaction_One_Sided (const VECTOR_3D<T>& x, const VECTOR_3D<T>& v, const VECTOR_3D<T>& v1, const VECTOR_3D<T>& v2, const VECTOR_3D<T>& v3, const T interaction_distance,
			T& distance, VECTOR_3D<T>& interaction_normal, VECTOR_3D<T>& weights, T& relative_speed, const bool exit_early = false) const;
	static POINT_TRIANGLE_COLLISION_TYPE Robust_Point_Triangle_Collision (const TRIANGLE_3D<T>& initial_triangle, const TRIANGLE_3D<T>& final_triangle, const VECTOR_3D<T>& x,
			const VECTOR_3D<T>& final_x, const T dt, const T collision_thickness, T& collision_time, VECTOR_3D<T>& normal, VECTOR_3D<T>&weights, T& relative_speed);
	bool Point_Triangle_Collision (const VECTOR_3D<T>& x, const VECTOR_3D<T>& v, const VECTOR_3D<T>& v1, const VECTOR_3D<T>& v2, const VECTOR_3D<T>& v3, const T dt, const T collision_thickness,
				       T& collision_time, VECTOR_3D<T>& normal, VECTOR_3D<T>& weights, T& relative_speed, const bool exit_early = false) const;
//#####################################################################
};
}
#endif

