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
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include "LSH.h"

extern int LSH_release (cass_table_t *table);
extern int LSH_dump (cass_table_t *table);

int LSH_init_private (cass_table_t *table, const char *param)
		
{
	LSH_t *lsh;
	cass_size_t D, M, L, H;
	cass_size_t n;
	const gsl_rng_type *T;
	gsl_rng *r;
	int i, j;
	float w;

	lsh = type_calloc(LSH_t, 1);
	assert(table->parent_cfg != NULL);
	D = lsh->D = table->parent_cfg->vec_dim;
	M = lsh->M = param_get_int(param, "-M", 10);
	assert(M <= sizeof(uint32_t) * 8);
	L = lsh->L = param_get_int(param, "-L", 1);
	H = lsh->H = param_get_int(param, "-H", 1017881);

	lsh->W = type_calloc(float, L);

	w = param_get_float(param, "-w", 1.0);

	for (i = 0; i < L; i++) lsh->W[i] = w;

	if (strstr(param, "-W") && strstr(param, "-recall")) debug("-W and -recall conflicts.\n");

	param_get_float_array(param, "-W", &n, lsh->W);

	lsh->alphas = type_matrix_alloc(float, L * M, D);
	lsh->rnd = type_matrix_alloc(uint32_t, L, M);
	lsh->betas = (float *)malloc(L * M * sizeof(float));

	lsh->tmp = type_matrix_alloc(uint32_t, L, M);
	lsh->tmp2 = type_calloc(uint32_t, L);

	gsl_rng_env_setup();

	T = gsl_rng_default;
	r = gsl_rng_alloc(T);

	for (i = 0; i < L * M; i++)
	{
		for (j = 0; j < D; j++)
		{
			lsh->alphas[i][j] = gsl_ran_gaussian(r, 1.0);
		}
	}

	for (i = 0; i < L * M; i++)
	{
		lsh->betas[i] = gsl_rng_uniform(r) * lsh->W[i / M];
	}
	gsl_rng_free(r);

	srand(0);
	for (i = 0; i < L; i++)
		for (j = 0; j < M; j++)
			lsh->rnd[i][j] = rand();


	lsh->est = NULL;

	if (strstr(param, "-recall"))
	{
		cass_size_t d_step, max_T;
		float d_min, d_max;

		d_step = param_get_int(param, "-d_step", 100);
		max_T = param_get_int(param, "-T_max", 100);
		d_min = param_get_float(param, "-d_min", 0.0);
		d_max = param_get_float(param, "-d_max", 100);
		LSH_recall_init(&lsh->recall, d_step, d_min, d_max, lsh->M, lsh->L, max_T, lsh->W[0]);
	}

	table->__private = lsh;
	return 0;
}

int LSH_free_private (cass_table_t *table)
{
	LSH_t *lsh = table->__private;
	if (table->loaded) LSH_release(table);
	matrix_free(lsh->rnd);
	free(lsh->betas);
	matrix_free(lsh->alphas);
	free(lsh->W);

	matrix_free(lsh->tmp);
	free(lsh->tmp2);

	if (lsh->recall.table != NULL) LSH_recall_cleanup(&lsh->recall);

	free(lsh);

	table->__private = NULL;
	return 0;
}

void LSH_hash2 (LSH_t *lsh, uint32_t **hash, uint32_t *hash2)
{
	int i, j;
	for (i = 0; i < lsh->L; i++)
	{
		hash2[i] = 0;
		for (j = 0; j < lsh->M; j++)
		{
			hash2[i] += lsh->rnd[i][j] * hash[i][j];
		}
		hash2[i] %= lsh->H;
	}
}

void LSH_hash (LSH_t *lsh, const float *pnt, uint32_t **hash)
{
	float s;
	int i, j, k, l;

	l = 0;
	for (i = 0; i < lsh->L; i++)
	{
		for (j = 0; j < lsh->M; j++)
		{
			s = lsh->betas[l];
			for (k = 0; k < lsh->D; k++)
			{
				s += pnt[k] * lsh->alphas[l][k];
			}

			s /= lsh->W[i];
			hash[i][j] = floor(s);
			l++;
		}
	}
}

static inline void LSH_hash_L (LSH_t *lsh, float *pnt, uint32_t **hash, int L)
{
	float s;
	int j, k, l;

	l = L * lsh->M;	
	for (j = 0; j < lsh->M; j++)
	{
		s = lsh->betas[l];
		for (k = 0; k < lsh->D; k++)
		{
			s += pnt[k] * lsh->alphas[l][k];
		}

		s /= lsh->W[L];
		hash[L][j] = floor(s);
		l++;
	}
}

static inline void LSH_hash2_L (LSH_t *lsh, uint32_t **hash, uint32_t *hash2, int L)
{
	int j;
	hash2[L] = 0;
	for (j = 0; j < lsh->M; j++)
	{
		hash2[L] += lsh->rnd[L][j] * hash[L][j];
	}
	hash2[L] %= lsh->H;
}

static void LSH_insert (LSH_t *lsh, float *pnt, int tag)
{
	int l;
	LSH_hash(lsh, pnt, lsh->tmp);
	LSH_hash2(lsh, lsh->tmp, lsh->tmp2);

	for (l = 0; l < lsh->L; l++)
	{
		ohash_insert(&lsh->hash[l], lsh->tmp2[l], tag);
	}
	lsh->count++;
}

int LSH_batch_insert(cass_table_t *table, cass_dataset_t *parent, cass_vecset_id_t start, cass_vecset_id_t end)
{
	LSH_t *lsh = table->__private;
	uint32_t i, j, k;
	cass_vec_t *vec;
	for (i = start; i <= end; i++)
	{
		for (j = 0;  j < parent->vecset[i].num_regions; j++)
		{
			k =  j + parent->vecset[i].start_vecid;
			vec = DATASET_VEC(parent, k);
			LSH_insert(lsh, vec->u.float_data, k);
			lsh->count++;
		}
	}
	return 0;
}

/*
void LSH_mass_insert_parallel (LSH_t *lsh, float **pnt, int cnt)
{
	int l;
#pragma omp parallel for schedule(static, 1) shared(lsh, pnt, cnt)
	for (l =  0; l < lsh->L; l++)
	{
		printf("#TH = %d / %d\n", omp_get_thread_num(),
				omp_get_num_threads());
		int i;
		for (i = 0; i < cnt; i++)
		{
			LSH_hash_L(lsh, pnt[i], tmp, l);
			LSH_hash2_L(lsh, tmp, tmp2, l);
			ohash_insert(&lsh->hash[l], tmp2[l], i);
		}
	}
}
*/

int LSH_restore_private (cass_table_t *table, CASS_FILE *fin)
{
	int ret;
	cass_size_t L, M, D;
	int32_t recall;
	LSH_t *lsh;

	lsh = type_calloc(LSH_t, 1);
	
	ret = cass_read_size(&lsh->D, 1, fin);
	ret += cass_read_size(&lsh->M, 1, fin);
	ret += cass_read_size(&lsh->L, 1, fin);
	ret += cass_read_size(&lsh->H, 1, fin);
	ret += cass_read_size(&lsh->count, 1, fin);

	assert(ret == 5);

	lsh->W = type_calloc(float, lsh->L);
	ret  = cass_read_float(lsh->W, lsh->L, fin);
	assert(ret == lsh->L);

	L = lsh->L;
	M = lsh->M;
	D = lsh->D;

	lsh->alphas = type_matrix_alloc(float, L * M, D);
	lsh->betas = type_calloc(float, L * M);
	lsh->hash = NULL; //(ohash_t *)malloc(L * sizeof(ohash_t));

	lsh->tmp = type_matrix_alloc(uint32_t, L, M);
	lsh->tmp2 = malloc(sizeof(uint32_t) *L );

	ret = cass_read_float(lsh->alphas[0], L * M * D, fin);
	assert(ret == L * M * D);
	ret = cass_read_float(lsh->betas, L * M, fin);
	assert(ret == L * M);

	lsh->rnd = type_matrix_alloc(uint32_t, L, M);

	ret = cass_read_uint32(lsh->rnd[0], L * M, fin);
	assert(ret == L * M);

	ret = cass_read_int32(&recall, 1, fin);
	assert(ret == 1);

	if (recall)
	{
		if (LSH_recall_load(&lsh->recall, fin) != 0) assert(0);
	}

	table->__private = lsh;

	return 0;
}

int LSH_checkpoint_private (cass_table_t *table, CASS_FILE *fout)
{
	LSH_t *lsh = table->__private;
	int ret;
	cass_size_t L, M, D;
	int32_t recall;
	ret = cass_write_size(&lsh->D, 1, fout);
	ret += cass_write_size(&lsh->M, 1, fout);
	ret += cass_write_size(&lsh->L, 1, fout);
	ret += cass_write_size(&lsh->H, 1, fout);
	ret += cass_write_size(&lsh->count, 1, fout);
	assert(ret == 5);
	L = lsh->L;
	M = lsh->M;
	D = lsh->D;

	ret = cass_write_float(lsh->W, L, fout);
	assert(ret == L);

	ret = cass_write_float(lsh->alphas[0], L * M * D, fout);
	assert(ret == L * M * D);
	ret = cass_write_float(lsh->betas, L * M, fout);
	assert(ret == L * M);
	ret = cass_write_uint32(lsh->rnd[0], L * M, fout);
	assert(ret == L * M);

	recall = (lsh->recall.table != NULL);

	ret = cass_write_int32(&recall, 1, fout);
	assert(ret == 1);

	if (recall)
	{
		if (LSH_recall_dump(&lsh->recall, fout) != 0) assert(0);
	}

	if (lsh->hash != NULL && table->dirty) return LSH_dump(table);

	return 0;
}

int LSH_load (cass_table_t *table)
{
	LSH_t *lsh = table->__private;
	CASS_FILE *fin;
	uint32_t i;
	int ret;

	assert(lsh->hash == NULL);

	if (lsh->count == 0)
	{
		lsh->hash = type_calloc(ohash_t, lsh->L);
		for (i = 0; i < lsh->L; i++)
		{
			ohash_init(&(lsh->hash[i]), lsh->H);
		}
		return 0;
	}

	fin = cass_open(table->filename, "r");
	if (fin == NULL) return CASS_ERR_IO;

	lsh->hash = type_calloc(ohash_t, lsh->L);
	for (i = 0; i < lsh->L; i++)
	{
		ret = ohash_init_with_stream(&lsh->hash[i], fin);
		assert(ret == 0);
	}

	cass_close(fin);
	return 0;
}

int LSH_release (cass_table_t *table)
{
	int err = 0;
	uint32_t i;
	LSH_t *lsh = table->__private;
	if (table->loaded && table->dirty) err = LSH_dump(table);
	if (err != 0) return err;
	for (i = 0; i < lsh->L; i++)
	{
		ohash_cleanup(&(lsh->hash[i]));
	}
	free(lsh->hash);
	lsh->hash = NULL;
	return 0;
}

int LSH_dump (cass_table_t *table)
{
	uint32_t i;
	int ret;
	CASS_FILE *fout;
	LSH_t *lsh = table->__private;
	if (lsh->count == 0) return 0;
	assert(lsh->hash != NULL);
	fout = cass_open(table->filename, "w");
	assert(fout != NULL);
	for (i = 0; i < lsh->L; i++)
	{
		ret = ohash_dump_stream(&lsh->hash[i], fout);
		assert(ret == 0);
	}
	cass_close(fout);
	table->dirty = 0;
	return ret;
}

void LSH_stat (LSH_t *lsh)
{
	int i;
	for (i = 0; i < lsh->L; i++)
	{
		printf("%d:", i);
		ohash_stat(&lsh->hash[i]);
	}
}

/*
int LSH_est_dump_file (LSH_est_t *est, char *filename)
{
	int ret;
	CASS_FILE *fout = cass_open(filename, "wb");
	assert(fout != NULL);
	ret = 0;
	ret += cass_write_int(&est->a_step, 1, fout);
	ret += cass_write_double(&est->a_min, 1, fout);
	ret += cass_write_double(&est->a_max, 1, fout);
	ret += cass_write_int(&est->b_step, 1, fout);
	ret += cass_write_double(&est->b_min, 1, fout);
	ret += cass_write_double(&est->b_max, 1, fout);

	assert(ret == 6);

	est->a_delta = (est->a_max - est->a_min) / est->a_step;
	est->b_delta = (est->b_max - est->b_min) / est->b_step;


	type_matrix_dump_stream(double, fout, est->a_step, est->b_step, est->a_table);
	type_matrix_dump_stream(double, fout, est->a_step, est->b_step, est->b_table);

	cass_close(fout);

	return 0;
}

int LSH_est_init_with_file (LSH_est_t *est, char *filename)
{
	int ret;
	cass_size_t row, col;
	CASS_FILE *fin = cass_open(filename, "rb");
	assert(fin != NULL);
	ret = 0;
	ret += cass_read_int32(&est->a_step, 1, fin);
	ret += cass_read_double(&est->a_min, 1, fin);
	ret += cass_read_double(&est->a_max, 1, fin);
	ret += cass_read_int32(&est->b_step, 1, fin);
	ret += cass_read_double(&est->b_min, 1, fin);
	ret += cass_read_double(&est->b_max, 1, fin);

	assert(ret == 6);

	est->a_delta = (est->a_max - est->a_min) / est->a_step;
	est->b_delta = (est->b_max - est->b_min) / est->b_step;


	type_matrix_load_stream(double, fin, &row, &col, &est->a_table);
	assert(row == est->a_step);
	assert(col == est->b_step);
	type_matrix_load_stream(double, fin, &row, &col, &est->b_table);
	assert(row == est->a_step);
	assert(col == est->b_step);

	cass_close(fin);

	return 0;
}
*/

void LSH_est_cleanup (LSH_est_t *est)
{
	matrix_free(est->a_table);
	matrix_free(est->b_table);
}

double LSH_est (LSH_est_t *est, double a, double b, int K)
{
	int A, B;
	double alpha, beta;
	if (!(finite(a) && finite(b))) return 0.0;

	A = (a - est->a_min) / est->a_delta;
	B = (b - est->b_min) / est->b_delta;

	if (A < 0) return 0.0;
	if (B < 0) return 0.0;
	if (A >= est->a_step) A = est->a_step - 1;
	if (B >= est->a_step) B = est->a_step - 1;

	alpha = est->a_table[A][B];
	beta = est->b_table[A][B];

	return alpha * pow(K, beta);
}



int __LSH_query(cass_table_t *table, 
                     cass_query_t *query, cass_result_t *result)
{
	LSH_t *lsh = (LSH_t *)table->__private;
	LSH_query_t query2;
	cass_table_t *parent;
	cass_dataset_t *ds;
	cass_vec_dist_t *vec_dist;
	cass_vecset_t *vecset;

	float recall;

	cass_size_t K, L, T;

	uint32_t i, j;

	assert(table->loaded);
	vec_dist = cass_reg_get(&table->env->vec_dist, query->vec_dist_id);
	assert(vec_dist != NULL);
	if (vec_dist->__class != &vec_dist_L2_float) debug("LSH only works for L2 distance for float.\n");

	if (table->parent_id == CASS_ID_INV) debug("LSH query requires parent.\n");
	parent = cass_reg_get(&table->env->table, table->parent_id);
	assert(parent != NULL);
	if (!parent->loaded) debug("LSH query requires parent to be loaded.\n");
	if (!(parent->opr->type & CASS_DATA)) debug("LSH only works on CASS_DATA.\n");

	ds = (cass_dataset_t *)parent->__private;

	K = query->topk;
	if (K == 0) debug("LSH only works for KNN queries.\n");

	L = param_get_int(query->extra_params, "-L", 1);
	T = param_get_int(query->extra_params, "-T", 0);

	recall = param_get_float(query->extra_params, "-recall", 0);

	LSH_query_init(&query2, lsh, ds, K, L, T);


	assert((query->flags & CASS_RESULT_BITMAPS) == 0);
	assert((query->flags & CASS_RESULT_BITMAP) == 0);
	assert((query->flags & CASS_RESULT_LIST) == 0);
	assert(query->flags & CASS_RESULT_LISTS);
	assert(query->candidate == NULL);

	result->flags = CASS_RESULT_LISTS;

	vecset = query->dataset->vecset + query->vecset_id;

	if (query->flags & CASS_RESULT_USERMEM)
	{
		assert(result->u.lists.len >= vecset->num_regions);
	}
	else
	{
		result->flags |= CASS_RESULT_MALLOC;
		ARRAY_INIT_SIZE(result->u.lists, vecset->num_regions);
		result->u.lists.len = vecset->num_regions;
	}

	for (i = 0; i < vecset->num_regions; i++)
	{
		if (query->flags & CASS_RESULT_USERMEM)
		{
			assert(result->u.lists.data[i].size >= query->topk);
		}
		else
		{
			ARRAY_INIT_SIZE(result->u.lists.data[i], query->topk);
		}
		result->u.lists.data[i].len = query->topk;

		if (recall == 0)
		{
			LSH_query(&query2, DATASET_VEC(query->dataset, vecset->start_vecid + i)->u.float_data);
		}
		else
		{
			LSH_query_recall(&query2, DATASET_VEC(query->dataset, vecset->start_vecid + i)->u.float_data, recall);
		}

		for (j = 0; j < K; j++)
		{
			result->u.lists.data[i].data[j] = query2.topk[j];
		}
	}


	if (query->flags & CASS_RESULT_SORT)
	{
		for (j = 0; j < vecset->num_regions; j++)
		{
			TOPK_SORT_MIN(result->u.lists.data[j].data, cass_list_entry_t, dist, query->topk);
		}
		result->flags |= CASS_RESULT_SORT;
	}

	LSH_query_cleanup(&query2);

	return 0;

}

int __LSH_batch_query(cass_table_t *table, 
/* the dataset of all the queries should be the same */
	uint32_t count, cass_query_t **queries, cass_result_t **results)
{
	LSH_t *lsh = (LSH_t *)table->__private;
	cass_result_t *result;
	cass_query_t *query;

	LSH_query_t query2;

	const float **points;
	cass_list_entry_t **topks;
	
	cass_table_t *parent;
	cass_dataset_t *ds;
	cass_vec_dist_t *vec_dist;
	cass_vecset_t *vecset;

	cass_size_t K, L, T, vec_count;

	uint32_t i, j, k;

	query = queries[0];

	assert(table->loaded);
	vec_dist = cass_reg_get(&table->env->vec_dist, query->vec_dist_id);
	assert(vec_dist != NULL);
	if (vec_dist->__class != &vec_dist_L2_float) debug("LSH only works for L2 distance for float.\n");

	if (table->parent_id == CASS_ID_INV) debug("LSH query requires parent.\n");
	parent = cass_reg_get(&table->env->table, table->parent_id);
	assert(parent != NULL);
	if (!parent->loaded) debug("LSH query requires parent to be loaded.\n");
	if (!(parent->opr->type & CASS_DATA)) debug("LSH only works on CASS_DATA.\n");

	ds = (cass_dataset_t *)parent->__private;


	K = query->topk;
	if (K == 0) debug("LSH only works for KNN queries.\n");

	L = param_get_int(query->extra_params, "-L", 1);
	T = param_get_int(query->extra_params, "-T", 0);

	LSH_query_init(&query2, lsh, ds, K, L, T);


	assert((query->flags & CASS_RESULT_BITMAPS) == 0);
	assert((query->flags & CASS_RESULT_BITMAP) == 0);
	assert((query->flags & CASS_RESULT_LIST) == 0);
	assert(query->flags & CASS_RESULT_LISTS);
	assert(query->candidate == NULL);

	vec_count = 0;
	for (i = 0; i < count; i++)
	{
		result = results[i];
		query = queries[i];
		result->flags = CASS_RESULT_LISTS;
		vecset = query->dataset->vecset + query->vecset_id;

		if (query->flags & CASS_RESULT_USERMEM)
		{
			assert(result->u.lists.len >= vecset->num_regions);
		}
		else
		{
			result->flags |= CASS_RESULT_MALLOC;
			ARRAY_INIT_SIZE(result->u.lists, vecset->num_regions);
			result->u.lists.len = vecset->num_regions;
		}

		for (j = 0; j < vecset->num_regions; j++)
		{
			if (query->flags & CASS_RESULT_USERMEM)
			{
				assert(result->u.lists.data[j].size >= query->topk);
			}
			else
			{
				ARRAY_INIT_SIZE(result->u.lists.data[j], query->topk);
			}
			result->u.lists.data[j].len = K;
		}
		vec_count += vecset->num_regions;
	}

	points = type_calloc(const float *, vec_count);
	topks = type_calloc(cass_list_entry_t *, vec_count);

	k = 0;
	for (i = 0; i < count; i++)
	{
		result = results[i];
		query = queries[i];
		vecset = query->dataset->vecset + query->vecset_id;
		for (j = 0; j < vecset->num_regions; j++)
		{
			points[k] = DATASET_VEC(query->dataset, vecset->start_vecid + j)->u.float_data;
			topks[k] = result->u.lists.data[j].data;
			k++;
		}
	}

	assert(k == vec_count);

	if (strstr(query->extra_params, "-ca"))
	{
		LSH_query_batch_ca(&query2, vec_count, points, topks);
	}
	else
	{
		LSH_query_batch(&query2, vec_count, points, topks);
	}

	query = queries[0];

	if (query->flags & CASS_RESULT_SORT)
	{
		for (i = 0; i < vec_count; i++)
		{
			TOPK_SORT_MIN(topks[i], cass_list_entry_t, dist, K);
		}
		for (i = 0; i < count; i++)
		{
			results[i]->flags |= CASS_RESULT_SORT;
		}
	}

	LSH_query_cleanup(&query2);

	return 0;
}


cass_table_opr_t opr_lsh = {
	.name = "lsh", 
	.type = CASS_VEC_INDEX, 
	.vecset_type = CASS_ANY, 
	.vec_type = CASS_VEC_FLOAT, 
	.dist_vecset = CASS_ANY,
	.dist_vec = CASS_VEC_DIST_TYPE_L2, 

	.cfg = NULL,
	.tune = NULL, //mplsh_tune, 
	.init_private = LSH_init_private, 
	.batch_insert = LSH_batch_insert, 
	.query = __LSH_query, //mplsh_query,
	.batch_query = __LSH_batch_query, 
	.load = LSH_load, 
	.release = LSH_release, 
	.checkpoint_private = LSH_checkpoint_private, 
	.restore_private = LSH_restore_private, 
	.free_private = LSH_free_private
}; 

