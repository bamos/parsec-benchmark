//#####################################################################
// Copyright 2005-2006, Andrew Selle, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#ifndef __ARRAY_PARALLEL_OPERATIONS__
#define __ARRAY_PARALLEL_OPERATIONS__

namespace PhysBAM
{

template<class T> class ARRAY;
template<class T> class VECTOR_2D;

template<class T, class TS, class TV> struct ARRAY_PARALLEL_OPERATIONS_DATA_HELPER
{
	union
	{
		const ARRAY<T>* array_input_1;
		const ARRAY<TS>* scalar_array_input_1;
		const ARRAY<TV>* vector_array_input_1;
	};
	union
	{
		const ARRAY<T>* array_input_2;
		const ARRAY<TS>* scalar_array_input_2;
		const ARRAY<TV>* vector_array_input_2;
	};
	union
	{
		const ARRAY<T>* array_input_3;
		const ARRAY<TS>* scalar_array_input_3;
		const ARRAY<TV>* vector_array_input_3;
	};
	union
	{
		const T* element_input;
		const TS* scalar_element_input;
		const TV* vector_element_input;
	};
	union
	{
		ARRAY<T>* array_output;
		ARRAY<TS>* scalar_array_output;
		ARRAY<TV>* vector_array_output;
	};
	enum
	{
		CLEAR,
		COPY_ARRAY,
		SCALED_ARRAY_PLUS_ARRAY,
		SCALED_NORMALIZED_ARRAY_PLUS_ARRAY,
		ADD_SCALED_ARRAY_PLUS_ARRAY,
		ADD_SCALED_ARRAY,
		ADD_SCALED_NORMALIZED_ARRAY,
		SCALE_NORMALIZE_ARRAY,
		DOT_PRODUCT,
		SCALED_DOT_PRODUCT,
		MAXIMUM_MAGNITUDE_SQUARED,
		MAXIMUM_SCALED_MAGNITUDE_SQUARED
	} operation;
};

template<class T, class TS, class TV> struct ARRAY_PARALLEL_OPERATIONS_HELPER
{
	ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>* data;
	double double_output;
	VECTOR_2D<int> range;
};

template<class T, class TS, class TV>
class ARRAY_PARALLEL_OPERATIONS
{
public:
	static void Array_Parallel_Operations_Helper (long thread_id, void* helper_raw);
	static void Clear_Parallel (ARRAY<T>& array_output, const ARRAY<VECTOR_2D<int> >& ranges);
	static void Copy_Array_Parallel (const ARRAY<T>& array_input_1, ARRAY<T>& array_output, const ARRAY<VECTOR_2D<int> >& ranges);
	static void Scaled_Array_Plus_Array_Parallel (const ARRAY<T>& array_input_1, const ARRAY<T>& array_input_2, const TS scalar_element_input, ARRAY<T>& array_output,
			const ARRAY<VECTOR_2D<int> >& ranges);
	static void Scaled_Normalized_Array_Plus_Array_Parallel (const ARRAY<T>& array_input_1, const ARRAY<T>& array_input_2, const ARRAY<TS>& scalar_array_input_3,
			const TS scalar_element_input, ARRAY<T>& array_output, const ARRAY<VECTOR_2D<int> >& ranges);
	static void Add_Scaled_Array_Plus_Array_Parallel (const ARRAY<T>& array_input_1, const ARRAY<T>& array_input_2, const TS scalar_element_input, ARRAY<T>& array_output,
			const ARRAY<VECTOR_2D<int> >& ranges);
	static void Add_Scaled_Array_Parallel (const ARRAY<T>& array_input_1, const TS scalar_element_input, ARRAY<T>& array_output, const ARRAY<VECTOR_2D<int> >& ranges);
	static void Add_Scaled_Normalized_Array_Parallel (const ARRAY<T>& array_input_1, const ARRAY<TS>& scalar_array_input_2, const TS scalar_element_input, ARRAY<T>& array_output,
			const ARRAY<VECTOR_2D<int> >& ranges);
	static void Scale_Normalize_Array_Parallel (const ARRAY<TS>& scalar_array_input_1, const TS scalar_element_input, ARRAY<T>& array_output, const ARRAY<VECTOR_2D<int> >& ranges);
	static double Dot_Product_Parallel (const ARRAY<TV>& vector_array_input_1, const ARRAY<TV>& vector_array_input_2, const ARRAY<VECTOR_2D<int> >& ranges);
	static double Scaled_Dot_Product_Parallel (const ARRAY<TV>& vector_array_input_1, const ARRAY<TV>& vector_array_input_2, const ARRAY<TS>& scalar_array_input_3, const ARRAY<VECTOR_2D<int> >& ranges);
	static double Maximum_Magnitude_Squared_Parallel (const ARRAY<TV>& vector_array_input_1, const ARRAY<VECTOR_2D<int> >& ranges);
	static double Maximum_Scaled_Magnitude_Squared_Parallel (const ARRAY<TV>& vector_array_input_1, const ARRAY<TS>& scalar_array_input_2, const ARRAY<VECTOR_2D<int> >& ranges);
//#####################################################################
};
}
#endif
