//#####################################################################
// Copyright 2004-2006, Ronald Fedkiw, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class ARRAY
//#####################################################################
#ifndef __ARRAY__
#define __ARRAY__

#include <cstdlib>

#include <assert.h>
#include <iostream>
#include "../Matrices_And_Vectors/VECTOR_2D.h"
#include "../Math_Tools/exchange.h"
#include "../Math_Tools/max.h"
#include "../Math_Tools/maxabs.h"
#include "../Math_Tools/maxmag.h"
#include "../Math_Tools/min.h"
#include "../Math_Tools/minmag.h"
#include "../Read_Write/READ_WRITE_FUNCTIONS.h"
#include "../Utilities/TYPE_UTILITIES.h"
#include "../Utilities/STATIC_ASSERT.h"

namespace PhysBAM
{

template<class T_ARRAY> class ARRAY_RANGE;

template<class T>
class ARRAY
{
public:
	typedef T ELEMENT;

	int m; // end of the array
	T* base_pointer; // starts at [1], not [0]

	ARRAY()
		: m (0)
	{
		Set_Base_Pointer (0);
	}

	explicit ARRAY (const int m_input, const bool initialize_using_default_constructor = true)
		: m (m_input)
	{
		T* array = new T[m];
		Set_Base_Pointer (array);

		if (initialize_using_default_constructor) for (int index = 0; index < m; index++) array[index] = T();
	}

	ARRAY (const ARRAY<T>& array_input, const bool initialize_using_default_constructor = true)
		: m (array_input.m)
	{
		T* array = new T[m];
		Set_Base_Pointer (array);
		T* old_array = array_input.Get_Array_Pointer();

		if (initialize_using_default_constructor) for (int index = 0; index < m; index++) array[index] = old_array[index];
	}

	ARRAY (std::istream& input_stream)
		: m (0)
	{
		Set_Base_Pointer (0);
		Read<T> (input_stream);
	}

	~ARRAY()
	{
		Deallocate_Base_Pointer();
	}

	void Clean_Memory()
	{
		Resize_Array (0, false, false);
	}

	void Delete_Pointers_And_Clean_Memory() // only valid if T is a pointer type
	{
		for (int i = 1; i <= m; i++) delete base_pointer[i];

		Clean_Memory();
	}

	T* Allocate_Base_Pointer (const int m_input)
	{
		return (new T[m_input]) - 1;
	}

	void Set_Base_Pointer (T* array)
	{
		base_pointer = array - 1;
	}

	T* Get_Array_Pointer() const
	{
		return base_pointer + 1;
	}

	void Deallocate_Base_Pointer()
	{
		delete[] (base_pointer + 1);
	}

	ARRAY<T>& operator= (const ARRAY<T>& source)
	{
		if (!Equal_Dimensions (*this, source))
		{
			Deallocate_Base_Pointer();
			m = source.m;
			base_pointer = Allocate_Base_Pointer (m);
		}

		T *array = Get_Array_Pointer(), *source_array = source.Get_Array_Pointer();

		for (int index = 0; index < m; index++) array[index] = source_array[index];

		return *this;
	}

	T& operator() (const int i)
	{
		assert (Valid_Index (i));
		return base_pointer[i];
	}

	const T& operator() (const int i) const
	{
		assert (Valid_Index (i));
		return base_pointer[i];
	}

	ARRAY_RANGE<ARRAY> Range (const VECTOR_2D<int>& range)
	{
		return ARRAY_RANGE<ARRAY> (*this, range);
	}

	ARRAY_RANGE<const ARRAY> Range (const VECTOR_2D<int>& range) const
	{
		return ARRAY_RANGE<const ARRAY> (*this, range);
	}

	bool Valid_Index (const int i) const
	{
		return 1 <= i && i <= m;
	}

	ARRAY<T>& operator+= (const ARRAY<T>& v)
	{
		assert (Equal_Dimensions (*this, v));

		for (int index = 1; index <= m; index++) (*this) (index) += v (index);

		return *this;
	}

	ARRAY<T>& operator+= (const T& a)
	{
		for (int index = 1; index <= m; index++) (*this) (index) += a;

		return *this;
	}

	ARRAY<T>& operator-= (const ARRAY<T>& v)
	{
		assert (Equal_Dimensions (*this, v));

		for (int index = 1; index <= m; index++) (*this) (index) -= v (index);

		return *this;
	}

	ARRAY<T>& operator-= (const T& a)
	{
		for (int index = 1; index <= m; index++) (*this) (index) -= a;

		return *this;
	}

	template<class T2>
	ARRAY<T>& operator*= (const ARRAY<T2>& v)
	{
		assert (Equal_Dimensions (*this, v));

		for (int index = 1; index <= m; index++) (*this) (index) *= v (index);

		return *this;
	}

	template<class T2>
	ARRAY<T>& operator*= (const T2 a)
	{
		for (int index = 1; index <= m; index++) (*this) (index) *= a;

		return *this;
	}

	template<class T2>
	ARRAY<T>& operator/= (const T2 a)
	{
		T2 one_over_a = 1 / a;

		for (int index = 1; index <= m; index++) (*this) (index) *= one_over_a;

		return *this;
	}

	template<class T2>
	ARRAY<T>& operator/= (const ARRAY<T2>& a)
	{
		for (int index = 1; index <= m; index++)
		{
			assert (a (index) > 0);
			(*this) (index) /= a (index);
		}

		return *this;
	}

	void Resize_Array (const int m_new, const bool initialize_new_elements = true, const bool copy_existing_elements = true, const T& initialization_value = T())
	{
		if (Equal_Dimensions (*this, m_new)) return;

		T* base_pointer_new = Allocate_Base_Pointer (m_new);

		if (copy_existing_elements)
		{
			int m_end = PhysBAM::min (m, m_new);

			for (int index = 1; index <= m_end; index++) base_pointer_new[index] = base_pointer[index];
		}

		if (initialize_new_elements)
		{
			int m_end = PhysBAM::min (m, m_new);

			for (int index = m_end + 1; index <= m_new; index++) base_pointer_new[index] = initialization_value;
		}

		Deallocate_Base_Pointer();
		m = m_new;
		base_pointer = base_pointer_new;
	}

	void Append_Element (const T& element)
	{
		assert (&element - &base_pointer[1] < 0 || &element - &base_pointer[1] >= m); // make sure element isn't a reference into the current array
		Resize_Array (m + 1);
		(*this) (m) = element;
	}

	void Append_Elements (const ARRAY<T>& append_array)
	{
		int index = m;
		Resize_Array (m + append_array.m);

		for (int i = 1; i <= append_array.m; i++)
		{
			index++;
			(*this) (index) = append_array (i);
		}
	}

	void Append_Unique_Element (const T& element)
	{
		int index;

		if (Find (element, index)) return;

		Resize_Array (m + 1);
		(*this) (m) = element;
	}

	void Append_Unique_Elements (const ARRAY<T>& append_array)
	{
		ARRAY<int> append (append_array.m);
		int count = 0, j;

		for (j = 1; j <= append_array.m; j++)
		{
			int unique = 1, index;

			if (Find (append_array (j), index)) unique = 0;

			if (unique && append_array.Find (append_array (j), j + 1, index)) unique = 0; // element occurs duplicated in given array (only copy last instance)

			if (unique)
			{
				count++;
				append (j) = 1;
			}
		}

		int index = m + 1;
		Resize_Array (m + count);

		for (int k = 1; k <= append.m; k++) if (append (k)) (*this) (index++) = append_array (k);
	}

	void Remove_Index (const int index) // maintains ordering of remaining elements
	{
		assert (Valid_Index (index));
		T* base_pointer_new = Allocate_Base_Pointer (m - 1);
		int i, cut = index;

		for (i = 1; i < cut; i++) base_pointer_new[i] = base_pointer[i];

		for (int j = i + 1; i <= m - 1; i++, j++) base_pointer_new[i] = base_pointer[j];

		Deallocate_Base_Pointer();
		m--;
		base_pointer = base_pointer_new;
	}

	void Remove_Indices (const ARRAY<int>& index)
	{
		if (index.m == 0) return;

		for (int kk = 1; kk <= index.m - 1; kk++)
		{
			assert (Valid_Index (index (kk)));

			for (int i = index (kk) + 1 - kk; i <= index (kk + 1) - 1 - kk; i++) (*this) (i) = (*this) (i + kk);
		}

		for (int i = index (index.m) + 1 - index.m; i <= m - index.m; i++) (*this) (i) = (*this) (i + index.m);

		Resize_Array (m - index.m);
	}

	bool Find (const T& element, int& index) const // returns the first occurence of an element in an array
	{
		return Find (element, 1, index);
	}

	bool Find (const T& element, const int start_index, int& index) const // returns the first occurence after start_index of an element in an array
	{
		for (int i = start_index; i <= m; i++) if ( (*this) (i) == element)
			{
				index = i;
				return true;
			}

		return false;
	}

	void Insert_Element (const T& element, const int index)
	{
		Resize_Array (m + 1);

		for (int i = m; i > index; i--) (*this) (i) = (*this) (i - 1);

		(*this) (index) = element;
	}

	int Count_Matches (const T& value) const
	{
		int count = 0;

		for (int index = 1; index <= m; index++) if ( (*this) (index) == value) count++;

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

	template<class T_ARRAY>
	static void copy (const T& constant, T_ARRAY& new_copy)
	{
		STATIC_ASSERT ( (IS_SAME<T, typename T_ARRAY::ELEMENT>::value));

		for (int index = 1; index <= new_copy.m; index++) new_copy (index) = constant;
	}

	template<class T_ARRAY>
	static void copy (const T& constant, ARRAY_RANGE<T_ARRAY> new_copy)
	{
		STATIC_ASSERT ( (IS_SAME<T, typename T_ARRAY::ELEMENT>::value));

		for (int index = 1; index <= new_copy.m; index++) new_copy (index) = constant;
	}

	static void copy (const ARRAY<T>& old_copy, ARRAY<T>& new_copy)
	{
		assert (Equal_Dimensions (old_copy, new_copy));

		for (int index = 1; index <= old_copy.m; index++) new_copy (index) = old_copy (index);
	}

	static void copy_up_to (const ARRAY<T>& old_copy, ARRAY<T>& new_copy, const int up_to_index)
	{
		assert (up_to_index <= old_copy.m && up_to_index <= new_copy.m);

		for (int index = 1; index <= up_to_index; index++) new_copy (index) = old_copy (index);
	}

	template<class T2>
	static void copy (const T2 constant, const ARRAY<T>& old_copy, ARRAY<T>& new_copy)
	{
		assert (Equal_Dimensions (old_copy, new_copy));

		for (int index = 1; index <= old_copy.m; index++) new_copy (index) = constant * old_copy (index);
	}

	template<class T2>
	static void copy (const T2 c1, const ARRAY<T>& v1, const ARRAY<T>& v2, ARRAY<T>& result)
	{
		assert (Equal_Dimensions (v1, v2) && Equal_Dimensions (v2, result));

		for (int index = 1; index <= result.m; index++) result (index) = c1 * v1 (index) + v2 (index);
	}

	template<class T2>
	static void copy_up_to (const T2 c1, const ARRAY<T>& v1, const ARRAY<T>& v2, ARRAY<T>& result, const int up_to_index)
	{
		assert (v1.m >= up_to_index && v2.m >= up_to_index && result.m >= up_to_index);

		for (int index = 1; index <= up_to_index; index++) result (index) = c1 * v1 (index) + v2 (index);
	}

	template<class T2>
	static void copy (const T2 c1, const ARRAY<T>& v1, const T2 c2, const ARRAY<T>& v2, ARRAY<T>& result)
	{
		assert (Equal_Dimensions (v1, v2) && Equal_Dimensions (v2, result));

		for (int index = 1; index <= result.m; index++) result (index) = c1 * v1 (index) + c2 * v2 (index);
	}

	template<class T2>
	static void copy (const T2 c1, const ARRAY<T>& v1, const T2 c2, const ARRAY<T>& v2, const T2 c3, const ARRAY<T>& v3, ARRAY<T>& result)
	{
		assert (Equal_Dimensions (v1, v2) && Equal_Dimensions (v2, v3) && Equal_Dimensions (v3, result));

		for (int index = 1; index <= result.m; index++) result (index) = c1 * v1 (index) + c2 * v2 (index) + c3 * v3 (index);
	}

	template<class T2>
	static void copy_up_to (const T2 c1, const ARRAY<T>& v1, const T2 c2, const ARRAY<T>& v2, ARRAY<T>& result, const int up_to_index)
	{
		assert (v1.m >= up_to_index && v2.m >= up_to_index && result.m >= up_to_index);

		for (int index = 1; index <= up_to_index; index++) result (index) = c1 * v1 (index) + c2 * v2 (index);
	}

	static void get (ARRAY<T>& new_copy, const ARRAY<T>& old_copy)
	{
		ARRAY<T>::put (old_copy, new_copy, new_copy.m);
	}

	static void put (const ARRAY<T>& old_copy, ARRAY<T>& new_copy)
	{
		ARRAY<T>::put (old_copy, new_copy, old_copy.m);
	}

	template<class T2>
	static void put (const T2 constant, const ARRAY<T>& old_copy, ARRAY<T>& new_copy)
	{
		ARRAY<T>::put (constant, old_copy, new_copy, old_copy.m);
	}

	static void put (const ARRAY<T>& old_copy, ARRAY<T>& new_copy, const int m)
	{
		assert (m <= old_copy.m);
		assert (m <= new_copy.m);

		for (int index = 1; index <= m; index++) new_copy (index) = old_copy (index);
	}

	template<class T2>
	static void put (T2 constant, const ARRAY<T>& old_copy, ARRAY<T>& new_copy, const int m)
	{
		assert (m <= old_copy.m);
		assert (m <= new_copy.m);

		for (int index = 1; index <= m; index++) new_copy (index) = constant * old_copy (index);
	}

	static T max (const ARRAY<T>& a)
	{
		return max (a, a.m);
	}

	static T max (const ARRAY<T>& a, const int up_to_index)
	{
		T result = a (1);

		for (int index = 2; index <= up_to_index; index++) result = PhysBAM::max (result, a (index));

		return result;
	}

	static T maxabs (const ARRAY<T>& a)
	{
		return maxabs (a, a.m);
	}

	static T maxabs (const ARRAY<T>& a, const int up_to_index)
	{
		T result = (T) fabs (a (1));

		for (int index = 2; index <= up_to_index; index++) result = maxabs_incremental (result, a (index));

		return result;
	}

	static T maxmag (const ARRAY<T>& a)
	{
		return maxmag (a, a.m);
	}

	static T maxmag (const ARRAY<T>& a, const int up_to_index)
	{
		T result = a (1);

		for (int index = 2; index <= up_to_index; index++) result = PhysBAM::maxmag (result, a (index));

		return result;
	}

	static int argmax (const ARRAY<T>& a)
	{
		return argmax (a, a.m);
	}

	static int argmax (const ARRAY<T>& a, const int up_to_index)
	{
		int result = 1;

		for (int index = 2; index <= up_to_index; index++) if (a (index) > a (result)) result = index;

		return result;
	}

	static T min (const ARRAY<T>& a)
	{
		return min (a, a.m);
	}

	static T min (const ARRAY<T>& a, const int up_to_index)
	{
		T result = a (1);

		for (int index = 2; index <= up_to_index; index++) result = PhysBAM::min (result, a (index));

		return result;
	}

	static T minmag (ARRAY<T>& a)
	{
		return minmag (a, a.m);
	}

	static T minmag (ARRAY<T>& a, const int up_to_index)
	{
		T result = a (1);

		for (int index = 2; index <= up_to_index; index++) result = PhysBAM::minmag (result, a (index));

		return result;
	}

	static int argmin (const ARRAY<T>& a)
	{
		return argmin (a, a.m);
	}

	static int argmin (const ARRAY<T>& a, const int up_to_index)
	{
		int result = 1;

		for (int index = 2; index <= up_to_index; index++) if (a (index) < a (result)) result = index;

		return result;
	}

	static T sum (const ARRAY<T>& a)
	{
		return sum (a, a.m);
	}

	static T sum (const ARRAY<T>& a, const int up_to_index)
	{
		T result = T();

		for (int index = 1; index <= up_to_index; index++) result += a (index);

		return result;
	}

	static T sumabs (const ARRAY<T>& a)
	{
		return sumabs (a, a.m);
	}

	static T sumabs (const ARRAY<T>& a, const int up_to_index)
	{
		T result = T();

		for (int index = 1; index <= up_to_index; index++) result += fabs (a (index));

		return result;
	}

	static T Dot_Product (const ARRAY<T>& a1, const ARRAY<T>& a2)
	{
		assert (a1.m == a2.m);
		T result = T();

		for (int index = 1; index <= a1.m; index++) result += a1 (index) * a2 (index);

		return result;
	}

	template<class TS, class T_ARRAY1, class T_ARRAY2>
	static TS Vector_Dot_Product (const T_ARRAY1& a1, const T_ARRAY2& a2)
	{
		STATIC_ASSERT ( (IS_SAME<T, typename T_ARRAY1::ELEMENT>::value && IS_SAME<T, typename T_ARRAY2::ELEMENT>::value));
		assert (a1.m == a2.m);
		TS result = (TS) 0;

		for (int index = 1; index <= a1.m; index++) result += T::Dot_Product (a1 (index), a2 (index));

		return result;
	}

	template<class TS, class T_ARRAY>
	static TS Vector_Magnitude_Squared (const T_ARRAY& a)
	{
		STATIC_ASSERT ( (IS_SAME<T, typename T_ARRAY::ELEMENT>::value));
		TS result = (TS) 0;

		for (int index = 1; index <= a.m; index++) result += a (index).Magnitude_Squared();

		return result;
	}

	template<class TS, class T_ARRAY>
	static TS Vector_Magnitude (const T_ARRAY& a)
	{
		return sqrt (Vector_Magnitude_Squared (a));
	}

	template<class TS>
	static TS Vector_Distance (const ARRAY<T>& a, const ARRAY<T>& b)
	{
		assert (a.m == b.m);
		TS distance = (TS) 0;

		for (int index = 1; index <= a.m; index++) distance += (a (index) - b (index)).Magnitude_Squared();

		return sqrt (distance);
	}

	template<class TS, class T_ARRAY>
	static TS Maximum_Vector_Magnitude (const T_ARRAY& a)
	{
		return Maximum_Vector_Magnitude<TS> (a, a.m);
	}

	template<class TS, class T_ARRAY>
	static TS Maximum_Vector_Magnitude (const T_ARRAY& a, const int up_to_index)
	{
		STATIC_ASSERT ( (IS_SAME<T, typename T_ARRAY::ELEMENT>::value));
		TS result = (TS) 0;

		for (int index = 1; index <= up_to_index; index++) result = PhysBAM::max (result, a (index).Magnitude_Squared());

		return sqrt (result);
	}

	template<class TS>
	static TS Maximum_Pairwise_Vector_Distance (const ARRAY<T>& a, const ARRAY<T>& b)
	{
		assert (a.m == b.m);
		return Maximum_Pairwise_Vector_Distance<TS> (a, b, a.m);
	}

	template<class TS>
	static TS Maximum_Pairwise_Vector_Distance (const ARRAY<T>& a, const ARRAY<T>& b, const int up_to_index)
	{
		TS result = (TS) 0;

		for (int index = 1; index <= up_to_index; index++) result = PhysBAM::max (result, (a (index) - b (index)).Magnitude_Squared());

		return sqrt (result);
	}

	static void find_common_elements (const ARRAY<T>& a, const ARRAY<T>& b, ARRAY<T>& result)
	{
		assert (a.base_pointer != result.base_pointer);
		assert (b.base_pointer != result.base_pointer);
		result.Resize_Array (0);
		int j;

		for (int i = 1; i <= a.m; i++) if (b.Find (a (i), j)) result.Append_Element (a (i));
	}

	static void exchange_arrays (ARRAY<T>& a, ARRAY<T>& b)
	{
		exchange (a.base_pointer, b.base_pointer);
		exchange (a.m, b.m);
	}

	template<class T2>
	static bool Equal_Dimensions (const ARRAY<T>& a, const ARRAY<T2>& b)
	{
		return a.m == b.m;
	}

	static bool Equal_Dimensions (const ARRAY<T>& a, const int m)
	{
		return a.m == m;
	}

	static void heapify (ARRAY<T>& a) // largest on top
	{
		for (int i = a.m / 2; i >= 1; i--) heapify (a, i, a.m);
	}

	static void heapify (ARRAY<T>& a, const int max_index) // largest on top, only does from 1 to max_index
	{
		for (int i = max_index / 2; i >= 1; i--) heapify (a, i, max_index);
	}

	template<class T2>
	static void heapify (ARRAY<T>& a, ARRAY<T2>& aux) // largest on top
	{
		for (int i = a.m / 2; i >= 1; i--) heapify (a, aux, i, a.m);
	}

	template<class T2>
	static void heapify (ARRAY<T>& a, ARRAY<T2>& aux, const int max_index) // largest on top, only does from 1 to max_index
	{
		for (int i = max_index / 2; i >= 1; i--) heapify (a, aux, i, max_index);
	}

	static void heapify (ARRAY<T>& a, int index, const int heap_size) // largest on top, only sorts down from index (not up!)
	{
		int left, right, index_of_largest;

		for (;;)
		{
			left = 2 * index;
			right = 2 * index + 1;
			index_of_largest = index;

			if (left <= heap_size && a (left) > a (index_of_largest)) index_of_largest = left;

			if (right <= heap_size && a (right) > a (index_of_largest)) index_of_largest = right;

			if (index_of_largest != index)
			{
				exchange (a (index), a (index_of_largest));
				index = index_of_largest;
			}
			else return;
		}
	}

	template<class T2>
	static void heapify (ARRAY<T>& a, ARRAY<T2>& aux, int index, const int heap_size) // largest on top, only sorts down from index (not up!)
	{
		int left, right, index_of_largest;

		for (;;)
		{
			left = 2 * index;
			right = 2 * index + 1;
			index_of_largest = index;

			if (left <= heap_size && a (left) > a (index_of_largest)) index_of_largest = left;

			if (right <= heap_size && a (right) > a (index_of_largest)) index_of_largest = right;

			if (index_of_largest != index)
			{
				exchange (a (index), a (index_of_largest));
				exchange (aux (index), aux (index_of_largest));
				index = index_of_largest;
			}
			else return;
		}
	}

	static void sort (ARRAY<T>& a) // from smallest to largest
	{
		heapify (a);

		for (int i = 1; i < a.m; i++)
		{
			exchange (a (1), a (a.m - i + 1));
			heapify (a, 1, a.m - i);
		}
	}

	static void sort (ARRAY<T>& a, const int max_index) // sort a(1:max_index) from smallest to largest
	{
		heapify (a, max_index);

		for (int i = 1; i < max_index; i++)
		{
			exchange (a (1), a (max_index - i + 1));
			heapify (a, 1, max_index - i);
		}
	}

	template<class T2>
	static void sort (ARRAY<T>& a, ARRAY<T2>& aux) // from smallest to largest
	{
		heapify (a, aux);

		for (int i = 1; i < a.m; i++)
		{
			exchange (a (1), a (a.m + 1 - i));
			exchange (aux (1), aux (a.m + 1 - i));
			heapify (a, aux, 1, a.m - i);
		}
	}

	template<class T2>
	static void sort (ARRAY<T>& a, ARRAY<T2>& aux, const int max_index) // sort a(1:max_index) from smallest to largest
	{
		heapify (a, aux, max_index);

		for (int i = 1; i < max_index; i++)
		{
			exchange (a (1), a (max_index + 1 - i));
			exchange (aux (1), aux (max_index + 1 - i));
			heapify (a, aux, 1, max_index - i);
		}
	}

	static void permute (const ARRAY<T>& source, ARRAY<T>& destination, const ARRAY<int>& permutation)
	{
		for (int i = 1; i <= permutation.m; i++) destination (i) = source (permutation (i));
	}

	static void unpermute (const ARRAY<T>& source, ARRAY<T>& destination, const ARRAY<int>& permutation)
	{
		for (int i = 1; i <= permutation.m; i++) destination (permutation (i)) = source (i);
	}

	template<class T2>
	static void Compact_Array_Using_Compaction_Array (ARRAY<T2>& array, ARRAY<int>& compaction_array, ARRAY<T2>* temporary_array = 0)
	{
		bool temporary_array_defined = temporary_array != 0;

		if (!temporary_array_defined) temporary_array = new ARRAY<T2> (compaction_array.m, false);

		ARRAY<T2>::exchange_arrays (array, *temporary_array);

		for (int i = 1; i <= compaction_array.m; i++) if (compaction_array (i) > 0) array (compaction_array (i)) = (*temporary_array) (i);

		if (!temporary_array_defined)
		{
			delete temporary_array;
			temporary_array = 0;
		}
	}

	void Get (const int i, T& element1) const
	{
		assert (Valid_Index (i));
		T* a = base_pointer + i;
		element1 = a[0];
	}

	void Set (const int i, const T& element1)
	{
		assert (Valid_Index (i));
		T* a = base_pointer + i;
		a[0] = element1;
	}

	template<class RW>
	void Read (std::istream& input_stream)
	{
		Deallocate_Base_Pointer();
		Read_Binary<RW> (input_stream, m);
		assert (m >= 0);
		T* array = 0;

		if (m > 0)
		{
			array = new T[m];
			Read_Binary_Array<RW> (input_stream, array, m);
		}

		Set_Base_Pointer (array);
	}

	template<class RW>
	void Write (std::ostream& output_stream) const
	{
		Write_Prefix<RW> (output_stream, m);
	}

	template<class RW>
	void Write_Prefix (std::ostream& output_stream, const int prefix) const
	{
		assert (0 <= prefix && prefix <= m);
		Write_Binary<RW> (output_stream, prefix);
		Write_Binary_Array<RW> (output_stream, Get_Array_Pointer(), prefix);
	}

private:
	// old forms of functions which should not be called!
	ARRAY (const int m_start, const int m_end) {}
	void Resize_Array (const int m_start, const int m_end) {}
	void Resize_Array (const int m_start, const int m_end, const bool initialize_new_elements) {}
	void Resize_Array (const int m_start, const int m_end, const bool initialize_new_elements, const bool copy_existing_elements) {}
//#####################################################################
};
}
#endif

