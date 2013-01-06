//#####################################################################
// Copyright 2004, Ronald Fedkiw.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class LIST_ARRAYS
//#####################################################################
#ifndef __LIST_ARRAYS__
#define __LIST_ARRAYS__

#include <assert.h>
#include "ARRAYS.h"
namespace PhysBAM
{

template<class T>
class LIST_ARRAYS
{
public:
	typedef T ELEMENT;

	ARRAYS<T> array;
	int m; // the current size of the array (array.m may be larger for elbow room)

	LIST_ARRAYS()
		: m (0)
	{}

	LIST_ARRAYS (const int length, const int m_input, const bool initialize_using_default_constructor = true)
		: array (length, m_input, initialize_using_default_constructor), m (m_input)
	{}

	LIST_ARRAYS (const ARRAYS<T>& array_input)
		: array (array_input), m (array_input.m)
	{}

	LIST_ARRAYS (std::istream& input_stream)
		: array (input_stream)
	{
		m = array.m;
	}

	~LIST_ARRAYS()
	{}

	LIST_ARRAYS<T>& operator= (const ARRAYS<T>& source)
	{
		array = source;
		m = source.m;
		return *this;
	}

	LIST_ARRAYS<T>& operator= (const LIST_ARRAYS<T>& source)
	{
		array = source.array;
		m = source.m;
		return *this;
	}

	T& operator() (const int k, const int i)
	{
		assert (i <= m);
		return array (k, i);
	}

	const T& operator() (const int k, const int i) const
	{
		assert (i <= m);
		return array (k, i);
	}

	void Get (const int i, T& element1, T& element2) const
	{
		assert (i <= m);
		array.Get (i, element1, element2);
	}

	void Get (const int i, T& element1, T& element2, T& element3) const
	{
		assert (i <= m);
		array.Get (i, element1, element2, element3);
	}

	void Get (const int i, T& element1, T& element2, T& element3, T& element4) const
	{
		assert (i <= m);
		array.Get (i, element1, element2, element3, element4);
	}

	void Get (const int i, T& element1, T& element2, T& element3, T& element4, T& element5) const
	{
		assert (i <= m);
		array.Get (i, element1, element2, element3, element4, element5);
	}

	void Get (const int i, T& element1, T& element2, T& element3, T& element4, T& element5, T& element6) const
	{
		assert (i <= m);
		array.Get (i, element1, element2, element3, element4, element5, element6);
	}

	void Set (const int i, const T& element1, const T& element2)
	{
		assert (i <= m);
		array.Set (i, element1, element2);
	}

	void Set (const int i, const T& element1, const T& element2, const T& element3)
	{
		assert (i <= m);
		array.Set (i, element1, element2, element3);
	}

	void Set (const int i, const T& element1, const T& element2, const T& element3, const T& element4)
	{
		assert (i <= m);
		array.Set (i, element1, element2, element3, element4);
	}

	void Set (const int i, const T& element1, const T& element2, const T& element3, const T& element4, const T& element5)
	{
		assert (i <= m);
		array.Set (i, element1, element2, element3, element4, element5);
	}

	void Set (const int i, const T& element1, const T& element2, const T& element3, const T& element4, const T& element5, const T& element6)
	{
		assert (i <= m);
		array.Set (i, element1, element2, element3, element4, element5, element6);
	}

	int Max_Size() const
	{
		return array.m;
	}

	void Compact()
	{
		if (m < array.m) Exact_Resize_Array (array.length, m);
	}

private:
	void Ensure_Enough_Space (const int m_new, const bool copy_existing_elements = true)
	{
		if (array.m < m_new) array.Resize_Array (array.length, 4 * m_new / 3 + 2, false, copy_existing_elements);
	}

public:
	void Preallocate (const int max_size)
	{
		if (array.m < max_size) array.Resize_Array (array.length, max_size, false);
	}

	void Resize_Array (const int length_new, const int m_new, const bool initialize_new_elements = true, const bool copy_existing_elements = true)
	{
		if (array.length != length_new)
		{
			array.Resize_Array (length_new, m_new, initialize_new_elements, copy_existing_elements);
			m = m_new;
		}
		else
		{
			Ensure_Enough_Space (m_new, copy_existing_elements);

			if (initialize_new_elements && m_new > m) for (int i = m + 1; i <= m_new; i++) for (int k = 1; k <= array.length; k++) array (k, i) = T();

			m = m_new;
		}
	}

	void Exact_Resize_Array (const int length_new, const int m_new) // zero elbow room
	{
		array.Resize_Array (length_new, m_new);
		m = array.m;
	}

	void Append_Element (const T& element1, const T& element2)
	{
		assert (array.length == 2);
		m++;
		Ensure_Enough_Space (m);
		array (1, m) = element1;
		array (2, m) = element2;
	}

	void Append_Element (const T& element1, const T& element2, const T& element3)
	{
		assert (array.length == 3);
		m++;
		Ensure_Enough_Space (m);
		array (1, m) = element1;
		array (2, m) = element2;
		array (3, m) = element3;
	}

	void Append_Element (const T& element1, const T& element2, const T& element3, const T& element4)
	{
		assert (array.length == 4);
		m++;
		Ensure_Enough_Space (m);
		array (1, m) = element1;
		array (2, m) = element2;
		array (3, m) = element3;
		array (4, m) = element4;
	}

	void Append_Element (const T& element1, const T& element2, const T& element3, const T& element4, const T& element5)
	{
		assert (array.length == 5);
		m++;
		Ensure_Enough_Space (m);
		array (1, m) = element1;
		array (2, m) = element2;
		array (3, m) = element3;
		array (4, m) = element4;
		array (5, m) = element5;
	}

	void Append_Element (const T& element1, const T& element2, const T& element3, const T& element4, const T& element5, const T& element6)
	{
		assert (array.length == 6);
		m++;
		Ensure_Enough_Space (m);
		array (1, m) = element1;
		array (2, m) = element2;
		array (3, m) = element3;
		array (4, m) = element4;
		array (5, m) = element5;
		array (6, m) = element6;
	}

	void Append_Elements (const ARRAYS<T>& append_array)
	{
		assert (array.length == append_array.length);
		m += append_array.m;
		Ensure_Enough_Space (m);

		for (int i = 1; i <= append_array.m; i++) for (int k = 1; k <= array.length; k++) array (k, m - append_array.m + i) = append_array (k, i);
	}

	void Append_Elements (const LIST_ARRAYS<T>& append_list)
	{
		assert (array.length == append_list.array.length);
		m += append_list.m;
		Ensure_Enough_Space (m);

		for (int i = 1; i <= append_list.m; i++) for (int k = 1; k <= array.length; k++) array (k, m - append_list.m + i) = append_list.array (k, i);
	}

	void Remove_End()
	{
		if (m)
		{
			for (int k = 1; k <= array.length; k++) array (k, m) = T();

			m--;
		}
	}

	void Remove_End_Without_Clearing_Value()
	{
		assert (m > 0);
		m--;
	}

	void Remove_Index (const int index) // preserves ordering of remaining elements
	{
		assert (1 <= index && index <= m);

		for (int i = index; i < m; i++) for (int k = 1; k <= array.length; k++) array (k, i) = array (k, i + 1);

		Remove_End();
	}

	void Remove_Indices (const ARRAY<int>& index)
	{
		if (index.m == 0) return;

		for (int kk = 1; kk <= index.m - 1; kk++)
		{
			assert (1 <= index (kk) && index (kk) <= m);

			for (int i = index (kk) + 1 - kk; i <= index (kk + 1) - 1 - kk; i++) for (int k = 1; k <= array.length; k++) (*this) (k, i) = (*this) (k, i + kk);
		}

		for (int i = index (index.m) + 1 - index.m; i <= m - index.m; i++) for (int k = 1; k <= array.length; k++) (*this) (k, i) = (*this) (k, i + index.m);

		m -= index.m;
	}

	void Remove_Index_Lazy (const int index)
	{
		assert (1 <= index && index <= m);

		if (index < m) for (int k = 1; k <= array.length; k++)
			{
				array (k, index) = array (k, m);
				array (k, m) = T();
			}
		else for (int k = 1; k <= array.length; k++) array (k, m) = T();

		m--;
	}

	void Remove_All_Entries (const bool reinitialize_entries = true)
	{
		if (reinitialize_entries) ARRAYS<T>::copy (T(), array);

		m = 0;
	}

	void Reset_Current_Size_To_Zero() // doesn't actually remove entries or set them to zero
	{
		m = 0;
	}

	void Clean_Up_Memory()
	{
		Exact_Resize_Array (array.length, 0);
	}

	void Pop (T& last_element_1, T& last_element_2)
	{
		Get (m, last_element_1, last_element_2);
		Remove_End();
	}

	void Pop (T& last_element_1, T& last_element_2, T& last_element_3)
	{
		Get (m, last_element_1, last_element_2, last_element_3);
		Remove_End();
	}

	static void copy (const T& constant, LIST_ARRAYS<T>& new_copy)
	{
		T* array = new_copy.array.Get_Array_Pointer();

		for (int index = 0, size = new_copy.array.length * new_copy.m; index < size; index++) array[index] = constant;
	}

	static void copy (const LIST_ARRAYS<T>& old_copy, LIST_ARRAYS<T>& new_copy)
	{
		assert (old_copy.array.length == new_copy.array.length);
		assert (old_copy.m == new_copy.m);
		T *old_array = old_copy.array.Get_Array_Pointer(), *new_array = new_copy.array.Get_Array_Pointer();

		for (int index = 0, size = new_copy.array.length * new_copy.m; index < size; index++) new_array[index] = old_array[index];
	}

	static void exchange_arrays (LIST_ARRAYS<T>& a, LIST_ARRAYS<T>& b)
	{
		ARRAYS<T>::exchange_arrays (a.array, b.array);
		exchange (a.m, b.m);
	}

	template<class RW>
	void Read (std::istream& input_stream)
	{
		array.template Read<RW> (input_stream);
		m = array.m;
	}

	template<class RW>
	void Write (std::ostream& output_stream) const
	{
		array.template Write_Prefix<RW> (output_stream, m);
	}

//#####################################################################
};
}
#endif

