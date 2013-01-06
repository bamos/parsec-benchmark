//#####################################################################
// Copyright 2004-2005, Eran Guendelman, Geoffrey Irving.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class DYNAMIC_LIST
//#####################################################################
// Elements in this list are identified by persistent id's (assigned sequentially as elements are added to the list).
// At any given time, the set of active elements is a subset of all of the elements previously allocated.
// One typically accesses elements by their unique id using Element(id), but it is also possible to acccess them using their index in the list of currently active elements using Active_Element(index).
// The latter is less preferrable since this index is not persistent.
// The state of the dynamic list can be read/written at any time using Read/Write.
// Elements are destroyed when they become inactive using the Destroy_Element callback (implemented in derived classes).
//#####################################################################
#ifndef __DYNAMIC_LIST__
#define __DYNAMIC_LIST__

#include "../Arrays/LIST_ARRAY.h"
#include "../Read_Write/FILE_UTILITIES.h"
namespace PhysBAM
{

template<class T>
class DYNAMIC_LIST
{
protected:
	LIST_ARRAY<T> array; // active elements
	LIST_ARRAY<int> index_to_id_map, id_to_index_map;
	mutable LIST_ARRAY<int> needs_write; // by id
	int last_unique_id;

public:
	DYNAMIC_LIST()
		: last_unique_id (0)
	{}

	virtual ~DYNAMIC_LIST()
	{}

	virtual void Clean_Up_Memory()
	{
		for (int i = 1; i <= array.m; i++)
		{
			Destroy_Element (array (i), index_to_id_map (i));
		}

		array.Clean_Up_Memory();
		needs_write.Clean_Up_Memory();
		index_to_id_map.Clean_Up_Memory();
		id_to_index_map.Clean_Up_Memory();
		last_unique_id = 0;
	}

	T& Element (const int id)
	{
		return array (id_to_index_map (id));
	}

	const T& Element (const int id) const
	{
		return array (id_to_index_map (id));
	}

	int Element_Index (const int id) const
	{
		return id_to_index_map (id);
	}

	int Number_Of_Elements() const // including inactive elements
	{
		return last_unique_id;
	}

	bool Is_Active (const int id) const
	{
		return id <= id_to_index_map.m && id_to_index_map (id);
	}

	T& Active_Element (const int index)
	{
		return array (index);
	}

	const T& Active_Element (const int index) const
	{
		return array (index);
	}

	int Active_Element_Id (const int index) const
	{
		return index_to_id_map (index);
	}

	int Number_Of_Active_Elements() const
	{
		return array.m;
	}

	// returns id
	virtual int Add_Element (const T& element)
	{
		last_unique_id++;
		array.Append_Element (element);
		needs_write.Append_Element (last_unique_id);
		index_to_id_map.Append_Element (last_unique_id);
		id_to_index_map.Append_Element (array.m);
		assert (id_to_index_map.m == last_unique_id);
		return last_unique_id;
	}

	void Remove_Element (const int id)
	{
		int index = id_to_index_map (id);
		assert (index);
		Destroy_Element (array (index), id);
		id_to_index_map (id) = 0;
		array.Remove_Index_Lazy (index);
		index_to_id_map.Remove_Index_Lazy (index);

		if (index <= array.m) id_to_index_map (index_to_id_map (index)) = index;
	}

//#####################################################################
	template<class RW> void Read (const std::string& prefix, const int frame, LIST_ARRAY<int>& needs_init);
	template<class RW> void Write (const std::string& prefix, const int frame) const;
	virtual void Destroy_Element (T& element, const int id) = 0;
//#####################################################################
};
//#####################################################################
// Function Read
//#####################################################################
// Reads the set of active id's and updates the index<->id maps.
// needs_init is filled with id's of those elements which have become newly active and need to be iniitalized.
template<class T> template<class RW> void DYNAMIC_LIST<T>::
Read (const std::string& prefix, const int frame, LIST_ARRAY<int>& needs_init)
{
	needs_init.Reset_Current_Size_To_Zero();
	LIST_ARRAY<int> active_ids;
	char version;
	FILE_UTILITIES::Read_From_File<RW> (STRING_UTILITIES::string_sprintf ("%sactive_ids.%d", prefix.c_str(), frame), version, last_unique_id, active_ids);
	assert (version == 1);
	LIST_ARRAY<T> new_array;
	ARRAY<bool> element_copied (array.m);
	id_to_index_map.Resize_Array (last_unique_id);

	for (int i = 1; i <= active_ids.m; i++)
	{
		int index = id_to_index_map (active_ids (i));

		if (index)
		{
			new_array.Append_Element (array (index));
			element_copied (index) = true;
		}
		else
		{
			new_array.Append_Element (T());
			needs_init.Append_Element (active_ids (i));
		}
	}

	for (int i = 1; i <= array.m; i++) if (!element_copied (i)) Destroy_Element (array (i), index_to_id_map (i));

	index_to_id_map.Resize_Array (active_ids.m);
	LIST_ARRAY<int>::copy (0, id_to_index_map);

	for (int i = 1; i <= new_array.m; i++)
	{
		index_to_id_map (i) = active_ids (i);
		id_to_index_map (active_ids (i)) = i;
	}

	LIST_ARRAY<T>::exchange_arrays (array, new_array);
}
//#####################################################################
// Function Write
//#####################################################################
// Writes the set of active id's.
// needs_write indicates which elements have not been written since their creation, so derived classes should write those out (and reset the needs_write list).
template<class T> template<class RW> void DYNAMIC_LIST<T>::
Write (const std::string& prefix, const int frame) const
{
	const char version = 1;
	FILE_UTILITIES::Write_To_File<RW> (STRING_UTILITIES::string_sprintf ("%sactive_ids.%d", prefix.c_str(), frame), version, last_unique_id, index_to_id_map);

	for (int i = needs_write.m; i >= 1; i--) if (!id_to_index_map (needs_write (i))) needs_write.Remove_Index_Lazy (i); // handle case of new element which was removed without being written
}
//#####################################################################
}
#endif
