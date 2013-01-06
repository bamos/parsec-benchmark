//#####################################################################
// Copyright 2003, Eran Guendelman.
// This file is part of PhysBAM whose distribution is governed by the license
// contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class ORIENTED_BOX_2D
//#####################################################################
#ifndef __ORIENTED_BOX_2D__
#define __ORIENTED_BOX_2D__

#include "../Matrices_And_Vectors/MATRIX_2X2.h"
#include "../Matrices_And_Vectors/VECTOR_2D.h"
#include "BOX_2D.h"
#include "RAY_2D.h"
#include "SEGMENT_2D.h"
namespace PhysBAM
{

template<class T> class BOX_2D;

template<class T>
class ORIENTED_BOX_2D
{
public:
	VECTOR_2D<T> corner; // root corner of the box
	VECTOR_2D<T> edge1, edge2; // principle edges of the box, emanating from the corner

	ORIENTED_BOX_2D()
		: corner (0, 0), edge1 (1, 0), edge2 (0, 1)
	{}

	ORIENTED_BOX_2D (const VECTOR_2D<T>& corner_input, const VECTOR_2D<T>& edge1_input, const VECTOR_2D<T>& edge2_input)
		: corner (corner_input), edge1 (edge1_input), edge2 (edge2_input)
	{}

	ORIENTED_BOX_2D (const BOX_2D<T>& box, const T rotation_angle)
	{
		MATRIX_2X2<T> mat = MATRIX_2X2<T>::Rotation_Matrix (rotation_angle);
		corner = mat * box.Minimum_Corner();
		edge1 = (box.xmax - box.xmin) * mat.Column (1);
		edge2 = (box.ymax - box.ymin) * mat.Column (2);
	}

	ORIENTED_BOX_2D (const BOX_2D<T>& box, const T rotation_angle, const VECTOR_2D<T>& corner_input)
		: corner (corner_input)
	{
		MATRIX_2X2<T> mat = MATRIX_2X2<T>::Rotation_Matrix (rotation_angle);
		edge1 = (box.xmax - box.xmin) * mat.Column (1);
		edge2 = (box.ymax - box.ymin) * mat.Column (2);
	}

	BOX_2D<T> Axis_Aligned_Bounding_Box() const
	{
		T xmin = corner.x, xmax = corner.x, ymin = corner.y, ymax = corner.y;

		if (edge1.x > 0) xmax += edge1.x;
		else xmin += edge1.x;

		if (edge2.x > 0) xmax += edge2.x;
		else xmin += edge2.x;

		if (edge1.y > 0) ymax += edge1.y;
		else ymin += edge1.y;

		if (edge2.y > 0) ymax += edge2.y;
		else ymin += edge2.y;

		return BOX_2D<T> (xmin, xmax, ymin, ymax);
	}

	bool Lazy_Inside (const VECTOR_2D<T> &location) const
	{
		VECTOR_2D<T> vec = location - corner;
		T edge1_projection = VECTOR_2D<T>::Dot_Product (vec, edge1);
		T edge2_projection = VECTOR_2D<T>::Dot_Product (vec, edge2);
		return (0 <= edge1_projection && edge1_projection <= edge1.Magnitude_Squared() &&
			0 <= edge2_projection && edge2_projection <= edge2.Magnitude_Squared());
	}

	bool Intersection (const ORIENTED_BOX_2D<T>& box) const
	{
		if (Separating_Line_Test (box, edge1)) return false;

		if (Separating_Line_Test (box, edge2)) return false;

		if (Separating_Line_Test (box, box.edge1)) return false;

		if (Separating_Line_Test (box, box.edge2)) return false;

		return true;
	} // otherwise

	bool Intersection (const BOX_2D<T>& box) const
	{
		T line_min = corner.x, line_max = corner.x;

		if (edge1.x > 0) line_max += edge1.x;
		else line_min += edge1.x;

		if (edge2.x > 0) line_max += edge2.x;
		else line_min += edge2.x;

		if (line_max < box.xmin || line_min > box.xmax) return false;

		line_min = line_max = corner.y;

		if (edge1.y > 0) line_max += edge1.y;
		else line_min += edge1.y;

		if (edge2.y > 0) line_max += edge2.y;
		else line_min += edge2.y;

		if (line_max < box.ymin || line_min > box.ymax) return false;

		if (Separating_Line_Test (box, edge1)) return false;

		if (Separating_Line_Test (box, edge2)) return false;

		return true;
	} // otherwise

	bool Intersection (RAY_2D<T>& ray, T segment_intersection_epsilon) const // done like this to prevent short circuit errors
	{
		bool test1 = SEGMENT_2D<T> (corner, corner + edge1).Intersection (ray, segment_intersection_epsilon),
		     test2 = SEGMENT_2D<T> (corner, corner + edge2).Intersection (ray, segment_intersection_epsilon),
		     test3 = SEGMENT_2D<T> (corner + edge2, corner + edge1 + edge2).Intersection (ray, segment_intersection_epsilon),
		     test4 = SEGMENT_2D<T> (corner + edge1, corner + edge1 + edge2).Intersection (ray, segment_intersection_epsilon);
		return (test1 || test2 || test3 || test4);
	}

	bool Fuzzy_Intersection (RAY_2D<T>& ray, T segment_intersection_epsilon) const // done like this to prevent short circuit errors
	{
		bool test1 = SEGMENT_2D<T> (corner, corner + edge1).Fuzzy_Intersection (ray, segment_intersection_epsilon),
		     test2 = SEGMENT_2D<T> (corner, corner + edge2).Fuzzy_Intersection (ray, segment_intersection_epsilon),
		     test3 = SEGMENT_2D<T> (corner + edge2, corner + edge1 + edge2).Fuzzy_Intersection (ray, segment_intersection_epsilon),
		     test4 = SEGMENT_2D<T> (corner + edge1, corner + edge1 + edge2).Fuzzy_Intersection (ray, segment_intersection_epsilon);
		return (test1 || test2 || test3 || test4);
	}

	bool Separating_Line_Test (const ORIENTED_BOX_2D<T>& box, const VECTOR_2D<T>& line_direction) const
	{
		T min1, max1;
		Project_Points_Onto_Line (line_direction, min1, max1);
		T min2, max2;
		box.Project_Points_Onto_Line (line_direction, min2, max2);

		if (max2 < min1 || min2 > max1) return true;
		else return false;
	}

	bool Separating_Line_Test (const BOX_2D<T>& box, const VECTOR_2D<T>& line_direction) const
	{
		T min1, max1;
		Project_Points_Onto_Line (line_direction, min1, max1);
		T min2, max2;
		box.Project_Points_Onto_Line (line_direction, min2, max2);

		if (max2 < min1 || min2 > max1) return true;
		else return false;
	}

	void Project_Points_Onto_Line (const VECTOR_2D<T>& direction, T& line_min, T& line_max) const
	{
		line_min = line_max = VECTOR_2D<T>::Dot_Product (direction, corner);
		T e1 = VECTOR_2D<T>::Dot_Product (direction, edge1), e2 = VECTOR_2D<T>::Dot_Product (direction, edge2);

		if (e1 > 0) line_max += e1;
		else line_min += e1;

		if (e2 > 0) line_max += e2;
		else line_min += e2;
	}

//#####################################################################
//#####################################################################
};
}
#endif

