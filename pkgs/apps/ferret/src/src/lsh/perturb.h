#ifndef __PERTURB__
#define __PERTURB__

typedef struct
{
	unsigned long long set;
	float key;
	unsigned dir;
	unsigned max;
	float key1;
} ptb_vec_t;

ptb_vec_t *gen_score (int M);

int gen_perturb_set (const ptb_vec_t *score, ptb_vec_t *set, int M, int T);

int map_perturb_vector (const ptb_vec_t set[], ptb_vec_t vector[], const ptb_vec_t mapping[], int M, int T);

void ptb_qsort (ptb_vec_t *, cass_size_t);

#endif
