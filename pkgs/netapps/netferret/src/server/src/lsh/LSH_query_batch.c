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
#include <cass.h>
#include <cass_timer.h>
#include "LSH.h"

static inline int prime (int n)
{
	long g, j;
	double k;
	if (n % 2 == 0) n++;
	for (;;)
	{
		g = sqrt((double)n);
		j = 3;
		k = (double)n / (double)j;      

		while ((k != (double)(long)k) && (j<=g)) {
	        	j += 2;
	        	k = (double)n / (double)j;
        	}
		if (j>g) return n;
		n += 2;
	}
	assert(0);
	return 0;
}

static inline void LSH_hash2_L (LSH_t *lsh, const unsigned **hash, unsigned *hash2, int L)
{
	int i, j;
	for (i = 0; i < L; i++)
	{
		hash2[i] = 0;
		for (j = 0; j < lsh->M; j++)
		{
			hash2[i] += lsh->rnd[i][j] * hash[i][j];
		}
		hash2[i] %= lsh->H;
	}
}

static inline void LSH_hash_L (LSH_t *lsh, const float *pnt, unsigned **hash, int L)
{
	float s;
	int i, j, k, l;

	l = 0;
	for (i = 0; i < L; i++)
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


void LSH_query_batch (const LSH_query_t *query, int N, const float **point, cass_list_entry_t **topk)
{
	LSH_t *lsh = query->lsh;
	int max_th;
	int D = lsh->D;
	int T = query->T;
	unsigned ***_tmp = NULL;
	unsigned **_tmp2 = NULL;
	int i, L = query->L, K = query->K, M = lsh->M;

	ptb_vec_t ***_score = NULL;
	ptb_vec_t **_vec = NULL;
#ifdef _OPENMP
	max_th = omp_get_max_threads();
#else
	max_th = 1;
#endif

//	fprintf(stderr, "#TH = %d\n", max_th);
	_tmp = type_matrix3_alloc(unsigned, max_th, L, M);
	_tmp2 = type_matrix_alloc(unsigned, max_th, L);
	if (T > 0)
	{
		_score = type_matrix3_alloc(ptb_vec_t, max_th, L, M * 2);
		_vec = type_matrix_alloc(ptb_vec_t, max_th, T);
	}

#pragma omp parallel for schedule(guided, 1) default(shared)
	for (i = 0; i < N; i++)
	{
#ifdef _OPENMP
		int tid = omp_get_thread_num();
#else
		int tid = 0;
#endif
		assert(tid < max_th);
		unsigned **tmp = _tmp[tid];
		unsigned *tmp2 = _tmp2[tid];
		ptb_vec_t **score = T > 0 ? _score[tid] : NULL;
		ptb_vec_t *vec = T > 0 ? _vec[tid] : NULL;
		cass_list_entry_t entry;
		int j;
		unsigned h;
		
		if (query->T == 0)
		{
			LSH_hash_L(lsh, point[i], tmp, L);
		}
		else
		{
			LSH_hash_score(lsh, L, point[i], tmp, score);
		}
		LSH_hash2_noperturb(lsh, tmp, tmp2, L);

		TOPK_INIT(topk[i], dist, K, HUGE);
		for (j = 0; j < L; j++)
		{
			int k;
			ptb_vec_t ptb;

			ARRAY_BEGIN_FOREACH(lsh->hash[j].bucket[tmp2[j]], uint32_t id) {
				cass_vec_t *vec = DATASET_VEC(query->ds, id);
				entry.id = id;
				entry.dist = dist_L2_float(D, vec->u.float_data, point[i]);
				TOPK_INSERT_MIN_UNIQ(topk[i], dist, id, K, entry);
			}
			ARRAY_END_FOREACH;
			if (T == 0) continue;
			ptb_qsort(score[j], M * 2);
			map_perturb_vector(query->ptb_set, vec, score[j], M, T);
			for (k = 0; k < T; k++)
			{
				ptb = vec[k];
				LSH_hash2_perturb(lsh, tmp, &h, &ptb, j);
				ARRAY_BEGIN_FOREACH(lsh->hash[j].bucket[h], uint32_t id) {
					cass_vec_t *vec = DATASET_VEC(query->ds, id);
					entry.id = id;
					entry.dist = dist_L2_float(D, vec->u.float_data, point[i]);
					TOPK_INSERT_MIN_UNIQ(topk[i], dist, id, K, entry);
				}
				ARRAY_END_FOREACH;
			}
		}
	}
	if (_vec != NULL) matrix_free(_vec);
	if (_score != NULL) matrix3_free(_score);
	matrix3_free(_tmp);
	matrix_free(_tmp2);
}

struct b2s {
	unsigned bucket;
	int qry;
	int t;
	struct b2s *next;
};

struct b2s_r {
	int qry;
	int t;
};

static inline void LSH_hash2_b2s_L (LSH_t *lsh, unsigned **hash, struct b2s **hash2, int L, int qry)
{
	int i, j;
	unsigned h2;
	for (i = 0; i < L; i++)
	{
		h2 = 0;
		for (j = 0; j < lsh->M; j++)
		{
			h2 += lsh->rnd[i][j] * hash[i][j];
		}
//		hash2[i].L = i;
		hash2[i][0].qry = qry;
		hash2[i][0].t = -1;
		hash2[i][0].bucket = h2 % lsh->H;
		hash2[i][0].next = NULL;
	}
}

void LSH_query_batch_ca (const LSH_query_t *query, int N, const float **point, cass_list_entry_t **topk)
{
//	stimer_t	tmr; 
	LSH_t *lsh = query->lsh;
	int max_th;
	int D = lsh->D;
	unsigned ***_tmp = NULL;
	int i, l;
	int L = query->L, K = query->K, T = query->T, M = lsh->M;

	struct b2s ***hash;
	struct b2s ***b2s;
	ptb_vec_t ***_score = NULL;
	ptb_vec_t **_vec = NULL;
	ARRAY_TYPE(struct b2s_r) *_2scan;

	size_t B2S_SIZE;

	cass_list_entry_t ***ptopk;

	b2s = type_matrix3_alloc(struct b2s, N, L, T + 1);

#ifdef _OPENMP
	max_th = omp_get_max_threads();
#else
	max_th = 1;
#endif
//	fprintf(stderr, "#TH = %d\n", max_th);

	_tmp = type_matrix3_alloc(unsigned, max_th, L, M);

	if (T > 0)
	{
		_score = type_matrix3_alloc(ptb_vec_t, max_th, L, M * 2);
		_vec = type_matrix_alloc(ptb_vec_t, max_th, T);
	}

	/* hashing */
	//stimer_tick(&tmr);
#pragma omp parallel for schedule(guided, 1) default(shared)
	for (i = 0; i < N; i++)
	{
#ifdef _OPENMP
		int tid = omp_get_thread_num();
#else
		int tid = 0;
#endif
		unsigned **tmp;
		int j, k;
		ptb_vec_t **score = T > 0 ? _score[tid] : NULL;
		ptb_vec_t *vec = T > 0 ? _vec[tid] : NULL;
		assert(tid < max_th);
		tmp = _tmp[tid];

		if (T == 0)LSH_hash_L(lsh, point[i], tmp, L);
		else LSH_hash_score(lsh, L, point[i], tmp, score);
		LSH_hash2_b2s_L(query->lsh, tmp, b2s[i], L, i);
		if (T == 0) continue;
		for (j = 0; j < L; j++)
		{
			ptb_qsort(score[j], M * 2);
			map_perturb_vector(query->ptb_set, vec, score[j], M, T);
			for (k = 0; k < T; k++)
			{
				unsigned h;
				LSH_hash2_perturb(lsh, tmp, &h, &vec[k], j);
				b2s[i][j][k+1].qry = i;
				b2s[i][j][k+1].t = k;
				b2s[i][j][k+1].bucket = h;
				b2s[i][j][k+1].next = NULL;
			}
		}
	}
	matrix3_free(_tmp);

	if (_score != NULL) matrix3_free(_score);
	if (_vec != NULL) matrix_free(_vec);

	//stimer_tuck(&tmr, "Stage-1");

	//stimer_tick(&tmr);
	/* hash to bucket */
	B2S_SIZE = prime(N * (T + 1) * 5);
	hash = type_matrix_alloc(struct b2s *, L, B2S_SIZE);
#pragma omp parallel for schedule(guided, 1) default(shared)
	for (i = 0; i < L; i++)
	{
		int j, t;
		for (j = 0; j < N; j++)
		for (t = 0; t <= T; t++)
		{
			struct b2s *b = &b2s[j][i][t];
			unsigned k = b->bucket % B2S_SIZE;
			for (;;)
			{
				if (hash[i][k] == NULL) break;
				if (hash[i][k]->bucket == b->bucket) break;
				k = (k + 1) % B2S_SIZE;
			}
			b->next = hash[i][k];
			hash[i][k] = b;
		}
	}
	/* scan the bucket */
	_2scan = malloc(max_th * sizeof (*_2scan));
	for (i = 0; i < max_th; i++) ARRAY_INIT(_2scan[i]);

	ptopk = NULL;
	if (T > 0) ptopk = type_matrix3_alloc(cass_list_entry_t, N, T, K);

#pragma omp parallel for schedule(guided, 1) default(shared)
	for (i = 0; i < N; i++)
	{
		int j;
		TOPK_INIT(topk[i], dist, K, HUGE);
		for (j = 0; j < T; j++)
			TOPK_INIT(ptopk[i][j], dist, K, HUGE);
	}

	//stimer_tuck(&tmr, "Stage-2");

//	stimer_tick(&tmr);
		
//	double p = 0;

	for (l = 0; l < L; l++)
#pragma omp parallel for schedule(guided, 1) default(shared) //reduction(+:p)
	for (i = 0; i < B2S_SIZE; i++)
		if (hash[l][i] != NULL)
		{
			struct b2s *tmp;
#ifdef _OPENMP
			int tid = omp_get_thread_num();
#else
			int tid = 0;
#endif
			assert(tid < max_th);
			unsigned bucket = 0;
			cass_list_entry_t entry;
			ARRAY_TRUNC(_2scan[tid]);
			tmp = hash[l][i];
			bucket = tmp->bucket;
			while (tmp != NULL)
			{
				struct b2s_r t = {.qry = tmp->qry, .t = tmp->t};
				ARRAY_APPEND(_2scan[tid], t);
				tmp = tmp->next;
			}

			/*
			if (lsh->hash[l].bucket[bucket].len == 0) continue;

			int id = lsh->hash[l].bucket[bucket].data[0];
			*/

			ARRAY_BEGIN_FOREACH(lsh->hash[l].bucket[bucket], uint32_t id) {

				ARRAY_BEGIN_FOREACH_P(_2scan[tid], struct b2s_r *b)
				{
					cass_vec_t *vec = DATASET_VEC(query->ds, id);
					entry.id = id;
					entry.dist = dist_L2_float(D, vec->u.float_data, point[b->qry]);
					if (b->t == -1)
					{
						TOPK_INSERT_MIN_UNIQ(topk[b->qry], dist, id, K, entry);
					}
					else
					{
						TOPK_INSERT_MIN_UNIQ(ptopk[b->qry][b->t], dist, id, K, entry);
					}
				}
				ARRAY_END_FOREACH;
			}
			ARRAY_END_FOREACH;
		}

	for (i = 0; i < max_th; i++) ARRAY_CLEANUP(_2scan[i]);
	free(_2scan);
	matrix_free(hash);
	matrix_free(b2s);

//	stimer_tuck(&tmr, "Stage-2");
	//stimer_tick(&tmr);

	if (T > 0)
#pragma omp parallel for schedule(guided, 1) default(shared)
	for (i = 0; i < N; i++)
	{
		int j, k;
		for (j = 0; j < T; j++)
		{
			for (k = 0; k < K; k++)
			{
				TOPK_INSERT_MIN_UNIQ(topk[i], dist, id, K, ptopk[i][j][k]);
			}
		}
	}

	if (ptopk != NULL) matrix3_free(ptopk);
	//stimer_tuck(&tmr, "Stage-4");
}

