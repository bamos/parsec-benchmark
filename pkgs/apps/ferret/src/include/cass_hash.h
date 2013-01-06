#ifndef __WDONG_SHASH__
#define __WDONG_SHASH__

#include <cass_file.h>
#include <cass_array.h>

typedef ARRAY_TYPE(int) bucket_t;

/* open hash */
typedef struct
{
	cass_size_t size;
	bucket_t *bucket;
} ohash_t;

static inline void ohash_init (ohash_t *ohash, cass_size_t size)
{
	int i;
	ohash->size = size;
	ohash->bucket = malloc(size * sizeof(*ohash->bucket));
	assert(ohash->bucket != NULL);
	for (i = 0; i < size; i++)
	{
		ARRAY_INIT(ohash->bucket[i]);
	}
}

static inline void ohash_cleanup (ohash_t *ohash)
{
	int i;
	for (i = 0; i < ohash->size; i++)
	{
		ARRAY_CLEANUP(ohash->bucket[i]);
	}
	free(ohash->bucket);
}

static inline void ohash_insert (ohash_t *ohash, int hash, int val)
{
	int *ent;
	size_t len, i;
	ARRAY_BEGIN_WRITE_RAW(ohash->bucket[hash % ohash->size], ent, len);
	for (i = 0; i < len; i++)
	{
		if (ent[i] == val)
		{
			break;
		}
	}
	if (i >= len)
	{
		len++;
		ARRAY_APPEND(ohash->bucket[hash % ohash->size], val);
	}
	ARRAY_END_WRITE_RAW(ohash->bucket[hash % ohash->size], len);
}

int ohash_init_with_file (ohash_t *ohash, const char *filename);
int ohash_dump_file (ohash_t *ohash, const char *filename);
int ohash_init_with_stream (ohash_t *ohash, CASS_FILE *);
int ohash_dump_stream (ohash_t *ohash, CASS_FILE *);

int ohash_init_with_txt (ohash_t *ohash, const char *filename);
int ohash_dump_txt (ohash_t *ohash, const char *filename);
int ohash_init_with_txt_stream (ohash_t *ohash, CASS_FILE *);
int ohash_dump_txt_stream (ohash_t *ohash, CASS_FILE *);
void ohash_stat (ohash_t *ohash);

#endif

