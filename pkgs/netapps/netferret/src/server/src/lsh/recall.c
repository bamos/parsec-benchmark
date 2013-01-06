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
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <fenv.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_vector.h>
#include "LSH.h"
#include "perturb.h"

#define MISS_GAMMA	1

#define MAX_M	64
#define MAX_T	600

static inline double p_col_helper (double x)
{
	double result;
	result = 2.0*gsl_cdf_ugaussian_P(x) - 1.0;
	result += sqrt(2.0/M_PI) * (exp(-x*x/2.0)-1.0)/x;
	return result; 
}

static inline double p_col_p_helper (double x, double k)
{
	return gsl_cdf_ugaussian_P((1.0 + k) * x) - gsl_cdf_ugaussian_P(k * x);
}

int LSH_recall_init (LSH_recall_t *recall, int d_step, float d_min, float d_max,
		int M, int L, int T, float W)
{
	ptb_vec_t *ptb_score;
	ptb_vec_t *ptb_set;
	float **table; 
	double dist, r, p;
	int i, j, k;
	int l;

	ptb_score = gen_score(M);
	assert(ptb_score != NULL);
	ptb_set = type_calloc(ptb_vec_t, T);
	assert(ptb_set != NULL);
	gen_perturb_set(ptb_score, ptb_set, M, T);

	recall->d_step = d_step;
	recall->d_min = d_min;
	recall->d_max = d_max;
	recall->T = T;
	recall->table = table = type_matrix_alloc(float, T + 1, d_step);

	for (i = 0; i < d_step; i++)
	{
		dist = d_min + (d_max - d_min) * i / d_step;
		dist = W / sqrt(dist);
		p = p_col_helper(dist);
		table[0][i] = log(1.0 - pow(p, M));
		for (j = 1; j <= T; j++)
		{
			r = 1.0;
			l = 0;
			for (k = 0; k < 2 * M; k++)
			if (ptb_set[j-1].set & (1 << k))
			{
				r *= p_col_p_helper(dist, ptb_score[k].key1);
				l++;
			}
			assert(l > 0);
			l = M - l;
			r *= pow(p, l);
			table[j][i] = table[j-1][i] + log(1.0-r);
		}
		for (j = 0; j <= T; j++)
		{
			table[j][i] = 1.0 - exp(L * table[j][i]);
		}
	}

	free(ptb_score);
	free(ptb_set);
	return 0;
}

int LSH_recall_dump (LSH_recall_t *recall, CASS_FILE *fout)
{
	int ret;
	ret = cass_write_size(&recall->d_step, 1, fout);
	ret += cass_write_size(&recall->T, 1, fout);
	ret += cass_write_float(&recall->d_min, 1, fout);
	ret += cass_write_float(&recall->d_max, 1, fout);

	if (ret != 4) return CASS_ERR_IO;

	if (type_matrix_dump_stream(float, fout, recall->T + 1, recall->d_step, recall->table) != 0) return CASS_ERR_IO;

	return 0;
}

int LSH_recall_load (LSH_recall_t *recall, CASS_FILE *fin)
{
	int ret;
	cass_size_t row, col;
	ret = cass_read_size(&recall->d_step, 1, fin);
	ret += cass_read_size(&recall->T, 1, fin);
	ret += cass_read_float(&recall->d_min, 1, fin);
	ret += cass_read_float(&recall->d_max, 1, fin);

	if (ret != 4) return CASS_ERR_IO;

	type_matrix_load_stream(float, fin, &row, &col, &recall->table);

	assert(row == recall->T +1);
	assert(col == recall->d_step);

	return 0;
}

