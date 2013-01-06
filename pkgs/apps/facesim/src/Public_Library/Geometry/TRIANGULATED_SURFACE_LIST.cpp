//#########################################################################################################################################
// Copyright 2004, Eran Guendelman.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#########################################################################################################################################
#include "TRIANGULATED_SURFACE_LIST.h"
#include "TRIANGULATED_SURFACE.h"
using namespace PhysBAM;
template class TRIANGULATED_SURFACE_LIST<float>;
template class TRIANGULATED_SURFACE_LIST<double>;
template void TRIANGULATED_SURFACE_LIST<float>::Read<float> (const std::string& prefix, const int frame);
template void TRIANGULATED_SURFACE_LIST<double>::Read<double> (const std::string& prefix, const int frame);
template void TRIANGULATED_SURFACE_LIST<double>::Read<float> (const std::string& prefix, const int frame);
template void TRIANGULATED_SURFACE_LIST<float>::Write<float> (const std::string& prefix, const int frame) const;
template void TRIANGULATED_SURFACE_LIST<double>::Write<double> (const std::string& prefix, const int frame) const;
template void TRIANGULATED_SURFACE_LIST<double>::Write<float> (const std::string& prefix, const int frame) const;
//#####################################################################
// Function Read
//#####################################################################
template<class T> template<class RW> void TRIANGULATED_SURFACE_LIST<T>::
Read (const std::string& prefix, const int frame)
{
	LIST_ARRAY<int> needs_init;
	DYNAMIC_LIST<TRIANGULATED_SURFACE<T>*>::template Read<RW> (prefix, frame, needs_init);

	for (int i = 1; i <= needs_init.m; i++)
	{
		int id = needs_init (i), index = id_to_index_map (id);
		assert (index);
		TRIANGULATED_SURFACE<T>* triangulated_surface = 0;
		FILE_UTILITIES::Create_From_File<RW> (STRING_UTILITIES::string_sprintf ("%s%d.tri", prefix.c_str(), id), triangulated_surface);
		array (index) = triangulated_surface;
	}
}
//#####################################################################
// Function Write
//#####################################################################
template<class T> template <class RW> void TRIANGULATED_SURFACE_LIST<T>::
Write (const std::string& prefix, const int frame) const
{
	DYNAMIC_LIST<TRIANGULATED_SURFACE<T>*>::template Write<RW> (prefix, frame);

	for (int i = 1; i <= needs_write.m; i++)
	{
		int id = needs_write (i), index = id_to_index_map (id);
		assert (index);
		FILE_UTILITIES::Write_To_File<RW> (STRING_UTILITIES::string_sprintf ("%s%d.tri", prefix.c_str(), id), *array (index));
	}

	needs_write.Reset_Current_Size_To_Zero();
}
//#####################################################################
// Function Destroy_Element
//#####################################################################
template<class T> void TRIANGULATED_SURFACE_LIST<T>::
Destroy_Element (TRIANGULATED_SURFACE<T>*& triangulated_surface, const int id)
{
	triangulated_surface->Destroy_Data();
	delete triangulated_surface;
	triangulated_surface = 0;
}
//#####################################################################
