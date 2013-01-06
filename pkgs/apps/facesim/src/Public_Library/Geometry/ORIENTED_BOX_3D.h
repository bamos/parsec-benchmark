//#####################################################################
// Copyright 2002,2003,2004, Ronald Fedkiw, Eran Guendelman.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class ORIENTED_BOX_3D
//#####################################################################
#ifndef __ORIENTED_BOX_3D__
#define __ORIENTED_BOX_3D__

#include "../Matrices_And_Vectors/VECTOR_3D.h"
#include "../Matrices_And_Vectors/QUATERNION.h"
#include "BOX_3D.h"
namespace PhysBAM
{

template<class T>
class ORIENTED_BOX_3D
{
public:
	VECTOR_3D<T> corner; // root corner of the box
	VECTOR_3D<T> edge1, edge2, edge3; // principle edges of the box, emanating from the corner

	ORIENTED_BOX_3D()
		: corner (0, 0, 0), edge1 (1, 0, 0), edge2 (0, 1, 0), edge3 (0, 0, 1)
	{}

	ORIENTED_BOX_3D (const VECTOR_3D<T>& corner_input, const VECTOR_3D<T>& edge1_input, const VECTOR_3D<T>& edge2_input, const VECTOR_3D<T>& edge3_input)
		: corner (corner_input), edge1 (edge1_input), edge2 (edge2_input), edge3 (edge3_input)
	{}

	ORIENTED_BOX_3D (const BOX_3D<T>& box, const QUATERNION<T>& rotation)
	{
		rotation.Get_Rotated_Frame (edge1, edge2, edge3);
		corner = box.xmin * edge1 + box.ymin * edge2 + box.zmin * edge3; // same as rotation.Rotate(box.Minimum_Corner())
		edge1 *= (box.xmax - box.xmin);
		edge2 *= (box.ymax - box.ymin);
		edge3 *= (box.zmax - box.zmin);
	}

	ORIENTED_BOX_3D (const BOX_3D<T>& box, const MATRIX_4X4<T>& transform)
	{
		corner = transform * box.Minimum_Corner();
		MATRIX_3X3<T> rotation_only = transform.Upper_3X3();
		edge1 = (box.xmax - box.xmin) * rotation_only.Column (1);
		edge2 = (box.ymax - box.ymin) * rotation_only.Column (2);
		edge3 = (box.zmax - box.zmin) * rotation_only.Column (3);
	}

	ORIENTED_BOX_3D (const BOX_3D<T>& box, const QUATERNION<T>& rotation, const VECTOR_3D<T>& corner_input)
		: corner (corner_input)
	{
		rotation.Get_Rotated_Frame (edge1, edge2, edge3);
		edge1 *= (box.xmax - box.xmin);
		edge2 *= (box.ymax - box.ymin);
		edge3 *= (box.zmax - box.zmin);
	}

	BOX_3D<T> Axis_Aligned_Bounding_Box() const
	{
		T xmin = corner.x, xmax = corner.x, ymin = corner.y, ymax = corner.y, zmin = corner.z, zmax = corner.z;

		if (edge1.x > 0) xmax += edge1.x;
		else xmin += edge1.x;

		if (edge2.x > 0) xmax += edge2.x;
		else xmin += edge2.x;

		if (edge3.x > 0) xmax += edge3.x;
		else xmin += edge3.x;

		if (edge1.y > 0) ymax += edge1.y;
		else ymin += edge1.y;

		if (edge2.y > 0) ymax += edge2.y;
		else ymin += edge2.y;

		if (edge3.y > 0) ymax += edge3.y;
		else ymin += edge3.y;

		if (edge1.z > 0) zmax += edge1.z;
		else zmin += edge1.z;

		if (edge2.z > 0) zmax += edge2.z;
		else zmin += edge2.z;

		if (edge3.z > 0) zmax += edge3.z;
		else zmin += edge3.z;

		return BOX_3D<T> (xmin, xmax, ymin, ymax, zmin, zmax);
	}

	bool Intersection (const ORIENTED_BOX_3D<T>& box) const
	{
		if (Separating_Plane_Test (box, edge1)) return false;

		if (Separating_Plane_Test (box, edge2)) return false;

		if (Separating_Plane_Test (box, edge3)) return false;

		if (Separating_Plane_Test (box, box.edge1)) return false;

		if (Separating_Plane_Test (box, box.edge2)) return false;

		if (Separating_Plane_Test (box, box.edge3)) return false;

		if (Separating_Plane_Test (box, VECTOR_3D<T>::Cross_Product (edge1, box.edge1))) return false;

		if (Separating_Plane_Test (box, VECTOR_3D<T>::Cross_Product (edge1, box.edge2))) return false;

		if (Separating_Plane_Test (box, VECTOR_3D<T>::Cross_Product (edge1, box.edge3))) return false;

		if (Separating_Plane_Test (box, VECTOR_3D<T>::Cross_Product (edge2, box.edge1))) return false;

		if (Separating_Plane_Test (box, VECTOR_3D<T>::Cross_Product (edge2, box.edge2))) return false;

		if (Separating_Plane_Test (box, VECTOR_3D<T>::Cross_Product (edge2, box.edge3))) return false;

		if (Separating_Plane_Test (box, VECTOR_3D<T>::Cross_Product (edge3, box.edge1))) return false;

		if (Separating_Plane_Test (box, VECTOR_3D<T>::Cross_Product (edge3, box.edge2))) return false;

		if (Separating_Plane_Test (box, VECTOR_3D<T>::Cross_Product (edge3, box.edge3))) return false;

		return true;
	} // otherwise

	bool Intersection (const BOX_3D<T>& box) const
	{
		T line_min = corner.x, line_max = corner.x;

		if (edge1.x > 0) line_max += edge1.x;
		else line_min += edge1.x;

		if (edge2.x > 0) line_max += edge2.x;
		else line_min += edge2.x;

		if (edge3.x > 0) line_max += edge3.x;
		else line_min += edge3.x;

		if (line_max < box.xmin || line_min > box.xmax) return false;

		line_min = line_max = corner.y;

		if (edge1.y > 0) line_max += edge1.y;
		else line_min += edge1.y;

		if (edge2.y > 0) line_max += edge2.y;
		else line_min += edge2.y;

		if (edge3.y > 0) line_max += edge3.y;
		else line_min += edge3.y;

		if (line_max < box.ymin || line_min > box.ymax) return false;

		line_min = line_max = corner.z;

		if (edge1.z > 0) line_max += edge1.z;
		else line_min += edge1.z;

		if (edge2.z > 0) line_max += edge2.z;
		else line_min += edge2.z;

		if (edge3.z > 0) line_max += edge3.z;
		else line_min += edge3.z;

		if (line_max < box.zmin || line_min > box.zmax) return false;

		if (Separating_Plane_Test (box, edge1)) return false;

		if (Separating_Plane_Test (box, edge2)) return false;

		if (Separating_Plane_Test (box, edge3)) return false;

		VECTOR_3D<T> box_edge1 (box.xmax - box.xmin, 0, 0);

		if (Separating_Plane_Test (box, VECTOR_3D<T>::Cross_Product (edge1, box_edge1))) return false;

		if (Separating_Plane_Test (box, VECTOR_3D<T>::Cross_Product (edge2, box_edge1))) return false;

		if (Separating_Plane_Test (box, VECTOR_3D<T>::Cross_Product (edge3, box_edge1))) return false;

		VECTOR_3D<T> box_edge2 (0, box.ymax - box.ymin, 0);

		if (Separating_Plane_Test (box, VECTOR_3D<T>::Cross_Product (edge1, box_edge2))) return false;

		if (Separating_Plane_Test (box, VECTOR_3D<T>::Cross_Product (edge2, box_edge2))) return false;

		if (Separating_Plane_Test (box, VECTOR_3D<T>::Cross_Product (edge3, box_edge2))) return false;

		VECTOR_3D<T> box_edge3 (0, 0, box.zmax - box.zmin);

		if (Separating_Plane_Test (box, VECTOR_3D<T>::Cross_Product (edge1, box_edge3))) return false;

		if (Separating_Plane_Test (box, VECTOR_3D<T>::Cross_Product (edge2, box_edge3))) return false;

		if (Separating_Plane_Test (box, VECTOR_3D<T>::Cross_Product (edge3, box_edge3))) return false;

		return true;
	} // otherwise

	bool Separating_Plane_Test (const ORIENTED_BOX_3D<T>& box, const VECTOR_3D<T>& plane_normal_direction) const
	{
		T min1, max1;
		Project_Points_Onto_Line (plane_normal_direction, min1, max1);
		T min2, max2;
		box.Project_Points_Onto_Line (plane_normal_direction, min2, max2);

		if (max2 < min1 || min2 > max1) return true;
		else return false;
	}

	bool Separating_Plane_Test (const BOX_3D<T>& box, const VECTOR_3D<T>& plane_normal_direction) const
	{
		T min1, max1;
		Project_Points_Onto_Line (plane_normal_direction, min1, max1);
		T min2, max2;
		box.Project_Points_Onto_Line (plane_normal_direction, min2, max2);

		if (max2 < min1 || min2 > max1) return true;
		else return false;
	}

	void Project_Points_Onto_Line (const VECTOR_3D<T>& direction, T& line_min, T& line_max) const
	{
		line_min = line_max = VECTOR_3D<T>::Dot_Product (direction, corner);
		T e1 = VECTOR_3D<T>::Dot_Product (direction, edge1), e2 = VECTOR_3D<T>::Dot_Product (direction, edge2), e3 = VECTOR_3D<T>::Dot_Product (direction, edge3);

		if (e1 > 0) line_max += e1;
		else line_min += e1;

		if (e2 > 0) line_max += e2;
		else line_min += e2;

		if (e3 > 0) line_max += e3;
		else line_min += e3;
	}

//#####################################################################
//#####################################################################
};
}
#endif

