#ifndef _HASH_TABLE_H_
#define _HASH_TABLE_H_

#include <cass.h>
#include <cass_bitmap.h>

typedef int * 	hashT; 

typedef struct hashTable_t hashTable_t; 

void hashT_print(hashT key, int key_size, const char *tag, FILE *fp); 

void hashTable_init(void); 

hashTable_t *hashTable_new(int key_size); 

int hashTable_free(hashTable_t *table); 

void hashTable_insert(hashTable_t *table, hashT key, int val); 

//bucketT *hashTable_get_bucket(hashTable_t *table, hashT key); 

int hashTable_search(hashTable_t *table, bitmap_t *map, hashT key); 

hashTable_t *hashTable_load(CASS_FILE *in); 

int hashTable_dump(hashTable_t *hash_table, CASS_FILE *out); 

#endif /* _HASH_TABLE_H_ */
