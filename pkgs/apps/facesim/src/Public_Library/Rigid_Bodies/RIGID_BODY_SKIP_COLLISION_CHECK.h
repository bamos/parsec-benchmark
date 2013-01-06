//#####################################################################
// Copyright 2004, Eran Guendelman.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class RIGID_BODY_SKIP_COLLISION_CHECK
//#####################################################################
#ifndef __RIGID_BODY_SKIP_COLLISION_CHECK__
#define __RIGID_BODY_SKIP_COLLISION_CHECK__

#include "../Arrays/ARRAY.h"
#include "../Arrays/ARRAYS_2D.h"
namespace PhysBAM
{

class RIGID_BODY_SKIP_COLLISION_CHECK
{
private:
	int skip_counter;
	ARRAYS_2D<unsigned int> pair_last_checked;
	ARRAY<unsigned int> rigid_body_last_moved;

public:
	RIGID_BODY_SKIP_COLLISION_CHECK (const int number_of_rigid_bodies = 0)
	{
		Initialize (number_of_rigid_bodies);
	}

	void Reset()
	{
		skip_counter = 1;
		ARRAYS_2D<unsigned int>::copy (0, pair_last_checked);
		ARRAY<unsigned int>::copy (0, rigid_body_last_moved);
	}

	void Initialize (const int number_of_rigid_bodies)
	{
		pair_last_checked.Resize_Array (1, number_of_rigid_bodies, 1, number_of_rigid_bodies, false, false);
		rigid_body_last_moved.Resize_Array (number_of_rigid_bodies, false, false);
		Reset();
	}

	void Increment_Counter()
	{
		skip_counter++;
	}

	void Set_Last_Checked (const int index_1, const int index_2)
	{
		pair_last_checked (index_1, index_2) = pair_last_checked (index_2, index_1) = skip_counter;
	}

	void Set_Last_Moved (const int index)
	{
		rigid_body_last_moved (index) = skip_counter;
	}

	bool Skip_Pair (const int index_1, const int index_2) const
	{
		return pair_last_checked (index_1, index_2) > rigid_body_last_moved (index_1) && pair_last_checked (index_1, index_2) > rigid_body_last_moved (index_2);
	}

//#####################################################################
};
}
#endif
