//#####################################################################
// Copyright 2004, Ronald Fedkiw.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class ARRAYS
//#####################################################################
#ifndef __ARRAYS__
#define __ARRAYS__

#include "ARRAY.h"
namespace PhysBAM
{

template<class T>
class ARRAYS
{
public:
	typedef T ELEMENT;

	int length; // length of the vector
	int m; // end_of_the_array
	T* base_pointer;

	ARRAYS()
		: length (1), m (0)
	{
		Set_Base_Pointer (0);
	}

	ARRAYS (const int length_input, const int m_input, const bool initialize_using_default_constructor = true)
		: length (length_input), m (m_input)
	{
		T *array = new T[length * m];
		Set_Base_Pointer (array);

		if (initialize_using_default_constructor) for (int index = 0, size = length * m; index < size; index++) array[index] = T();
	}

	ARRAYS (const ARRAYS<T>& array_input)
		: length (array_input.length), m (array_input.m)
	{
		T* array = new T[length * m];
		Set_Base_Pointer (array);
		T* old_array = array_input.Get_Array_Pointer();

		for (int index = 0, size = length * m; index < size; index++) array[index] = old_array[index];
	}

	ARRAYS (std::istream& input_stream)
		: length (1), m (0)
	{
		Set_Base_Pointer (0);
		Read<T> (input_stream);
	}

	~ARRAYS()
	{
		Deallocate_Base_Pointer();
	}

	void Clean_Memory()
	{
		Resize_Array (1, 0, false, false);
	}

	T* Allocate_Base_Pointer (const int length_input, const int m_input)
	{
		return (new T[length_input * m_input]) - length_input - 1;
	}

	void Set_Base_Pointer (T* array) // using current length for offset
	{
		base_pointer = array - length - 1;
	}

	T* Get_Array_Pointer() const // using current length for offset
	{
		return base_pointer + length + 1;
	}

	void Deallocate_Base_Pointer() // using current length for offset
	{
		delete[] (base_pointer + length + 1);
	}

	ARRAYS<T>& operator= (const ARRAYS<T>& source)
	{
		if (!Equal_Dimensions (*this, source))
		{
			Deallocate_Base_Pointer();
			length = source.length;
			m = source.m;
			base_pointer = Allocate_Base_Pointer (length, m);
		}

		for (int index = Standard_Begin(), end = Standard_End(); index <= end; index++) base_pointer[index] = source.base_pointer[index];

		return *this;
	}

	T& operator() (const int k, const int i)
	{
		assert (Valid_Index (k, i));
		return * (base_pointer + i * length + k);
	}

	const T& operator() (const int k, const int i) const
	{
		assert (Valid_Index (k, i));
		return * (base_pointer + i * length + k);
	}

	int Standard_Index (const int k, const int i) const
	{
		return i * length + k;
	}

	int Standard_Begin() const
	{
		return Standard_Index (1, 1);
	}

	int Standard_End() const
	{
		return Standard_Index (length, m);
	}

	bool Valid_Index (const int k, const int i) const
	{
		return 1 <= k && k <= length && 1 <= i && i <= m;
	}

	bool Valid_Index (const int i) const
	{
		return 1 <= i && i <= m;
	}

	ARRAYS<T>& operator+= (const ARRAYS<T>& v)
	{
		assert (Equal_Dimensions (*this, v));

		for (int index = Standard_Begin(), end = Standard_End(); index <= end; index++) base_pointer[index] += v.base_pointer[index];

		return *this;
	}

	ARRAYS<T>& operator+= (const T& a)
	{
		for (int index = Standard_Begin(), end = Standard_End(); index <= end; index++) base_pointer[index] += a;

		return *this;
	}

	ARRAYS<T>& operator-= (const ARRAYS<T>& v)
	{
		assert (Equal_Dimensions (*this, v));

		for (int index = Standard_Begin(), end = Standard_End(); index <= end; index++) base_pointer[index] -= v.base_pointer[index];

		return *this;
	}

	ARRAYS<T>& operator-= (const T& a)
	{
		for (int index = Standard_Begin(), end = Standard_End(); index <= end; index++) base_pointer[index] -= a;

		return *this;
	}

	template<class T2>
	ARRAYS<T>& operator*= (const ARRAYS<T2>& v)
	{
		assert (Equal_Dimensions (*this, v));

		for (int index = Standard_Begin(), end = Standard_End(); index <= end; index++) base_pointer[index] *= v.base_pointer[index];

		return *this;
	}

	template<class T2>
	ARRAYS<T>& operator*= (const T2 a)
	{
		for (int index = Standard_Begin(), end = Standard_End(); index <= end; index++) base_pointer[index] *= a;

		return *this;
	}

	template<class T2>
	ARRAYS<T>& operator/= (const T2 a)
	{
		T2 one_over_a = 1 / a;

		for (int index = Standard_Begin(), end = Standard_End(); index <= end; index++) base_pointer[index] *= one_over_a;

		return *this;
	}

	template<class T2>
	ARRAYS<T>& operator/= (const ARRAYS<T2>& a)
	{
		for (int index = Standard_Begin(), end = Standard_End(); index <= end; index++)
		{
			assert (a.base_pointer[index] > 0);
			base_pointer[index] /= a.base_pointer[index];
		}

		return *this;
	}

	void Resize_Array (const int length_new, const int m_new, const bool initialize_new_elements = true, const bool copy_existing_elements = true, const T& initialization_value = T())
	{
		if (Equal_Dimensions (*this, length_new, m_new)) return;

		T* base_pointer_new = Allocate_Base_Pointer (length_new, m_new);

		if (copy_existing_elements)
		{
			int length_min = PhysBAM::min (length, length_new), m_end = PhysBAM::min (m, m_new);

			for (int i = 1; i <= m_end; i++) for (int k = 1; k <= length_min; k++) base_pointer_new[i * length_new + k] = base_pointer[i * length + k];
		}

		if (initialize_new_elements)
		{
			int length_min = PhysBAM::min (length, length_new), m_end = PhysBAM::min (m, m_new);

			if (length_new > length) for (int i = 1; i <= m_end; i++) for (int k = length_min + 1; k <= length_new; k++) base_pointer_new[i * length_new + k] = initialization_value;

			for (int index = (m_end + 1) * length_new + 1, end = m_new * length_new + length_new; index <= end; index++) base_pointer_new[index] = initialization_value;
		}

		Deallocate_Base_Pointer();
		length = length_new;
		m = m_new;
		base_pointer = base_pointer_new;
	}

	void Append_Elements (const ARRAYS<T>& append_array)
	{
		int index = m;
		Resize_Array (length, m + append_array.m);

		for (int i = 1; i <= append_array.m; i++)
		{
			index++;

			for (int k = 1; k <= length; k++) (*this) (k, index) = append_array (k, i);
		}
	}

	void Remove_Index (const int index) // maintains ordering of remaining elements
	{
		assert (Valid_Index (index));
		T* base_pointer_new = Allocate_Base_Pointer (length, m - 1);
		int i, cut = Standard_Index (1, index);

		for (i = Standard_Index (1, 1); i < cut; i++) base_pointer_new[i] = base_pointer[i];

		for (int j = i + length, end = Standard_Index (length, m - 1); i <= end; i++, j++) base_pointer_new[i] = base_pointer[j];

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

			for (int i = index (kk) + 1 - kk; i <= index (kk + 1) - 1 - kk; i++) for (int k = 1; k <= length; k++) (*this) (k, i) = (*this) (k, i + kk);
		}

		for (int i = index (index.m) + 1 - index.m; i <= m - index.m; i++) for (int k = 1; k <= length; k++) (*this) (k, i) = (*this) (k, i + index.m);

		Resize_Array (length, m - index.m);
	}

	static void copy (const T& constant, ARRAYS<T>& new_copy)
	{
		for (int index = new_copy.Standard_Begin(), end = new_copy.Standard_End(); index <= end; index++) new_copy.base_pointer[index] = constant;
	}

	static void copy (const ARRAYS<T>& old_copy, ARRAYS<T>& new_copy)
	{
		assert (Equal_Dimensions (old_copy, new_copy));

		for (int index = new_copy.Standard_Begin(), end = new_copy.Standard_End(); index <= end; index++) new_copy.base_pointer[index] = old_copy.base_pointer[index];
	}

	template<class T2>
	static void copy (const T2 constant, const ARRAYS<T>& old_copy, ARRAYS<T>& new_copy)
	{
		assert (Equal_Dimensions (old_copy, new_copy));

		for (int index = new_copy.Standard_Begin(), end = new_copy.Standard_End(); index <= end; index++) new_copy.base_pointer[index] = constant * old_copy.base_pointer[index];
	}

	template<class T2>
	static void copy (const T2 c1, const ARRAYS<T>& v1, const ARRAYS<T>& v2, ARRAYS<T>& result)
	{
		assert (Equal_Dimensions (v1, v2) && Equal_Dimensions (v2, result));

		for (int index = result.Standard_Begin(), end = result.Standard_End(); index <= end; index++) result.base_pointer[index] = c1 * v1.base_pointer[index] + v2.base_pointer[index];
	}

	template<class T2>
	static void copy (const T2 c1, const ARRAYS<T>& v1, const T2 c2, const ARRAYS<T>& v2, ARRAYS<T>& result)
	{
		assert (Equal_Dimensions (v1, v2) && Equal_Dimensions (v2, result));

		for (int index = result.Standard_Begin(), end = result.Standard_End(); index <= end; index++) result.base_pointer[index] = c1 * v1.base_pointer[index] + c2 * v2.base_pointer[index];
	}

	static void get (ARRAYS<T>& new_copy, const ARRAYS<T>& old_copy)
	{
		ARRAYS<T>::put (old_copy, new_copy, new_copy.length, new_copy.m);
	}

	static void put (const ARRAYS<T>& old_copy, ARRAYS<T>& new_copy)
	{
		ARRAYS<T>::put (old_copy, new_copy, old_copy.length, old_copy.m);
	}

	template<class T2>
	static void put (const T2 constant, const ARRAYS<T>& old_copy, ARRAYS<T>& new_copy)
	{
		ARRAYS<T>::put (constant, old_copy, new_copy, old_copy.length, old_copy.m);
	}

	static void put (const ARRAYS<T>& old_copy, ARRAYS<T>& new_copy, const int length, const int m)
	{
		if (length == old_copy.length && old_copy.length == new_copy.length)
		{
			assert (m <= old_copy.m);
			assert (m <= new_copy.m);
			T *new_array = new_copy.Get_Array_Pointer(), *old_array = old_copy.Get_Array_Pointer();

			for (int t = 0, size = length * m; t < size; t++) new_array[t] = old_array[t];
		}
		else for (int i = 1; i <= m; i++) for (int k = 1; k <= length; k++) new_copy (k, i) = old_copy (k, i);
	}

	template<class T2>
	static void put (T2 constant, const ARRAYS<T>& old_copy, ARRAYS<T>& new_copy, const int length, const int m)
	{
		if (length == old_copy.length && old_copy.length == new_copy.length)
		{
			assert (m <= old_copy.m);
			assert (m <= new_copy.m);
			T *new_array = new_copy.Get_Array_Pointer(), *old_array = old_copy.Get_Array_Pointer();

			for (int t = 0, size = length * m; t < size; t++) new_array[t] = constant * old_array[t];
		}
		else for (int i = 1; i <= m; i++) for (int k = 1; k <= length; k++) new_copy (k, i) = constant * old_copy (k, i);
	}

	static void exchange_arrays (ARRAYS<T>& a, ARRAYS<T>& b)
	{
		exchange (a.base_pointer, b.base_pointer);
		exchange (a.length, b.length);
		exchange (a.m, b.m);
	}

	template<class T2>
	static bool Equal_Dimensions (const ARRAYS<T>& a, const ARRAYS<T2>& b)
	{
		return a.length == b.length && a.m == b.m;
	}

	static bool Equal_Dimensions (const ARRAYS<T>& a, const int length, const int m)
	{
		return a.length == length && a.m == m;
	}

	void Get (const int i, T& element1, T& element2) const
	{
		assert (length == 2);
		assert (Valid_Index (i));
		T* a = base_pointer + i * 2 + 1;
		element1 = a[0];
		element2 = a[1];
	}

	void Get (const int i, T& element1, T& element2, T& element3) const
	{
		assert (length == 3);
		assert (Valid_Index (i));
		T* a = base_pointer + i * 3 + 1;
		element1 = a[0];
		element2 = a[1];
		element3 = a[2];
	}

	void Get (const int i, T& element1, T& element2, T& element3, T& element4) const
	{
		assert (length == 4);
		assert (Valid_Index (i));
		T* a = base_pointer + i * 4 + 1;
		element1 = a[0];
		element2 = a[1];
		element3 = a[2];
		element4 = a[3];
	}

	void Get (const int i, T& element1, T& element2, T& element3, T& element4, T& element5) const
	{
		assert (length == 5);
		assert (Valid_Index (i));
		T* a = base_pointer + i * 5 + 1;
		element1 = a[0];
		element2 = a[1];
		element3 = a[2];
		element4 = a[3];
		element5 = a[4];
	}

	void Get (const int i, T& element1, T& element2, T& element3, T& element4, T& element5, T& element6) const
	{
		assert (length == 6);
		assert (Valid_Index (i));
		T* a = base_pointer + i * 6 + 1;
		element1 = a[0];
		element2 = a[1];
		element3 = a[2];
		element4 = a[3];
		element5 = a[4];
		element6 = a[5];
	}

	void Get (const int i, T& element1, T& element2, T& element3, T& element4, T& element5, T& element6, T& element7) const
	{
		assert (length == 7);
		assert (Valid_Index (i));
		T* a = base_pointer + i * 7 + 1;
		element1 = a[0];
		element2 = a[1];
		element3 = a[2];
		element4 = a[3];
		element5 = a[4];
		element6 = a[5];
		element7 = a[6];
	}

	void Get (const int i, T& element1, T& element2, T& element3, T& element4, T& element5, T& element6, T& element7, T& element8) const
	{
		assert (length == 8);
		assert (Valid_Index (i));
		T* a = base_pointer + i * 8 + 1;
		element1 = a[0];
		element2 = a[1];
		element3 = a[2];
		element4 = a[3];
		element5 = a[4];
		element6 = a[5];
		element7 = a[6];
		element8 = a[7];
	}

	void Set (const int i, const T& element1, const T& element2)
	{
		assert (length == 2);
		assert (Valid_Index (i));
		T* a = base_pointer + i * 2 + 1;
		a[0] = element1;
		a[1] = element2;
	}

	void Set (const int i, const T& element1, const T& element2, const T& element3)
	{
		assert (length == 3);
		assert (Valid_Index (i));
		T* a = base_pointer + i * 3 + 1;
		a[0] = element1;
		a[1] = element2;
		a[2] = element3;
	}

	void Set (const int i, const T& element1, const T& element2, const T& element3, const T& element4)
	{
		assert (length == 4);
		assert (Valid_Index (i));
		T* a = base_pointer + i * 4 + 1;
		a[0] = element1;
		a[1] = element2;
		a[2] = element3;
		a[3] = element4;
	}

	void Set (const int i, const T& element1, const T& element2, const T& element3, const T& element4, const T& element5)
	{
		assert (length == 5);
		assert (Valid_Index (i));
		T* a = base_pointer + i * 5 + 1;
		a[0] = element1;
		a[1] = element2;
		a[2] = element3;
		a[3] = element4;
		a[4] = element5;
	}

	void Set (const int i, const T& element1, const T& element2, const T& element3, const T& element4, const T& element5, const T& element6)
	{
		assert (length == 6);
		assert (Valid_Index (i));
		T* a = base_pointer + i * 6 + 1;
		a[0] = element1;
		a[1] = element2;
		a[2] = element3;
		a[3] = element4;
		a[4] = element5;
		a[5] = element6;
	}

	void Set (const int i, const T& element1, const T& element2, const T& element3, const T& element4, const T& element5, const T& element6, const T& element7)
	{
		assert (length == 7);
		assert (Valid_Index (i));
		T* a = base_pointer + i * 7 + 1;
		a[0] = element1;
		a[1] = element2;
		a[2] = element3;
		a[3] = element4;
		a[4] = element5;
		a[5] = element6;
		a[6] = element7;
	}

	void Set (const int i, const T& element1, const T& element2, const T& element3, const T& element4, const T& element5, const T& element6, const T& element7, const T& element8)
	{
		assert (length == 8);
		assert (Valid_Index (i));
		T* a = base_pointer + i * 8 + 1;
		a[0] = element1;
		a[1] = element2;
		a[2] = element3;
		a[3] = element4;
		a[4] = element5;
		a[5] = element6;
		a[6] = element7;
		a[7] = element8;
	}

	template<class RW>
	void Read (std::istream& input_stream)
	{
		Deallocate_Base_Pointer();
		Read_Binary<RW> (input_stream, length, m);
		assert (length > 0);
		assert (m >= 0);
		T* array = 0;

		if (length * m > 0)
		{
			array = new T[length * m];
			Read_Binary_Array<RW> (input_stream, array, length * m);
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
		Write_Binary<RW> (output_stream, length, prefix);
		Write_Binary_Array<RW> (output_stream, Get_Array_Pointer(), length * prefix);
	}

private:
	// old forms of functions which should not be called!
	ARRAYS (const int length_input, const int m_start, const int m_end) {}
	void Resize_Array (const int length_new, const int m_start, const int m_end) {}
	void Resize_Array (const int length_new, const int m_start, const int m_end, const bool initialize_new_elements) {}
	void Resize_Array (const int length_new, const int m_start, const int m_end, const bool initialize_new_elements, const bool copy_existing_elements) {}
//#####################################################################
};
}
#endif

