//#####################################################################
// Copyright 2002-2004, Robert Bridson, Ron Fedkiw, Eran Guendelman, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class IMPLICIT_SURFACE_LIST
//#####################################################################
#ifndef __IMPLICIT_SURFACE_LIST__
#define __IMPLICIT_SURFACE_LIST__

#include "../Data_Structures/DYNAMIC_LIST.h"
namespace PhysBAM
{

template<class T> class IMPLICIT_SURFACE;

template<class T>
class IMPLICIT_SURFACE_LIST: public DYNAMIC_LIST<IMPLICIT_SURFACE<T>*>
{
private:
	using DYNAMIC_LIST<IMPLICIT_SURFACE<T>*>::array;
	using DYNAMIC_LIST<IMPLICIT_SURFACE<T>*>::id_to_index_map;
	using DYNAMIC_LIST<IMPLICIT_SURFACE<T>*>::last_unique_id;
	using DYNAMIC_LIST<IMPLICIT_SURFACE<T>*>::needs_write;

	mutable LIST_ARRAY<bool> levelset_data; // not quadtree data, indexed by id

public:
//#####################################################################
	template<class RW> void Read (const std::string& prefix, const int frame);
	template<class RW> void Write (const std::string& prefix, const int frame) const;
	virtual void Destroy_Element (IMPLICIT_SURFACE<T>*& segmented_surface, const int id);
//#####################################################################
};
}
#endif
