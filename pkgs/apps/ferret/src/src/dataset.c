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
#include <string.h>
#include <cass.h>

#define MEM_OVERHEAD	3/2

static inline int cass_read_vecset (cass_vecset_t *vecset, size_t nmemb, CASS_FILE *in) {
    int n = fread(vecset, sizeof(cass_vecset_t), nmemb, in);
    if (!isLittleEndian()) {
        int i;
        assert(sizeof(vecset[0].start_vecid) == sizeof(uint32_t));
        for (i = 0; i < n; i++) {
            vecset[i].num_regions = bswap_int32(vecset[i].num_regions);
            vecset[i].start_vecid = bswap_int32(vecset[i].start_vecid);
        }
    }
    return n;
}


int cass_dataset_init (cass_dataset_t *ds, cass_size_t vec_size, cass_size_t vec_dim, uint32_t flags)
{
	memset(ds, 0, sizeof *ds);
	ds->vec_size = vec_size;
	ds->vec_dim = vec_dim;
	ds->flags = flags;
	return 0;
}

int cass_dataset_release (cass_dataset_t *ds)
{
	if (ds->vec != NULL) free(ds->vec);
	if (ds->vec != NULL) free(ds->vecset);
	ds->vec = NULL;
	ds->vecset = NULL;
	ds->max_vec = ds->max_vecset = 0;
	ds->loaded = 0;
	return 0;
}

#define MEM_1G	(1024*1024*1024)

cass_size_t grow (cass_size_t from, cass_size_t to)
{
	if (from >= to) return from;
	if (from == 0) from++;
	while ((from < to) && (from < MEM_1G)) from *= 2;
	while (from < to) from += MEM_1G / 2;
	
	return from;
}

int cass_dataset_grow (cass_dataset_t *ds, cass_size_t num_vecset, cass_size_t num_vec)
{
	if ((ds->flags & CASS_DATASET_VEC) && (ds->max_vec < num_vec))
	{
		ds->max_vec = grow(ds->max_vec, num_vec);
		ds->vec = (cass_vec_t *)realloc(ds->vec, ds->vec_size * ds->max_vec);
		if (ds->vec == NULL) return CASS_ERR_OUTOFMEM;
	}
	if ((ds->flags & CASS_DATASET_VECSET) && (ds->max_vecset < num_vecset))
	{
		ds->max_vecset = grow(ds->max_vecset, num_vecset);
		ds->vecset = (cass_vecset_t *)realloc(ds->vecset, sizeof (cass_vecset_t) * ds->max_vecset);
		if (ds->vecset == NULL) return CASS_ERR_OUTOFMEM;
	}
	return 0;
}


/* if src->flags & CASS_DATASET_VECSET == 0 then start & num are for vectors,
 * otherwise they are for vecsets. */
int cass_dataset_merge (cass_dataset_t *ds, const cass_dataset_t *src, cass_vecset_id_t start, cass_vecset_id_t num, cass_dataset_map_t map, void *map_param)
{
	void *vec;
	void *src_vec; 
	int i;
	cass_vec_id_t start_vec, num_vec;
	int parent_delta = 0;

	start_vec = num_vec = 0;

	assert(ds->loaded);
	assert(src->loaded);

	if (ds->flags & CASS_DATASET_VECSET)
	/* copy the vecset data */
	{
		assert(src->flags & CASS_DATASET_VECSET);
		parent_delta = ds->num_vecset - start;
		if (ds->num_vecset + num > ds->max_vecset)
		{
			ds->max_vecset = grow(ds->max_vecset, ds->num_vecset + num);
			ds->vecset = (cass_vecset_t *)realloc(ds->vecset, sizeof (cass_vecset_t) * ds->max_vecset);
			if (ds->vecset == NULL) return CASS_ERR_OUTOFMEM;
		}
		num_vec = 0;
		for (i = 0; i < num; i++)
		{
			ds->vecset[ds->num_vecset].num_regions = src->vecset[start + i].num_regions;
			ds->vecset[ds->num_vecset].start_vecid = ds->num_vec + num_vec;
			ds->num_vecset++;
			num_vec += src->vecset[start + i].num_regions;
		}
	}
	else
	{
		ds->num_vecset += num;
		num_vec = 0;
		for (i = 0; i < num; i++) num_vec += src->vecset[start + i].num_regions;
	}

	/* deal with vecs */
	if (ds->flags & CASS_DATASET_VEC)
	{
		if (src->flags & CASS_DATASET_VECSET)
		{
			start_vec = src->vecset[start].start_vecid;
		}
		else
		{
			num_vec = num;
			start_vec = start;
			parent_delta = 0;
		}

	
		if (ds->num_vec + num_vec > ds->max_vec)
		{
			ds->max_vec = grow(ds->max_vec, ds->num_vec + num_vec);
			ds->vec = (cass_vec_t *)realloc(ds->vec, ds->vec_size * ds->max_vec);
			if (ds->vec == NULL) return CASS_ERR_OUTOFMEM;
		}

		vec = ds->vec + ds->num_vec * ds->vec_size;
		src_vec = src->vec + start_vec * src->vec_size;

		if (map == NULL)
		{
			map = memcpy;
			map_param = (void *)ds->vec_size;
		}

		for (i = 0; i < num_vec; i++)
		{
			((cass_vec_t *)vec)->weight = ((cass_vec_t *)src_vec)->weight;
			((cass_vec_t *)vec)->parent = ((cass_vec_t *)src_vec)->parent + parent_delta;
			map(((cass_vec_t *)vec)->u.data, ((cass_vec_t *)src_vec)->u.data, map_param);

			vec += ds->vec_size;
			src_vec += src->vec_size;
			ds->num_vec++;
		}
	}
	else
	{
		ds->num_vec += num_vec;
	}

	return 0;
}


int cass_dataset_checkpoint (cass_dataset_t *ds, CASS_FILE *out)
{
	if (cass_write_uint32(&ds->flags, 1, out) != 1) return CASS_ERR_IO;
	if (cass_write_size(&ds->vec_size, 1, out) != 1) return CASS_ERR_IO;
	if (cass_write_size(&ds->vec_dim, 1, out) != 1) return CASS_ERR_IO;
	if (cass_write_size(&ds->num_vec, 1, out) != 1) return CASS_ERR_IO;
	if (cass_write_size(&ds->num_vecset, 1, out) != 1) return CASS_ERR_IO;
	return 0;
}

int cass_dataset_restore (cass_dataset_t *ds, CASS_FILE *in)
{
	memset(ds, 0, sizeof *ds);
	if (cass_read_uint32(&ds->flags, 1, in) != 1) return CASS_ERR_IO;
	if (cass_read_size(&ds->vec_size, 1, in) != 1) return CASS_ERR_IO;
	if (cass_read_size(&ds->vec_dim, 1, in) != 1) return CASS_ERR_IO;
	if (cass_read_size(&ds->num_vec, 1, in) != 1) return CASS_ERR_IO;
	if (cass_read_size(&ds->num_vecset, 1, in) != 1) return CASS_ERR_IO;
	return 0;
}

int cass_dataset_load (cass_dataset_t *ds, CASS_FILE *in, cass_vec_type_t vec_type)
{
	int ret;
	assert(!ds->loaded);
	assert(ds->vec == NULL);
	assert(ds->vecset == NULL);

	if (ds->flags & CASS_DATASET_VECSET)
	{
		ret = CASS_ERR_OUTOFMEM;
		ds->max_vecset = ds->num_vecset * MEM_OVERHEAD;
		ds->vecset = (cass_vecset_t *)malloc(sizeof(cass_vecset_t) * ds->max_vecset);
		if (ds->vecset == NULL) goto err;
		ret = CASS_ERR_IO;
		if (cass_read_vecset(ds->vecset, ds->num_vecset, in) != ds->num_vecset) goto err;
	}

	if (ds->flags & CASS_DATASET_VEC)
	{
		ds->max_vec = ds->num_vec;// * MEM_OVERHEAD;
		ret = CASS_ERR_OUTOFMEM;
		ds->vec = (cass_vec_t *)malloc(ds->vec_size * ds->max_vec);
		if (ds->vec == NULL) goto err;
		ret = CASS_ERR_IO;
        if (isLittleEndian()) {
		    if (cass_read(ds->vec, ds->vec_size, ds->num_vec, in) != ds->num_vec) goto err;
        }
        else {
            if (vec_type == CASS_VEC_INT) {
                unsigned cnt = ds->vec_size / sizeof(int32_t) * ds->num_vec;
                if (cass_read_int32(ds->vec, cnt, in) != cnt) goto err;
            }
            else if (vec_type == CASS_VEC_FLOAT) {
                unsigned cnt = ds->vec_size / sizeof(float) * ds->num_vec;
                if (cass_read_float(ds->vec, cnt, in) != cnt) goto err;
            }
            else if (vec_type == CASS_VEC_BIT) {
		    if (cass_read(ds->vec, ds->vec_size, ds->num_vec, in) != ds->num_vec) goto err;
            }
            else assert(0);
        }
	}

	ds->loaded = 1;

	return 0;

err:
	cass_dataset_release(ds);

	return ret;
}

int cass_dataset_dump (cass_dataset_t *ds, CASS_FILE *out)
{
	assert(ds->loaded);

	if (ds->flags & CASS_DATASET_VECSET)
	{
		if (cass_write(ds->vecset, sizeof (cass_vecset_t), ds->num_vecset, out) != ds->num_vecset) return CASS_ERR_IO;
	}

	if (ds->flags & CASS_DATASET_VEC)
	{
		if (cass_write(ds->vec, ds->vec_size, ds->num_vec, out) != ds->num_vec) return CASS_ERR_IO;
	}

	return 0;
}

cass_vecset_id_t cass_dataset_vec2vecset (cass_dataset_t *ds, cass_vec_id_t id)
{
	cass_vecset_id_t l, r, m;
	l = 0;
	r = ds->num_vecset - 1;
	assert(ds->flags & CASS_DATASET_VECSET);
	assert(id < ds->num_vec);
	if (id >= ds->vecset[r].start_vecid) return r;
	for (;;)
	{
		if (r == l + 1) return l;
		m = (l + r) / 2;
		if (id < ds->vecset[m].start_vecid) r = m;
		if (id >= ds->vecset[m].start_vecid) l = m;
	}
	assert(0);
}

