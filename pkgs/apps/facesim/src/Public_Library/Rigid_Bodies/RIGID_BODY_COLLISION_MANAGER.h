//#####################################################################
// Copyright 2004, Ron Fedkiw, Eran Guendelman.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class RIGID_BODY_COLLISION_MANAGER
//#####################################################################
#ifndef __RIGID_BODY_COLLISION_MANAGER__
#define __RIGID_BODY_COLLISION_MANAGER__

namespace PhysBAM
{

class RIGID_BODY_COLLISION_MANAGER
{
public:
	ARRAYS_2D<bool>* collision_matrix; // indexed by rigid body id; collision_matrix(i,j)==true means body i collides with (is affected by) body j

	RIGID_BODY_COLLISION_MANAGER()
		: collision_matrix (0)
	{}

	~RIGID_BODY_COLLISION_MANAGER()
	{
		delete collision_matrix;
	}

	void Use_Collision_Matrix()
	{
		if (!collision_matrix) collision_matrix = new ARRAYS_2D<bool>();
	}

	void Resize_Collision_Matrix (const int number_of_rigid_bodies)
	{
		assert (collision_matrix);
		collision_matrix->Resize_Array (1, number_of_rigid_bodies, 1, number_of_rigid_bodies, true, true, true);
	}

	bool Either_Body_Collides_With_The_Other (const int rigid_body_id_1, const int rigid_body_id_2)
	{
		return Rigid_Body_Collides_With_Other_Rigid_Body (rigid_body_id_1, rigid_body_id_2) || Rigid_Body_Collides_With_Other_Rigid_Body (rigid_body_id_2, rigid_body_id_1);
	}

	bool Rigid_Body_Collides_With_Other_Rigid_Body (const int rigid_body_id_1, const int rigid_body_id_2) const
	{
		return !collision_matrix || !collision_matrix->Valid_Index (rigid_body_id_1, rigid_body_id_2) || (*collision_matrix) (rigid_body_id_1, rigid_body_id_2);
	}

	void Set_Rigid_Body_Collides_With_Other_Rigid_Body (const int rigid_body_id_1, const int rigid_body_id_2, const bool body_1_collides_with_body_2)
	{
		assert (collision_matrix);
		Resize_Collision_Matrix (max (rigid_body_id_1, rigid_body_id_2, collision_matrix->m));
		(*collision_matrix) (rigid_body_id_1, rigid_body_id_2) = body_1_collides_with_body_2;
	}

//#####################################################################
};
}
#endif
