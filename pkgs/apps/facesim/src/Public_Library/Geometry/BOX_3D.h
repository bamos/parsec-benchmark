//#####################################################################
// Copyright 2002-2004, Robert Bridson, Ronald Fedkiw, Eran Guendelman, Geoffrey Irving, Neil Molino, Igor Neverov.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class BOX_3D
//#####################################################################
#ifndef __BOX_3D__
#define __BOX_3D__

#include <cfloat>
#include "../Matrices_And_Vectors/VECTOR_3D.h"
#include "../Math_Tools/min.h"
#include "../Math_Tools/max.h"
#include "../Math_Tools/clamp.h"
namespace PhysBAM
{

template<class T> class GRID_3D;
template<class T> class RAY_3D;
template<class T> class ARRAYS_3D;

template<class T>
class BOX_3D
{
public:
	T xmin, xmax, ymin, ymax, zmin, zmax;

	BOX_3D()
		: xmin (-1), xmax (1), ymin (-1), ymax (1), zmin (-1), zmax (1)
	{}

	BOX_3D (const T xmin_input, const T xmax_input, const T ymin_input, const T ymax_input, const T zmin_input, const T zmax_input)
		: xmin (xmin_input), xmax (xmax_input), ymin (ymin_input), ymax (ymax_input), zmin (zmin_input), zmax (zmax_input)
	{}

	BOX_3D (const VECTOR_3D<T>& minimum_corner, const VECTOR_3D<T>& maximum_corner)
		: xmin (minimum_corner.x), xmax (maximum_corner.x), ymin (minimum_corner.y), ymax (maximum_corner.y), zmin (minimum_corner.z), zmax (maximum_corner.z)
	{}

	template<class T2> BOX_3D (const BOX_3D<T2>& box)
		: xmin ( (T) box.xmin), xmax ( (T) box.xmax), ymin ( (T) box.ymin), ymax ( (T) box.ymax), zmin ( (T) box.zmin), zmax ( (T) box.zmax)
	{}

	BOX_3D (const VECTOR_3D<T>& point)
	{
		Reset_Bounds (point);
	}

	bool operator== (const BOX_3D<T>& b) const
	{
		return xmin == b.xmin && xmax == b.xmax && ymin == b.ymin && ymax == b.ymax && zmin == b.zmin && zmax == b.zmax;
	}

	bool operator!= (const BOX_3D<T>& b) const
	{
		return ! (*this == b);
	}

	static BOX_3D<T> Empty_Box()
	{
		return BOX_3D<T> (FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX);
	}

	VECTOR_3D<T> Size() const
	{
		return VECTOR_3D<T> (xmax - xmin, ymax - ymin, zmax - zmin);
	}

	VECTOR_3D<T> Center() const
	{
		return VECTOR_3D<T> ( (T).5 * (xmin + xmax), (T).5 * (ymin + ymax), (T).5 * (zmin + zmax));
	}

	VECTOR_3D<T> Minimum_Corner() const
	{
		return VECTOR_3D<T> (xmin, ymin, zmin);
	}

	VECTOR_3D<T> Maximum_Corner() const
	{
		return VECTOR_3D<T> (xmax, ymax, zmax);
	}

	T Volume() const
	{
		return (xmax - xmin) * (ymax - ymin) * (zmax - zmin);
	}

	void Reset_Bounds (const BOX_3D<T>& box)
	{
		xmin = box.xmin;
		xmax = box.xmax;
		ymin = box.ymin;
		ymax = box.ymax;
		zmin = box.zmin;
		zmax = box.zmax;
	}

	void Reset_Bounds (const VECTOR_3D<T>& point)
	{
		xmin = xmax = point.x;
		ymin = ymax = point.y;
		zmin = zmax = point.z;
	}

	void Enlarge_To_Include_Point (const VECTOR_3D<T>& point)
	{
		if (point.x < xmin) xmin = point.x;
		else if (point.x > xmax) xmax = point.x;

		if (point.y < ymin) ymin = point.y;
		else if (point.y > ymax) ymax = point.y;

		if (point.z < zmin) zmin = point.z;
		else if (point.z > zmax) zmax = point.z;
	}

	void Enlarge_To_Include_Box (const BOX_3D<T>& box)
	{
		xmin = min (xmin, box.xmin);
		xmax = max (xmax, box.xmax);
		ymin = min (ymin, box.ymin);
		ymax = max (ymax, box.ymax);
		zmin = min (zmin, box.zmin);
		zmax = max (zmax, box.zmax);
	}

	void Change_Size (const T delta)
	{
		xmin -= delta;
		xmax += delta;
		ymin -= delta;
		ymax += delta;
		zmin -= delta;
		zmax += delta;
	}

	BOX_3D<T> Thickened (const T thickness_over_2) const
	{
		return BOX_3D<T> (xmin - thickness_over_2, xmax + thickness_over_2, ymin - thickness_over_2, ymax + thickness_over_2, zmin - thickness_over_2, zmax + thickness_over_2);
	}

	void Scale_About_Center (const T factor)
	{
		T x_center = (T).5 * (xmin + xmax), y_center = (T).5 * (ymin + ymax), z_center = (T).5 * (zmin + zmax), x_length_over_two = factor * (T).5 * (xmax - xmin), y_length_over_two = factor * (T).5 * (ymax - ymin),
		  z_length_over_two = factor * (T).5 * (zmax - zmin);
		xmin = x_center - x_length_over_two;
		xmax = x_center + x_length_over_two;
		ymin = y_center - y_length_over_two;
		ymax = y_center + y_length_over_two;
		zmin = z_center - z_length_over_two;
		zmax = z_center + z_length_over_two;
	}

	void Scale_About_Center (const T x_factor, const T y_factor, const T z_factor)
	{
		T x_center = (T).5 * (xmin + xmax), y_center = (T).5 * (ymin + ymax), z_center = (T).5 * (zmin + zmax), x_length_over_two = x_factor * (T).5 * (xmax - xmin), y_length_over_two = y_factor * (T).5 * (ymax - ymin),
		  z_length_over_two = z_factor * (T).5 * (zmax - zmin);
		xmin = x_center - x_length_over_two;
		xmax = x_center + x_length_over_two;
		ymin = y_center - y_length_over_two;
		ymax = y_center + y_length_over_two;
		zmin = z_center - z_length_over_two;
		zmax = z_center + z_length_over_two;
	}

	static BOX_3D<T> Combine (const BOX_3D<T>& box1, const BOX_3D<T>& box2)
	{
		return BOX_3D<T> (min (box1.xmin, box2.xmin), max (box1.xmax, box2.xmax), min (box1.ymin, box2.ymin), max (box1.ymax, box2.ymax), min (box1.zmin, box2.zmin), max (box1.zmax, box2.zmax));
	}

	bool Inside (const VECTOR_3D<T>& location, const T thickness_over_2 = 0) const
	{
		return location.x >= xmin + thickness_over_2 && location.x <= xmax - thickness_over_2 && location.y >= ymin + thickness_over_2 && location.y <= ymax - thickness_over_2 &&
		       location.z >= zmin + thickness_over_2 && location.z <= zmax - thickness_over_2;
	}

	int Lazy_Inside (const VECTOR_3D<T>& location) const
	{
		return location.x >= xmin && location.x <= xmax && location.y >= ymin && location.y <= ymax && location.z >= zmin && location.z <= zmax;
	}

	bool Outside (const VECTOR_3D<T>& location, const T thickness_over_2 = 0) const
	{
		return location.x <= xmin - thickness_over_2 || location.x >= xmax + thickness_over_2 || location.y <= ymin - thickness_over_2 || location.y >= ymax + thickness_over_2 ||
		       location.z <= zmin - thickness_over_2 || location.z >= zmax + thickness_over_2;
	}

	int Lazy_Outside (const VECTOR_3D<T>& location) const
	{
		return location.x <= xmin || location.x >= xmax || location.y <= ymin || location.y >= ymax || location.z <= zmin || location.z >= zmax;
	}

	bool Outside (const RAY_3D<T>& ray, const T thickness_over_2 = 0) const
	{
		return Thickened (thickness_over_2).Lazy_Outside (ray);
	}

	bool Boundary (const VECTOR_3D<T>& location, const T thickness_over_2 = 0) const
	{
		return !Inside (location, thickness_over_2) && !Outside (location, thickness_over_2);
	}

	VECTOR_3D<T> Clamp_Location_To_Box (const VECTOR_3D<T>& location) const
	{
		return VECTOR_3D<T> (clamp (location.x, xmin, xmax), clamp (location.y, ymin, ymax), clamp (location.z, zmin, zmax));
	}

	void Project_Points_Onto_Line (const VECTOR_3D<T>& direction, T& line_min, T& line_max) const
	{
		line_min = line_max = direction.x * xmin + direction.y * ymin + direction.z * zmin;
		T e1 = direction.x * (xmax - xmin), e2 = direction.y * (ymax - ymin), e3 = direction.z * (zmax - zmin);

		if (e1 > 0) line_max += e1;
		else line_min += e1;

		if (e2 > 0) line_max += e2;
		else line_min += e2;

		if (e3 > 0) line_max += e3;
		else line_min += e3;
	}

	VECTOR_3D<T> Point_From_Normalized_Coordinates (const VECTOR_3D<T>& weights) const
	{
		return VECTOR_3D<T> (xmin + weights.x * (xmax - xmin), ymin + weights.y * (ymax - ymin), zmin + weights.z * (zmax - zmin));
	}

	bool Intersection (const BOX_3D<T>& box, const T thickness_over_2 = 0) const
	{
		return ! (xmin > box.xmax + thickness_over_2 || xmax < box.xmin - thickness_over_2 || ymin > box.ymax + thickness_over_2 || ymax < box.ymin - thickness_over_2 ||
			  zmin > box.zmax + thickness_over_2 || zmax < box.zmin - thickness_over_2);
	}

	bool Lazy_Intersection (const BOX_3D<T>& box) const
	{
		return ! (xmin > box.xmax || xmax < box.xmin || ymin > box.ymax || ymax < box.ymin || zmin > box.zmax || zmax < box.zmin);
	}

	T Signed_Distance (const VECTOR_3D<T>& location) const // TODO: make more efficient version
	{
		T sign = 1;

		if (Lazy_Inside (location)) sign = -1;

		return sign * (location - Surface (location)).Magnitude();
	}

	template<class RW>
	void Read (std::istream& input_stream)
	{
		Read_Binary<RW> (input_stream, xmin, xmax, ymin, ymax, zmin, zmax);
	}

	template<class RW>
	void Write (std::ostream& output_stream) const
	{
		Write_Binary<RW> (output_stream, xmin, xmax, ymin, ymax, zmin, zmax);
	}

//#####################################################################
	bool Lazy_Intersection (RAY_3D<T>& ray, const T box_enlargement = 0) const;
	bool Intersection (RAY_3D<T>& ray, const T thickness_over_2 = 0) const;
	bool Get_Intersection_Range (const RAY_3D<T>& ray, T& start_t, T& end_t) const;
	bool Lazy_Outside (const RAY_3D<T>& ray) const;
	VECTOR_3D<T> Normal (const int aggregate) const;
	VECTOR_3D<T> Surface (const VECTOR_3D<T>& location) const;
	void Calculate_Signed_Distance_Function (const GRID_3D<T>& grid, ARRAYS_3D<T>& phi) const;
//#####################################################################
};
template<class T>
inline std::ostream& operator<< (std::ostream& output_stream, const BOX_3D<T>& box)
{
	output_stream << "(" << box.xmin << "," << box.xmax << ") x (" << box.ymin << "," << box.ymax << ") x (" << box.zmin << "," << box.zmax << ")";
	return output_stream;
}

template<class T>
inline std::istream& operator>> (std::istream& input_stream, BOX_3D<T>& box)
{
	input_stream >> box.xmin >> box.xmax >> box.ymin >> box.ymax >> box.zmin >> box.zmax;
	return input_stream;
}
}
#endif
