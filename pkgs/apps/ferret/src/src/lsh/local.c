/* AUTORIGHTS
Copyright (C) 2007 Princeton University
      
This file is part of Ferret Toolkit.

Ferret Toolkit is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include <math.h>
#ifdef _OPENMP
#include <omp.h>
#endif
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multiroots.h>
#include <cass.h>
#include "LSH.h"
#include "local.h"

#define ERR	1e-4

struct local_params
{
	double a;
	double b;
	double W;
	int M;
	int L;
	int K;
};


static inline double p_col_helper (double x)
{
	double result;
	result = 2.0*gsl_cdf_ugaussian_P(x) - 1.0;
	result += sqrt(2.0/M_PI) * (exp(-x*x/2.0)-1.0)/x;
	return result; 
}

static inline double p_col (double x, double W, int M, int L)
{
	return 1.0 - pow(1.0 - pow(p_col_helper(W/x), M), L);
}

static int fun (const gsl_vector *x, void *params, gsl_vector *f)
{
	double W = ((struct local_params *)params)->W;
	int M = ((struct local_params *)params)->M;
	int L = ((struct local_params *)params)->L;
	int K = ((struct local_params *)params)->K;
	double fa = ((struct local_params *)params)->a;
	double fb = ((struct local_params *)params)->b;
	double sxx, sxy, sx, sy, k, d;
	double a, b;
	int n;

	double alpha = gsl_vector_get(x, 0);
	double beta = gsl_vector_get(x, 1);

	sxx = sxy = sx = sy = 0;

	n = 0;
	k = 0;
	while ((k < K) && (n < 1000))
	{
		double lk, ld;
		n++;
		d = alpha * pow(n, beta);
		k += p_col(sqrt(d), W, M, L);
		lk = log(k);
		ld = log(d);
		sx += lk;
		sy += ld;
		sxx += lk * lk;
		sxy += lk * ld;
	}
	
	least_squares(&a, &b, n, sxx, sxy, sx, sy);

	a = exp(a);

	gsl_vector_set(f, 0, a - fa);
	gsl_vector_set(f, 1, b - fb);
	return GSL_SUCCESS;
}

int print_state (size_t iter, gsl_multiroot_fsolver *s)
{
	fprintf(stderr, "iter = %3u x = %.3f %.3f "
		"f(x) = %.3e %.3e\n",
		iter, 
		gsl_vector_get(s->x, 0),
		gsl_vector_get(s->x, 1),
		gsl_vector_get(s->f, 0),
		gsl_vector_get(s->f, 1));
	return 0;
}

int localdim (double a, double b, double *alpha, double *beta, double W, int M, int L, int K)
{
	const gsl_multiroot_fsolver_type *T = NULL;
	gsl_multiroot_fsolver *s = NULL;

	int status;
	size_t iter = 0;

	struct local_params params = {.a = a, .b = b, .W = W, .M = M, .L = L, .K = K};
	gsl_multiroot_function f = {&fun, 2, &params};

	gsl_vector *x = gsl_vector_alloc(2);
	gsl_vector_set(x, 0, a);
	gsl_vector_set(x, 1, b);

	if (s == NULL)
	{
		T = gsl_multiroot_fsolver_hybrids;
		s = gsl_multiroot_fsolver_alloc(T, 2);
	}

	gsl_multiroot_fsolver_set(s, &f, x);

	do
	{
		status = gsl_multiroot_fsolver_iterate(s);

//		print_state(iter, s);

		if (status) break;

		status = gsl_multiroot_test_residual(s->f, ERR);
		iter++;
	} while ((status == GSL_CONTINUE) && (iter < 1000));

//	fprintf(stderr, "status = %s\n", gsl_strerror(status));
	*alpha = gsl_vector_get(s->x, 0);
	*beta = gsl_vector_get(s->x, 1);

	gsl_multiroot_fsolver_free(s);
	gsl_vector_free(x);
	return 0;
}

int LSH_est_init (LSH_est_t *est, int a_step, double a_min, double a_max,
				int b_step, double b_min, double b_max, double W, int M, int L, int K)
{
	double a_delta, b_delta;
	int i, j;
	est->a_step = a_step;
	est->b_step = b_step;
	est->a_min = a_min;
	est->b_min = b_min;
	est->a_max = a_max;
	est->b_max = b_max;
	a_delta = est->a_delta = (a_max - a_min) / a_step;
	b_delta = est->b_delta = (b_max - b_min) / b_step;
	est->a_table = type_matrix_alloc(double , a_step, b_step);
	est->b_table = type_matrix_alloc(double , a_step, b_step);

	for (i = 0; i < a_step; i++)
	{
		for (j = 0; j < b_step; j++)
		{
			localdim(a_min + i * a_delta,
				b_min + j * b_delta,
				&est->a_table[i][j],
				&est->b_table[i][j],
				W, M, L, K);
		}
	}

	return 0;
}

