#ifndef _LSH_TABLE_H_
#define _LSH_TABLE_H_

#include <cass.h>
#include <cass_hash.h>		// dynamic array based
// #include <hash_table.h>	// link list based 
#include <cass_bitmap.h>

typedef struct _lshTable_t {
	cass_size_t		D; // cass_vec_t dimension
	float			W; // hash bin size
	cass_size_t		M; // number of hash functions per table
	cass_size_t		H; 

	float			**alphas; 
	float			*betas; 

	uint32_t		*rndvec; 
	ohash_t			*hash_table;
//	hashTable_t		*hash_table; 
} lshTable_t; 

lshTable_t *lshTable_init(cass_size_t D, float W, cass_size_t M,
	cass_size_t H);

int	lshTable_free(lshTable_t *lsh_table); 

lshTable_t *lshTable_load(CASS_FILE *in); 

int	lshTable_dump(lshTable_t *lsh_table, CASS_FILE *out); 

int lshTable_hash(lshTable_t *lsh_table, float *vec, int *hash);

int lshTable_hash_diff(lshTable_t *lsh_table, float *vec, 
	int *hash, float *diff); 

int lshTable_insert(lshTable_t *lsh_table, float *vec, 
	cass_vec_id_t id); 

int lshTable_query(lshTable_t *lsh_table, float *vec, 
	bitmap_t *map); 

inline int lshTable_query_hash(lshTable_t *lsh_table, int *hash,
	bitmap_t *map); 

#endif /* _LSH_TABLE_H_ */

