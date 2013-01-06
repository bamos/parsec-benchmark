//#####################################################################
// Copyright 2002-2003, Ronald Fedkiw.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class RIGID_BODY_BOUNDING_VOLUMES
//#####################################################################
#ifndef __RIGID_BODY_BOUNDING_VOLUMES__
#define __RIGID_BODY_BOUNDING_VOLUMES__

#include "../Arrays/ARRAY.h"
#include "../Matrices_And_Vectors/VECTOR_3D.h"
#include "../Geometry/BOX_3D.h"
#include "../Rigid_Bodies/RIGID_BODY_3D.h"
namespace PhysBAM
{
template<class T> class RIGID_BODY_3D;

template<class T>
class RIGID_BODY_BOUNDING_VOLUMES
{
public:
	LIST_ARRAY<RIGID_BODY_3D<T>*>& rigid_bodies;
	ARRAY<T> rigid_body_radius;
	ARRAY<VECTOR_3D<T> > rigid_body_box_size;
	bool use_bounding_boxes, use_bounding_spheres;

	RIGID_BODY_BOUNDING_VOLUMES (LIST_ARRAY<RIGID_BODY_3D<T>*>& rigid_bodies)
		: rigid_bodies (rigid_bodies)
	{
		Use_Bounding_Boxes();
		Use_Bounding_Spheres();
		Initialize_Data_Structures();
	}

	void Initialize_Data_Structures()
	{
		rigid_body_radius.Resize_Array (rigid_bodies.m);
		rigid_body_box_size.Resize_Array (rigid_bodies.m);

		for (int i = 1; i <= rigid_bodies.m; i++)
		{
			BOX_3D<T>& box = *rigid_bodies (i)->triangulated_surface->bounding_box;
			VECTOR_3D<T> radius (maxabs (box.xmin, box.xmax), maxabs (box.ymin, box.ymax), maxabs (box.zmin, box.zmax));
			rigid_body_radius (i) = radius.Magnitude();
			rigid_body_box_size (i) = box.Size();
		}
	}

	void Use_Bounding_Boxes (bool use_bounding_boxes_input = true)
	{
		use_bounding_boxes = use_bounding_boxes_input;
	}

	void Use_Bounding_Spheres (bool use_bounding_spheres_input = true)
	{
		use_bounding_spheres = use_bounding_spheres_input;
	}

	bool Bounding_Volumes_Intersect (const int index_1, const int index_2) const
	{
		if (use_bounding_spheres && !Bounding_Spheres_Intersect (index_1, index_2)) return false;
		else if (use_bounding_boxes && !Bounding_Boxes_Intersect (index_1, index_2)) return false;
		else return true;
	}

	bool Bounding_Spheres_Intersect (const int index_1, const int index_2) const
	{
		return ( (rigid_bodies (index_1)->position - rigid_bodies (index_2)->position).Magnitude_Squared() <=
			 sqr (rigid_body_radius (index_1) + rigid_body_radius (index_2)));
	}

	bool Bounding_Boxes_Intersect (const int index_1, const int index_2) const
	{
		return rigid_bodies (index_1)->Oriented_Bounding_Box().Intersection (rigid_bodies (index_2)->Oriented_Bounding_Box());
	}

//#####################################################################
};
}
#endif
