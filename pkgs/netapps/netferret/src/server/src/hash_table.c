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
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <math.h>
#include <gsl/gsl_rng.h>

#include <cass.h>
#include <hash_table.h>
#include <arena.h>

#define	MAX_INT	2147483647

#define DEFAULT_PRIME	1017881	
//#define DEFAULT_PRIME	 507217	
//#define DEFAULT_PRIME	 432251	
//#define DEFAULT_PRIME	 300301 
//#define DEFAULT_PRIME	 128311
//#define DEFAULT_PRIME	  10007 

MemArena	*hasharena;

typedef struct nodeT {
	int				val;
	struct nodeT	*next; 
} nodeT; 

typedef struct bucketT {
	int		cnt, idx; 
	nodeT	*nodes; 
} bucketT; 

struct hashTable_t {
	int		size;
	int		cnt_item; 	// number of items
	int		cnt_bucket;	// number of non-empty buckets
	int		key_size;	// size of hash key (number of integers)
	hashT	rndVec; 
	bucketT	**buckets; 
}; 

void
hashTable_init(void)
{
	hasharena = mkmemarena(NULL, NULL, NULL, 0);
}

void
hashT_print(hashT key, int key_size, const char *tag, FILE *fp)
{
	int		i; 

	fprintf(fp, "[%d", key[0]); 
	for (i=1; i<key_size; i++) 
		fprintf(fp, ",%d", key[i]); 
	fprintf(fp, "]%s", tag); 
}

nodeT *
nodeT_new(int val)
{
	nodeT	*ret = (nodeT *)memarenamalloc(hasharena, sizeof(nodeT)); 

	if (ret) { 
		ret->val = val; 
		ret->next = NULL; 
	}

	return ret; 
} 

bucketT *
bucketT_new(int idx)
{
	bucketT	*ret = (bucketT *)memarenamalloc(hasharena, sizeof(bucketT)); 

	if (ret) { 	
		ret->idx = idx; 
		ret->cnt = 0; 
		ret->nodes = NULL; 
	}

	return ret;
}

void
bucketT_insert(bucketT *bucket, int val)
{
	nodeT	*node = nodeT_new(val); 
	if (bucket->cnt > 0) 
		node->next = bucket->nodes; 

	bucket->nodes = node; 
	bucket->cnt++; 
} 	

void
bucketT_free(bucketT *bucket)
{
#ifdef NOTDEF  // arena takes care of freeing the bucket
	nodeT	*p, *q; 

	if (bucket) { 
		p = bucket->nodes; 
		while (p != NULL) {
			q = p->next; 
			free(p); 
			p = q; 
		}  

		free(bucket); 
	} 
#endif
} 

int
bucketT_dump(bucketT *bucket, CASS_FILE *out)
{
	int		ret; 
	nodeT	*p = bucket->nodes; 

	ret = cass_write_uint32(&(bucket->cnt), 1, out); 
	assert(ret == 1); 

	ret = 0; 
	while (p != NULL) {
		ret += cass_write_uint32(&(p->val), 1, out); 
		p = p->next; 
	} 

	assert(ret == bucket->cnt); 
	return 0; 
}

bucketT *
bucketT_load(int idx, CASS_FILE *in)
{
	int			i, ret, cnt, val; 
	bucketT		*bucket = bucketT_new(idx); 

	ret = cass_read_uint32(&cnt, 1, in); 
	assert(ret == 1); 

	ret = 0; 
	for (i=0; i<cnt; i++) { 
		ret += cass_read_uint32(&val, 1, in); 
		bucketT_insert(bucket, val); 
	}

	assert(ret == cnt); 

	return bucket; 
}

hashTable_t *
hashTable_new(int key_size)
{
	int				i; 
	hashTable_t		*ret = (hashTable_t *)malloc(sizeof(hashTable_t)); 

	const gsl_rng_type	*T; 
	gsl_rng				*r; 

	ret->key_size = key_size; 
	ret->rndVec = (int *)calloc(key_size, sizeof(int)); 

	gsl_rng_env_setup(); 
	T = gsl_rng_default;
	r = gsl_rng_alloc(T); 
	
	for (i=0; i<key_size; i++) 
		ret->rndVec[i] = (int)(gsl_rng_uniform(r) * MAX_INT); 

	gsl_rng_free(r); 

	ret->size = DEFAULT_PRIME; 
	ret->buckets = (bucketT **)calloc(ret->size, sizeof(bucketT *)); 
	for (i=0; i<ret->size; i++) 
		ret->buckets[i] = NULL; 

	ret->cnt_item = 0; 
	ret->cnt_bucket = 0; 

	return ret; 
} 

int
hashTable_free(hashTable_t *hash_table)
{
	int		i; 

	if (hash_table != NULL) { 
		for (i=0; i<hash_table->size; i++) {
			if (hash_table->buckets[i] != NULL)
				bucketT_free(hash_table->buckets[i]);
		}

		free(hash_table->buckets); 
		free(hash_table->rndVec); 
		free(hash_table); 
	}

	return 0;  
} 

int
hashTable_dump(hashTable_t *hash_table, CASS_FILE *out)
{
	int			i, ret; 

	ret = cass_write_uint32(&(hash_table->key_size), 1, out); 
	ret += cass_write_uint32(&(hash_table->cnt_item), 1, out); 	
	ret += cass_write_uint32(&(hash_table->cnt_bucket), 1, out); 	
	assert(ret == 3); 

	ret = cass_write_uint32(hash_table->rndVec, 
		hash_table->key_size, out); 
	assert(ret == hash_table->key_size); 

	for (i=0; i<hash_table->size; i++) 
		if (hash_table->buckets[i] != NULL) { 
			ret = cass_write_uint32(&i, 1, out); 
			assert(ret == 1); 

			ret = bucketT_dump(hash_table->buckets[i], out);
			if (ret != 0)
				return ret; 
		} 

	return 0; 
} 
	
hashTable_t *
hashTable_load(CASS_FILE *in)
{
	int				i, x, ret;
	int				 key_size, cnt_item, cnt_bucket; 
	hashTable_t		*hash_table = type_calloc(hashTable_t, 1);

	ret = cass_read_uint32(&key_size, 1, in); 	
	ret += cass_read_uint32(&cnt_item, 1, in); 
	ret += cass_read_uint32(&cnt_bucket, 1, in); 
	assert(ret == 3); 

	hash_table->size = DEFAULT_PRIME; 
	hash_table->key_size = key_size; 
	hash_table->cnt_item = cnt_item; 
	hash_table->cnt_bucket = cnt_bucket; 

	hash_table->rndVec = (int *)calloc(key_size, sizeof(int)); 
	ret = cass_read_uint32(hash_table->rndVec, key_size, in); 
	assert(ret == key_size); 

	hash_table->buckets = (bucketT **)calloc(hash_table->size, 
			sizeof(bucketT *)); 
	for (i=0; i<cnt_bucket; i++) { 
		ret = cass_read_uint32(&x, 1, in); 
		assert(ret == 1); 

		hash_table->buckets[x] = bucketT_load(x, in); 
		if (hash_table->buckets[x] == NULL)
			return NULL; 
	}		

	return hash_table; 
} 

int
hashTable_hash(hashTable_t *table, hashT key)
{
	int		i, k, sum = 0; 

	for (i=0; i<table->key_size; i++) {
		//printf("%d\t", key[i]); 
		sum += table->rndVec[i] * key[i]; 
	}

	k = abs(sum % (table->size)); 
	//printf("%d\n", k); 

	return k; 
}

void 
hashTable_insert(hashTable_t *table, hashT key, int val)
{
	int			idx; 
	bucketT		*bucket; 

	idx = hashTable_hash(table, key); 
	bucket = table->buckets[idx]; 
	if (bucket == NULL) {
		bucket = bucketT_new(idx); 
		table->buckets[idx] = bucket; 
		table->cnt_bucket++; 
	} 
	bucketT_insert(bucket, val); 
	table->cnt_item++; 
}

bucketT *
hashTable_get_bucket(hashTable_t *table, hashT key)
{
	int		idx = hashTable_hash(table, key); 
	
	return table->buckets[idx]; 
}

int
hashTable_search(hashTable_t *table, bitmap_t *map, hashT key)
{
	int			i; 
	nodeT		*p; 
	int			idx = hashTable_hash(table, key); 
	bucketT		*bucket = table->buckets[idx];

	if (bucket) {
		p = bucket->nodes;
		for (i=0; i<bucket->cnt; i++) {
			bitmap_insert(map, p->val); 
			p = p->next; 
		} 
	}

	return 0; 
}

