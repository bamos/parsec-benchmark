//#####################################################################
// Copyright 2004, Ron Fedkiw, Eran Guendelman, Frank Losasso, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "SOLIDS_FLUIDS_EXAMPLE_3D.h"
#include "../Utilities/DEBUG_UTILITIES.h"
#include "../Read_Write/FILE_UTILITIES.h"
using namespace PhysBAM;
//#####################################################################
// Function Write_Output_Files
//#####################################################################
template<class T, class RW> void SOLIDS_FLUIDS_EXAMPLE_3D<T, RW>::
Write_Output_Files (const int frame) const
{
	FILE_UTILITIES::Create_Directory (output_directory);
	Write_Frame_Title (frame);
	solids_parameters.template Write_Output_Files<RW> (output_directory, first_frame, frame);

	if (frame < first_frame) return; // preroll: only write solids
}

//#####################################################################
template class SOLIDS_FLUIDS_EXAMPLE_3D<float, float>;
template class SOLIDS_FLUIDS_EXAMPLE_3D<double, double>;
template class SOLIDS_FLUIDS_EXAMPLE_3D<double, float>;
