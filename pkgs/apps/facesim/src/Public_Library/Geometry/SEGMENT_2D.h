//#####################################################################
// Copyright 2003, 2004, Zhaosheng Bao, Ronald Fedkiw, Eran Guendelman, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SEGMENT_2D
//#####################################################################
#ifndef __SEGMENT_2D__
#define __SEGMENT_2D__
#include "../Matrices_And_Vectors/VECTOR_2D.h"

namespace PhysBAM
{

template<class T> class RAY_2D;
template<class T> class ORIENTED_BOX_2D;

template<class T>
class SEGMENT_2D
{
public:
	VECTOR_2D<T> x1, x2;
	enum POINT_SEGMENT_COLLISION_TYPE {POINT_SEGMENT_NO_COLLISION, POINT_SEGMENT_COLLISION_ENDS_OUTSIDE, POINT_SEGMENT_COLLISION_ENDS_INSIDE,
					   POINT_SEGMENT_UNKNOWN_COLLISION
					  };

	SEGMENT_2D()
		: x1 (0, 0), x2 (1, 0)
	{}

	SEGMENT_2D (const VECTOR_2D<T>& x1_input, const VECTOR_2D<T>& x2_input)
		: x1 (x1_input), x2 (x2_input)
	{}

	T Length() const
	{
		return (x2 - x1).Magnitude();
	}

	VECTOR_2D<T> Center() const
	{
		return (T).5 * (x1 + x2);
	}

	static VECTOR_2D<T> Clockwise_Normal (const VECTOR_2D<T>& x1, const VECTOR_2D<T>& x2)
	{
		return (x2 - x1).Normalized().Rotate_Clockwise_90();
	}

	static VECTOR_2D<T> Robust_Clockwise_Normal (const VECTOR_2D<T>& x1, const VECTOR_2D<T>& x2, const T tolerance = 1e-8)
	{
		return (x2 - x1).Robust_Normalized (tolerance).Rotate_Clockwise_90();
	}

	VECTOR_2D<T> Clockwise_Normal() const
	{
		return SEGMENT_2D<T>::Clockwise_Normal (x1, x2);
	}

	static VECTOR_2D<T> Barycentric_Coordinates (const VECTOR_2D<T>& location, const VECTOR_2D<T>& x1, const VECTOR_2D<T>& x2)
	{
		VECTOR_2D<T> v = x2 - x1;
		T denominator = VECTOR_2D<T>::Dot_Product (v, v);

		if (denominator == 0) return VECTOR_2D<T> (1, 0); // x1 and x2 are a single point
		else
		{
			T t = VECTOR_2D<T>::Dot_Product (location - x1, v) / denominator;
			return VECTOR_2D<T> (1 - t, t);
		}
	}

	VECTOR_2D<T> Barycentric_Coordinates (const VECTOR_2D<T>& location) const
	{
		return Barycentric_Coordinates (location, x1, x2);
	}

	static VECTOR_2D<T> Point_From_Barycentric_Coordinates (const T alpha, const VECTOR_2D<T>& x1, const VECTOR_2D<T>& x2)
	{
		return (x2 - x1) * alpha + x1;
	}

	VECTOR_2D<T> Point_From_Barycentric_Coordinates (const T alpha) const
	{
		return (x2 - x1) * alpha + x1;
	}

//#####################################################################
	bool Segment_Line_Intersection (const VECTOR_2D<T>& point_on_line, const VECTOR_2D<T>& normal_of_line, T &interpolation_fraction) const;
	VECTOR_2D<T> Closest_Point_On_Segment (const VECTOR_2D<T>& point) const;
	T Distance_From_Point_To_Segment (const VECTOR_2D<T>& point) const;
	VECTOR_2D<T> Closest_Point_On_Line (const VECTOR_2D<T>& point) const;
	T Distance_From_Point_To_Line (const VECTOR_2D<T>& point) const;
	VECTOR_2D<T> Shortest_Vector_Between_Segments (const SEGMENT_2D<T>& segment, T& a, T& b) const;
	int Segment_Segment_Interaction (const SEGMENT_2D<T>& segment, const VECTOR_2D<T>& v1, const VECTOR_2D<T>& v2, const VECTOR_2D<T>& v3, const VECTOR_2D<T>& v4,
					 const T interaction_distance, T& distance, VECTOR_2D<T>& normal, T& a, T& b, T& relative_speed, const T small_number = 0) const;
//    int Segment_Segment_Collision(const SEGMENT_2D<T>& segment,const VECTOR_2D<T>& v1,const VECTOR_2D<T>& v2,const VECTOR_2D<T>& v3,const VECTOR_2D<T>& v4,const T dt,
//        const T collision_thickness,T& collision_time,VECTOR_2D<T>& normal,T& a,T& b,T& relative_speed,const T small_number=0) const;
	ORIENTED_BOX_2D<T> Thickened_Oriented_Box (const T thickness_over_two = 0) const;
	bool Intersection (RAY_2D<T>& ray, const T thickness_over_two) const;
	static bool Intersection_X_Segment (RAY_2D<T>& ray, const T x1, const T x2, const T y, const T thickness_over_two);
	static bool Intersection_Y_Segment (RAY_2D<T>& ray, const T x, const T y1, const T y2, const T thickness_over_two);
	bool Fuzzy_Intersection (RAY_2D<T>& ray, const T thickness_over_two = 0) const;
	bool Inside (const VECTOR_2D<T>& point, const T thickness_over_two = 0) const;
	bool Closest_Non_Intersecting_Point (RAY_2D<T>& ray, const T thickness_over_two) const;
	static POINT_SEGMENT_COLLISION_TYPE Robust_Point_Segment_Collision (const SEGMENT_2D<T>& initial_segment, const SEGMENT_2D<T>& final_segment, const VECTOR_2D<T>& x,
			const VECTOR_2D<T>& final_x, const T dt, const T collision_thickness, T& collision_time, T& collision_alpha);
	bool Segment_Point_Collision (const VECTOR_2D<T>& x, const VECTOR_2D<T>& v, const VECTOR_2D<T>& v1, const VECTOR_2D<T>& v2, const T dt, const T collision_thickness, T& collision_time, T& collision_alpha) const;
//#####################################################################
};

template<class T> std::ostream &operator<< (std::ostream &output, const SEGMENT_2D<T> &segment)
{
	output << segment.x1 << ", " << segment.x2;
	return output;
}

}
#endif

