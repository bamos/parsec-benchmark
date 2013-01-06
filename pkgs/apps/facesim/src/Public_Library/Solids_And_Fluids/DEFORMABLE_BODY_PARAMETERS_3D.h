//#####################################################################
// Copyright 2004, Ron Fedkiw, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class DEFORMABLE_BODY_PARAMETERS_3D
//#####################################################################
#ifndef __DEFORMABLE_BODY_PARAMETERS_3D__
#define __DEFORMABLE_BODY_PARAMETERS_3D__

#include "DEFORMABLE_BODY_PARAMETERS.h"
#include "../Deformable_Objects/DEFORMABLE_OBJECT_LIST_3D.h"
namespace PhysBAM
{

template <class T>
class DEFORMABLE_BODY_PARAMETERS_3D: public DEFORMABLE_BODY_PARAMETERS<T>
{
public:
	using DEFORMABLE_BODY_PARAMETERS<T>::print_diagnostics;
	using DEFORMABLE_BODY_PARAMETERS<T>::print_residuals;

	DEFORMABLE_OBJECT_LIST_3D<T> list;

	DEFORMABLE_BODY_PARAMETERS_3D()
	{}

	virtual ~DEFORMABLE_BODY_PARAMETERS_3D()
	{}

//#####################################################################
// Function Initialize_Bodies
//#####################################################################
	void Initialize_Bodies (const T cfl)
	{
		list.Print_Diagnostics (print_diagnostics);
		list.Print_Residuals (print_residuals);
		list.Set_CFL_Number (cfl);
	}
//#####################################################################
};
}
#endif
