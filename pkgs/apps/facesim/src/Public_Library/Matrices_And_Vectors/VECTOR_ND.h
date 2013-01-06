//#####################################################################
// Copyright 2002-2005, Ronald Fedkiw, Frank Losasso, Igor Neverov, Eftychios Sifakis, Joseph Teran, Rachel Weinstein.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class VECTOR_ND
//#####################################################################
#ifndef __VECTOR_ND__
#define __VECTOR_ND__

#include <assert.h>
#include <iostream>
#include <math.h>
#include "../Math_Tools/sqr.h"
#include "../Math_Tools/maxabs.h"
#include "../Read_Write/READ_WRITE_FUNCTIONS.h"
namespace PhysBAM
{

template<class T>
class VECTOR_ND
{
public:
	int n; // size of the n vector
	T* x; // pointer to the n vector

	explicit VECTOR_ND (const int n_input = 0)
		: n (n_input)
	{
		x = new T[n];

		for (int k = 0; k < n; k++) x[k] = 0;
	}

	VECTOR_ND (const VECTOR_ND<T>& vector_input)
		: n (vector_input.n)
	{
		x = new T[n];

		for (int k = 0; k < n; k++) x[k] = vector_input.x[k];
	}

	~VECTOR_ND()
	{
		delete[] x;
	}

	const VECTOR_ND<T>& operator= (const VECTOR_ND<T>& vector_input)
	{
		n = vector_input.n;
		delete[] x;
		x = new T[n];

		for (int k = 0; k < n; k++) x[k] = vector_input.x[k];

		return *this;
	}

	T& operator() (const int i)
	{
		assert (i >= 1 && i <= n);
		return * (x + i - 1);
	}

	const T& operator() (const int i) const
	{
		assert (i >= 1 && i <= n);
		return * (x + i - 1);
	}

	VECTOR_ND<T>& operator+= (const VECTOR_ND<T>& vector_input)
	{
		assert (vector_input.n >= n);

		for (int i = 0; i < n; i++) * (x + i) += * (vector_input.x + i);

		return *this;
	}

	VECTOR_ND<T>& operator-= (const VECTOR_ND<T>& vector_input)
	{
		assert (vector_input.n >= n);

		for (int i = 0; i < n; i++) * (x + i) -= * (vector_input.x + i);

		return *this;
	}

	VECTOR_ND<T>& operator-= (const T a)
	{
		for (int i = 0; i < n; i++) * (x + i) -= a;

		return *this;
	}

	VECTOR_ND<T>& operator*= (const T a)
	{
		for (int i = 0; i < n; i++) * (x + i) *= a;

		return *this;
	}

	VECTOR_ND<T>& operator/= (const T a)
	{
		assert (a != 0);
		T one_over_a = 1 / a;

		for (int i = 0; i < n; i++) * (x + i) *= one_over_a;

		return *this;
	}

	VECTOR_ND<T> operator+ (const VECTOR_ND<T>& vector_input) const
	{
		assert (vector_input.n >= n);
		VECTOR_ND<T> result (n);

		for (int i = 0; i < n; i++) (* (result.x + i)) = (* (x + i)) + (* (vector_input.x + i));

		return result;
	}

	VECTOR_ND<T> operator-() const
	{
		VECTOR_ND<T> result (*this);

		for (int i = 0; i < n; i++) * (result.x + i) = -* (result.x + i);

		return result;
	}

	VECTOR_ND<T> operator- (const VECTOR_ND<T>& vector_input) const
	{
		assert (vector_input.n >= n);
		VECTOR_ND<T> result (n);

		for (int i = 0; i < n; i++) (* (result.x + i)) = (* (x + i)) - (* (vector_input.x + i));

		return result;
	}

	VECTOR_ND<T> operator* (const T a) const
	{
		VECTOR_ND<T> result (*this);

		for (int i = 0; i < n; i++) * (result.x + i) *= a;

		return result;
	}

	VECTOR_ND<T> operator/ (const T a) const
	{
		assert (a != 0);
		T one_over_a = 1 / a;
		VECTOR_ND<T> result (*this);

		for (int i = 0; i < n; i++) * (result.x + i) *= one_over_a;

		return result;
	}

	void Resize_Array (const int n_new)
	{
		if (n_new == n) return;

		T* x_new = new T[n_new];
		int n1 = n < n_new ? n : n_new;

		for (int i = 0; i < n1; i++) x_new[i] = x[i];

		for (int i = n1; i < n_new; i++) x_new[i] = 0;

		delete[] x;
		x = x_new;
		n = n_new;
	}

	T Magnitude_Squared() const
	{
		T norm_squared = 0;

		for (int i = 0; i < n; i++) norm_squared += sqr (* (x + i));

		return norm_squared;
	}

	T Magnitude() const
	{
		T norm_squared = 0;

		for (int i = 0; i < n; i++) norm_squared += sqr (* (x + i));

		return sqrt (norm_squared);
	}

	T Sup_Norm() const
	{
		T norm = fabs (*x);

		for (int i = 1; i < n; i++)
		{
			T a = fabs (* (x + i));

			if (a > norm) norm = a;
		}

		return norm;
	}

	double Double_Precision_Maxabs()
	{
		double result = fabs (x[0]);

		for (int index = 1; index < n; index++) result = maxabs_incremental (result, (double) x[index]);

		return result;
	}

	T Maxabs() const
	{
		T result = (T) fabs (x[0]);

		for (int index = 1; index < n; index++) result = maxabs_incremental (result, x[index]);

		return result;
	}

	T Sum()
	{
		T result = x[0];

		for (int index = 1; index < n; index++) result += x[index];

		return result;
	}

	double Double_Precision_Sum()
	{
		double result = x[0];

		for (int index = 1; index < n; index++) result += x[index];

		return result;
	}

	static T Dot_Product (const VECTOR_ND<T>& v1, const VECTOR_ND<T>& v2)
	{
		T sum = 0;
		assert (v2.n == v1.n);

		for (int i = 0; i < v1.n; i++) sum += (* (v1.x + i)) * (* (v2.x + i));

		return sum;
	}

	static double Double_Precision_Dot_Product (const VECTOR_ND<T>& v1, const VECTOR_ND<T>& v2)
	{
		double sum = 0;
		assert (v2.n == v1.n);

		for (int i = 0; i < v1.n; i++) sum += (* (v1.x + i)) * (* (v2.x + i));

		return sum;
	}


	VECTOR_ND<T> Permute (const VECTOR_ND<T>& p)
	{
		assert (n == p.n);
		VECTOR_ND<T> x (n);

		for (int i = 1; i <= n; i++) x (i) = (*this) ( (int) p (i));

		return x;
	}

	VECTOR_ND<T> Householder_Vector (const int k)
	{
		VECTOR_ND<T> v (n);
		T v_dot_v = 0;

		for (int i = k; i <= n; i++)
		{
			v (i) = (*this) (i);
			v_dot_v += sqr (v (i));
		}

		if ( (*this) (k) >= 0) v (k) += sqrt (v_dot_v);
		else v (k) -= sqrt (v_dot_v);

		return v;
	}

	VECTOR_ND<T> Householder_Transform (VECTOR_ND<T>& v)
	{
		int i;
		assert (n = v.n);
		VECTOR_ND<T> x (n);
		T v_dot_a = 0, v_dot_v = 0;

		for (i = 1; i <= n; i++)
		{
			v_dot_a += v (i) * (*this) (i);
			v_dot_v += sqr (v (i));
		}

		T coefficient = 2 * v_dot_a / v_dot_v;

		for (i = 1; i <= n; i++) x (i) = (*this) (i) - coefficient * v (i);

		return x;
	}

	void Write_To_Screen() const
	{
		for (int i = 1; i <= n; i++) std::cout << (*this) (i) << std::endl;
	}

	template<class RW>
	void Read (std::istream& input_stream)
	{
		Read_Binary<RW> (input_stream, n);
		delete [] x;
		x = new T[n];
		Read_Binary_Array<RW> (input_stream, x, n);
	}

	template<class RW>
	void Write (std::ostream& output_stream) const
	{
		Write_Binary<RW> (output_stream, n);
		Write_Binary_Array<RW> (output_stream, x, n);
	}

//#####################################################################
};
template<class T> inline VECTOR_ND<T> operator* (const T a, const VECTOR_ND<T>& v)
{
	VECTOR_ND<T> result (v.n);

	for (int i = 0; i < v.n; i++) * (result.x + i) = (* (v.x + i)) * a;

	return result;
}

template<class T> inline VECTOR_ND<T> operator* (const int a, const VECTOR_ND<T>& v)
{
	VECTOR_ND<T> result (v.n);

	for (int i = 0; i < v.n; i++) * (result.x + i) = (* (v.x + i)) * a;

	return result;
}

template<class T> inline std::istream& operator>> (std::istream& input_stream, VECTOR_ND<T>& v)
{
	for (int i = 0; i < v.n; i++) input_stream >> (* (v.x + (i)));

	return input_stream;
}

template<class T> inline std::ostream& operator<< (std::ostream& output_stream, const VECTOR_ND<T>& v)
{
	for (int i = 0; i < v.n; i++)
	{
		output_stream << (* (v.x + (i))) << " ";
		output_stream << std::endl;
	}

	return output_stream;
}
}
#endif
