//#####################################################################
// Copyright 2002-2004, Robert Bridson, Ronald Fedkiw, Sergey Koltakov, Andrew Selle, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class SEGMENT_3D
//#####################################################################
#ifndef __SEGMENT_3D__
#define __SEGMENT_3D__

#include "../Matrices_And_Vectors/VECTOR_3D.h"
#include "../Matrices_And_Vectors/VECTOR_2D.h"
namespace PhysBAM
{

template<class T>
class SEGMENT_3D
{
public:
	VECTOR_3D<T> x1, x2;

	SEGMENT_3D()
		: x1 (0, 0, 0), x2 (1, 0, 0)
	{}

	SEGMENT_3D (const VECTOR_3D<T>& x1_input, const VECTOR_3D<T>& x2_input)
		: x1 (x1_input), x2 (x2_input)
	{}

	T Length() const
	{
		return (x2 - x1).Magnitude();
	}

	static VECTOR_3D<T> Clockwise_Normal (VECTOR_3D<T> x1, VECTOR_3D<T> x2, VECTOR_3D<T> x3)
	{
		VECTOR_3D<T> v = x2 - x1, face_normal = VECTOR_3D<T>::Cross_Product (v, x3 - x1);
		return VECTOR_3D<T>::Cross_Product (face_normal, v).Normalized();
	} // rotate by 90 degrees clockwise

	static VECTOR_3D<T> Robust_Clockwise_Normal (VECTOR_3D<T> x1, VECTOR_3D<T> x2, VECTOR_3D<T> x3, const T tolerance = 1e-8)
	{
		VECTOR_3D<T> v = x2 - x1, face_normal = VECTOR_3D<T>::Cross_Product (v, x3 - x1);
		return VECTOR_3D<T>::Cross_Product (face_normal, v).Robust_Normalized (tolerance);
	} // rotate by 90 degrees clockwise

	static VECTOR_3D<T> Clockwise_Normal_Direction (VECTOR_3D<T> x1, VECTOR_3D<T> x2, VECTOR_3D<T> x3)
	{
		VECTOR_3D<T> v = x2 - x1, face_normal = VECTOR_3D<T>::Cross_Product (v, x3 - x1);
		return VECTOR_3D<T>::Cross_Product (face_normal, v);
	} // can have any magnitude

//#####################################################################
	VECTOR_3D<T> Closest_Point_On_Segment (const VECTOR_3D<T>& point) const;
	T Distance_From_Point_To_Segment (const VECTOR_3D<T>& point) const;
	VECTOR_3D<T> Closest_Point_On_Line (const VECTOR_3D<T>& point) const;
	T Distance_From_Point_To_Line (const VECTOR_3D<T>& point) const;
	VECTOR_3D<T> Shortest_Vector_Between_Segments (const SEGMENT_3D<T>& segment, VECTOR_2D<T>& weights) const;
	bool Segment_Segment_Interaction (const SEGMENT_3D<T>& segment, const T interaction_distance, T& distance, VECTOR_3D<T>& normal, VECTOR_2D<T>& weights) const;
	void Segment_Segment_Interaction_Data (const SEGMENT_3D<T>& segment, const VECTOR_3D<T>& v1, const VECTOR_3D<T>& v2, const VECTOR_3D<T>& v3, const VECTOR_3D<T>& v4,
					       T& distance, VECTOR_3D<T>& normal, const VECTOR_2D<T>& weights, const T small_number = 0, const bool verbose = true) const;
	bool Segment_Segment_Interaction (const SEGMENT_3D<T>& segment, const VECTOR_3D<T>& v1, const VECTOR_3D<T>& v2, const VECTOR_3D<T>& v3, const VECTOR_3D<T>& v4,
					  const T interaction_distance, T& distance, VECTOR_3D<T>& normal, VECTOR_2D<T>& weights, T& relative_speed, const T small_number = 0, const bool exit_early = false, const bool verbose = true) const;
	bool Segment_Segment_Collision (const SEGMENT_3D<T>& segment, const VECTOR_3D<T>& v1, const VECTOR_3D<T>& v2, const VECTOR_3D<T>& v3, const VECTOR_3D<T>& v4, const T dt,
					const T collision_thickness, T& collision_time, VECTOR_3D<T>& normal, VECTOR_2D<T>& weights, T& relative_speed, const T small_number = 0, const bool exit_early = false) const;
	static VECTOR_2D<T> Barycentric_Coordinates (const VECTOR_3D<T>& location, const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2);
//#####################################################################
};
}
#endif
