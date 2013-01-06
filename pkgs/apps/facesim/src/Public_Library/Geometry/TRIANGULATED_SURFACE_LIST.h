//#####################################################################
// Copyright 2002-2004, Ron Fedkiw, Eran Guendelman, Geoffrey Irving, Neil Molino, Robert Bridson.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class TRIANGULATED_SURFACE_LIST
//#####################################################################
#ifndef __TRIANGULATED_SURFACE_LIST__
#define __TRIANGULATED_SURFACE_LIST__

#include "../Data_Structures/DYNAMIC_LIST.h"
namespace PhysBAM
{

template<class T> class TRIANGULATED_SURFACE;

template<class T>
class TRIANGULATED_SURFACE_LIST: public DYNAMIC_LIST<TRIANGULATED_SURFACE<T>*>
{
private:
	using DYNAMIC_LIST<TRIANGULATED_SURFACE<T>*>::array;
	using DYNAMIC_LIST<TRIANGULATED_SURFACE<T>*>::id_to_index_map;
	using DYNAMIC_LIST<TRIANGULATED_SURFACE<T>*>::last_unique_id;
	using DYNAMIC_LIST<TRIANGULATED_SURFACE<T>*>::needs_write;

public:
//#####################################################################
	template<class RW> void Read (const std::string& prefix, const int frame);
	template<class RW> void Write (const std::string& prefix, const int frame) const;
	virtual void Destroy_Element (TRIANGULATED_SURFACE<T>*& triangulated_surface, const int id);
//#####################################################################
};
}
#endif
