//#####################################################################
// Copyright 2002, 2003, Robert Bridson, Ronald Fedkiw.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class POLYGON
//#####################################################################
#ifndef __POLYGON__
#define __POLYGON__

#include "../Arrays/ARRAYS_1D.h"
#include "../Matrices_And_Vectors/VECTOR_3D.h"
#include "SEGMENT_3D.h"
namespace PhysBAM
{

template<class T>
class POLYGON
{
public:
	int closed_polygon; // (1) closed with first and last vertex connected, (0) open - one less side
	int number_of_vertices;
	ARRAYS_1D<T>* x; // x coordinates of vertices
	ARRAYS_1D<T>* y; // y coordinates of vertices

	POLYGON()
	{
		closed_polygon = 1;
		number_of_vertices = 0; // default is no polygon
		x = new ARRAYS_1D<T> (1, 1);
		y = new ARRAYS_1D<T> (1, 1);
	}

	POLYGON (const int number_of_vertices_input)
		: number_of_vertices (number_of_vertices_input)
	{
		closed_polygon = 1;
		x = new ARRAYS_1D<T> (1, number_of_vertices);
		y = new ARRAYS_1D<T> (1, number_of_vertices);
	}

	void Set_Closed_Polygon()
	{
		closed_polygon = 1;
	}

	void Set_Open_Polygon()
	{
		closed_polygon = 0;
	}

	void Set_Number_Of_Vertices (const int number_of_vertices_input)
	{
		number_of_vertices = number_of_vertices_input;
		delete x;
		delete y;
		x = new ARRAYS_1D<T> (1, number_of_vertices);
		y = new ARRAYS_1D<T> (1, number_of_vertices);
	}

	void Set_Vertex_Coordinates (const int vertex, const T x_input, const T y_input)
	{
		(*x) (vertex) = x_input;
		(*y) (vertex) = y_input;
	}

	~POLYGON()
	{
		delete x;
		delete y;
	}

//#####################################################################
	T Area();
	void Find_Closest_Point_On_Polygon (const T x, const T y, T& x_polygon, T& y_polygon, int& side);
	T Distance_From_Polygon_To_Point (const T x, const T y);
	bool Inside_Polygon (const T x, const T y);
//#####################################################################
};

//#####################################################################
// Function Area
//#####################################################################
// doesn't work if the polygon is self intersecting
template<class T> T POLYGON<T>::
Area()
{
	if (!closed_polygon) return 0;

	T area = 0;

	for (int k = 1; k <= number_of_vertices - 1; k++) area += ( (*y) (k) + (*y) (k + 1)) * ( (*x) (k + 1) - (*x) (k));

	area += ( (*y) (number_of_vertices) + (*y) (1)) * ( (*x) (1) - (*x) (number_of_vertices)); // last edge
	return fabs (area) / 2; // should have done ((*y)(k)+(*y)(k+1))/2 above
}
//#####################################################################
// Function Find_Closest_Point_On_Polygon
//#####################################################################
template<class T> void POLYGON<T>::
Find_Closest_Point_On_Polygon (const T x_point, const T y_point, T& x_polygon, T& y_polygon, int& side)
{
	VECTOR_3D<T> point (x_point, y_point, 0);

	// first side
	VECTOR_3D<T> v1 ( (*x) (1), (*y) (1), 0), v2 ( (*x) (2), (*y) (2), 0);
	SEGMENT_3D<T> segment (v1, v2);
	VECTOR_3D<T> closest = segment.Closest_Point_On_Segment (point), d = closest - point;
	T distance = d.Magnitude();
	x_polygon = closest.x;
	y_polygon = closest.y;
	side = 1;

	// all the other sides except for the last one
	for (int k = 2; k <= number_of_vertices - 1; k++)
	{
		VECTOR_3D<T> v1 ( (*x) (k), (*y) (k), 0), v2 ( (*x) (k + 1), (*y) (k + 1), 0);
		SEGMENT_3D<T> segment (v1, v2);
		VECTOR_3D<T> closest = segment.Closest_Point_On_Segment (point), d = closest - point;
		T distance_temp = d.Magnitude();

		if (distance_temp < distance)
		{
			distance = distance_temp;
			x_polygon = closest.x;
			y_polygon = closest.y;
			side = k;
		}
	}

	// last side - if the polygon is closed
	if (closed_polygon)
	{
		VECTOR_3D<T> v1 ( (*x) (number_of_vertices), (*y) (number_of_vertices), 0), v2 ( (*x) (1), (*y) (1), 0);
		SEGMENT_3D<T> segment (v1, v2);
		VECTOR_3D<T> closest = segment.Closest_Point_On_Segment (point), d = closest - point;
		T distance_temp = d.Magnitude();

		if (distance_temp < distance)
		{
			x_polygon = closest.x;
			y_polygon = closest.y;
			side = number_of_vertices;
		}
	}
}
//#####################################################################
// Function Distance_From_Polygon_To_Point
//#####################################################################
template<class T> T POLYGON<T>::
Distance_From_Polygon_To_Point (const T x_point, const T y_point)
{
	T x_polygon = 0, y_polygon = 0;
	int side = 0;
	Find_Closest_Point_On_Polygon (x_point, y_point, x_polygon, y_polygon, side);
	return sqrt (sqr (x_point - x_polygon) + sqr (y_point - y_polygon));
}
//#####################################################################
// Function Inside_Polygon
//#####################################################################
template<class T> bool POLYGON<T>::
Inside_Polygon (const T x_point, const T y_point)
{
	T theta_total = 0;

	// all sides except for the last one
	for (int k = 1; k <= number_of_vertices - 1; k++)
	{
		T x1 = (*x) (k) - x_point, y1 = (*y) (k) - y_point, x2 = (*x) (k + 1) - x_point, y2 = (*y) (k + 1) - y_point;

		if ( (x1 == 0 && y1 == 0) || (x2 == 0 && y2 == 0)) return true; // (x,y) lies on the polygon

		T theta1 = atan2 (y1, x1), theta2 = atan2 (y2, x2); // atan2 returns values between -pi and pi, if (x,y) != (0,0)
		T theta = theta2 - theta1;

		if (theta == pi || theta == -pi) return true; // (x,y) lies on the polygon

		if (theta > pi) theta -= 2 * pi;     // make sure the smaller angle is swept out
		else if (theta < -pi) theta += 2 * pi; // make sure the smaller angle is swept out

		theta_total += theta;
	}

	// last side
	T x1 = (*x) (number_of_vertices) - x_point, y1 = (*y) (number_of_vertices) - y_point, x2 = (*x) (1) - x_point, y2 = (*y) (1) - y_point;

	if ( (x1 == 0 && y1 == 0) || (x2 == 0 && y2 == 0)) return true; // (x,y) lies on the polygon

	T theta1 = atan2 (y1, x1), theta2 = atan2 (y2, x2); // atan2 returns values between -pi and pi, if (x,y) != (0,0)
	T theta = theta2 - theta1;

	if (theta == pi || theta == -pi) return true; // (x,y) lies on the polygon

	if (theta > pi) theta -= 2 * pi;     // make sure the smaller angle is swept out
	else if (theta < -pi) theta += 2 * pi; // make sure the smaller angle is swept out

	theta_total += theta;

	// decide on inside or outside
	if (fabs (theta_total) >= pi)  return true; // theta_total = +2*pi or -2*pi for a point inside the polygon
	else return false;                          // theta_total = 0 for a point outside the polygon
}
//#####################################################################
}
#endif

