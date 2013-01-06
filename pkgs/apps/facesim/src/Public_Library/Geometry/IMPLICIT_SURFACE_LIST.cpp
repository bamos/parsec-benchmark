//#########################################################################################################################################
// Copyright 2004, Eran Guendelman.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#########################################################################################################################################
#include "IMPLICIT_SURFACE_LIST.h"
#include "LEVELSET_IMPLICIT_SURFACE.h"
using namespace PhysBAM;
template class IMPLICIT_SURFACE_LIST<float>;
template class IMPLICIT_SURFACE_LIST<double>;
template void IMPLICIT_SURFACE_LIST<float>::Read<float> (const std::string& prefix, const int frame);
template void IMPLICIT_SURFACE_LIST<double>::Read<double> (const std::string& prefix, const int frame);
template void IMPLICIT_SURFACE_LIST<double>::Read<float> (const std::string& prefix, const int frame);
template void IMPLICIT_SURFACE_LIST<float>::Write<float> (const std::string& prefix, const int frame) const;
template void IMPLICIT_SURFACE_LIST<double>::Write<double> (const std::string& prefix, const int frame) const;
template void IMPLICIT_SURFACE_LIST<double>::Write<float> (const std::string& prefix, const int frame) const;
//#####################################################################
// Function Read
//#####################################################################
template<class T> template<class RW> void IMPLICIT_SURFACE_LIST<T>::
Read (const std::string& prefix, const int frame)
{
	LIST_ARRAY<int> needs_init;
	DYNAMIC_LIST<IMPLICIT_SURFACE<T>*>::template Read<RW> (prefix, frame, needs_init);
	FILE_UTILITIES::Read_From_File<RW> (STRING_UTILITIES::string_sprintf ("%skey", prefix.c_str()), levelset_data);

	for (int i = 1; i <= needs_init.m; i++)
	{
		int id = needs_init (i), index = id_to_index_map (id);
		assert (index);

		if (levelset_data (id))
		{
			LEVELSET_IMPLICIT_SURFACE<T>* levelset_implicit_surface = 0;
			FILE_UTILITIES::Create_From_File<RW> (STRING_UTILITIES::string_sprintf ("%s%d.phi", prefix.c_str(), id), levelset_implicit_surface);
			array (index) = levelset_implicit_surface;
		}
		else
		{
			NOT_IMPLEMENTED();
		}
	}
}
//#####################################################################
// Function Write
//#####################################################################
template<class T> template<class RW> void IMPLICIT_SURFACE_LIST<T>::
Write (const std::string& prefix, const int frame) const
{
	levelset_data.Resize_Array (last_unique_id);
	DYNAMIC_LIST<IMPLICIT_SURFACE<T>*>::template Write<RW> (prefix, frame);

	for (int i = 1; i <= needs_write.m; i++)
	{
		int id = needs_write (i), index = id_to_index_map (id);
		assert (index);

		if (array (index)->levelset_data)
		{
			levelset_data (id) = true;
			FILE_UTILITIES::Write_To_File<RW> (STRING_UTILITIES::string_sprintf ("%s%d.phi", prefix.c_str(), id), * (LEVELSET_IMPLICIT_SURFACE<T>*) array (index));
		}
		else
		{
			NOT_IMPLEMENTED();
		}
	}

	needs_write.Reset_Current_Size_To_Zero();
	FILE_UTILITIES::Write_To_File<RW> (STRING_UTILITIES::string_sprintf ("%skey", prefix.c_str()), levelset_data);
}
//#####################################################################
// Function Destroy_Element
//#####################################################################
template<class T> void IMPLICIT_SURFACE_LIST<T>::
Destroy_Element (IMPLICIT_SURFACE<T>*& implicit_surface, const int id)
{
	implicit_surface->Destroy_Data();
	delete implicit_surface;
	implicit_surface = 0;
}
//#####################################################################
