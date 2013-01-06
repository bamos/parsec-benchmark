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
#include "LSH.h"
#include <cass_topk.h>
#include "local.h"

#define ptb_vec_ge(a,b)	((a)->key >= (b)->key)

void LSH_query_init (LSH_query_t *query, LSH_t *lsh, cass_dataset_t *ds, cass_size_t K, cass_size_t L, cass_size_t T)
{
	memset(query, 0, sizeof(*query));
	query->lsh = lsh;
	query->ds = ds;

	query->K = K;
	query->L = L;
	query->T = T;

	query->bitmap = bitmap_new(lsh->count);

	query->tmp = type_matrix_alloc(uint32_t, lsh->L, lsh->M);
	assert(query->tmp != NULL);
	query->tmp2 = (uint32_t *)malloc(sizeof(uint32_t) * lsh->L);
	assert(query->tmp2 != NULL);

	query->ptb = type_matrix_alloc(ptb_vec_t, L, query->lsh->M * 2);
	assert(query->ptb != NULL);
#ifdef QUERY_DIRECT
	{
		int i;
		query->heap = type_calloc(typeof(query->heap), L);
		assert(query->heap != NULL);
		for (i = 0; i < L; i++) ARRAY_INIT(query->heap[i]);
	}
#else
	{
		ptb_vec_t *scr = gen_score(query->lsh->M);
		assert(scr != NULL);
		query->ptb_set = type_calloc(ptb_vec_t, T);
		assert(query->ptb_set != NULL);
		gen_perturb_set(scr, query->ptb_set, query->lsh->M, T);
		query->ptb_vec = type_matrix_alloc(ptb_vec_t, L, T);
		assert(query->ptb_vec != NULL);
		query->ptb_step = type_calloc(int, L);
		assert(query->ptb_step != NULL);
		free(scr);
	}
#endif
	query->topk = type_calloc(cass_list_entry_t, K);
	query->_topk = type_matrix_alloc(cass_list_entry_t, L, K);

	assert(query->topk != NULL);
	assert(query->_topk != NULL);

	query->C = (int*)malloc(L * sizeof(int));
	query->H = (int*)malloc(L * sizeof(int));
	query->S = (float*)malloc(L * sizeof(float));

	assert(query->C != NULL);
	assert(query->H != NULL);
	assert(query->S != NULL);

	query->gamma = 1.0;

}

void LSH_query_cleanup (LSH_query_t *query)
{
	bitmap_free(&query->bitmap);
	matrix_free(query->tmp);
	matrix_free(query->_topk);
	free(query->topk);
	free(query->tmp2);
#ifdef QUERY_DIRECT
	{
		int i;
		for (i = 0; i < query->L; i++) ARRAY_CLEANUP(query->heap[i]);
		free(query->heap);
	}
#else
	free(query->ptb_step);
	free(query->ptb_set);
	matrix_free(query->ptb_vec);
#endif
	matrix_free(query->ptb);
	free(query->C);
	free(query->H);
	free(query->S);
}

void LSH_hash_score (LSH_t *lsh, int L, const float *pnt, uint32_t **hash, ptb_vec_t **ptb)
{
	float s, t;
	int i, j, k, p, l;

	l = 0;
	for (i = 0; i < L; i++)
	{
		p = 0;
		for (j = 0; j < lsh->M; j++)
		{
			s = lsh->betas[l];
			for (k = 0; k < lsh->D; k++)
			{
				s += pnt[k] * lsh->alphas[l][k];
			}

			t = floor(s / lsh->W[i]);
			hash[i][j] = t;
			t = s - t * lsh->W[i];
			ptb[i][p].set = 1 << j;
			ptb[i][p].dir = 0;
			ptb[i][p].key = t * t;
			p++;
			t = lsh->W[i] - t;
			ptb[i][p].set = 1 << j;
			ptb[i][p].dir = 1 << j;
			ptb[i][p].key = t * t;
			p++;
			l++;
		}
	}
}

void LSH_hash2_noperturb (LSH_t *lsh, uint32_t **hash, uint32_t *hash2, int L)
{
	int i, j;
	uint32_t h;
	for (i = 0; i < L; i++)
	{
		h = 0;
		for (j = 0; j < lsh->M; j++)
		{
			h += lsh->rnd[i][j] * hash[i][j];
		}
		hash2[i] = h % lsh->H;
	}
}

void LSH_hash2_perturb (LSH_t *lsh, uint32_t **hash, uint32_t *hash2, ptb_vec_t *ptb, int l)
{
	uint32_t j, mask;
	uint32_t set, dir;
	uint32_t h;
	set = ptb->set;
	dir = ptb->dir;
	mask = 1;
	h = 0;
	for (j = 0; j < lsh->M; j++)
	{
		if (set & mask)
		{
			if (dir & mask)
			{
				h += lsh->rnd[l][j] * (hash[l][j] + 1);
			}
			else
			{
				h += lsh->rnd[l][j] * (hash[l][j] - 1);
			}
		}
		else
		{
			h += lsh->rnd[l][j] * hash[l][j];
		}
		mask = mask << 1;
	}
	*hash2  = h % lsh->H;
}


static void LSH_query_local (LSH_query_t *query)
{
	double sx, sy, sxx, sxy;
	double ld, lk;
	double a, b;
	int j;
	int K = query->K;
	sx = sy = sxx = sxy = 0.0;
	for (j = 0; j < K-1; j++)
	{
		if (query->topk[K - j - 2].dist >= HUGE) break;
		lk = log(j+1); 
		ld = log(query->topk[K - j - 2].dist);
		sx += lk;
		sy += ld;
		sxx += lk * lk;
		sxy += lk * ld;
	}
	least_squares(&a, &b, j, sxx, sxy, sx, sy);
	a = exp(a);
	//fprintf(stderr, ">%g\t%g\n", a, b);
	query->dist = LSH_est(query->lsh->est, a, b, K -1) * query->gamma;
//		fprintf(stderr, "%g\n", query->dist);
}

extern void LSH_query_merge (LSH_query_t * query);

static void LSH_query_bootstrap (LSH_query_t *query, const float *point)
{
	cass_size_t D = query->lsh->D;
	cass_size_t K = query->K;
	cass_size_t L = query->L;
	LSH_t *lsh = query->lsh;
	ptb_vec_t **score = query->ptb;
	uint32_t **tmp = query->tmp;
	uint32_t *tmp2 = query->tmp2;
	cass_list_entry_t **_topk = query->_topk;
	cass_list_entry_t entry;

	int *C = query->C;
	int *H = query->H;

	int i;

	memset(C, 0, L * sizeof(int));
	memset(H, 0, L * sizeof(int));
	memset(query->S, 0, L * sizeof(float));

	bitmap_clear(query->bitmap);

	LSH_hash_score(query->lsh, L, point, tmp, score);
	LSH_hash2_noperturb(query->lsh, tmp, tmp2, L);

	for (i = 0; i < L; i++)
	{
		memset(_topk[i], 0xff, sizeof (*_topk[i]) * K);
		TOPK_INIT(_topk[i], dist, K, HUGE);
		ARRAY_BEGIN_FOREACH(lsh->hash[i].bucket[tmp2[i]], uint32_t id) {
			if (!bitmap_contain(query->bitmap, id))
			{
				cass_vec_t *vec;
				bitmap_insert(query->bitmap, id);
		   		vec = DATASET_VEC(query->ds, id);
				entry.id = id;
				entry.dist = dist_L2_float(D, vec->u.float_data, point);
				C[i]++;
				query->CC++;
				TOPK_INSERT_MIN_UNIQ_DO(_topk[i], dist, id, K, entry, H[i]++);
			}
		}
		ARRAY_END_FOREACH;

		ptb_qsort(score[i], lsh->M * 2);

#ifdef	QUERY_DIRECT
		ARRAY_TRUNC(query->heap[i]);
		HEAP_ENQUEUE(query->heap[i], score[i][0], ptb_vec_ge);
#else
		query->ptb_step[i] = 0;
		map_perturb_vector(query->ptb_set, query->ptb_vec[i], score[i], lsh->M, query->T);
#endif
	}

	if (lsh->est != NULL)
	{
		LSH_query_merge(query);
		LSH_query_local(query);
	}

}

static void LSH_query_probe (LSH_query_t *query, const float *point, int l, int g)
{
	cass_size_t D = query->lsh->D;
	cass_size_t K = query->K;
	LSH_t *lsh = query->lsh;
	uint32_t **tmp = query->tmp;
#ifdef QUERY_DIRECT
	cass_size_t M = query->lsh->M;
	ptb_vec_t *score = query->ptb[l];
#endif
	cass_list_entry_t *topk = g == 0? query->_topk[l] : query->topk;
	cass_list_entry_t entry;
	ptb_vec_t ptb;
	int *C = query->C;
	int *H = query->H;
	uint32_t h;
#ifdef QUERY_DIRECT
	typeof(query->heap) heap = &query->heap[l];
	if (HEAP_EMPTY(*heap)) return;
	ptb = HEAP_HEAD(*heap);
	HEAP_DEQUEUE(*heap, ptb_vec_ge);
#else
	ptb = query->ptb_vec[l][query->ptb_step[l]++];
#endif
	LSH_hash2_perturb(query->lsh, tmp, &h, &ptb, l);
	ARRAY_BEGIN_FOREACH(lsh->hash[l].bucket[h], uint32_t id) {
		if (!bitmap_contain(query->bitmap, id))
		{
			cass_vec_t *vec;
			bitmap_insert(query->bitmap, id);
			vec = DATASET_VEC(query->ds, id);
			C[l]++;
			query->CC++;
			entry.id = id;
			entry.dist = dist_L2_float(D, vec->u.float_data, point);
			TOPK_INSERT_MIN_UNIQ_DO(topk, dist, id, K, entry, H[l]++);
		}
	}
	ARRAY_END_FOREACH;
#ifdef QUERY_DIRECT
	{
		ptb_vec_t ptb2;
		int m;
		/* add expand */
		m = ptb.max;
		ptb.max++;
		if (ptb.max >= M * 2) return;
		ptb2 = ptb;
		ptb2.set &= ~score[m].set;
		ptb2.dir &= ~score[m].dir;
		ptb2.key -= score[m].key;

		if ((ptb.set & score[ptb.max].set) == 0)
		{
			ptb.set |= score[ptb.max].set;
			ptb.dir |= score[ptb.max].dir;
			ptb.key += score[ptb.max].key;
			HEAP_ENQUEUE(*heap, ptb, ptb_vec_ge);
		}

		if ((ptb2.set & score[ptb2.max].set) == 0)
		{
			ptb2.set |= score[ptb2.max].set;
			ptb2.dir |= score[ptb2.max].dir;
			ptb2.key += score[ptb2.max].key;
			HEAP_ENQUEUE(*heap, ptb2, ptb_vec_ge);
		}
	}
#endif
}

void LSH_query_merge (LSH_query_t *query)
{
	cass_size_t K = query->K;
	cass_size_t L = query->L;
	cass_list_entry_t **_topk = query->_topk;
	cass_list_entry_t *topk = query->topk;
	int i, j;

	memset(topk, 0xff, sizeof (*topk) * K);
	TOPK_INIT(topk, dist, K, HUGE);

//	query->CC = 0;

	for (i = 0; i < L; i++)
	{
		for (j = 0; j < K; j++)
		{
			TOPK_INSERT_MIN_UNIQ(topk, dist, id, K, _topk[i][j]);
		}
//		query->CC += query->C[i];
	}
}

void LSH_query (LSH_query_t *query, const float *point)
{
	LSH_est_t *est = query->lsh->est;
	cass_list_entry_t *topk = query->topk;
	float dist = query->dist;

	cass_size_t T = query->T;
	cass_size_t L = query->L;
	int i, j;

	query->CC = 0;
	LSH_query_bootstrap(query, point);

	for (i = 0; i < L; i++)
	{
		for (j = 0; j < T; j++)
		{
			if (est != NULL) if (topk[0].dist <= dist) return;
			LSH_query_probe(query, point, i, 0);
		}
	}
	LSH_query_merge(query);
}

void LSH_query_recall (LSH_query_t *query, const float *point, float R)
{
	LSH_recall_t *recall = &query->lsh->recall;
	cass_list_entry_t *topk = query->topk;

	cass_size_t T = query->T;
	cass_size_t L = query->L;
	int K = query->K;
	int i, j;
	query->CC = 0;
	LSH_query_bootstrap(query, point);
	LSH_query_merge(query);
	for (i = 0; i < T; i++)
	{
		if (recall != NULL)
		{
			float r = 0;
			for (j = 0; j < K; j++)
			{
				r += LSH_recall(recall, topk[j].dist, i);
			}
			r /= K;
			if (r >= R) return;

		}
		for (j = 0; j < L; j++)
		{
			LSH_query_probe(query, point, j, 1);
		}
	}
}


int LSH_query_select (LSH_query_t *query, int l)
{
	cass_list_entry_t foo[query->L];
	int i;
	for (i = 0; i < query->L; i++)
	{
		//query->S[i] = query->C[i] == 0 ? 0.0 : (float)query->H[i] / (float)query->C[i];
		foo[i].id = i;
		foo[i].dist = query->S[i];
		//foo[i].index = query->_topk[i][0].key;
	}
	__cass_list_entry_qsort(foo, query->L);
	/*
	for (i = 0; i < query->L; i++)
	{
		printf("%d\t%g\n", foo[i].index, foo[i].key);
	}
	getchar();
	*/
	for (i = 0; i < query->L - 1; i++)
	{
		if (rand() % 3 < 2) return foo[i].id;
	}
	return foo[i].id;
}

void LSH_query_boost (LSH_query_t *query, const float *point)
{
	cass_size_t T = query->T;
	int i, l = 0;
	query->CC = 0;
	LSH_query_bootstrap(query, point);
	for (i = 0; i < T; i++)
	{
		l = LSH_query_select(query, l);
		/*
		query->C[l] /= 2;
		query->H[l] /= 2;
		*/
		LSH_query_probe(query, point, l, 0);
		if (query->C[l] > 0) query->S[l] += (1.0 - (double)query->H[l] / (double)query->C[l]) * query->_topk[l][0].dist;
		else query->S[l] = 0;
		/*
		if (query->H[l] == 0) query->S[l] *= 1.5;
		else query->S[l] = query->_topk[l][0].key;
		*/
	}
	LSH_query_merge(query);
}

