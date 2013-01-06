#ifndef __WDONG_PSLSH__
#define __WDONG_PSLSH__

#include <cass.h>
#include <cass_file.h>
#include <cass_hash.h>
#include <cass_bitmap.h>

typedef struct {
	int a_step, b_step;
	double a_min, a_max;
	double b_min, b_max;
	double a_delta, b_delta;
	double **a_table;
	double **b_table;
} LSH_est_t;

int LSH_est_init (LSH_est_t *est, int a_step, double a_min, double a_max,
	int b_step, double b_min, double b_max, double W, int M, int L, int K);
int LSH_est_dump_file (LSH_est_t *est, char *);
int LSH_est_init_with_file (LSH_est_t *est, char *);

void LSH_est_cleanup (LSH_est_t *est);

double LSH_est (LSH_est_t *est, double a, double b, int K);

typedef struct {
	cass_size_t d_step;
	cass_size_t T;
	float d_min, d_max;
	float **table;
} LSH_recall_t;

int LSH_recall_init (LSH_recall_t *recall, int d_step, float d_min, float d_max,
		int M, int L, int T, float W);

int LSH_recall_dump (LSH_recall_t *recall, CASS_FILE *fout);

int LSH_recall_load (LSH_recall_t *recall, CASS_FILE *fin);

static inline void LSH_recall_cleanup (LSH_recall_t *recall)
{
	matrix_free(recall->table);
}

static inline float LSH_recall (LSH_recall_t *recall, float dist, int T)
{
	int d;
	if (dist < recall->d_min) return 1.0;
	if (dist > recall->d_max) return 0.0;
	d = (dist - recall->d_min) * recall->d_step / (recall->d_max - recall->d_min);
	return recall->table[T][d];
}

typedef struct
{
	cass_size_t D, M, L, H, count;
	float *W;
	float **alphas;
	float *betas;
	uint32_t **rnd;
	ohash_t *hash;

	uint32_t **tmp;	/* used as hash index, for insertion*/
	uint32_t *tmp2;	/* used as 2nd hash, for insertion */

	LSH_est_t *est;
	LSH_recall_t recall;
} LSH_t;

static inline void LSH_set_est (LSH_t *lsh, LSH_est_t *est)
{
	lsh->est = est;
}

void LSH_hash2 (LSH_t *lsh, uint32_t **hash, uint32_t *hash2);

void LSH_hash (LSH_t *lsh, const float *pnt, uint32_t **hash);

/* -------------------------------------- QUERY ----------------------------- */
#include "perturb.h"
//#define QUERY_DIRECT 1

typedef struct {
	LSH_t *lsh;
	cass_dataset_t *ds;

	cass_size_t K;
	cass_size_t L;
	cass_size_t T;

	bitmap_t *bitmap;

	uint32_t **tmp;
	uint32_t *tmp2;

	ptb_vec_t **ptb;	/* L * 2M */
#ifdef QUERY_DIRECT
	ARRAY_TYPE(ptb_vec_t) *heap;
#else
	int *ptb_step;
	ptb_vec_t *ptb_set;
	ptb_vec_t **ptb_vec;
#endif

	cass_list_entry_t *topk;
	cass_list_entry_t **_topk;

	int *C;
	int *H;
	float *S;

	int CC;
	float dist;
	float min;
	float gamma;
} LSH_query_t;

void LSH_query_init (LSH_query_t *query, LSH_t *lsh, cass_dataset_t *ds, cass_size_t K, cass_size_t L, cass_size_t T);

static inline void LSH_query_set_gamma (LSH_query_t *query, float gamma)
{
	query->gamma = gamma;
}

void LSH_query_cleanup (LSH_query_t *query);

void LSH_query (LSH_query_t *query, const float *point);

void LSH_query_recall (LSH_query_t *query, const float *point, float R);

void LSH_query_boost (LSH_query_t *query, const float *point);

void LSH_query_batch (const LSH_query_t *query, int N, const float **point, cass_list_entry_t **topk);
void LSH_query_batch_ca (const LSH_query_t *query, int N, const float **point, cass_list_entry_t **topk);

void LSH_hash_score (LSH_t *lsh, int L, const float *pnt, uint32_t **hash, ptb_vec_t **ptb);

void LSH_hash2_noperturb (LSH_t *lsh, uint32_t **hash, uint32_t *hash2, int L);

void LSH_hash2_perturb (LSH_t *lsh, uint32_t **hash, uint32_t *hash2, ptb_vec_t *ptb, int l);
		
#endif

