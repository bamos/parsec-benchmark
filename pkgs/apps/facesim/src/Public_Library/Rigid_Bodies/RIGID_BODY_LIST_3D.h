//#####################################################################
// Copyright 2003-2005, Zhaosheng Bao, Ron Fedkiw, Eran Guendelman, Geoffrey Irving, Sergey Koltakov, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class RIGID_BODY_LIST_3D
//#####################################################################
#ifndef __RIGID_BODY_LIST_3D__
#define __RIGID_BODY_LIST_3D__

#include "../Arrays/ARRAY.h"
#include "../Data_Structures/DYNAMIC_LIST.h"
#include "../Data_Structures/SPLAY_TREE.h"
#include "../Geometry/TRIANGULATED_SURFACE_LIST.h"
#include "../Geometry/TETRAHEDRALIZED_VOLUME_LIST.h"
#include "../Geometry/IMPLICIT_SURFACE_LIST.h"
#include "../Rigid_Bodies/RIGID_BODY_3D.h"
namespace PhysBAM
{

template<class T>
class RIGID_BODY_LIST_3D: public DYNAMIC_LIST<RIGID_BODY_3D<T>*>
{
private:
	using DYNAMIC_LIST<RIGID_BODY_3D<T>*>::array;
	using DYNAMIC_LIST<RIGID_BODY_3D<T>*>::id_to_index_map;
	using DYNAMIC_LIST<RIGID_BODY_3D<T>*>::index_to_id_map;
	using DYNAMIC_LIST<RIGID_BODY_3D<T>*>::last_unique_id;
	using DYNAMIC_LIST<RIGID_BODY_3D<T>*>::needs_write;
	using DYNAMIC_LIST<RIGID_BODY_3D<T>*>::Element;

public:
	LIST_ARRAY<RIGID_BODY_3D<T>*>& rigid_bodies; // reference to array for traditional naming
	TRIANGULATED_SURFACE_LIST<T> triangulated_surface_list;
	IMPLICIT_SURFACE_LIST<T> implicit_surface_list;
	TETRAHEDRALIZED_VOLUME_LIST<T> tetrahedralized_volume_list;
	LIST_ARRAY<int> rigid_body_id_to_triangulated_surface_id;
	LIST_ARRAY<int> rigid_body_id_to_implicit_surface_id;
	LIST_ARRAY<int> rigid_body_id_to_tetrahedralized_volume_id;
private:
	SPLAY_TREE<std::string, int> triangulated_surface_hash, implicit_surface_hash, tetrahedralized_volume_hash; // maps to id
	mutable LIST_ARRAY<std::string> rigid_body_names; // indexed by id

public:
	RIGID_BODY_LIST_3D()
		: rigid_bodies (array)
	{}

	virtual void Clean_Up_Memory()
	{
		DYNAMIC_LIST<RIGID_BODY_3D<T>*>::Clean_Up_Memory();
		triangulated_surface_list.Clean_Up_Memory();
		implicit_surface_list.Clean_Up_Memory();
		tetrahedralized_volume_list.Clean_Up_Memory();
		triangulated_surface_hash.Clean_Up_Memory();
		implicit_surface_hash.Clean_Up_Memory();
		tetrahedralized_volume_hash.Clean_Up_Memory();
	}

	RIGID_BODY_3D<T>* operator() (const int id)
	{
		return Element (id);
	}

	const RIGID_BODY_3D<T>* operator() (const int id) const
	{
		return Element (id);
	}

//#####################################################################
	int Add_Rigid_Body (RIGID_BODY_3D<T>* const& rigid_body, const int triangulated_surface_id, const int implicit_surface_id, const int tetrahedralized_volume_id);
	template<class RW> int Add_Rigid_Body (const std::string& basename, T scaling_factor = 1, const bool read_triangulated_surface = true, const bool read_implicit_surface = true, const bool read_tetrahedralized_volume = false, const bool read_rgd_file = true);
	void Set_External_Forces_And_Velocities (EXTERNAL_FORCES_AND_VELOCITIES<T, VECTOR_3D<T> >& external_forces_and_velocities);
	template<class RW> void Read (const std::string& directory, const int frame, const bool read_triangulated_surface_list = true, const bool read_implicit_surface_list = true, const bool read_tetrahedralized_volume_list = false, LIST_ARRAY<int>* needs_init = 0);
	template<class RW> void Write (const std::string& directory, const int frame, const bool write_triangulated_surface_list = true, const bool write_implicit_surface_list = true, const bool write_tetrahedralized_volume_list = false) const;

private:
	void Write_Rigid_Body_Names (const std::string& output_directory) const;
	bool Read_Rigid_Body_Names (const std::string& output_directory);
	virtual void Destroy_Element (RIGID_BODY_3D<T>*& rigid_body, const int id); // DYNAMIC_LIST callback
//#####################################################################
};
}
#endif
