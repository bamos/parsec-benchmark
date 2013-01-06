//#####################################################################
// Copyright 2002-2004, Robert Bridson, Doug Enright, Ronald Fedkiw, Eran Guendelman, Geoffrey Irving, Duc Nguyen, Andrew Selle.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class ARRAYS_2D
//#####################################################################
#ifndef __ARRAYS_2D__
#define __ARRAYS_2D__

#include <assert.h>
#include <iostream>
#include "../Math_Tools/max.h"
#include "../Math_Tools/maxabs.h"
#include "../Math_Tools/maxmag.h"
#include "../Math_Tools/min.h"
#include "../Math_Tools/minmag.h"
#include "../Math_Tools/clamp.h"
#include "../Math_Tools/exchange.h"
#include "../Matrices_And_Vectors/VECTOR_2D.h"
#include "../Read_Write/READ_WRITE_FUNCTIONS.h"
#include "../Grids/GRID_2D.h"
namespace PhysBAM
{

template<class T>
class ARRAYS_2D
{
public:
	T* array; // pointer to the array
	int length; // length of the vector
	int m_start, m_end, n_start, n_end; // starting and ending values for x and y direction
	int m, n; // used in overloaded () operator
	int size; // total length of the one dimensional array
private:
	int n_plus_one, n_minus_one;
	T* base_pointer;

public:
	ARRAYS_2D()
		: array (0), length (1), m_start (1), m_end (0), n_start (1), n_end (0), m (0), n (0), size (0)
	{
		Calculate_Acceleration_Constants();
	}

	ARRAYS_2D (const int m_start_input, const int m_end_input, const int n_start_input, const int n_end_input, const bool initialize_using_default_constructor = true)
		: length (1), m_start (m_start_input), m_end (m_end_input), n_start (n_start_input), n_end (n_end_input), m (m_end - m_start + 1), n (n_end - n_start + 1), size (m*n)
	{
		array = new T[size]; // allocate a new array
		Calculate_Acceleration_Constants();

		if (initialize_using_default_constructor) for (int index = 0; index < size; index++) array[index] = T(); // initialize array using default constructor
	}

	ARRAYS_2D (const int length_input, const int m_start_input, const int m_end_input, const int n_start_input, const int n_end_input, const bool initialize_using_default_constructor = true)
		: length (length_input), m_start (m_start_input), m_end (m_end_input), n_start (n_start_input), n_end (n_end_input), m (m_end - m_start + 1), n (n_end - n_start + 1), size (length*m*n)
	{
		array = new T[size]; // allocate a new array
		Calculate_Acceleration_Constants();

		if (initialize_using_default_constructor) for (int index = 0; index < size; index++) array[index] = T(); // initialize array using default constructor
	}

	template<class T2>
	ARRAYS_2D (const GRID_2D<T2>& grid, const int ghost_cells = 0, const bool initialize_using_default_constructor = true)
		: length (1), m_start (1 - ghost_cells), m_end (grid.m + ghost_cells), n_start (1 - ghost_cells), n_end (grid.n + ghost_cells), m (m_end - m_start + 1), n (n_end - n_start + 1), size (m*n)
	{
		array = new T[size]; // allocate a new array
		Calculate_Acceleration_Constants();

		if (initialize_using_default_constructor) for (int index = 0; index < size; index++) array[index] = T(); // initialize array using default constructor
	}

	template<class T2>
	ARRAYS_2D (const int length_input, const GRID_2D<T2>& grid, const int ghost_cells = 0, const bool initialize_using_default_constructor = true)
		: length (length_input), m_start (1 - ghost_cells), m_end (grid.m + ghost_cells), n_start (1 - ghost_cells), n_end (grid.n + ghost_cells), m (m_end - m_start + 1), n (n_end - n_start + 1), size (length*m*n)
	{
		array = new T[size]; // allocate a new array
		Calculate_Acceleration_Constants();

		if (initialize_using_default_constructor) for (int index = 0; index < size; index++) array[index] = T(); // initialize array using default constructor
	}

	ARRAYS_2D (const ARRAYS_2D<T>& old_array, const bool initialize_with_old_array = true)
		: length (old_array.length), m_start (old_array.m_start), m_end (old_array.m_end), n_start (old_array.n_start), n_end (old_array.n_end), m (old_array.m), n (old_array.n), size (old_array.size)
	{
		array = new T[size]; // allocate a new array
		Calculate_Acceleration_Constants();

		if (initialize_with_old_array) for (int index = 0; index < size; index++) array[index] = old_array.array[index];
	}

	~ARRAYS_2D()
	{
		delete [] array;
	}

	void Clean_Memory()
	{
		Resize_Array (1, 0, 1, 0, false, false);
	}

	void Delete_Pointers_And_Clean_Memory() // only valid if T is a pointer type
	{
		for (int index = 0; index < size; index++) delete array[index];

		Clean_Memory();
	}

	void Calculate_Acceleration_Constants()
	{
		n_plus_one = n + 1;
		n_minus_one = n - 1;
		base_pointer = array ? array + ( (-m_start * n - n_start) * length) : 0;
	}

	ARRAYS_2D<T>& operator= (const ARRAYS_2D<T>& source)
	{
		if (size != source.size)
		{
			delete [] array;
			size = source.size;
			array = new T[source.size];
		}

		length = source.length;
		m = source.m;
		m_start = source.m_start;
		m_end = source.m_end;
		n = source.n;
		n_start = source.n_start;
		n_end = source.n_end;
		Calculate_Acceleration_Constants();

		for (int index = 0; index < size; index++) array[index] = source.array[index];

		return *this;
	}

	T& operator() (const int i, const int j)
	{
		assert (length == 1);
		assert (m_start <= i && i <= m_end);
		assert (n_start <= j && j <= n_end);
		return * (base_pointer + (i * n + j));
	}

	const T& operator() (const int i, const int j) const
	{
		assert (length == 1);
		assert (m_start <= i && i <= m_end);
		assert (n_start <= j && j <= n_end);
		return * (base_pointer + (i * n + j));
	}

	T& operator() (const VECTOR_2D<int>& index)
	{
		assert (length == 1);
		assert (m_start <= index.x && index.x <= m_end);
		assert (n_start <= index.y && index.y <= n_end);
		return * (base_pointer + (index.x * n + index.y));
	}

	const T& operator() (const VECTOR_2D<int>& index) const
	{
		assert (length == 1);
		assert (m_start <= index.x && index.x <= m_end);
		assert (n_start <= index.y && index.y <= n_end);
		return * (base_pointer + (index.x * n + index.y));
	}

	T& operator() (const int k, const int i, const int j)
	{
		assert (1 <= k && k <= length);
		assert (m_start <= i && i <= m_end);
		assert (n_start <= j && j <= n_end);
		return * (base_pointer + (i * n + j) * length + (k - 1));
	}

	const T& operator() (const int k, const int i, const int j) const
	{
		assert (1 <= k && k <= length);
		assert (m_start <= i && i <= m_end);
		assert (n_start <= j && j <= n_end);
		return * (base_pointer + (i * n + j) * length + (k - 1));
	}

	bool Valid_Index (const VECTOR_2D<int>& index) const
	{
		return m_start <= index.x && index.x <= m_end && n_start <= index.y && index.y <= n_end;
	}

	bool Valid_Index (const int i, const int j) const
	{
		return m_start <= i && i <= m_end && n_start <= j && j <= n_end;
	}

	bool Valid_Index (const int k, const int i, const int j) const
	{
		return 1 <= k && k <= length && m_start <= i && i <= m_end && n_start <= j && j <= n_end;
	}

	int Standard_Index (const int i, const int j) const
	{
		assert (length == 1);
		assert (m_start <= i && i <= m_end);
		assert (n_start <= j && j <= n_end);
		return (i - m_start) * n + (j - n_start);
	}

	int Standard_Index (const VECTOR_2D<int>& index) const
	{
		assert (length == 1);
		assert (m_start <= index.x && index.x <= m_end);
		assert (n_start <= index.y && index.y <= n_end);
		return (index.x - m_start) * n + (index.y - n_start);
	}

	int I_Plus_One (const int index) const
	{
		assert (index + n >= 0 && index + n < size);
		return index + n;
	}

	int I_Minus_One (const int index) const
	{
		assert (index - n >= 0 && index - n < size);
		return index - n;
	}

	int J_Plus_One (const int index) const
	{
		assert (index + 1 >= 0 && index + 1 < size);
		return index + 1;
	}

	int J_Minus_One (const int index) const
	{
		assert (index - 1 >= 0 && index - 1 < size);
		return index - 1;
	}

	int I_Plus_One_J_Plus_One (const int index) const
	{
		assert (index + n_plus_one >= 0 && index + n_plus_one < size);
		return index + n_plus_one;
	}

	int I_Plus_One_J_Minus_One (const int index) const
	{
		assert (index + n_minus_one >= 0 && index + n_minus_one < size);
		return index + n_minus_one;
	}

	int I_Minus_One_J_Plus_One (const int index) const
	{
		assert (index - n_minus_one >= 0 && index - n_minus_one < size);
		return index - n_minus_one;
	}

	int I_Minus_One_J_Minus_One (const int index) const
	{
		assert (index - n_plus_one >= 0 && index - n_plus_one < size);
		return index - n_plus_one;
	}

	ARRAYS_2D<T>& operator+= (const ARRAYS_2D<T>& v)
	{
		assert (Equal_Dimensions (*this, v));

		for (int index = 0; index < size; index++) array[index] += v.array[index];

		return *this;
	}

	ARRAYS_2D<T>& operator+= (const T& a)
	{
		for (int index = 0; index < size; index++) array[index] += a;

		return *this;
	}

	ARRAYS_2D<T>& operator-= (const ARRAYS_2D<T>& v)
	{
		assert (Equal_Dimensions (*this, v));

		for (int index = 0; index < size; index++) array[index] -= v.array[index];

		return *this;
	}

	ARRAYS_2D<T>& operator-= (const T& a)
	{
		for (int index = 0; index < size; index++) array[index] -= a;

		return *this;
	}

	template<class T2>
	ARRAYS_2D<T>& operator*= (const ARRAYS_2D<T2>& v)
	{
		assert (Equal_Dimensions (*this, v));

		for (int index = 0; index < size; index++) array[index] *= v.array[index];

		return *this;
	}

	template<class T2>
	ARRAYS_2D<T>& operator*= (const T2 a)
	{
		for (int index = 0; index < size; index++) array[index] *= a;

		return *this;
	}

	template<class T2>
	ARRAYS_2D<T>& operator/= (const T2 a)
	{
		T2 one_over_a = 1 / a;

		for (int index = 0; index < size; index++) array[index] *= one_over_a;

		return *this;
	}

	void Resize_Array (int m_start_new, int m_end_new, int n_start_new, int n_end_new, const bool initialize_new_elements = true, const bool copy_existing_elements = true, const T& initialization_value = T())
	{
		if (m_start_new == m_start && m_end_new == m_end && n_start_new == n_start && n_end_new == n_end) return;

		int m_new = m_end_new - m_start_new + 1, n_new = n_end_new - n_start_new + 1;
		size = length * m_new * n_new;
		T* array_new = new T[size];

		if (initialize_new_elements) for (int index = 0; index < size; index++) array_new[index] = initialization_value;

		if (copy_existing_elements)
		{
			int m1 = PhysBAM::max (m_start, m_start_new), m2 = PhysBAM::min (m_end, m_end_new), n1 = PhysBAM::max (n_start, n_start_new), n2 = PhysBAM::min (n_end, n_end_new);

			for (int i = m1; i <= m2; i++) for (int j = n1; j <= n2; j++) for (int k = 1; k <= length; k++)
						array_new[ ( (i - m_start_new) *n_new + (j - n_start_new)) *length + (k - 1)] = array[ ( (i - m_start) * n + (j - n_start)) * length + (k - 1)];
		}

		m_start = m_start_new;
		m_end = m_end_new;
		m = m_end - m_start + 1;
		n_start = n_start_new;
		n_end = n_end_new;
		n = n_end - n_start + 1;
		delete [] array;
		array = array_new;
		Calculate_Acceleration_Constants();
	}

	void Resize_Array (int length_new, int m_start_new, int m_end_new, int n_start_new, int n_end_new, const bool initialize_new_elements = true, const bool copy_existing_elements = true, const T& initialization_value = T())
	{
		if (length_new == length && m_start_new == m_start && m_end_new == m_end && n_start_new == n_start && n_end_new == n_end) return;

		int m_new = m_end_new - m_start_new + 1, n_new = n_end_new - n_start_new + 1;
		size = length_new * m_new * n_new;
		T* array_new = new T[size];

		if (initialize_new_elements) for (int index = 0; index < size; index++) array_new[index] = initialization_value;

		if (copy_existing_elements)
		{
			int length_min = PhysBAM::min (length, length_new), m1 = PhysBAM::max (m_start, m_start_new), m2 = PhysBAM::min (m_end, m_end_new), n1 = PhysBAM::max (n_start, n_start_new),
			    n2 = PhysBAM::min (n_end, n_end_new);

			for (int i = m1; i <= m2; i++) for (int j = n1; j <= n2; j++) for (int k = 1; k <= length_min; k++)
						array_new[ ( (i - m_start_new) *n_new + (j - n_start_new)) *length_new + (k - 1)] = array[ ( (i - m_start) * n + (j - n_start)) * length + (k - 1)];
		}

		length = length_new;
		m_start = m_start_new;
		m_end = m_end_new;
		m = m_end - m_start + 1;
		n_start = n_start_new;
		n_end = n_end_new;
		n = n_end - n_start + 1;
		delete [] array;
		array = array_new;
		Calculate_Acceleration_Constants();
	}

	template<class T2>
	void Resize_Array (const GRID_2D<T2>& grid, const int ghost_cells = 0, const bool initialize_new_elements = true, const bool copy_existing_elements = true, const T& initialization_value = T())
	{
		Resize_Array (1 - ghost_cells, grid.m + ghost_cells, 1 - ghost_cells, grid.n + ghost_cells, initialize_new_elements, copy_existing_elements, initialization_value);
	}

	template<class T2>
	void Resize_Array (const int length_new, const GRID_2D<T2>& grid, const int ghost_cells = 0, const bool initialize_new_elements = true, const bool copy_existing_elements = true, const T& initialization_value = T())
	{
		Resize_Array (length_new, 1 - ghost_cells, grid.m + ghost_cells, 1 - ghost_cells, grid.n + ghost_cells, initialize_new_elements, copy_existing_elements, initialization_value);
	}

	void Clamp (int& i, int& j) const
	{
		i = clamp (i, m_start, m_end);
		j = clamp (j, n_start, n_end);
	}

	void Clamp_End_Minus_One (int& i, int& j) const
	{
		i = clamp (i, m_start, m_end - 1);
		j = clamp (j, n_start, n_end - 1);
	}

	void Clamp_End_Minus_Two (int& i, int& j) const
	{
		i = clamp (i, m_start, m_end - 2);
		j = clamp (j, n_start, n_end - 2);
	}

	void Clamp_End_Minus_Three (int& i, int& j) const
	{
		i = clamp (i, m_start, m_end - 3);
		j = clamp (j, n_start, n_end - 3);
	}

	void Clamp_Interior (int& i, int& j) const
	{
		i = clamp (i, m_start + 1, m_end - 1);
		j = clamp (j, n_start + 1, n_end - 1);
	}

	void Clamp_Interior_End_Minus_One (int& i, int& j) const
	{
		i = clamp (i, m_start + 1, m_end - 2);
		j = clamp (j, n_start + 1, n_end - 2);
	}

	int Number_True() const
	{
		int count = 0;

		for (int index = 0; index < size; index++) if (array[index]) count++;

		return count;
	}

	static void copy (const T& constant, ARRAYS_2D<T>& new_copy)
	{
		for (int index = 0; index < new_copy.size; index++) new_copy.array[index] = constant;
	}

	static void copy (const ARRAYS_2D<T>& old_copy, ARRAYS_2D<T>& new_copy)
	{
		assert (Equal_Dimensions (old_copy, new_copy));

		for (int index = 0; index < old_copy.size; index++) new_copy.array[index] = old_copy.array[index];
	}

	template<class T2>
	static void copy (const T2 constant, const ARRAYS_2D<T>& old_copy, ARRAYS_2D<T>& new_copy)
	{
		assert (Equal_Dimensions (old_copy, new_copy));

		for (int index = 0; index < old_copy.size; index++) new_copy.array[index] = constant * old_copy.array[index];
	}

	template<class T2>
	static void copy (const T2 c1, const ARRAYS_2D<T>& v1, const ARRAYS_2D<T>& v2, ARRAYS_2D<T>& result)
	{
		assert (Equal_Dimensions (v1, v2) && Equal_Dimensions (v2, result));

		for (int index = 0; index < result.size; index++) result.array[index] = c1 * v1.array[index] + v2.array[index];
	}

	template<class T2>
	static void copy (const T2 c1, const ARRAYS_2D<T>& v1, const T2 c2, const ARRAYS_2D<T>& v2, ARRAYS_2D<T>& result)
	{
		assert (Equal_Dimensions (v1, v2) && Equal_Dimensions (v2, result));

		for (int index = 0; index < result.size; index++) result.array[index] = c1 * v1.array[index] + c2 * v2.array[index];
	}

	template<class T2>
	static void copy (const T2 c1, const ARRAYS_2D<T>& v1, const T2 c2, const ARRAYS_2D<T>& v2, const T2 c3, const ARRAYS_2D<T>& v3, ARRAYS_2D<T>& result)
	{
		assert (Equal_Dimensions (v1, v2) && Equal_Dimensions (v2, v3) && Equal_Dimensions (v3, result));

		for (int index = 0; index < result.size; index++) result.array[index] = c1 * v1.array[index] + c2 * v2.array[index] + c3 * v3.array[index];
	}

	static void get (ARRAYS_2D<T>& new_copy, const ARRAYS_2D<T>& old_copy)
	{
		ARRAYS_2D<T>::put (old_copy, new_copy, new_copy.length, new_copy.m_start, new_copy.m_end, new_copy.n_start, new_copy.n_end);
	}

	static void put (const ARRAYS_2D<T>& old_copy, ARRAYS_2D<T>& new_copy)
	{
		ARRAYS_2D<T>::put (old_copy, new_copy, old_copy.length, old_copy.m_start, old_copy.m_end, old_copy.n_start, old_copy.n_end);
	}

	template<class T2>
	static void put (const T2 constant, const ARRAYS_2D<T>& old_copy, ARRAYS_2D<T>& new_copy)
	{
		ARRAYS_2D<T>::put (constant, old_copy, new_copy, old_copy.length, old_copy.m_start, old_copy.m_end, old_copy.n_start, old_copy.n_end);
	}

	static void put (const ARRAYS_2D<T>& old_copy, ARRAYS_2D<T>& new_copy, const int length, const int m_start, const int m_end, const int n_start, const int n_end)
	{
		if (length == 1 && length == old_copy.length && old_copy.length == new_copy.length)
		{
			// currently only optimized if everything is length 1
			assert (old_copy.m_start <= m_start && m_end <= old_copy.m_end && old_copy.n_start <= n_start && n_end <= old_copy.n_end);
			assert (new_copy.m_start <= m_start && m_end <= new_copy.m_end && new_copy.n_start <= n_start && n_end <= new_copy.n_end);
			int old_index = old_copy.Standard_Index (m_start, n_start), new_index = new_copy.Standard_Index (m_start, n_start);
			const int inner_loop_copy_size = (n_end - n_start + 1);

			for (int i = m_start; i < m_end; i++)
			{
				int old_save_index = old_index, new_save_index = new_index;

				for (int t = 1; t <= inner_loop_copy_size; t++) new_copy.array[new_index++] = old_copy.array[old_index++];

				old_index = old_copy.I_Plus_One (old_save_index);
				new_index = new_copy.I_Plus_One (new_save_index);
			}

			for (int t = 1; t <= inner_loop_copy_size; t++) new_copy.array[new_index++] = old_copy.array[old_index++];
		} // unroll last iteration so we don't call I_Plus_One beyond end of array
		else for (int i = m_start; i <= m_end; i++) for (int j = n_start; j <= n_end; j++) for (int k = 1; k <= length; k++) new_copy (k, i, j) = old_copy (k, i, j);
	}

	template<class T2>
	static void put (T2 constant, const ARRAYS_2D<T>& old_copy, ARRAYS_2D<T>& new_copy, const int length, const int m_start, const int m_end, const int n_start, const int n_end)
	{
		if (length == 1 && length == old_copy.length && old_copy.length == new_copy.length)
		{
			// currently only optimized if everything is length 1
			assert (old_copy.m_start <= m_start && m_end <= old_copy.m_end && old_copy.n_start <= n_start && n_end <= old_copy.n_end);
			assert (new_copy.m_start <= m_start && m_end <= new_copy.m_end && new_copy.n_start <= n_start && n_end <= new_copy.n_end);
			int old_index = old_copy.Standard_Index (m_start, n_start), new_index = new_copy.Standard_Index (m_start, n_start);
			const int inner_loop_copy_size = (n_end - n_start + 1);

			for (int i = m_start; i < m_end; i++)
			{
				int old_save_index = old_index, new_save_index = new_index;

				for (int t = 1; t <= inner_loop_copy_size; t++) new_copy.array[new_index++] = constant * old_copy.array[old_index++];

				old_index = old_copy.I_Plus_One (old_save_index);
				new_index = new_copy.I_Plus_One (new_save_index);
			}

			for (int t = 1; t <= inner_loop_copy_size; t++) new_copy.array[new_index++] = constant * old_copy.array[old_index++];
		} // unroll last iteration so we don't call I_Plus_One beyond end of array
		else for (int i = m_start; i <= m_end; i++) for (int j = n_start; j <= n_end; j++) for (int k = 1; k <= length; k++) new_copy (k, i, j) = constant * old_copy (k, i, j);
	}

	template<class TCONSTANT, class TGRID>
	static void put_ghost (const TCONSTANT constant, ARRAYS_2D<T>& x, const GRID_2D<TGRID>& grid, const int ghost_cells)
	{
		if (x.length == 1)
		{
			for (int j = 1 - ghost_cells; j <= grid.n + ghost_cells; j++) for (int s = 1; s <= ghost_cells; s++) x (1 - s, j) = x (grid.m + s, j) = constant;

			for (int i = 1; i <= grid.m; i++) for (int s = 1; s <= ghost_cells; s++) x (i, 1 - s) = x (i, grid.n + s) = constant;
		}
		else
		{
			for (int j = 1 - ghost_cells; j <= grid.n + ghost_cells; j++) for (int s = 1; s <= ghost_cells; s++) for (int k = 1; k <= x.length; k++) x (k, 1 - s, j) = x (k, grid.m + s, j) = constant;

			for (int i = 1; i <= grid.m; i++) for (int s = 1; s <= ghost_cells; s++) for (int k = 1; k <= x.length; k++) x (k, i, 1 - s) = x (k, i, grid.n + s) = constant;
		}
	}

	static T max (const ARRAYS_2D<T>& a)
	{
		assert (a.size > 0);
		T result = a.array[0];

		for (int index = 1; index < a.size; index++) result = PhysBAM::max (result, a.array[index]);

		return result;
	}

	static T maxabs (const ARRAYS_2D<T>& a)
	{
		assert (a.size > 0);
		T result = (T) fabs (a.array[0]);

		for (int index = 1; index < a.size; index++) result = maxabs_incremental (result, a.array[index]);

		return result;
	}

	static T maxmag (const ARRAYS_2D<T>& a)
	{
		assert (a.size > 0);
		T result = a.array[0];

		for (int index = 1; index < a.size; index++) result = PhysBAM::maxmag (result, a.array[index]);

		return result;
	}

	static T min (const ARRAYS_2D<T>& a)
	{
		assert (a.size > 0);
		T result = a.array[0];

		for (int index = 1; index < a.size; index++) result = PhysBAM::min (result, a.array[index]);

		return result;
	}

	static T minmag (ARRAYS_2D<T>& a)
	{
		assert (a.size > 0);
		T result = a.array[0];

		for (int index = 1; index < a.size; index++) result = PhysBAM::minmag (result, a.array[index]);

		return result;
	}

	static T sum (const ARRAYS_2D<T>& a)
	{
		T result = (T) 0;

		for (int index = 0; index < a.size; index++) result += a.array[index];

		return result;
	}

	static T sumabs (const ARRAYS_2D<T>& a)
	{
		T result = (T) 0;

		for (int index = 0; index < a.size; index++) result += fabs (a.array[index]);

		return result;
	}

	static T Dot_Product (const ARRAYS_2D<T>& a1, const ARRAYS_2D<T>& a2)
	{
		assert (a1.length == 1 && a2.length == 1);
		assert (a1.m_start == a2.m_start && a1.m_end == a2.m_end);
		assert (a1.n_start == a2.n_start && a1.n_end == a2.n_end);
		T result = (T) 0;

		for (int index = 0; index < a1.size; index++) result += a1.array[index] * a2.array[index];

		return result;
	}

	template<class TS>
	static TS Maximum_Vector_Magnitude (const ARRAYS_2D<T>& a)
	{
		assert (a.length == 1);
		TS result = (TS) 0;

		for (int index = 0; index < a.size; index++) result = PhysBAM::max (result, a.array[index].Magnitude_Squared());

		return sqrt (result);
	}

	static void exchange_arrays (ARRAYS_2D<T>& a, ARRAYS_2D<T>& b)
	{
		exchange (a.array, b.array);
		exchange (a.length, b.length);
		exchange (a.size, b.size);
		exchange (a.m_start, b.m_start);
		exchange (a.m_end, b.m_end);
		exchange (a.m, b.m);
		exchange (a.n_start, b.n_start);
		exchange (a.n_end, b.n_end);
		exchange (a.n, b.n);
		a.Calculate_Acceleration_Constants();
		b.Calculate_Acceleration_Constants();
	}

	template<class T2>
	static bool Equal_Dimensions (const ARRAYS_2D<T>& a, const ARRAYS_2D<T2>& b)
	{
		return a.length == b.length && a.m_start == b.m_start && a.m_end == b.m_end && a.n_start == b.n_start && a.n_end == b.n_end;
	}

	static bool Equal_Dimensions (const ARRAYS_2D<T>& a, const int length, const int m_start, const int m_end, const int n_start, const int n_end)
	{
		return a.length == length && a.m_start == m_start && a.m_end == m_end && a.n_start == n_start && a.n_end == n_end;
	}

	static bool Equal_Dimensions (const ARRAYS_2D<T>& a, const int m_start, const int m_end, const int n_start, const int n_end)
	{
		return Equal_Dimensions (a, 1, m_start, m_end, n_start, n_end);
	}

	void Move_Left (const int increment = 1)
	{
		for (int i = m_start; i <= m_end - increment; i++) for (int j = n_start; j <= n_end; j++) (*this) (i, j) = (*this) (i + increment, j);
	}

	void Move_Right (const int increment = 1)
	{
		for (int i = m_end; i >= m_start + increment; i--) for (int j = n_start; j <= n_end; j++) (*this) (i, j) = (*this) (i - increment, j);
	}

	void Move_Down (const int increment = 1)
	{
		for (int i = m_start; i <= m_end; i++) for (int j = n_start; j <= n_end - increment; j++) (*this) (i, j) = (*this) (i, j + increment);
	}

	void Move_Up (const int increment = 1)
	{
		for (int i = m_start; i <= m_end; i++) for (int j = n_end; j >= n_start + increment; j--) (*this) (i, j) = (*this) (i, j - increment);
	}

	template<class RW>
	void Read (std::istream &input_stream)
	{
		Read_Binary<RW> (input_stream, length, m_start, m_end, n_start, n_end);
		assert (length > 0);
		m = m_end - m_start + 1;
		assert (m >= 0);
		n = n_end - n_start + 1;
		assert (n >= 0);
		size = length * m * n;
		delete[] array;

		if (size > 0)
		{
			array = new T[size];
			Read_Binary_Array<RW> (input_stream, array, size);
		}
		else array = 0;

		Calculate_Acceleration_Constants();
	}

	template<class RW>
	void Write (std::ostream &output_stream) const
	{
		Write_Binary<RW> (output_stream, length, m_start, m_end, n_start, n_end);
		Write_Binary_Array<RW> (output_stream, array, size);
	}

//#####################################################################
};
}
#endif

