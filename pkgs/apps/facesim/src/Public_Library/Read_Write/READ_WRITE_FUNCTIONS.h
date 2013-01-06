//#####################################################################
// Copyright 2004, Eran Guendelman, Igor Neverov, Andrew Selle, Eftychios Sifakis.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class READ_WRITE_FUNCTIONS
//#####################################################################
//
// Functions for reading and writing which do the correct thing for objects, pointers, primitive types, etc. In general, use Read/Write_Binary (and Read/Write_Binary_Array) using T for the type
// of the object you're reading/writing and RW the underlying floating point scalar type (float/double).
//
//#####################################################################
#ifndef __READ_WRITE_FUNCTIONS__
#define __READ_WRITE_FUNCTIONS__
#include <iostream>
#include <assert.h>

namespace PhysBAM
{

//#####################################################################
// Function isLittleEndian
//#####################################################################
static inline int isLittleEndian()
{
	union
	{
		unsigned short int word;
		unsigned char byte;
	} endian_test;

	endian_test.word = 0x00FF;
	return (endian_test.byte == 0xFF);
}
//#####################################################################
// Function Swap_Endianity
//#####################################################################
template<class T>
inline void Swap_Endianity (T& x)
{
	assert (sizeof (T) <= 8);

	if (sizeof (T) > 1)
	{
		T old = x;

		for (unsigned int k = 1; k <= sizeof (T); k++) ( (char*) &x) [k - 1] = ( (char*) &old) [sizeof (T) - k];
	}
}
//#####################################################################
// Read/Write_Primitive (handles endianness)
//#####################################################################

#ifdef __sparc__
void sparc_seg_fault_prevent_dummy (void *ptr);
#endif

template<class T>
inline void Read_Primitive (std::istream& input_stream, T& d)
{
	input_stream.read ( (char*) &d, sizeof (T));

	if (!isLittleEndian()) Swap_Endianity (d);

#ifdef __sparc__
	/* For unknown reasons I am getting a segfault on my sparc box compiling
	   with -O3, unless I place a dummy use of the value just read here.
	   Might just be an issue with a particular gcc version. Unfortunately,
	   this function call does affect the region of interest. */
	sparc_seg_fault_prevent_dummy ( (void *) &d);
#endif
}

template<class T>
inline void Write_Primitive (std::ostream& output_stream, const T& d)
{
	T d2 = d;

	if (!isLittleEndian()) Swap_Endianity (d2);

	output_stream.write ( (const char*) &d2, sizeof (T));
}


#ifdef __APPLE__ // PhysBAM data formats assume sizeof(bool)==1 but the Mac apparently has bool==int with sizeof(bool)==4, so need to specialize these

template<>
inline void Read_Primitive<bool> (std::istream& input_stream, bool& d)
{
	char c;
	input_stream.read (&c, 1);
	d = (bool) c;
}

template<>
inline void Write_Primitive<bool> (std::ostream& output_stream, const bool& d)
{
	char c = (char) d;
	output_stream.write (&c, 1);
}

#endif

//#####################################################################
// Read/Write_Float_Or_Double
//#####################################################################
template<class T, class RW>
inline void Read_Float_Or_Double (std::istream& input_stream, T& d)
{
	std::cerr << "Read_Float_Or_Double called with bad types" << std::endl;
	exit (1);
}

template<class T, class RW>
inline void Write_Float_Or_Double (std::ostream& output_stream, T d)
{
	std::cerr << "Write_Float_Or_Double called with bad types" << std::endl;
	exit (1);
}

template<> inline void Read_Float_Or_Double<float, float> (std::istream& input_stream, float& d)
{
	Read_Primitive (input_stream, d);       // no conversion
}

template<> inline void Read_Float_Or_Double<double, double> (std::istream& input_stream, double& d)
{
	Read_Primitive (input_stream, d);       // no conversion
}

template<> inline void Read_Float_Or_Double<float, double> (std::istream& input_stream, float& d)
{
	double tmp;        // convert types
	Read_Primitive (input_stream, tmp);
	d = (float) tmp;
}

template<> inline void Read_Float_Or_Double<double, float> (std::istream& input_stream, double& d)
{
	float tmp;        // convert types
	Read_Primitive (input_stream, tmp);
	d = (double) tmp;
}

template<> inline void Write_Float_Or_Double<float, float> (std::ostream& output_stream, float d)
{
	Write_Primitive (output_stream, d);       // no conversion
}

template<> inline void Write_Float_Or_Double<double, double> (std::ostream& output_stream, double d)
{
	Write_Primitive (output_stream, d);       // no conversion
}

template<> inline void Write_Float_Or_Double<float, double> (std::ostream& output_stream, float d)
{
	Write_Primitive (output_stream, (double) d);       // convert types
}

template<> inline void Write_Float_Or_Double<double, float> (std::ostream& output_stream, double d)
{
	Write_Primitive (output_stream, (float) d);       // convert types
}


//#####################################################################
// Read_Write for objects
//#####################################################################
template<class T>
struct Read_Write
{
	template<class RW>
	static void Read (std::istream& input_stream, T& d)
	{
		d.template Read<RW> (input_stream);
	}

	template<class RW>
	static void Write (std::ostream& output_stream, const T& d)
	{
		d.template Write<RW> (output_stream);
	}
};
//#####################################################################
// Read_Write for primitive types (other than float and double)
//#####################################################################
#define DEFINE_READ_WRITE_FOR_PRIMITIVE_TYPE(TYPE) \
template<> struct Read_Write<TYPE> { \
    template<class RW> static void Read(std::istream& input_stream,TYPE& d){Read_Primitive(input_stream,d);} \
    template<class RW> static void Write(std::ostream& output_stream,const TYPE& d) {Write_Primitive(output_stream,d);} \
};
DEFINE_READ_WRITE_FOR_PRIMITIVE_TYPE (bool);
DEFINE_READ_WRITE_FOR_PRIMITIVE_TYPE (char);
DEFINE_READ_WRITE_FOR_PRIMITIVE_TYPE (unsigned char);
DEFINE_READ_WRITE_FOR_PRIMITIVE_TYPE (short);
DEFINE_READ_WRITE_FOR_PRIMITIVE_TYPE (unsigned short);
DEFINE_READ_WRITE_FOR_PRIMITIVE_TYPE (int);
DEFINE_READ_WRITE_FOR_PRIMITIVE_TYPE (unsigned int);
//#####################################################################
// Specializations for float and double
//#####################################################################
template<> struct Read_Write<float>
{
	template<class RW>
	static void Read (std::istream& input_stream, float& d)
	{
		Read_Float_Or_Double<float, RW> (input_stream, d);
	}

	template<class RW>
	static void Write (std::ostream& output_stream, const float& d)
	{
		Write_Float_Or_Double<float, RW> (output_stream, d);
	}
};

template<> struct Read_Write<double>
{
	template<class RW>
	static void Read (std::istream& input_stream, double& d)
	{
		Read_Float_Or_Double<double, RW> (input_stream, d);
	}

	template<class RW>
	static void Write (std::ostream& output_stream, const double& d)
	{
		Write_Float_Or_Double<double, RW> (output_stream, d);
	}
};
//#####################################################################
// Read_Write for pointers to data
//#####################################################################
template<class T>
struct Read_Write<T*>
{
	template<class RW>
	static void Read (std::istream& input_stream, T*& d)
	{
		bool data_exists;
		Read_Write<bool>::template Read<RW> (input_stream, data_exists);

		if (data_exists)
		{
			d = new T();        // potential memory leak if d pointed elsewhere
			Read_Write<T>::template Read<RW> (input_stream, *d);
		}
		else d = 0;
	}

	template<class RW>
	static void Write (std::ostream& output_stream, T* const& d)
	{
		Read_Write<bool>::template Write<RW> (output_stream, d != 0); // Write a bool tag indicating whether pointer's data follows

		if (d) Read_Write<T>::template Write<RW> (output_stream, *d);
	}
};
//#####################################################################
// Read_Write for std::string's
//#####################################################################
template<>
struct Read_Write<std::string>
{
	template<class RW>
	static void Read (std::istream& input_stream, std::string& d)
	{
		int n;
		Read_Write<int>::template Read<RW> (input_stream, n);
		char* buffer = new char[n];
		input_stream.read (buffer, n);
		d.assign (buffer, buffer + n);
		delete buffer;
	}

	template<class RW>
	static void Write (std::ostream& output_stream, const std::string& d)
	{
		int n = int (d.size());
		Read_Write<int>::template Write<RW> (output_stream, n);
		const char* s = d.c_str();
		output_stream.write (s, n);
	}
};
//#####################################################################
// Read_Binary
//#####################################################################
template<class RW, class T1>
inline void Read_Binary (std::istream& input_stream, T1& d1)
{
	Read_Write<T1>::template Read<RW> (input_stream, d1);
}

template<class RW, class T1, class T2>
inline void Read_Binary (std::istream& input_stream, T1& d1, T2& d2)
{
	Read_Write<T1>::template Read<RW> (input_stream, d1);
	Read_Write<T2>::template Read<RW> (input_stream, d2);
}

template<class RW, class T1, class T2, class T3>
inline void Read_Binary (std::istream& input_stream, T1& d1, T2& d2, T3& d3)
{
	Read_Write<T1>::template Read<RW> (input_stream, d1);
	Read_Write<T2>::template Read<RW> (input_stream, d2);
	Read_Write<T3>::template Read<RW> (input_stream, d3);
}

template<class RW, class T1, class T2, class T3, class T4>
inline void Read_Binary (std::istream& input_stream, T1& d1, T2& d2, T3& d3, T4& d4)
{
	Read_Write<T1>::template Read<RW> (input_stream, d1);
	Read_Write<T2>::template Read<RW> (input_stream, d2);
	Read_Write<T3>::template Read<RW> (input_stream, d3);
	Read_Write<T4>::template Read<RW> (input_stream, d4);
}

template<class RW, class T1, class T2, class T3, class T4, class T5>
inline void Read_Binary (std::istream& input_stream, T1& d1, T2& d2, T3& d3, T4& d4, T5& d5)
{
	Read_Write<T1>::template Read<RW> (input_stream, d1);
	Read_Write<T2>::template Read<RW> (input_stream, d2);
	Read_Write<T3>::template Read<RW> (input_stream, d3);
	Read_Write<T4>::template Read<RW> (input_stream, d4);
	Read_Write<T5>::template Read<RW> (input_stream, d5);
}

template<class RW, class T1, class T2, class T3, class T4, class T5, class T6>
inline void Read_Binary (std::istream& input_stream, T1& d1, T2& d2, T3& d3, T4& d4, T5& d5, T6& d6)
{
	Read_Write<T1>::template Read<RW> (input_stream, d1);
	Read_Write<T2>::template Read<RW> (input_stream, d2);
	Read_Write<T3>::template Read<RW> (input_stream, d3);
	Read_Write<T4>::template Read<RW> (input_stream, d4);
	Read_Write<T5>::template Read<RW> (input_stream, d5);
	Read_Write<T6>::template Read<RW> (input_stream, d6);
}

template<class RW, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
inline void Read_Binary (std::istream& input_stream, T1& d1, T2& d2, T3& d3, T4& d4, T5& d5, T6& d6, T7& d7)
{
	Read_Write<T1>::template Read<RW> (input_stream, d1);
	Read_Write<T2>::template Read<RW> (input_stream, d2);
	Read_Write<T3>::template Read<RW> (input_stream, d3);
	Read_Write<T4>::template Read<RW> (input_stream, d4);
	Read_Write<T5>::template Read<RW> (input_stream, d5);
	Read_Write<T6>::template Read<RW> (input_stream, d6);
	Read_Write<T7>::template Read<RW> (input_stream, d7);
}

template<class RW, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
inline void Read_Binary (std::istream& input_stream, T1& d1, T2& d2, T3& d3, T4& d4, T5& d5, T6& d6, T7& d7, T8& d8)
{
	Read_Write<T1>::template Read<RW> (input_stream, d1);
	Read_Write<T2>::template Read<RW> (input_stream, d2);
	Read_Write<T3>::template Read<RW> (input_stream, d3);
	Read_Write<T4>::template Read<RW> (input_stream, d4);
	Read_Write<T5>::template Read<RW> (input_stream, d5);
	Read_Write<T6>::template Read<RW> (input_stream, d6);
	Read_Write<T7>::template Read<RW> (input_stream, d7);
	Read_Write<T8>::template Read<RW> (input_stream, d8);
}

template<class RW, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
inline void Read_Binary (std::istream& input_stream, T1& d1, T2& d2, T3& d3, T4& d4, T5& d5, T6& d6, T7& d7, T8& d8, T9& d9)
{
	Read_Write<T1>::template Read<RW> (input_stream, d1);
	Read_Write<T2>::template Read<RW> (input_stream, d2);
	Read_Write<T3>::template Read<RW> (input_stream, d3);
	Read_Write<T4>::template Read<RW> (input_stream, d4);
	Read_Write<T5>::template Read<RW> (input_stream, d5);
	Read_Write<T6>::template Read<RW> (input_stream, d6);
	Read_Write<T7>::template Read<RW> (input_stream, d7);
	Read_Write<T8>::template Read<RW> (input_stream, d8);
	Read_Write<T9>::template Read<RW> (input_stream, d9);
}
//#####################################################################
// Write_Binary
//#####################################################################
template<class RW, class T1>
inline void Write_Binary (std::ostream& output_stream, const T1& d1)
{
	Read_Write<T1>::template Write<RW> (output_stream, d1);
}

template<class RW, class T1, class T2>
inline void Write_Binary (std::ostream& output_stream, const T1& d1, const T2& d2)
{
	Read_Write<T1>::template Write<RW> (output_stream, d1);
	Read_Write<T2>::template Write<RW> (output_stream, d2);
}

template<class RW, class T1, class T2, class T3>
inline void Write_Binary (std::ostream& output_stream, const T1& d1, const T2& d2, const T3& d3)
{
	Read_Write<T1>::template Write<RW> (output_stream, d1);
	Read_Write<T2>::template Write<RW> (output_stream, d2);
	Read_Write<T3>::template Write<RW> (output_stream, d3);
}

template<class RW, class T1, class T2, class T3, class T4>
inline void Write_Binary (std::ostream& output_stream, const T1& d1, const T2& d2, const T3& d3, const T4& d4)
{
	Read_Write<T1>::template Write<RW> (output_stream, d1);
	Read_Write<T2>::template Write<RW> (output_stream, d2);
	Read_Write<T3>::template Write<RW> (output_stream, d3);
	Read_Write<T4>::template Write<RW> (output_stream, d4);
}

template<class RW, class T1, class T2, class T3, class T4, class T5>
inline void Write_Binary (std::ostream& output_stream, const T1& d1, const T2& d2, const T3& d3, const T4& d4, const T5& d5)
{
	Read_Write<T1>::template Write<RW> (output_stream, d1);
	Read_Write<T2>::template Write<RW> (output_stream, d2);
	Read_Write<T3>::template Write<RW> (output_stream, d3);
	Read_Write<T4>::template Write<RW> (output_stream, d4);
	Read_Write<T5>::template Write<RW> (output_stream, d5);
}

template<class RW, class T1, class T2, class T3, class T4, class T5, class T6>
inline void Write_Binary (std::ostream& output_stream, const T1& d1, const T2& d2, const T3& d3, const T4& d4, const T5& d5, const T6& d6)
{
	Read_Write<T1>::template Write<RW> (output_stream, d1);
	Read_Write<T2>::template Write<RW> (output_stream, d2);
	Read_Write<T3>::template Write<RW> (output_stream, d3);
	Read_Write<T4>::template Write<RW> (output_stream, d4);
	Read_Write<T5>::template Write<RW> (output_stream, d5);
	Read_Write<T6>::template Write<RW> (output_stream, d6);
}

template<class RW, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
inline void Write_Binary (std::ostream& output_stream, const T1& d1, const T2& d2, const T3& d3, const T4& d4, const T5& d5, const T6& d6, const T7& d7)
{
	Read_Write<T1>::template Write<RW> (output_stream, d1);
	Read_Write<T2>::template Write<RW> (output_stream, d2);
	Read_Write<T3>::template Write<RW> (output_stream, d3);
	Read_Write<T4>::template Write<RW> (output_stream, d4);
	Read_Write<T5>::template Write<RW> (output_stream, d5);
	Read_Write<T6>::template Write<RW> (output_stream, d6);
	Read_Write<T7>::template Write<RW> (output_stream, d7);
}

template<class RW, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
inline void Write_Binary (std::ostream& output_stream, const T1& d1, const T2& d2, const T3& d3, const T4& d4, const T5& d5, const T6& d6, const T7& d7, const T8& d8)
{
	Read_Write<T1>::template Write<RW> (output_stream, d1);
	Read_Write<T2>::template Write<RW> (output_stream, d2);
	Read_Write<T3>::template Write<RW> (output_stream, d3);
	Read_Write<T4>::template Write<RW> (output_stream, d4);
	Read_Write<T5>::template Write<RW> (output_stream, d5);
	Read_Write<T6>::template Write<RW> (output_stream, d6);
	Read_Write<T7>::template Write<RW> (output_stream, d7);
	Read_Write<T8>::template Write<RW> (output_stream, d8);
}

template<class RW, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
inline void Write_Binary (std::ostream& output_stream, const T1& d1, const T2& d2, const T3& d3, const T4& d4, const T5& d5, const T6& d6, const T7& d7, const T8& d8, const T9& d9)
{
	Read_Write<T1>::template Write<RW> (output_stream, d1);
	Read_Write<T2>::template Write<RW> (output_stream, d2);
	Read_Write<T3>::template Write<RW> (output_stream, d3);
	Read_Write<T4>::template Write<RW> (output_stream, d4);
	Read_Write<T5>::template Write<RW> (output_stream, d5);
	Read_Write<T6>::template Write<RW> (output_stream, d6);
	Read_Write<T7>::template Write<RW> (output_stream, d7);
	Read_Write<T8>::template Write<RW> (output_stream, d8);
	Read_Write<T9>::template Write<RW> (output_stream, d9);
}
//#####################################################################
// Read/Write_Binary_Array
//#####################################################################
// array is C-style (zero-based) array
template<class RW, class T>
inline void Read_Binary_Array (std::istream& input_stream, T* array, const int number_of_elements)
{
	for (int i = 0; i < number_of_elements; i++) Read_Write<T>::template Read<RW> (input_stream, array[i]);
}

template<class RW, class T>
inline void Write_Binary_Array (std::ostream& output_stream, const T* array, const int number_of_elements)
{
	for (int i = 0; i < number_of_elements; i++) Read_Write<T>::template Write<RW> (output_stream, array[i]);
}
//#####################################################################
}
#endif
