//#####################################################################
// Copyright 2004, Ronald Fedkiw.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class LIST_ARRAY
//#####################################################################
#ifndef __LIST_ARRAY__
#define __LIST_ARRAY__

#include <assert.h>
#include "ARRAY.h"
namespace PhysBAM
{

template<class T>
class LIST_ARRAY
{
public:
	typedef T ELEMENT;

	ARRAY<T> array;
	int m; // the current size of the array (array.m may be larger for elbow room)

	LIST_ARRAY()
		: m (0)
	{}

	explicit LIST_ARRAY (const int m_input, const bool initialize_using_default_constructor = true)
		: array (m_input, initialize_using_default_constructor), m (m_input)
	{}

	explicit LIST_ARRAY (const ARRAY<T>& array_input)
		: array (array_input), m (array_input.m)
	{}

	LIST_ARRAY (std::istream& input_stream)
		: array (input_stream)
	{
		m = array.m;
	}

	~LIST_ARRAY()
	{}

	LIST_ARRAY<T>& operator= (const ARRAY<T>& source)
	{
		array = source;
		m = source.m;
		return *this;
	}

	LIST_ARRAY<T>& operator= (const LIST_ARRAY<T>& source)
	{
		array = source.array;
		m = source.m;
		return *this;
	}

	T& operator() (const int i)
	{
		assert (i <= m);
		return array (i);
	}

	const T& operator() (const int i) const
	{
		assert (i <= m);
		return array (i);
	}

	ARRAY_RANGE<LIST_ARRAY> Range (const VECTOR_2D<int>& range)
	{
		return ARRAY_RANGE<LIST_ARRAY> (*this, range);
	}

	ARRAY_RANGE<const LIST_ARRAY> Range (const VECTOR_2D<int>& range) const
	{
		return ARRAY_RANGE<const LIST_ARRAY> (*this, range);
	}

	T& Last_Element()
	{
		return (*this) (m);
	}

	const T& Last_Element() const
	{
		return (*this) (m);
	}

	void Get (const int i, T& element) const
	{
		assert (i <= m);
		array.Get (i, element);
	}

	void Set (const int i, const T& element1)
	{
		assert (i <= m);
		array.Set (i, element1);
	}

	int Max_Size() const
	{
		return array.m;
	}

	void Compact()
	{
		if (m < array.m) Exact_Resize_Array (m);
	}

private:
	void Ensure_Enough_Space (const int m_new, const bool copy_existing_elements = true)
	{
		if (array.m < m_new) array.Resize_Array (4 * m_new / 3 + 2, false, copy_existing_elements);
	}

public:
	void Preallocate (const int max_size)
	{
		if (array.m < max_size) array.Resize_Array (max_size, false);
	}

	void Resize_Array (const int m_new, const bool initialize_new_elements = true, const bool copy_existing_elements = true)
	{
		Ensure_Enough_Space (m_new, copy_existing_elements);

		if (initialize_new_elements && m_new > m) for (int i = m + 1; i <= m_new; i++) array (i) = T();

		m = m_new;
	}

	void Exact_Resize_Array (const int m_new, const bool initialize_new_elements = true) // zero elbow room
	{
		array.Resize_Array (m_new, initialize_new_elements);
		m = array.m;
	}

	void Exact_Resize_Array2 (const int m_new, const bool initialize_new_elements = true) // zero elbow room
	{
		m = m_new;
	}

	void Append_Element (const T& element)
	{
		m++;
		Ensure_Enough_Space (m);
		array (m) = element;
	}

	void Append_Elements (const ARRAY<T>& append_array)
	{
		m += append_array.m;
		Ensure_Enough_Space (m);

		for (int i = 1; i <= append_array.m; i++) array (m - append_array.m + i) = append_array (i);
	}

	void Append_Elements (const LIST_ARRAY<T>& append_list)
	{
		m += append_list.m;
		Ensure_Enough_Space (m);

		for (int i = 1; i <= append_list.m; i++) array (m - append_list.m + i) = append_list.array (i);
	}

	void Append_Unique_Element (const T& element)
	{
		for (int i = 1; i <= m; i++) if (array (i) == element) return;

		Append_Element (element);
	}

	void Append_Unique_Elements (const ARRAY<T>& append_array)
	{
		for (int i = 1; i <= append_array.m; i++) Append_Unique_Element (append_array (i));
	}

	void Append_Unique_Elements (const LIST_ARRAY<T>& append_list)
	{
		for (int i = 1; i <= append_list.m; i++) Append_Unique_Element (append_list (i));
	}

	void Remove_End()
	{
		if (m)
		{
			array (m) = T();
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

		for (int i = index; i < m; i++) array (i) = array (i + 1);

		Remove_End();
	}

	void Remove_Indices (const ARRAY<int>& index)
	{
		if (index.m == 0) return;

		for (int kk = 1; kk <= index.m - 1; kk++)
		{
			assert (1 <= index (kk) && index (kk) <= m);

			for (int i = index (kk) + 1 - kk; i <= index (kk + 1) - 1 - kk; i++) (*this) (i) = (*this) (i + kk);
		}

		for (int i = index (index.m) + 1 - index.m; i <= m - index.m; i++) (*this) (i) = (*this) (i + index.m);

		m -= index.m;
	}

	void Remove_Index_Lazy (const int index)
	{
		assert (1 <= index && index <= m);

		if (index < m)
		{
			array (index) = array (m);
			array (m) = T();
		}
		else array (m) = T();

		m--;
	}

	void Remove_All_Entries (const bool reinitialize_entries = true)
	{
		if (reinitialize_entries) ARRAY<T>::copy (T(), array);

		m = 0;
	}

	void Reset_Current_Size_To_Zero() // doesn't actually remove entries or set them to zero
	{
		m = 0;
	}

	void Clean_Up_Memory()
	{
		Exact_Resize_Array (0);
	}

	void Delete_Pointers_And_Clean_Memory() // only valid if T is a pointer type
	{
		for (int i = 1; i <= m; i++) delete array (i);

		Clean_Up_Memory();
	}

	int Find (const T& element) const
	{
		for (int i = 1; i <= m; i++) if (array (i) == element) return i;

		return 0;
	}

	bool Find (const T& element, int& index) const
	{
		return Find (element, 1, index);
	}

	bool Find (const T& element, const int start_index, int& index) const // returns first occurence at or after start_index
	{
		for (int i = start_index; i <= m; i++) if (array (i) == element)
			{
				index = i;
				return true;
			}

		return false;
	}

	void Insert_Element (const T& element, const int index)
	{
		m++;
		Ensure_Enough_Space (m);

		for (int i = m; i > index; i--) array (i) = array (i - 1);

		array (index) = element;
	}

	void Pop (T& last_element)
	{
		Get (m, last_element);
		Remove_End();
	}

	int Count_Matches (const T& value) const
	{
		int count = 0;

		for (int index = 1; index <= m; index++) if (array (index) == value) count++;

		return count;
	}

	int Number_True() const
	{
		return Count_Matches (true);
	}

	int Number_False() const
	{
		return Count_Matches (false);
	}

	static void copy (const T& constant, LIST_ARRAY<T>& new_copy)
	{
		for (int index = 1; index <= new_copy.m; index++) new_copy.array.base_pointer[index] = constant;
	}

	static void copy (const LIST_ARRAY<T>& old_copy, LIST_ARRAY<T>& new_copy)
	{
		assert (old_copy.m == new_copy.m);
		copy_up_to (old_copy, new_copy, old_copy.m);
	}

	static void copy_up_to (const LIST_ARRAY<T>& old_copy, LIST_ARRAY<T>& new_copy, const int up_to_index)
	{
		assert (up_to_index <= old_copy.array.m && up_to_index <= new_copy.array.m);

		for (int index = 1; index <= up_to_index; index++) new_copy (index) = old_copy (index);
	}

	static T max (const LIST_ARRAY<T>& a)
	{
		return ARRAY<T>::max (a.array, a.m);
	}

	static T maxabs (const LIST_ARRAY<T>& a)
	{
		return ARRAY<T>::maxabs (a.array, a.m);
	}

	static T maxmag (const LIST_ARRAY<T>& a)
	{
		return ARRAY<T>::maxmag (a.array, a.m);
	}

	static int argmax (const LIST_ARRAY<T>& a)
	{
		return ARRAY<T>::argmax (a.array, a.m);
	}

	static T min (const LIST_ARRAY<T>& a)
	{
		return ARRAY<T>::min (a.array, a.m);
	}

	static T minmag (const LIST_ARRAY<T>& a)
	{
		return ARRAY<T>::minmag (a.array, a.m);
	}

	static int argmin (const LIST_ARRAY<T>& a)
	{
		return ARRAY<T>::argmin (a.array, a.m);
	}

	static T sum (const LIST_ARRAY<T>& a)
	{
		return ARRAY<T>::sum (a.array, a.m);
	}

	static T sumabs (const LIST_ARRAY<T>& a)
	{
		return ARRAY<T>::sumabs (a.array, a.m);
	}

	static T Maximum_Pairwise_Distance (const LIST_ARRAY<T>& a, const LIST_ARRAY<T>& b)
	{
		assert (a.m == b.m);
		T result = (T) 0;

		for (int index = 1; index <= a.m; index++) result = PhysBAM::max (result, fabs (a (index) - b (index)));

		return result;
	}

	template<class TS>
	static TS Maximum_Pairwise_Vector_Distance (const LIST_ARRAY<T>& a, const LIST_ARRAY<T>& b)
	{
		assert (a.m == b.m);
		return ARRAY<T>::template Maximum_Pairwise_Vector_Distance<TS> (a.array, b.array, a.m);
	}

	static void find_common_elements (const LIST_ARRAY<T>& a, const LIST_ARRAY<T>& b, LIST_ARRAY<T>& result)
	{
		assert (a.array.base_pointer != result.array.base_pointer);
		assert (b.array.base_pointer != result.array.base_pointer);
		result.Remove_All_Entries();
		int j;

		for (int i = 1; i <= a.m; i++) if (b.Find (a (i), j)) result.Append_Element (a (i));
	}

	static void exchange_arrays (LIST_ARRAY<T>& a, LIST_ARRAY<T>& b)
	{
		ARRAY<T>::exchange_arrays (a.array, b.array);
		exchange (a.m, b.m);
	}

	static void sort (LIST_ARRAY<T>& a)
	{
		ARRAY<T>::sort (a.array, a.m);
	}

	template<class T2>
	static void sort (LIST_ARRAY<T>& a, LIST_ARRAY<T2>& aux)
	{
		ARRAY<T>::sort (a.array, aux.array, a.m);
	}

	static void permute (const LIST_ARRAY<T>& source, LIST_ARRAY<T>& destination, const ARRAY<int>& permutation)
	{
		ARRAY<T>::permute (source.array, destination.array, permutation);
	}

	static void unpermute (const LIST_ARRAY<T>& source, LIST_ARRAY<T>& destination, const ARRAY<int>& permutation)
	{
		unpermute (source.array, permutation);
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

