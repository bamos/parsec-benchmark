//#####################################################################
// Copyright 2002-2004, Ronald Fedkiw, Neil Molino, Igor Neverov, Eftychios Sifakis, Joseph Teran.
// This file is part of PhysBAM whose distribution is governed by the license contained in the accompanying file PHYSBAM_COPYRIGHT.txt.
//#####################################################################
// Class MATRIX_NXN
//#####################################################################
#ifndef __MATRIX_NXN__
#define __MATRIX_NXN__

#include <assert.h>
#include "VECTOR_ND.h"
#include "../Math_Tools/exchange.h"
#include "../Math_Tools/max.h"
namespace PhysBAM
{

template<class T>
class MATRIX_NXN
{
public:
	int n; // size of the n by n matrix
	T *x; // pointer to the one dimensional data
	T small_number;
	MATRIX_NXN<T> *L, *U, *inverse;
	VECTOR_ND<T> *p;

	MATRIX_NXN()
		: n (0), x (0), small_number ( (T) 1e-8), L (0), U (0), inverse (0), p (0)
	{}

	MATRIX_NXN (const int n_input)
		: n (n_input), small_number ( (T) 1e-8), L (0), U (0), inverse (0), p (0)
	{
		x = new T[n * n];

		for (int k = 0; k < n * n; k++) x[k] = 0;
	}

	MATRIX_NXN (const MATRIX_NXN<T>& matrix_input)
		: n (matrix_input.n), small_number (matrix_input.small_number), L (0), U (0), inverse (0), p (0)
	{
		x = new T[n * n];

		for (int k = 0; k < n * n; k++) x[k] = matrix_input.x[k];
	}

	~MATRIX_NXN()
	{
		if (x) delete [] x;

		if (L) delete L;

		if (U) delete (U);

		if (p) delete p;

		if (inverse) delete inverse;
	}

	T& operator() (const int i, const int j)
	{
		assert (i >= 1 && i <= n);
		assert (j >= 1 && j <= n);
		return * (x + (i - 1) * n + (j - 1));
	}

	const T& operator() (const int i, const int j) const
	{
		assert (i >= 1 && i <= n);
		assert (j >= 1 && j <= n);
		return * (x + (i - 1) * n + (j - 1));
	}

	MATRIX_NXN<T>& operator= (const MATRIX_NXN<T>& A)
	{
		delete L;
		delete U;
		delete inverse;
		delete p;
		L = U = inverse = 0;
		p = 0;

		if (!x || n != A.n)
		{
			delete[] x;
			x = new T[A.n * A.n];
		}

		n = A.n;

		for (int k = 0; k < n * n; k++) x[k] = A.x[k];

		return *this;
	}

	MATRIX_NXN<T>& operator+= (const MATRIX_NXN<T>& A)
	{
		for (int i = 0; i < n * n; i++) x[i] += A.x[i];

		return *this;
	}

	MATRIX_NXN<T>& operator-= (const MATRIX_NXN<T>& A)
	{
		for (int i = 0; i < n * n; i++) x[i] -= A.x[i];

		return *this;
	}

	MATRIX_NXN<T>& operator*= (const T a)
	{
		for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) (* (x + i * n + j)) *= a;

		return *this;
	}

	MATRIX_NXN<T> operator+ (const MATRIX_NXN<T>& A) const
	{
		assert (A.n == n);
		MATRIX_NXN<T> result (n);

		for (int i = 0; i < n * n; i++) result.x[i] = x[i] + A.x[i];

		return result;
	}

	MATRIX_NXN<T> operator- (const MATRIX_NXN<T>& A) const
	{
		assert (A.n == n);
		MATRIX_NXN<T> result (n);

		for (int i = 0; i < n * n; i++) result.x[i] = x[i] - A.x[i];

		return result;
	}

	MATRIX_NXN<T> operator* (const T a) const
	{
		MATRIX_NXN<T> result (n);

		for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) (* (result.x + i * n + j)) = (* (x + i * n + j)) * a;

		return result;
	}

	VECTOR_ND<T> operator* (const VECTOR_ND<T>& y) const
	{
		assert (y.n == n);
		VECTOR_ND<T> result (n);

		for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) (* (result.x + i)) += (* (x + i * n + j)) * (* (y.x + j));

		return result;
	}

	VECTOR_ND<T> Transpose_Times (const VECTOR_ND<T>& y) const
	{
		assert (y.n == n);
		VECTOR_ND<T> result (n);

		for (int j = 0; j < n; j++) for (int i = 0; i < n; i++) (* (result.x + i)) += (* (x + j * n + i)) * (* (y.x + j));

		return result;
	}

	MATRIX_NXN<T> operator* (const MATRIX_NXN<T>& A) const
	{
		assert (A.n == n);
		MATRIX_NXN<T> result (n);

		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				T total = (T) 0;

				for (int k = 0; k < n; k++) total += (* (x + i * n + k)) * (* (A.x + k * n + j));

				(* (result.x + i * n + j)) = total;
			}
		}

		return result;
	}

	void Set_Identity_Matrix()
	{
		Set_Zero_Matrix();

		for (int i = 1; i <= n; i++) (*this) (i, i) = 1;
	}

	void Set_Zero_Matrix()
	{
		for (int i = 0; i < n * n; i++) x[i] = 0;
	}

	static MATRIX_NXN<T> Identity_Matrix (const int n)
	{
		MATRIX_NXN<T> A (n);

		for (int i = 1; i <= n; i++) A (i, i) = (T) 1;

		return A;
	}

	MATRIX_NXN<T> Transpose() const
	{
		MATRIX_NXN<T> A (n);

		for (int i = 1; i <= n; i++) for (int j = i; j <= n; j++) A (i, j) = A (j, i) = * (x + (i - 1) * n + (j - 1));

		return A;
	}

	T Trace() const
	{
		T trace = 0;

		for (int i = 1; i <= n; i++) trace += (* (x + (i - 1) * n + (i - 1)));

		return trace;
	}

	MATRIX_NXN<T> Normal_Equations_Matrix() const
	{
		MATRIX_NXN<T> result (n);

		for (int k = 1; k <= n; k++) for (int i = 1; i <= n; i++) for (int j = 1; j <= n; j++) result (i, j) += * (x + (k - 1) * n + i - 1)** (x + (k - 1) * n + j - 1);

		return result;
	}

	VECTOR_ND<T> Lower_Triangular_Solve (const VECTOR_ND<T>& b) const
	{
		assert (n == b.n);
		VECTOR_ND<T> x (n);

		for (int i = 1; i <= n; i++)
		{
			x (i) = b (i);

			for (int j = 1; j <= i - 1; j++) x (i) -= (*this) (i, j) * x (j);

			x (i) /= (*this) (i, i);
		}

		return x;
	}

	VECTOR_ND<T> Upper_Triangular_Solve (const VECTOR_ND<T>& b) const
	{
		assert (n == b.n);
		VECTOR_ND<T> x (n);

		for (int i = n; i >= 1; i--)
		{
			x (i) = b (i);

			for (int j = n; j >= i + 1; j--) x (i) -= (*this) (i, j) * x (j);

			x (i) /= (*this) (i, i);
		}

		return x;
	}

	void LU_Factorization()
	{
		int i, j, k;

		if (L) delete L;

		L = new MATRIX_NXN<T> (n);

		if (U) delete (U);

		U = new MATRIX_NXN<T> (*this);

		for (j = 1; j <= n; j++) // for each column
		{
			for (i = j; i <= n; i++) (*L) (i, j) = (*U) (i, j) / (*U) (j, j); // fill in the column for L

			for (i = j + 1; i <= n; i++) for (k = j; k <= n; k++) (*U) (i, k) -= (*L) (i, j) * (*U) (j, k);
		}
	} // sweep across each row below row j

	VECTOR_ND<T> LU_Solve (const VECTOR_ND<T>& b)
	{
		LU_Factorization();
		return U->Upper_Triangular_Solve (L->Lower_Triangular_Solve (b));
	}

	void LU_Inverse()
	{
		if (inverse) delete inverse;

		inverse = new MATRIX_NXN<T> (n);
		LU_Factorization();

		for (int j = 1; j <= n; j++) // for each column
		{
			VECTOR_ND<T> b (n);
			b (j) = 1; // piece of the identity matrix
			VECTOR_ND<T> x = U->Upper_Triangular_Solve (L->Lower_Triangular_Solve (b));

			for (int i = 1; i <= n; i++) (*inverse) (i, j) = x (i);
		}
	}

	void PLU_Factorization()
	{
		int i, j, k;

		if (L) delete L;

		L = new MATRIX_NXN<T> (n);

		if (U) delete (U);

		U = new MATRIX_NXN<T> (*this);

		if (p) delete p;

		p = new VECTOR_ND<T> (n);

		for (i = 1; i <= n; i++) (*p) (i) = (T) i; // initialize p

		for (j = 1; j <= n; j++) // for each column
		{
			// find the largest element and switch rows
			int row = j;
			T value = fabs ( (*U) (j, j));

			for (i = j + 1; i <= n; i++) if (fabs ( (*U) (i, j)) > value)
				{
					row = i;
					value = fabs ( (*U) (i, j));
				}

			if (row != j) // need to switch rows
			{
				exchange ( (*p) (j), (*p) (row)); // update permutation matrix

				for (k = 1; k <= j - 1; k++) exchange ( (*L) (j, k), (*L) (row, k)); // update L

				for (k = j; k <= n; k++) exchange ( (*U) (j, k), (*U) (row, k));
			} // update U

			// standard LU factorization steps
			for (i = j; i <= n; i++) (*L) (i, j) = (*U) (i, j) / (*U) (j, j); // fill in the column for L

			for (i = j + 1; i <= n; i++) for (k = j; k <= n; k++) (*U) (i, k) -= (*L) (i, j) * (*U) (j, k);
		}
	} // sweep across each row below row j

	VECTOR_ND<T> PLU_Solve (const VECTOR_ND<T>& b)
	{
		PLU_Factorization();
		VECTOR_ND<T> x = b;
		return U->Upper_Triangular_Solve (L->Lower_Triangular_Solve (x.Permute (*p)));
	}

	void PLU_Inverse()
	{
		if (inverse) delete inverse;

		inverse = new MATRIX_NXN<T> (n);
		PLU_Factorization();

		for (int j = 1; j <= n; j++) // for each column
		{
			VECTOR_ND<T> b (n);
			b (j) = 1; // piece of the identity matrix
			VECTOR_ND<T> x = U->Upper_Triangular_Solve (L->Lower_Triangular_Solve (b.Permute (*p)));

			for (int i = 1; i <= n; i++) (*inverse) (i, j) = x (i);
		}
	}

	void Gauss_Seidel_Single_Iteration (VECTOR_ND<T>& x, const VECTOR_ND<T>& b) const
	{
		assert (x.n == b.n && x.n == n);

		for (int i = 1; i <= n; i++)
		{
			T rho = 0;

			for (int j = 1; j < i; j++) rho += (*this) (i, j) * x (j);

			for (int j = i + 1; j <= n; j++) rho += (*this) (i, j) * x (j);

			x (i) = (b (i) - rho) / (*this) (i, i);
		}
	}

	T Norm() const // L_infinity norm - maximum row sum
	{
		T max_sum = 0;

		for (int i = 1; i <= n; i++)
		{
			T sum = 0;

			for (int j = 1; j <= n; j++) sum += fabs ( (*this) (i, j));

			max_sum = max (sum, max_sum);
		}

		return max_sum;
	}

	T Condition_Number()
	{
		PLU_Inverse();        // makes sure the inverse is up to date
		return Norm() * inverse->Norm();
	}

	void Cholesky_Factorization() // LL^{transpose} factorization
	{
		int i, j, k;

		if (L) delete L;

		L = new MATRIX_NXN<T> (n);

		if (U) delete (U);

		U = new MATRIX_NXN<T> (*this);

		for (j = 1; j <= n; j++) // for each column
		{
			for (k = 1; k <= j - 1; k++) for (i = j; i <= n; i++) (*U) (i, j) -= (*L) (j, k) * (*L) (i, k); // subtract off the known stuff in previous columns

			(*L) (j, j) = sqrt ( (*U) (j, j));

			for (i = j + 1; i <= n; i++) (*L) (i, j) = (*U) (i, j) / (*L) (j, j);
		} // update L

		for (i = 1; i <= n; i++) for (j = 1; j <= n; j++) (*U) (i, j) = (*L) (j, i);
	} // put L^{transpose} into U

	VECTOR_ND<T> Cholesky_Solve (const VECTOR_ND<T>& b)
	{
		Cholesky_Factorization();
		return U->Upper_Triangular_Solve (L->Lower_Triangular_Solve (b));
	}

	void Cholesky_Inverse()
	{
		if (inverse) delete inverse;

		inverse = new MATRIX_NXN<T> (n);
		Cholesky_Factorization();

		for (int j = 1; j <= n; j++) // for each column
		{
			VECTOR_ND<T> b (n);
			b (j) = 1; // piece of the identity matrix
			VECTOR_ND<T> x = U->Upper_Triangular_Solve (L->Lower_Triangular_Solve (b));

			for (int i = 1; i <= n; i++) (*inverse) (i, j) = x (i);
		}
	}

	static bool Conjugate_Gradient (const MATRIX_NXN<T>& A, VECTOR_ND<T>& x, const VECTOR_ND<T>& b, const int max_iter = 30, const T tolerance = 1e-6)
	{
		T alpha, beta, rho, rho_1;
		VECTOR_ND<T> p (b.n), q (b.n), r (b.n);
		r = b - A * x;

		if (r.Sup_Norm() <= tolerance) return true;

		for (int i = 1; i <= max_iter; i++)
		{
			rho = VECTOR_ND<T>::Dot_Product (r, r);

			if (i == 1) p = r;
			else
			{
				beta = rho / rho_1;
				p = r + beta * p;
			}

			q = A * p;
			alpha = rho / VECTOR_ND<T>::Dot_Product (p, q);
			x += alpha * p;
			r -= alpha * q;

			if (r.Sup_Norm() <= tolerance) return true;

			rho_1 = rho;
		}

		return false;
	}

	void Write_To_Screen() const
	{
		for (int i = 1; i <= n; i++)
		{
			for (int j = 1; j <= n; j++) std::cout << (*this) (i, j) << " ";

			std::cout << std::endl;
		}
	}

//#####################################################################
};
template<class T> inline MATRIX_NXN<T> operator* (const T a, const MATRIX_NXN<T>& A)
{
	MATRIX_NXN<T> result (A.n);

	for (int i = 0; i < A.n; i++) for (int j = 0; j < A.n; j++) (* (result.x + i * A.n + j)) = a * (* (A.x + i * A.n + j));

	return result;
}

template<class T> inline MATRIX_NXN<T> operator* (const int a, const MATRIX_NXN<T>& A)
{
	MATRIX_NXN<T> result (A.n);

	for (int i = 0; i < A.n; i++) for (int j = 0; j < A.n; j++) (* (result.x + i * A.n + j)) = a * (* (A.x + i * A.n + j));

	return result;
}

template<class T> inline std::istream& operator>> (std::istream& input_stream, MATRIX_NXN<T>& A)
{
	for (int i = 0; i < A.n; i++) for (int j = 0; j < A.n; j++) input_stream >> (* (A.x + i * A.n + j));

	return input_stream;
}

template<class T> inline std::ostream& operator<< (std::ostream& output_stream, const MATRIX_NXN<T>& A)
{
	for (int i = 0; i < A.n; i++)
	{
		for (int j = 0; j < A.n; j++) output_stream << (* (A.x + i * A.n + j)) << " ";

		output_stream << std::endl;
	}

	return output_stream;
}
}
#endif
