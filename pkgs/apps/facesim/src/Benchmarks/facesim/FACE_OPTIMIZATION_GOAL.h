//#####################################################################
// Copyright 2004-2005, Igor Neverov, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class FACE_OPTIMIZATION_GOAL
//#####################################################################
#ifndef __FACE_OPTIMIZATION_GOAL__
#define __FACE_OPTIMIZATION_GOAL__

#include <iostream>

namespace PhysBAM
{
template <class T>
class FACE_OPTIMIZATION_GOAL
{
public:

	FACE_OPTIMIZATION_GOAL()
	{}

	virtual ~FACE_OPTIMIZATION_GOAL()
	{}

	void Default() const
	{
		std::cout << "THIS FACE_OPTIMIZATION_GOAL FUNCTION IS NOT DEFINED!" << std::endl;
	}

//#####################################################################
	virtual void Update_Target (const int frame)
	{
		Default();
		exit (1);
	}
	virtual int Last_Frame() const
	{
		Default();
		exit (1);
	}
	virtual void Write_Goal_Data (const std::string& output_prefix, const int frame_input) const
	{
		Default();
		exit (1);
	}
//#####################################################################
};
}
#endif
