//#####################################################################
// Copyright 2005-2006, Andrew Selle, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
#include "../Arrays/ARRAY.h"
#include "../Matrices_And_Vectors/VECTOR_2D.h"
#include "ARRAY_PARALLEL_OPERATIONS.h"
#include "../Matrices_And_Vectors/VECTOR_3D.h"
#include "../Matrices_And_Vectors/MATRIX_3X3.h"
#include "../Matrices_And_Vectors/SYMMETRIC_MATRIX_3X3.h"
#include "../Math_Tools/max.h"
#include "../Thread_Utilities/THREAD_POOL.h"
using namespace PhysBAM;
//#####################################################################
// Function Array_Parallel_Operations_Helper
//#####################################################################
template<class T, class TS, class TV> void ARRAY_PARALLEL_OPERATIONS<T, TS, TV>::
Array_Parallel_Operations_Helper (long thread_id, void* helper_raw)
{
	ARRAY_PARALLEL_OPERATIONS_HELPER<T, TS, TV>& helper = * ( (ARRAY_PARALLEL_OPERATIONS_HELPER<T, TS, TV>*) helper_raw);
	ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>& data_helper = *helper.data;
	VECTOR_2D<int>& range = helper.range;

	switch (data_helper.operation)
	{
	case ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>::CLEAR:
	{
		ARRAY<T>& array_output = *data_helper.array_output;

		for (int i = range.x; i <= range.y; i++) array_output (i) = T();
	}
	break;
	case ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>::COPY_ARRAY:
	{
		const ARRAY<T>& array_input_1 = *data_helper.array_input_1;
		ARRAY<T>& array_output = *data_helper.array_output;

		for (int i = range.x; i <= range.y; i++) array_output (i) = array_input_1 (i);
	}
	break;
	case ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>::SCALED_ARRAY_PLUS_ARRAY:
	{
		const ARRAY<T>& array_input_1 = *data_helper.array_input_1;
		const ARRAY<T>& array_input_2 = *data_helper.array_input_2;
		const TS& scalar_element_input = *data_helper.scalar_element_input;
		ARRAY<T>& array_output = *data_helper.array_output;

		for (int i = range.x; i <= range.y; i++) array_output (i) = scalar_element_input * array_input_1 (i) + array_input_2 (i);
	}
	break;
	case ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>::SCALED_NORMALIZED_ARRAY_PLUS_ARRAY:
	{
		const ARRAY<T>& array_input_1 = *data_helper.array_input_1;
		const ARRAY<T>& array_input_2 = *data_helper.array_input_2;
		const ARRAY<TS>& scalar_array_input_3 = *data_helper.scalar_array_input_3;
		const TS& scalar_element_input = *data_helper.scalar_element_input;
		ARRAY<T>& array_output = *data_helper.array_output;

		for (int i = range.x; i <= range.y; i++) array_output (i) = (scalar_element_input / scalar_array_input_3 (i)) * array_input_1 (i) + array_input_2 (i);
	}
	break;
	case ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>::ADD_SCALED_ARRAY_PLUS_ARRAY:
	{
		const ARRAY<T>& array_input_1 = *data_helper.array_input_1;
		const ARRAY<T>& array_input_2 = *data_helper.array_input_2;
		const TS& scalar_element_input = *data_helper.scalar_element_input;
		ARRAY<T>& array_output = *data_helper.array_output;

		for (int i = range.x; i <= range.y; i++) array_output (i) += scalar_element_input * array_input_1 (i) + array_input_2 (i);
	}
	break;
	case ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>::ADD_SCALED_ARRAY:
	{
		const ARRAY<T>& array_input_1 = *data_helper.array_input_1;
		const TS& scalar_element_input = *data_helper.scalar_element_input;
		ARRAY<T>& array_output = *data_helper.array_output;

		for (int i = range.x; i <= range.y; i++) array_output (i) += scalar_element_input * array_input_1 (i);
	}
	break;
	case ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>::ADD_SCALED_NORMALIZED_ARRAY:
	{
		const ARRAY<T>& array_input_1 = *data_helper.array_input_1;
		const ARRAY<TS>& scalar_array_input_2 = *data_helper.scalar_array_input_2;
		const TS& scalar_element_input = *data_helper.scalar_element_input;
		ARRAY<T>& array_output = *data_helper.array_output;

		for (int i = range.x; i <= range.y; i++) array_output (i) += (scalar_element_input / scalar_array_input_2 (i)) * array_input_1 (i);
	}
	break;
	case ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>::SCALE_NORMALIZE_ARRAY:
	{
		const ARRAY<TS>& scalar_array_input_1 = *data_helper.scalar_array_input_1;
		const TS& scalar_element_input = *data_helper.scalar_element_input;
		ARRAY<T>& array_output = *data_helper.array_output;

		for (int i = range.x; i <= range.y; i++) array_output (i) *= (scalar_element_input / scalar_array_input_1 (i));
	}
	break;
	case ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>::DOT_PRODUCT:
	{
		const ARRAY<TV>& vector_array_input_1 = *data_helper.vector_array_input_1;
		const ARRAY<TV>& vector_array_input_2 = *data_helper.vector_array_input_2;
		helper.double_output = 0;

		for (int i = range.x; i <= range.y; i++) helper.double_output += (double) TV::Dot_Product (vector_array_input_1 (i), vector_array_input_2 (i));
	}
	break;
	case ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>::SCALED_DOT_PRODUCT:
	{
		const ARRAY<TV>& vector_array_input_1 = *data_helper.vector_array_input_1;
		const ARRAY<TV>& vector_array_input_2 = *data_helper.vector_array_input_2;
		const ARRAY<TS>& scalar_array_input_3 = *data_helper.scalar_array_input_3;
		helper.double_output = 0;

		for (int i = range.x; i <= range.y; i++) helper.double_output += (double) scalar_array_input_3 (i) * TV::Dot_Product (vector_array_input_1 (i), vector_array_input_2 (i));
	}
	break;
	case ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>::MAXIMUM_MAGNITUDE_SQUARED:
	{
		const ARRAY<TV>& vector_array_input_1 = *data_helper.vector_array_input_1;
		helper.double_output = 0;

		for (int i = range.x; i <= range.y; i++) helper.double_output = max<double> (helper.double_output, vector_array_input_1 (i).Magnitude_Squared());
	}
	break;
	case ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>::MAXIMUM_SCALED_MAGNITUDE_SQUARED:
	{
		const ARRAY<TV>& vector_array_input_1 = *data_helper.vector_array_input_1;
		const ARRAY<TS>& scalar_array_input_2 = *data_helper.scalar_array_input_2;
		helper.double_output = 0;

		for (int i = range.x; i <= range.y; i++) helper.double_output = max<double> (helper.double_output, scalar_array_input_2 (i) * vector_array_input_1 (i).Magnitude_Squared());
	}
	break;
	default:
		std::cerr << "UNKNOWN OPERATION CODE IN ARRAY_PARALLEL_OPERATIONS" << std::endl;
		exit (1);
	}
}
//#####################################################################
// Function Clear_Parallel
//#####################################################################
template<class T, class TS, class TV> void ARRAY_PARALLEL_OPERATIONS<T, TS, TV>::
Clear_Parallel (ARRAY<T>& array_output, const ARRAY<VECTOR_2D<int> >& ranges)
{
	ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV> data_helper;
	ARRAY<ARRAY_PARALLEL_OPERATIONS_HELPER<T, TS, TV> > helpers (ranges.m);

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	THREAD_POOL& pool = (*THREAD_POOL::Singleton());
#endif
	data_helper.array_output = &array_output;
	data_helper.operation = ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>::CLEAR;

	for (int i = 1; i <= helpers.m; i++)
	{
		helpers (i).data = &data_helper;
		helpers (i).range = ranges (i);
#ifndef NEW_SERIAL_IMPLEMENTATIOM
		pool.Add_Task (Array_Parallel_Operations_Helper, &helpers (i));
#else
		Array_Parallel_Operations_Helper (i, &helpers (i));
#endif
	}

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	pool.Wait_For_Completion();
#endif
}
//#####################################################################
// Function Copy_Array_Parallel
//#####################################################################
template<class T, class TS, class TV> void ARRAY_PARALLEL_OPERATIONS<T, TS, TV>::
Copy_Array_Parallel (const ARRAY<T>& array_input_1, ARRAY<T>& array_output, const ARRAY<VECTOR_2D<int> >& ranges)
{
	ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV> data_helper;
	ARRAY<ARRAY_PARALLEL_OPERATIONS_HELPER<T, TS, TV> > helpers (ranges.m);

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	THREAD_POOL& pool = (*THREAD_POOL::Singleton());
#endif
	data_helper.array_input_1 = &array_input_1;
	data_helper.array_output = &array_output;
	data_helper.operation = ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>::COPY_ARRAY;

	for (int i = 1; i <= helpers.m; i++)
	{
		helpers (i).data = &data_helper;
		helpers (i).range = ranges (i);
#ifndef NEW_SERIAL_IMPLEMENTATIOM
		pool.Add_Task (Array_Parallel_Operations_Helper, &helpers (i));
#else
		Array_Parallel_Operations_Helper (i, &helpers (i));
#endif
	}

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	pool.Wait_For_Completion();
#endif
}
//#####################################################################
// Function Scaled_Array_Plus_Array_Parallel
//#####################################################################
template<class T, class TS, class TV> void ARRAY_PARALLEL_OPERATIONS<T, TS, TV>::
Scaled_Array_Plus_Array_Parallel (const ARRAY<T>& array_input_1, const ARRAY<T>& array_input_2, const TS scalar_element_input, ARRAY<T>& array_output, const ARRAY<VECTOR_2D<int> >& ranges)
{
	ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV> data_helper;
	ARRAY<ARRAY_PARALLEL_OPERATIONS_HELPER<T, TS, TV> > helpers (ranges.m);

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	THREAD_POOL& pool = (*THREAD_POOL::Singleton());
#endif
	data_helper.array_input_1 = &array_input_1;
	data_helper.array_input_2 = &array_input_2;
	data_helper.scalar_element_input = &scalar_element_input;
	data_helper.array_output = &array_output;
	data_helper.operation = ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>::SCALED_ARRAY_PLUS_ARRAY;

	for (int i = 1; i <= helpers.m; i++)
	{
		helpers (i).data = &data_helper;
		helpers (i).range = ranges (i);
#ifndef NEW_SERIAL_IMPLEMENTATIOM
		pool.Add_Task (Array_Parallel_Operations_Helper, &helpers (i));
#else
		Array_Parallel_Operations_Helper (i, &helpers (i));
#endif
	}

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	pool.Wait_For_Completion();
#endif
}
//#####################################################################
// Function Scaled_Normalized_Array_Plus_Array_Parallel
//#####################################################################
template<class T, class TS, class TV> void ARRAY_PARALLEL_OPERATIONS<T, TS, TV>::
Scaled_Normalized_Array_Plus_Array_Parallel (const ARRAY<T>& array_input_1, const ARRAY<T>& array_input_2, const ARRAY<TS>& scalar_array_input_3, const TS scalar_element_input,
		ARRAY<T>& array_output, const ARRAY<VECTOR_2D<int> >& ranges)
{
	ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV> data_helper;
	ARRAY<ARRAY_PARALLEL_OPERATIONS_HELPER<T, TS, TV> > helpers (ranges.m);

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	THREAD_POOL& pool = (*THREAD_POOL::Singleton());
#endif
	data_helper.array_input_1 = &array_input_1;
	data_helper.array_input_2 = &array_input_2;
	data_helper.scalar_array_input_3 = &scalar_array_input_3;
	data_helper.scalar_element_input = &scalar_element_input;
	data_helper.array_output = &array_output;
	data_helper.operation = ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>::SCALED_NORMALIZED_ARRAY_PLUS_ARRAY;

	for (int i = 1; i <= helpers.m; i++)
	{
		helpers (i).data = &data_helper;
		helpers (i).range = ranges (i);
#ifndef NEW_SERIAL_IMPLEMENTATIOM
		pool.Add_Task (Array_Parallel_Operations_Helper, &helpers (i));
#else
		Array_Parallel_Operations_Helper (i, &helpers (i));
#endif
	}

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	pool.Wait_For_Completion();
#endif
}
//#####################################################################
// Function Add_Scaled_Array_Plus_Array_Parallel
//#####################################################################
template<class T, class TS, class TV> void ARRAY_PARALLEL_OPERATIONS<T, TS, TV>::
Add_Scaled_Array_Plus_Array_Parallel (const ARRAY<T>& array_input_1, const ARRAY<T>& array_input_2, const TS scalar_element_input, ARRAY<T>& array_output, const ARRAY<VECTOR_2D<int> >& ranges)
{
	ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV> data_helper;
	ARRAY<ARRAY_PARALLEL_OPERATIONS_HELPER<T, TS, TV> > helpers (ranges.m);

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	THREAD_POOL& pool = (*THREAD_POOL::Singleton());
#endif
	data_helper.array_input_1 = &array_input_1;
	data_helper.array_input_2 = &array_input_2;
	data_helper.scalar_element_input = &scalar_element_input;
	data_helper.array_output = &array_output;
	data_helper.operation = ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>::ADD_SCALED_ARRAY_PLUS_ARRAY;

	for (int i = 1; i <= helpers.m; i++)
	{
		helpers (i).data = &data_helper;
		helpers (i).range = ranges (i);
#ifndef NEW_SERIAL_IMPLEMENTATIOM
		pool.Add_Task (Array_Parallel_Operations_Helper, &helpers (i));
#else
		Array_Parallel_Operations_Helper (i, &helpers (i));
#endif
	}

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	pool.Wait_For_Completion();
#endif
}
//#####################################################################
// Function Add_Scaled_Array_Parallel
//#####################################################################
template<class T, class TS, class TV> void ARRAY_PARALLEL_OPERATIONS<T, TS, TV>::
Add_Scaled_Array_Parallel (const ARRAY<T>& array_input_1, const TS scalar_element_input, ARRAY<T>& array_output, const ARRAY<VECTOR_2D<int> >& ranges)
{
	ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV> data_helper;
	ARRAY<ARRAY_PARALLEL_OPERATIONS_HELPER<T, TS, TV> > helpers (ranges.m);

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	THREAD_POOL& pool = (*THREAD_POOL::Singleton());
#endif
	data_helper.array_input_1 = &array_input_1;
	data_helper.scalar_element_input = &scalar_element_input;
	data_helper.array_output = &array_output;
	data_helper.operation = ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>::ADD_SCALED_ARRAY;

	for (int i = 1; i <= helpers.m; i++)
	{
		helpers (i).data = &data_helper;
		helpers (i).range = ranges (i);
#ifndef NEW_SERIAL_IMPLEMENTATIOM
		pool.Add_Task (Array_Parallel_Operations_Helper, &helpers (i));
#else
		Array_Parallel_Operations_Helper (i, &helpers (i));
#endif
	}

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	pool.Wait_For_Completion();
#endif
}
//#####################################################################
// Function Add_Scaled_Normalized_Array_Parallel
//#####################################################################
template<class T, class TS, class TV> void ARRAY_PARALLEL_OPERATIONS<T, TS, TV>::
Add_Scaled_Normalized_Array_Parallel (const ARRAY<T>& array_input_1, const ARRAY<TS>& scalar_array_input_2, const TS scalar_element_input, ARRAY<T>& array_output, const ARRAY<VECTOR_2D<int> >& ranges)
{
	ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV> data_helper;
	ARRAY<ARRAY_PARALLEL_OPERATIONS_HELPER<T, TS, TV> > helpers (ranges.m);

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	THREAD_POOL& pool = (*THREAD_POOL::Singleton());
#endif
	data_helper.array_input_1 = &array_input_1;
	data_helper.scalar_array_input_2 = &scalar_array_input_2;
	data_helper.scalar_element_input = &scalar_element_input;
	data_helper.array_output = &array_output;
	data_helper.operation = ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>::ADD_SCALED_NORMALIZED_ARRAY;

	for (int i = 1; i <= helpers.m; i++)
	{
		helpers (i).data = &data_helper;
		helpers (i).range = ranges (i);
#ifndef NEW_SERIAL_IMPLEMENTATIOM
		pool.Add_Task (Array_Parallel_Operations_Helper, &helpers (i));
#else
		Array_Parallel_Operations_Helper (i, &helpers (i));
#endif
	}

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	pool.Wait_For_Completion();
#endif
}
//#####################################################################
// Function Scale_Normalize_Array_Parallel
//#####################################################################
template<class T, class TS, class TV> void ARRAY_PARALLEL_OPERATIONS<T, TS, TV>::
Scale_Normalize_Array_Parallel (const ARRAY<TS>& scalar_array_input_1, const TS scalar_element_input, ARRAY<T>& array_output, const ARRAY<VECTOR_2D<int> >& ranges)
{
	ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV> data_helper;
	ARRAY<ARRAY_PARALLEL_OPERATIONS_HELPER<T, TS, TV> > helpers (ranges.m);

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	THREAD_POOL& pool = (*THREAD_POOL::Singleton());
#endif
	data_helper.scalar_array_input_1 = &scalar_array_input_1;
	data_helper.scalar_element_input = &scalar_element_input;
	data_helper.array_output = &array_output;
	data_helper.operation = ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>::SCALE_NORMALIZE_ARRAY;

	for (int i = 1; i <= helpers.m; i++)
	{
		helpers (i).data = &data_helper;
		helpers (i).range = ranges (i);
#ifndef NEW_SERIAL_IMPLEMENTATIOM
		pool.Add_Task (Array_Parallel_Operations_Helper, &helpers (i));
#else
		Array_Parallel_Operations_Helper (i, &helpers (i));
#endif
	}

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	pool.Wait_For_Completion();
#endif
}
//#####################################################################
// Function Dot_Product_Parallel
//#####################################################################
template<class T, class TS, class TV> double ARRAY_PARALLEL_OPERATIONS<T, TS, TV>::
Dot_Product_Parallel (const ARRAY<TV>& vector_array_input_1, const ARRAY<TV>& vector_array_input_2, const ARRAY<VECTOR_2D<int> >& ranges)
{
	ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV> data_helper;
	ARRAY<ARRAY_PARALLEL_OPERATIONS_HELPER<T, TS, TV> > helpers (ranges.m);

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	THREAD_POOL& pool = (*THREAD_POOL::Singleton());
#endif
	data_helper.vector_array_input_1 = &vector_array_input_1;
	data_helper.vector_array_input_2 = &vector_array_input_2;
	data_helper.operation = ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>::DOT_PRODUCT;

	for (int i = 1; i <= helpers.m; i++)
	{
		helpers (i).data = &data_helper;
		helpers (i).range = ranges (i);
#ifndef NEW_SERIAL_IMPLEMENTATIOM
		pool.Add_Task (Array_Parallel_Operations_Helper, &helpers (i));
#else
		Array_Parallel_Operations_Helper (i, &helpers (i));
#endif
	}

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	pool.Wait_For_Completion();
#endif
	double result = 0;

	for (int i = 1; i <= helpers.m; i++) result += helpers (i).double_output;

	return result;
}
//#####################################################################
// Function Scaled_Dot_Product_Parallel
//#####################################################################
template<class T, class TS, class TV> double ARRAY_PARALLEL_OPERATIONS<T, TS, TV>::
Scaled_Dot_Product_Parallel (const ARRAY<TV>& vector_array_input_1, const ARRAY<TV>& vector_array_input_2, const ARRAY<TS>& scalar_array_input_3, const ARRAY<VECTOR_2D<int> >& ranges)
{
	ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV> data_helper;
	ARRAY<ARRAY_PARALLEL_OPERATIONS_HELPER<T, TS, TV> > helpers (ranges.m);

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	THREAD_POOL& pool = (*THREAD_POOL::Singleton());
#endif
	data_helper.vector_array_input_1 = &vector_array_input_1;
	data_helper.vector_array_input_2 = &vector_array_input_2;
	data_helper.scalar_array_input_3 = &scalar_array_input_3;
	data_helper.operation = ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>::SCALED_DOT_PRODUCT;

	for (int i = 1; i <= helpers.m; i++)
	{
		helpers (i).data = &data_helper;
		helpers (i).range = ranges (i);
#ifndef NEW_SERIAL_IMPLEMENTATIOM
		pool.Add_Task (Array_Parallel_Operations_Helper, &helpers (i));
#else
		Array_Parallel_Operations_Helper (i, &helpers (i));
#endif
	}

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	pool.Wait_For_Completion();
#endif
	double result = 0;

	for (int i = 1; i <= helpers.m; i++) result += helpers (i).double_output;

	return result;
}
//#####################################################################
// Function Maximum_Magnitude_Squared_Parallel
//#####################################################################
template<class T, class TS, class TV> double ARRAY_PARALLEL_OPERATIONS<T, TS, TV>::
Maximum_Magnitude_Squared_Parallel (const ARRAY<TV>& vector_array_input_1, const ARRAY<VECTOR_2D<int> >& ranges)
{
	ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV> data_helper;
	ARRAY<ARRAY_PARALLEL_OPERATIONS_HELPER<T, TS, TV> > helpers (ranges.m);

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	THREAD_POOL& pool = (*THREAD_POOL::Singleton());
#endif
	data_helper.vector_array_input_1 = &vector_array_input_1;
	data_helper.operation = ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>::MAXIMUM_MAGNITUDE_SQUARED;

	for (int i = 1; i <= helpers.m; i++)
	{
		helpers (i).data = &data_helper;
		helpers (i).range = ranges (i);
#ifndef NEW_SERIAL_IMPLEMENTATIOM
		pool.Add_Task (Array_Parallel_Operations_Helper, &helpers (i));
#else
		Array_Parallel_Operations_Helper (i, &helpers (i));
#endif
	}

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	pool.Wait_For_Completion();
#endif
	double result = 0;

	for (int i = 1; i <= helpers.m; i++) result = max<double> (result, helpers (i).double_output);

	return result;
}
//#####################################################################
// Function Maximum_Scaled_Magnitude_Squared_Parallel
//#####################################################################
template<class T, class TS, class TV> double ARRAY_PARALLEL_OPERATIONS<T, TS, TV>::
Maximum_Scaled_Magnitude_Squared_Parallel (const ARRAY<TV>& vector_array_input_1, const ARRAY<TS>& scalar_array_input_2, const ARRAY<VECTOR_2D<int> >& ranges)
{
	ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV> data_helper;
	ARRAY<ARRAY_PARALLEL_OPERATIONS_HELPER<T, TS, TV> > helpers (ranges.m);

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	THREAD_POOL& pool = (*THREAD_POOL::Singleton());
#endif
	data_helper.vector_array_input_1 = &vector_array_input_1;
	data_helper.scalar_array_input_2 = &scalar_array_input_2;
	data_helper.operation = ARRAY_PARALLEL_OPERATIONS_DATA_HELPER<T, TS, TV>::MAXIMUM_SCALED_MAGNITUDE_SQUARED;

	for (int i = 1; i <= helpers.m; i++)
	{
		helpers (i).data = &data_helper;
		helpers (i).range = ranges (i);
#ifndef NEW_SERIAL_IMPLEMENTATIOM
		pool.Add_Task (Array_Parallel_Operations_Helper, &helpers (i));
#else
		Array_Parallel_Operations_Helper (i, &helpers (i));
#endif
	}

#ifndef NEW_SERIAL_IMPLEMENTATIOM
	pool.Wait_For_Completion();
#endif
	double result = 0;

	for (int i = 1; i <= helpers.m; i++) result = max<double> (result, helpers (i).double_output);

	return result;
}
//#####################################################################
template class ARRAY_PARALLEL_OPERATIONS<VECTOR_2D<float>, float, VECTOR_2D<float> >;
template class ARRAY_PARALLEL_OPERATIONS<VECTOR_3D<float>, float, VECTOR_3D<float> >;
template class ARRAY_PARALLEL_OPERATIONS<VECTOR_2D<double>, double, VECTOR_2D<double> >;
template class ARRAY_PARALLEL_OPERATIONS<VECTOR_3D<double>, double, VECTOR_3D<double> >;
template class ARRAY_PARALLEL_OPERATIONS<MATRIX_3X3<float>, float, VECTOR_3D<float> >;
template class ARRAY_PARALLEL_OPERATIONS<SYMMETRIC_MATRIX_3X3<float>, float, VECTOR_3D<float> >;
template class ARRAY_PARALLEL_OPERATIONS<MATRIX_3X3<double>, double, VECTOR_3D<double> >;
template class ARRAY_PARALLEL_OPERATIONS<SYMMETRIC_MATRIX_3X3<double>, double, VECTOR_3D<double> >;

