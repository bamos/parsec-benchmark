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
#define _ISOC99_SOURCE
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <cass.h>

void __debug (const char *file, int line, const char *func, const char *fmt, ...)
{
	va_list argptr;
	int cnt;
	fprintf(stderr, "%s:%d: %s:", file, line, func);
	va_start(argptr, fmt);
	cnt = vfprintf(stderr, fmt, argptr);
	va_end(argptr);
}

int mkpath (char *buf, const char *base, const char *table, const char *ext)
{
	if (snprintf(buf, BUFSIZ, "%s/%s.%s", base, table, ext) >= BUFSIZ) assert(0);
	return 0;
}

int fexist (const char *path)
{
	struct stat buf;
	if (stat(path, &buf) != 0) return 0;
	return S_ISREG(buf.st_mode);
}


int dexist (const char *path)
{
	struct stat buf;
	if (lstat(path, &buf) != 0) return 0;
	return S_ISDIR(buf.st_mode);
}

char *cass_read_pchar (CASS_FILE *in)
{
	cass_size_t len;
	char *p;
	if (cass_read_size(&len, 1, in) != 1) return NULL;
	p = malloc(len);
	if (cass_read_char(p, len, in) != len)
	{
		free(p);
		return NULL;
	}
//	warn("READ: %s\n", p);
	return p;
}

int cass_write_pchar (const char *buf, CASS_FILE *out)
{
	cass_size_t len = strlen(buf) + 1;
	if (cass_write_size(&len, 1, out) != 1) return CASS_ERR_IO;
	if (cass_write_char(buf, len, out) != len) return CASS_ERR_IO;
	return 0;
}

int32_t param_get_int (const char *param,  const char *key, int32_t def)
{
	if (param == NULL) return def;
	const char *p = strstr(param, key);
	if (p == NULL) return def;
	p += strlen(key);
	return strtol(p, NULL, 0);
}

float param_get_float (const char *param,  const char *key, float def)
{
	if (param == NULL) return def;
	const char *p = strstr(param, key);
	float f;
	if (p == NULL) return def;
	p += strlen(key);
	sscanf(p, "%f", &f);
	return f;
}


int param_get_float_array (const char *param,  const char *key, cass_size_t *_n, float *f /* default */)
{
	cass_size_t  n = 0;
	char *p = strstr(param, key);
	if (p == NULL) return -1;
	p += strlen(key);
	while (*p != '(' && *p != 0 ) p++;
	if (*p == 0) return -1;
	for (;;)
	{
		p++;
		sscanf(p, "%f", &f[n]);
		n++;
		while (*p != ',' && *p != ')' && *p != 0) p++;
		if (*p == ')' || *p == 0) break;
	}
	*_n = n;
	return 0;
}


cass_size_t cass_vec_dim2size (cass_vec_type_t type, cass_size_t dim)
{
	cass_size_t size = CASS_VEC_HEAD_SIZE;
	switch (type)
	{
		case CASS_VEC_INT:	size += sizeof(int32_t) * dim; break;
		case CASS_VEC_FLOAT:	size += sizeof(float) * dim; break;
		case CASS_VEC_BIT:	size += (dim + CHUNK_BIT -1)/CHUNK_BIT * sizeof(chunk_t); break;
		default: assert(0);
	}
	return size;
}

int cass_result_alloc_list (cass_result_t *result, cass_size_t num_regions, cass_size_t topk)
{
	memset(result, 0, sizeof *result);
	if (num_regions == 0)
	{
		result->flags |= CASS_RESULT_LIST;
		ARRAY_INIT_SIZE(result->u.list, topk);
	}
	else
	{
		int i;
		result->flags |= CASS_RESULT_LISTS;
		ARRAY_INIT_SIZE(result->u.lists, num_regions);
		result->u.lists.len = num_regions;
		for (i = 0; i < num_regions; i++)
		{
			ARRAY_INIT_SIZE(result->u.lists.data[i], topk);
		}
	}
	return 0;
}

int
cass_result_alloc_bitmap(cass_result_t *result, cass_size_t num_regions, 
	cass_size_t size)
{
	memset(result, 0, sizeof(cass_result_t)); 
	
	if (num_regions == 0) { 
		result->flags |= CASS_RESULT_BITMAP;
		bitmap_init(&(result->u.bitmap), size); 
	}
	else {
		int i; 
		result->flags |= CASS_RESULT_BITMAPS; 
		ARRAY_INIT_SIZE(result->u.bitmaps, num_regions); 
		result->u.bitmaps.len = num_regions; 
		for (i=0; i<num_regions; i++) 
			bitmap_init(&(result->u.bitmaps.data[i]), size);
	}

	return 0; 
} 

int
cass_result_merge_bitmaps(cass_result_t *src, cass_result_t *dst)
{
	bitmap_t		*dst_map = &(dst->u.bitmap); 
	cass_size_t		dst_size = bitmap_get_size(dst_map); 

	assert(src->flags & CASS_RESULT_BITMAPS);
	assert(dst->flags & CASS_RESULT_BITMAP);
	
	bitmap_clear(dst_map); 

	ARRAY_BEGIN_FOREACH_P(src->u.bitmaps, bitmap_t *map)
	{
		assert(bitmap_get_size(map) == dst_size); 
		bitmap_union(dst_map, map); 
	} ARRAY_END_FOREACH_P;

	return 0; 
} 

int cass_result_free (cass_result_t *result)
{
	if (result->flags & CASS_RESULT_LIST)
	{
		ARRAY_CLEANUP(result->u.list);
	}
	else if (result->flags & CASS_RESULT_LISTS)
	{
		ARRAY_BEGIN_FOREACH_P(result->u.lists, cass_list_t *p)
		{
			ARRAY_CLEANUP(*p);
		} ARRAY_END_FOREACH_P;
		ARRAY_CLEANUP(result->u.lists);
	}
	else if (result->flags & CASS_RESULT_BITMAP)
	{ 
		bitmap_free_vec(&(result->u.bitmap)); 
	}
	else if (result->flags & CASS_RESULT_BITMAPS)
	{
		ARRAY_BEGIN_FOREACH_P(result->u.bitmaps, cass_map_t *map)
		{
			bitmap_free_vec(map); 
		} ARRAY_END_FOREACH_P;
		ARRAY_CLEANUP(result->u.bitmaps);
	}
	else debug("cass_result_free: unsupported formate.\n"); 
	
	return 0;
}

static inline int __cass_list_entry_ge (cass_list_entry_t a, cass_list_entry_t b)
{
	return a.dist >= b.dist;
}

static inline int __cass_list_entry_rev_ge (cass_list_entry_t a, cass_list_entry_t b)
{
	return b.dist >= a.dist;
}

QUICKSORT_GENERATE(__cass_list_entry, cass_list_entry_t);

cass_result_t *
cass_result_merge_lists (cass_result_t *src, cass_dataset_t *ds, int32_t topk)
{
    cass_result_t *merged_result;
    int merged_topk;

    assert((src->flags & CASS_RESULT_BITMAPS) == 0);
    assert((src->flags & CASS_RESULT_BITMAP) == 0);
    assert((src->flags & CASS_RESULT_LIST) == 0);
    assert(src->flags & CASS_RESULT_LISTS);

    assert(topk == 0); // xxx later will modify this.
    merged_topk = 0;
    ARRAY_BEGIN_FOREACH(src->u.lists, cass_list_t p)
    {
	merged_topk += ARRAY_LEN(p);
    }ARRAY_END_FOREACH;

    merged_result = type_calloc(cass_result_t, 1);
    cass_result_alloc_list(merged_result, 0, merged_topk);

//    ARRAY_INIT_SIZE(merged_result->u.list, merged_topk);
    merged_result->u.list.len = merged_topk;
    TOPK_INIT(merged_result->u.list.data, id, merged_topk, CASS_ID_MAX);
    ARRAY_BEGIN_FOREACH(src->u.lists, cass_list_t p)
    {
	ARRAY_BEGIN_FOREACH(p, cass_list_entry_t p2){
	    cass_vec_t *vec;
	    cass_list_entry_t entry;

//	    assert(p2.id >= 0 && p2.id <= ds->max_vec);
	    if (p2.id < 0 || p2.id > ds->max_vec)
	    {
//		    printf("%d\t%d\n", __array_foreach_index, p2.id);
		    continue;
	    }
	    vec = (void *)ds->vec + ds->vec_size * p2.id;
	    entry.id = vec->parent;
	    entry.dist = 0;
	    //ARRAY_APPEND(merged_result->u.list, entry);
	    TOPK_INSERT_MIN_UNIQ(merged_result->u.list.data, id, id, merged_topk, entry);
	    //TOPK_INSERT_MIN(merged_result->u.list.data, id, merged_topk, entry);
	}ARRAY_END_FOREACH;
    }ARRAY_END_FOREACH;
    merged_result->flags |= CASS_RESULT_MALLOC;
    return merged_result;
}
