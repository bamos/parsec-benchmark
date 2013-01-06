//#####################################################################
// Copyright 2004, Ron Fedkiw.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class DEFORMABLE_BODY_PARAMETERS
//#####################################################################
#ifndef __DEFORMABLE_BODY_PARAMETERS__
#define __DEFORMABLE_BODY_PARAMETERS__

namespace PhysBAM
{

template <class T>
class DEFORMABLE_BODY_PARAMETERS
{
public:
	bool write;
	bool print_diagnostics, print_residuals;

	DEFORMABLE_BODY_PARAMETERS()
		: write (true), print_diagnostics (true), print_residuals (false)
	{}

	virtual ~DEFORMABLE_BODY_PARAMETERS()
	{}

//#####################################################################
};
}
#endif
