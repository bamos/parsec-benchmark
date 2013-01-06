//#####################################################################
// Copyright 2002-2004, Robert Bridson, Ronald Fedkiw, Eran Guendelman.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class PLANE
//#####################################################################
#ifndef __PLANE__
#define __PLANE__

#include "../Matrices_And_Vectors/VECTOR_3D.h"
namespace PhysBAM
{

template<class T> class RAY_3D;
template<class T> class BOX_3D;

template<class T>
class PLANE
{
public:
	VECTOR_3D<T> normal;
	VECTOR_3D<T> x1; // point on the plane

	PLANE()
		: normal (0, 1, 0), x1 (0, 0, 0)
	{}

	PLANE (const VECTOR_3D<T>& normal_input, const VECTOR_3D<T>& x1_input)
		: normal (normal_input), x1 (x1_input)
	{}

	PLANE (const VECTOR_3D<T>& x1_input, const VECTOR_3D<T>& x2_input, const VECTOR_3D<T>& x3_input)
	{
		Specify_Three_Clockwise_Points (x1_input, x2_input, x3_input);
	}

	void Specify_Three_Clockwise_Points (const VECTOR_3D<T>& x1_input, const VECTOR_3D<T>& x2_input, const VECTOR_3D<T>& x3_input)
	{
		normal = VECTOR_3D<T>::Cross_Product (x2_input - x1_input, x3_input - x1_input);
		normal.Robust_Normalize();
		x1 = x1_input;
	}

	static VECTOR_3D<T> Normal (const VECTOR_3D<T>& x1_input, const VECTOR_3D<T>& x2_input, const VECTOR_3D<T>& x3_input)
	{
		return VECTOR_3D<T>::Cross_Product (x2_input - x1_input, x3_input - x1_input).Normalized();
	}

	VECTOR_3D<T> Normal() const
	{
		return normal;
	}

	T Shortest_Signed_Distance (const VECTOR_3D<T>& location) const // signed distance from the given location to the plane
	{
		return VECTOR_3D<T>::Dot_Product (normal, location - x1);
	}

	virtual ~PLANE()
	{}

	template<class RW>
	void Read (std::istream &input_stream)
	{
		normal.template Read<RW> (input_stream);
		x1.template Read<RW> (input_stream);
	}

	template<class RW>
	void Write (std::ostream &output_stream) const
	{
		normal.template Write<RW> (output_stream);
		x1.template Write<RW> (output_stream);
	}

//#####################################################################
	bool Intersection (RAY_3D<T>& ray, const T thickness_over_2, const T distance, const T rate_of_approach) const;
	bool Intersection (RAY_3D<T>& ray, const T thickness_over_2 = 0) const;
	bool Intersection (const BOX_3D<T>& box, const T thickness_over_2 = 0) const;
	bool Segment_Plane_Intersection (const VECTOR_3D<T>& endpoint1, const VECTOR_3D<T>& endpoint2, T& interpolation_fraction) const;
	bool Lazy_Intersection (RAY_3D<T>& ray) const;
	bool Rectangle_Intersection (RAY_3D<T>& ray, const PLANE<T>& bounding_plane_1, const PLANE<T>& bounding_plane_2, const PLANE<T>& bounding_plane_3, const PLANE<T>& bounding_plane_4,
				     const T thickness_over_2 = 0) const;
	bool Inside (const VECTOR_3D<T>& location, const T thickness_over_two = 0) const;
	bool Lazy_Inside (const VECTOR_3D<T>& location) const;
	bool Outside (const VECTOR_3D<T>& location, const T thickness_over_two = 0) const;
	bool Lazy_Outside (const VECTOR_3D<T>& location) const;
	bool Boundary (const VECTOR_3D<T>& location, const T thickness_over_two = 0) const;
	VECTOR_3D<T> Surface (const VECTOR_3D<T>& location) const;
//#####################################################################
};
}
#endif

