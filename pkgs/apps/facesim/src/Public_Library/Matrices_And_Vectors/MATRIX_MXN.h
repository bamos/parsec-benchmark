//#####################################################################
// Copyright 2002-2004, Ronald Fedkiw, Igor Neverov, Eftychios Sifakis, Huamin Wang, Rachel Weinstein.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class MATRIX_MXN
//#####################################################################
#ifndef __MATRIX_MXN__
#define __MATRIX_MXN__

#include <assert.h>
#include "VECTOR_ND.h"
#include "MATRIX_NXN.h"
#include "../Math_Tools/sqr.h"
namespace PhysBAM
{

template<class T>
class MATRIX_MXN
{
public:
	int m, n; // size of the m by n matrix
	T *x; // pointer to the one dimensional data
	MATRIX_MXN<T> *Q, *V;
	MATRIX_NXN<T> *R;

	MATRIX_MXN()
		: m (0), n (0), x (0), Q (0), V (0), R (0)
	{}

	MATRIX_MXN (const int m_input, const int n_input)
		: m (m_input), n (n_input), Q (0), V (0), R (0)
	{
		x = new T[m * n];

		for (int k = 0; k < m * n; k++) x[k] = 0;
	}

	MATRIX_MXN (const MATRIX_MXN<T>& A)
		: m (A.m), n (A.n), Q (0), V (0), R (0)
	{
		x = new T[m * n];

		for (int k = 0; k < m * n; k++) x[k] = A.x[k];
	}

	~MATRIX_MXN()
	{
		if (x) delete[] x;

		if (Q) delete Q;

		if (R) delete (R);

		if (V) delete (V);
	}

	T& operator() (const int i, const int j)
	{
		assert (i >= 1 && i <= m);
		assert (j >= 1 && j <= n);
		return * (x + (j - 1) * m + (i - 1));
	}

	const T& operator() (const int i, const int j) const
	{
		assert (i >= 1 && i <= m);
		assert (j >= 1 && j <= n);
		return * (x + (j - 1) * m + (i - 1));
	}

	MATRIX_MXN<T>& operator= (const MATRIX_MXN<T>& A)
	{
		delete Q;
		delete V;
		delete R;
		Q = V = 0;
		R = 0;

		if (!x || m*n != A.m * A.n)
		{
			delete[] x;
			x = new T[A.m * A.n];
		};

		m = A.m;

		n = A.n;

		for (int k = 0; k < m * n; k++) x[k] = A.x[k];

		return *this;
	}

	MATRIX_MXN<T> operator- (const MATRIX_MXN<T>& A) const
	{
		assert (n == A.n && m == A.m);
		int size = m * n;
		MATRIX_MXN<T> matrix (m, n);

		for (int i = 0; i < size; i++) matrix.x[i] = x[i] - A.x[i];

		return matrix;
	}

	VECTOR_ND<T> operator* (const VECTOR_ND<T>& y) const
	{
		assert (y.n == n);
		VECTOR_ND<T> result (m);

		for (int j = 1; j <= n; j++) for (int i = 1; i <= m; i++) result (i) += * (x + (j - 1) * m + i - 1) * y (j);

		return result;
	}

	MATRIX_MXN<T> operator* (const MATRIX_MXN<T>& A) const
	{
		assert (n == A.m);
		MATRIX_MXN<T> matrix (m, A.n);

		for (int j = 1; j <= A.n; j++) for (int i = 1; i <= m; i++) for (int k = 1; k <= n; k++) matrix (i, j) += * (x + (k - 1) * m + i - 1) * A (k, j);

		return matrix;
	}

	void Write_To_Screen()
	{
		for (int i = 1; i <= m; i++)
		{
			for (int j = 1; j <= n; j++) std::cout << (*this) (i, j) << " ";

			std::cout << std::endl;
		}
	}

	MATRIX_MXN<T> Transpose()
	{
		MATRIX_MXN<T> matrix (n, m);

		for (int i = 1; i <= m; i++) for (int j = 1; j <= n; j++) matrix (j, i) = * (x + (j - 1) * m + (i - 1));

		return matrix;
	}

	VECTOR_ND<T> Transpose_Times (const VECTOR_ND<T>& y) const
	{
		assert (y.n == m);
		VECTOR_ND<T> result (n);

		for (int j = 1; j <= n; j++) for (int i = 1; i <= m; i++) result (j) += * (x + (j - 1) * m + i - 1) * y (i);

		return result;
	}

	MATRIX_MXN<T> Transpose_Times (const MATRIX_MXN<T>& A) const
	{
		assert (n == A.m);
		MATRIX_MXN<T> matrix (m, A.n);

		for (int j = 1; j <= A.n; j++) for (int i = 1; i <= m; i++) for (int k = 1; k <= n; k++) matrix (i, j) += * (x + (i - 1) * m + k - 1) * A (k, j);

		return matrix;
	}

	T Infinity_Norm() const
	{
		T max_sum = 0;

		for (int j = 1; j <= n; j++)
		{
			T sum = 0;

			for (int i = 1; i <= m; i++) sum += fabs (* (x + (j - 1) * m + i - 1));

			max_sum = max (sum, max_sum);
		}

		return max_sum;
	}

//#####################################################################

	MATRIX_NXN<T> Normal_Equations_Matrix() const
	{
		MATRIX_NXN<T> result (n);

		for (int j = 1; j <= n; j++) for (int i = 1; i <= n; i++) for (int k = 1; k <= m; k++) result (i, j) += * (x + (i - 1) * m + k - 1)** (x + (j - 1) * m + k - 1);

		return result;
	}

	VECTOR_ND<T> Normal_Equations_Solve (const VECTOR_ND<T>& b) const
	{
		MATRIX_NXN<T> A_transpose_A (Normal_Equations_Matrix());
		VECTOR_ND<T> A_transpose_b (Transpose_Times (b));
		return A_transpose_A.Cholesky_Solve (A_transpose_b);
	}

	void Gram_Schmidt_QR_Factorization()
	{
		int i, j, k;

		if (Q) delete Q;

		Q = new MATRIX_MXN<T> (*this);

		if (R) delete (R);

		R = new MATRIX_NXN<T> (n);

		for (j = 1; j <= n; j++) // for each column
		{
			for (i = 1; i <= m; i++) (*R) (j, j) += sqr ( (*Q) (i, j));

			(*R) (j, j) = sqrt ( (*R) (j, j)); // compute the L2 norm
			T one_over_Rjj = 1 / (*R) (j, j);

			for (i = 1; i <= m; i++) (*Q) (i, j) *= one_over_Rjj; // orthogonalize the column

			for (k = j + 1; k <= n; k++) // subtract this columns contributution from the rest of the columns
			{
				for (i = 1; i <= m; i++) (*R) (j, k) += (*Q) (i, j) * (*Q) (i, k);

				for (i = 1; i <= m; i++) (*Q) (i, k) -= (*R) (j, k) * (*Q) (i, j);
			}
		}
	}

	VECTOR_ND<T> Gram_Schmidt_QR_Solve (const VECTOR_ND<T>& b)
	{
		Gram_Schmidt_QR_Factorization();
		VECTOR_ND<T> c (Q->Transpose_Times (b));
		return R->Upper_Triangular_Solve (c);
	}

	void Householder_QR_Factorization()
	{
		int i, j, k;

		if (V) delete V;

		V = new MATRIX_MXN<T> (m, n);

		if (R) delete (R);

		R = new MATRIX_NXN<T> (n);
		MATRIX_MXN<T> temp (*this);

		for (j = 1; j <= n; j++) // for each column
		{
			VECTOR_ND<T> a (m);

			for (i = 1; i <= m; i++) a (i) = temp (i, j);

			VECTOR_ND<T> v = a.Householder_Vector (j);

			for (i = 1; i <= m; i++) (*V) (i, j) = v (i); // store the v's in V

			for (k = j; k <= n; k++) // Householder transform each column
			{
				for (i = 1; i <= m; i++) a (i) = temp (i, k);

				VECTOR_ND<T> new_a = a.Householder_Transform (v);

				for (i = 1; i <= m; i++) temp (i, k) = new_a (i);
			}
		}

		for (i = 1; i <= n; i++) for (j = 1; j <= n; j++) (*R) (i, j) = temp (i, j);
	} // store R

	static VECTOR_ND<T> Householder_Transform (const VECTOR_ND<T>& b, const MATRIX_MXN<T>& V)
	{
		assert (V.m == b.n);
		VECTOR_ND<T> result (b);

		for (int j = 1; j <= V.n; j++)
		{
			VECTOR_ND<T> v (V.m);

			for (int i = 1; i <= V.m; i++) v (i) = V (i, j);

			result = result.Householder_Transform (v);
		}

		return result;
	}

	VECTOR_ND<T> Householder_QR_Solve (const VECTOR_ND<T>& b)
	{
		Householder_QR_Factorization();
		VECTOR_ND<T> c = Householder_Transform (b, *V), c_short (n);

		for (int i = 1; i <= n; i++) c_short (i) = c (i);

		return R->Upper_Triangular_Solve (c_short);
	}

//#####################################################################
};
}
#endif


