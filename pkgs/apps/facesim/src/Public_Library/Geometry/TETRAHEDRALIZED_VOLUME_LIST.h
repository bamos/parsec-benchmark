//#####################################################################
// Copyright 2004, Zhaosheng Bao.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class TETRAHEDRALIZED_VOLUME_LIST
//#####################################################################
#ifndef __TETRAHEDRALIZED_VOLUME_LIST__
#define __TETRAHEDRALIZED_VOLUME_LIST__

#include "../Data_Structures/DYNAMIC_LIST.h"
namespace PhysBAM
{

template<class T> class TETRAHEDRALIZED_VOLUME;

template<class T>
class TETRAHEDRALIZED_VOLUME_LIST: public DYNAMIC_LIST<TETRAHEDRALIZED_VOLUME<T>*>
{
private:
	using DYNAMIC_LIST<TETRAHEDRALIZED_VOLUME<T>*>::array;
	using DYNAMIC_LIST<TETRAHEDRALIZED_VOLUME<T>*>::id_to_index_map;
	using DYNAMIC_LIST<TETRAHEDRALIZED_VOLUME<T>*>::last_unique_id;
	using DYNAMIC_LIST<TETRAHEDRALIZED_VOLUME<T>*>::needs_write;

public:
//#####################################################################
	template<class RW> void Read (const std::string& prefix, const int frame);
	template<class RW> void Write (const std::string& prefix, const int frame) const;
	virtual void Destroy_Element (TETRAHEDRALIZED_VOLUME<T>*& tetrahedralized_volume, const int id);
//#####################################################################
};
}
#endif
