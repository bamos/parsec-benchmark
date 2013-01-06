//#####################################################################
// Copyright 2002, 2003, 2004, Robert Bridson, Ronald Fedkiw, Eran Guendelman, Geoffrey Irving, Sergey Koltakov, Neil Molino, Duc Nguyen, Andrew Selle, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class BOX_2D
//#####################################################################
#ifndef __BOX_2D__
#define __BOX_2D__

#include "../Math_Tools/min.h"
#include "../Math_Tools/max.h"
#include "../Matrices_And_Vectors/VECTOR_2D.h"
#include "../Geometry/SEGMENT_2D.h"
#include "../Geometry/RAY_2D.h"
namespace PhysBAM
{

template<class T> class GRID_2D;
template<class T> class ARRAYS_2D;
template<class T> class RAY_2D;

template<class T>
class BOX_2D
{
public:
	T xmin, xmax, ymin, ymax;

	BOX_2D()
		: xmin (-1), xmax (1), ymin (-1), ymax (1)
	{}

	BOX_2D (const T xmin_input, const T xmax_input, const T ymin_input, const T ymax_input)
		: xmin (xmin_input), xmax (xmax_input), ymin (ymin_input), ymax (ymax_input)
	{}

	BOX_2D (const VECTOR_2D<T>& minimum_corner, const VECTOR_2D<T>& maximum_corner)
		: xmin (minimum_corner.x), xmax (maximum_corner.x), ymin (minimum_corner.y), ymax (maximum_corner.y)
	{}

	template<class T2> BOX_2D (const BOX_2D<T2>& box)
		: xmin ( (T) box.xmin), xmax ( (T) box.xmax), ymin ( (T) box.ymin), ymax ( (T) box.ymax)
	{}

	BOX_2D (const VECTOR_2D<T>& point)
	{
		Reset_Bounds (point);
	}

	bool operator== (const BOX_2D<T>& r) const
	{
		return xmin == r.xmin && xmax == r.xmax && ymin == r.ymin && ymax == r.ymax;
	}

	bool operator!= (const BOX_2D<T>& r) const
	{
		return ! (*this == r);
	}

	VECTOR_2D<T> Size() const
	{
		return VECTOR_2D<T> (xmax - xmin, ymax - ymin);
	}

	VECTOR_2D<T> Center() const
	{
		return VECTOR_2D<T> ( (T).5 * (xmin + xmax), (T).5 * (ymin + ymax));
	}

	VECTOR_2D<T> Minimum_Corner() const
	{
		return VECTOR_2D<T> (xmin, ymin);
	}

	VECTOR_2D<T> Maximum_Corner() const
	{
		return VECTOR_2D<T> (xmax, ymax);
	}

	T Area() const
	{
		return (xmax - xmin) * (ymax - ymin);
	}

	void Reset_Bounds (const BOX_2D<T>& box)
	{
		xmin = box.xmin;
		xmax = box.xmax;
		ymin = box.ymin;
		ymax = box.ymax;
	}

	void Reset_Bounds (const VECTOR_2D<T>& point)
	{
		xmin = xmax = point.x;
		ymin = ymax = point.y;
	}

	void Enlarge_To_Include_Point (const VECTOR_2D<T>& point)
	{
		if (point.x < xmin) xmin = point.x;
		else if (point.x > xmax) xmax = point.x;

		if (point.y < ymin) ymin = point.y;
		else if (point.y > ymax) ymax = point.y;
	}

	void Enlarge_To_Include_Box (const BOX_2D<T>& box)
	{
		xmin = min (xmin, box.xmin);
		xmax = max (xmax, box.xmax);
		ymin = min (ymin, box.ymin);
		ymax = max (ymax, box.ymax);
	}

	void Change_Size (const T delta)
	{
		xmin -= delta;
		xmax += delta;
		ymin -= delta;
		ymax += delta;
	}

	BOX_2D<T> Thickened (const T thickness_over_2) const
	{
		return BOX_2D<T> (xmin - thickness_over_2, xmax + thickness_over_2, ymin - thickness_over_2, ymax + thickness_over_2);
	}

	void Scale_About_Center (const T factor)
	{
		T x_center = (T).5 * (xmin + xmax), y_center = (T).5 * (ymin + ymax), x_length_over_two = factor * (T).5 * (xmax - xmin), y_length_over_two = factor * (T).5 * (ymax - ymin);
		xmin = x_center - x_length_over_two;
		xmax = x_center + x_length_over_two;
		ymin = y_center - y_length_over_two;
		ymax = y_center + y_length_over_two;
	}

	static BOX_2D<T> Combine (const BOX_2D<T>& box1, const BOX_2D<T>& box2)
	{
		return BOX_2D<T> (min (box1.xmin, box2.xmin), max (box1.xmax, box2.xmax), min (box1.ymin, box2.ymin), max (box1.ymax, box2.ymax));
	}

	bool Inside (const VECTOR_2D<T>& location, const T thickness_over_two) const
	{
		return (location.x >= xmin + thickness_over_two && location.x <= xmax - thickness_over_two && location.y >= ymin + thickness_over_two && location.y <= ymax - thickness_over_two);
	}

	bool Lazy_Inside (const VECTOR_2D<T>& location) const
	{
		return (location.x >= xmin && location.x <= xmax && location.y >= ymin && location.y <= ymax);
	}

	bool Outside (const VECTOR_2D<T>& location, const T thickness_over_two) const
	{
		return (location.x < xmin - thickness_over_two || location.x > xmax + thickness_over_two || location.y < ymin - thickness_over_two || location.y >= ymax + thickness_over_two);
	}

	bool Lazy_Outside (const VECTOR_2D<T>& location) const
	{
		return location.x <= xmin || location.x >= xmax || location.y <= ymin || location.y >= ymax;
	}

	bool Boundary (const VECTOR_2D<T>& location, const T thickness_over_two) const
	{
		return !Inside (location, thickness_over_two) && !Outside (location, thickness_over_two);
	}

	VECTOR_2D<T> Clamp_Location_To_Box (const VECTOR_2D<T>& location) const
	{
		return VECTOR_2D<T> (clamp (location.x, xmin, xmax), clamp (location.y, ymin, ymax));
	}

	void Project_Points_Onto_Line (const VECTOR_2D<T>& direction, T& line_min, T& line_max) const
	{
		line_min = line_max = direction.x * xmin + direction.y * ymin;
		T e1 = direction.x * (xmax - xmin), e2 = direction.y * (ymax - ymin);

		if (e1 > 0) line_max += e1;
		else line_min += e1;

		if (e2 > 0) line_max += e2;
		else line_min += e2;
	}

	VECTOR_2D<T> Point_From_Normalized_Coordinates (const VECTOR_2D<T>& weights) const
	{
		return VECTOR_2D<T> (xmin + weights.x * (xmax - xmin), ymin + weights.y * (ymax - ymin));
	}

	bool Intersection (const BOX_2D<T>& box, const T thickness_over_two = 0) const
	{
		return ! (xmin > box.xmax + thickness_over_two || xmax < box.xmin - thickness_over_two || ymin > box.ymax + thickness_over_two || ymax < box.ymin - thickness_over_two);
	}

	bool Lazy_Intersection (const BOX_2D<T>& box) const
	{
		return ! (xmin > box.xmax || xmax < box.xmin || ymin > box.ymax || ymax < box.ymin);
	}

	T Signed_Distance (const VECTOR_2D<T>& location) const // TODO: make more efficient version
	{
		T sign = 1;

		if (Lazy_Inside (location)) sign = -1;

		return sign * (location - Surface (location)).Magnitude();
	}

	template<class RW>
	void Read (std::istream& input_stream)
	{
		Read_Binary<RW> (input_stream, xmin, xmax, ymin, ymax);
	}

	template<class RW>
	void Write (std::ostream& output_stream) const
	{
		Write_Binary<RW> (output_stream, xmin, xmax, ymin, ymax);
	}

//#####################################################################
	bool Intersection (RAY_2D<T>& ray, const T thickness_over_two = 0, const T segment_intersect_epsilon = 0) const;
	VECTOR_2D<T> Normal (const int aggregate) const;
	VECTOR_2D<T> Surface (const VECTOR_2D<T>& location) const;
	void Calculate_Signed_Distance_Function (const GRID_2D<T>& grid, ARRAYS_2D<T>& phi) const;
//#####################################################################
};

// global functions
template<class T>
inline std::ostream& operator<< (std::ostream& output_stream, const BOX_2D<T>& rect)
{
	output_stream << "(" << rect.xmin << "," << rect.xmax << ") x (" << rect.ymin << "," << rect.ymax << ")";
	return output_stream;
}

}
#endif
