//#####################################################################
// Copyright 2002-2003, Robert Bridson, Ronald Fedkiw, Eran Guendelman, Geoffrey Irving, Neil Molino, Eftychios Sifakis, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class TETRAHEDRON
//#####################################################################
#ifndef __TETRAHEDRON__
#define __TETRAHEDRON__

#include "TRIANGLE_3D.h"
namespace PhysBAM
{

template<class T>
class TETRAHEDRON
{
public:
	VECTOR_3D<T> x1, x2, x3, x4;
	TRIANGLE_3D<T> triangle1, triangle2, triangle3, triangle4; // includes the planes and the normals

	TETRAHEDRON()
		: x1 (0, 0, 0), x2 (0, 1, 0), x3 (1, 0, 0), x4 (0, 0, -1)
	{
		Create_Triangles();
	}

	TETRAHEDRON (const VECTOR_3D<T>& x1_input, const VECTOR_3D<T>& x2_input, const VECTOR_3D<T>& x3_input, const VECTOR_3D<T>& x4_input)
		: x1 (x1_input), x2 (x2_input), x3 (x3_input), x4 (x4_input)
	{
		Create_Triangles();
	}

	void Create_Triangles()
	{
		if (VECTOR_3D<T>::Dot_Product (VECTOR_3D<T>::Cross_Product (x2 - x1, x3 - x1), x4 - x1) <= 0)
		{
			triangle1.Specify_Three_Clockwise_Points (x1, x2, x3);
			triangle2.Specify_Three_Clockwise_Points (x1, x4, x2);
			triangle3.Specify_Three_Clockwise_Points (x1, x3, x4);
			triangle4.Specify_Three_Clockwise_Points (x2, x4, x3);
		}
		else
		{
			triangle1.Specify_Three_Clockwise_Points (x1, x3, x2);
			triangle2.Specify_Three_Clockwise_Points (x1, x2, x4);
			triangle3.Specify_Three_Clockwise_Points (x1, x4, x3);
			triangle4.Specify_Three_Clockwise_Points (x2, x3, x4);
		}
	}

	virtual ~TETRAHEDRON()
	{}

	static T Volume (const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2, const VECTOR_3D<T>& x3, const VECTOR_3D<T>& x4)
	{
		return fabs (Signed_Volume (x1, x2, x3, x4));
	}

	static T Signed_Volume (const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2, const VECTOR_3D<T>& x3, const VECTOR_3D<T>& x4)
	{
		return T (one_sixth) * VECTOR_3D<T>::Triple_Product (x2 - x1, x3 - x1, x4 - x1);
	}

	VECTOR_3D<T> Barycentric_Coordinates (const VECTOR_3D<T>& location) const
	{
		return Barycentric_Coordinates (location, x1, x2, x3, x4);
	}

	static VECTOR_3D<T> Barycentric_Coordinates (const VECTOR_3D<T>& location, const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2, const VECTOR_3D<T>& x3, const VECTOR_3D<T>& x4)
	{
		return MATRIX_3X3<T> (x1 - x4, x2 - x4, x3 - x4).Solve_Linear_System (location - x4);
	}

	VECTOR_3D<T> Point_From_Barycentric_Coordinates (const VECTOR_3D<T>& weights) const
	{
		return Point_From_Barycentric_Coordinates (weights, x1, x2, x3, x4);
	}

	static VECTOR_3D<T> Point_From_Barycentric_Coordinates (const VECTOR_3D<T>& weights, const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2, const VECTOR_3D<T>& x3, const VECTOR_3D<T>& x4)
	{
		return weights.x * x1 + weights.y * x2 + weights.z * x3 + ( (T) 1 - weights.x - weights.y - weights.z) * x4;
	}

	static bool Barycentric_Inside (const VECTOR_3D<T>& location, const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2, const VECTOR_3D<T>& x3, const VECTOR_3D<T>& x4, const T tolerance = 0)
	{
		VECTOR_3D<T> w = MATRIX_3X3<T> (x1 - x4, x2 - x4, x3 - x4).Solve_Linear_System (location - x4);
		return w.x >= -tolerance && w.y >= -tolerance && w.z >= -tolerance && w.x + w.y + w.z <= 1 + tolerance;
	}

	static VECTOR_3D<T> Center (const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2, const VECTOR_3D<T>& x3, const VECTOR_3D<T>& x4) // centroid
	{
		return (T).25 * (x1 + x2 + x3 + x4);
	}

	VECTOR_3D<T> Center() const // centroid
	{
		return Center (x1, x2, x3, x4);
	}

	static VECTOR_3D<T> Circumcenter (const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2, const VECTOR_3D<T>& x3, const VECTOR_3D<T>& x4)
	{
		VECTOR_3D<T> u = x2 - x1, v = x3 - x1, w = x4 - x1, cross_uv = VECTOR_3D<T>::Cross_Product (u, v), cross_wu = VECTOR_3D<T>::Cross_Product (w, u),
			     cross_vw = VECTOR_3D<T>::Cross_Product (v, w);
		T determinant = VECTOR_3D<T>::Dot_Product (cross_uv, w), uu = u.Magnitude_Squared(), vv = v.Magnitude_Squared(), ww = w.Magnitude_Squared();
		return x1 + (ww * cross_uv + vv * cross_wu + uu * cross_vw) * ( (T).5 / determinant);
	}

	static T Maximum_Edge_Length (const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2, const VECTOR_3D<T>& x3, const VECTOR_3D<T>& x4)
	{
		return sqrt (max ( (x1 - x2).Magnitude_Squared(), (x2 - x3).Magnitude_Squared(), (x3 - x1).Magnitude_Squared(), (x1 - x4).Magnitude_Squared(),
				   (x2 - x4).Magnitude_Squared(), (x3 - x4).Magnitude_Squared()));
	}

//#####################################################################
	bool Intersection (RAY_3D<T>& ray, const T thickness = 0) const;
	VECTOR_3D<T> Normal (const VECTOR_3D<T>& location, const int aggregate = 0) const;
	bool Inside (const VECTOR_3D<T>& location, const T thickness_over_two = 0) const;
	bool Outside (const VECTOR_3D<T>& location, const T thickness_over_two = 0) const;
	bool Boundary (const VECTOR_3D<T>& location, const T thickness_over_two = 0) const;
	VECTOR_3D<T> Surface (const VECTOR_3D<T>& location) const;
	T Volume() const;
	T Signed_Volume() const;
	T Minimum_Angle() const;
	T Maximum_Angle() const;
	T Minimum_Altitude() const;
	T Minimum_Edge_Length() const;
	T Maximum_Edge_Length() const;
	T Aspect_Ratio() const;  // largest_edge/smallest_altitude
	T Minimum_Dihedral_Angle() const;
	T Maximum_Dihedral_Angle() const;
	static T Signed_Reciprocal_Aspect_Ratio (const VECTOR_3D<T>& x1, const VECTOR_3D<T>& x2, const VECTOR_3D<T>& x3, const VECTOR_3D<T>& x4);
//#####################################################################
};
}
#endif

