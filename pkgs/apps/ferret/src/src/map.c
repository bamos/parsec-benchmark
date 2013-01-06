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
#include <cass.h>
#include <cass_array.h>

#include "cuckoo_hash.h"
#define MAP_MAGIC_SIG 0xdead0123
#define MAP_INIT_SIZE 256

int cass_map_private_init (cass_map_t *map, uint32_t size)
{
    ARRAY_INIT_SIZE(map->vtable, size);
    ARRAY_SET_INC(map->vtable, 16*1024);
    map->htable = ckh_alloc_table(size, (vtable_t *)&map->vtable);
    return 0;
}

static int cass_map_dump_private (cass_map_t *map)
{
    FILE *fout;
    char buf[BUFSIZ];
    
    snprintf(buf, BUFSIZ, "%s/%s.map", map->env->base_dir, map->name);
    fout = fopen(buf, "w");
    if (fout == NULL) return CASS_ERR_IO;

    if (cass_write_size(&map->vtable.len, 1, fout) != 1) return CASS_ERR_IO;

    ARRAY_BEGIN_FOREACH(map->vtable, const char *p)
    {
	if (cass_write_pchar(p, fout) != 0) return CASS_ERR_IO;
    } ARRAY_END_FOREACH;

    fclose(fout);
    map->dirty = 0;
    return 0;
}

static int cass_map_load_private (cass_map_t *map)
{
    FILE *fin;
    uint32_t tb_size, ix;
    cass_vecset_id_t vid;
    char *objname;
    char buf[BUFSIZ];
    
    assert(!map->loaded);
    
    snprintf(buf, BUFSIZ, "%s/%s.map", map->env->base_dir, map->name);
    fin = fopen(buf, "r");
    if (fin == NULL) return CASS_ERR_IO;

    if (cass_read_size(&tb_size, 1, fin) != 1) return CASS_ERR_IO;

    if (tb_size == 0) {
	cass_map_private_init(map, MAP_INIT_SIZE);
    } else {
	cass_map_private_init(map, tb_size);
	for (ix = 0; ix < tb_size; ix++) {
	    objname = cass_read_pchar(fin);
	    if (objname == NULL) return CASS_ERR_IO;
	    cass_map_insert(map, &vid, objname);
	    free(objname);
	    assert(vid == ix);
	}
    }

    fclose(fin);
    map->loaded = 1;
    map->dirty = 0;
    return 0;
}

// Deal with cass_map_t.
int cass_map_create(cass_map_t **_map, cass_env_t *env, char *map_name, uint32_t flag)
{
    cass_map_t *map;

    map = type_calloc(cass_map_t, 1);
    *_map = map;
    if (map == NULL) return CASS_ERR_OUTOFMEM;

    map->magic = MAP_MAGIC_SIG;
    map->env = env;
    map->name = strdup(map_name);
    map->flag = flag;
    map->refcnt ++;

    cass_map_private_init(map, MAP_INIT_SIZE);
    map->loaded = 1;
    map->dirty = 1; // So first checkpoint will pick up private data..
    assert(map->magic == MAP_MAGIC_SIG);
    return 0;
}

// flag says whether the dataobj name can be treated as id,
// then translation need no memory overhead.
int cass_map_associate_table(cass_map_t *map, int32_t table_id)
{
    ARRAY_APPEND(map->table_ids, table_id);
    return 0;
}

int cass_map_disassociate_table(cass_map_t *map, int32_t table_id)
{
    int ix;
    
    for (ix = 0; ix < map->table_ids.len; ix++)
	if (map->table_ids.data[ix] == table_id) break;
    
    for (; ix < map->table_ids.len-1; ix++) {
        map->table_ids.data[ix] = map->table_ids.data[ix+1];
    }
    map->table_ids.len--;
    return 0;
}

int cass_map_insert(cass_map_t *map, cass_vecset_id_t *id,
		    char *dataobj_name) // Will return the id assigned.
{
    char *name;
    if (map->flag & CASS_MAPPING)
	return 0;

//    name = strdup(dataobj_name);
    name = strdup(dataobj_name);
    if (name == NULL)
	return CASS_ERR_OUTOFMEM;

    *id = ARRAY_LEN(map->vtable);
    ARRAY_APPEND(map->vtable, name);
    ckh_insert(map->htable, *id);
    map->dirty = 1;
    return 0;
}

int cass_map_release(cass_map_t *map)  // release in-mem mapping.
{
    assert(!map->dirty);
    ARRAY_BEGIN_FOREACH(map->vtable, char *p)
    {
	if (p) free(p);
    } ARRAY_END_FOREACH;
    
    ARRAY_CLEANUP(map->vtable);
    ckh_destruct_table(map->htable);
    map->loaded = 0;
    return 0;
}

int cass_map_load(cass_map_t *map)  // bring mapping in-mem.
{
    // restore private data.
    return cass_map_load_private(map);
}

int cass_map_dataobj_to_id(cass_map_t *map, char *dataobj_name, cass_vecset_id_t *id)
{
    if (map->flag & CASS_MAPPING) {
	*id = atoi(dataobj_name);
	return 0;
    }

    *id = ckh_get(map->htable, dataobj_name);
    if (*id != CASS_ID_INV)
	return 0;
    else
	return -1;
}

static char local_buf[BUFSIZ];
int cass_map_id_to_dataobj(cass_map_t *map, cass_vecset_id_t id, char **dataobj_name)
{
    char *name;

    if (map->flag & CASS_MAPPING) {
	// XXX bad, not thread safe and also assume being consumed by caller immediately, need to revise..
	snprintf(local_buf, BUFSIZ, "%u", id);
	*dataobj_name = local_buf;
	return 0;
    }
    
    if (id < 0 || id >= ARRAY_LEN(map->vtable))
	return -1;
    name = ARRAY_GET(map->vtable, id);
    *dataobj_name = name;
    if (name == NULL)
	return -1;
    else
	return 0;
}

int cass_map_checkpoint(cass_map_t *map, CASS_FILE *out)
{
    assert(map->magic == MAP_MAGIC_SIG);
    if (cass_write_int32(&map->magic,1,  out) != 1) return CASS_ERR_IO;
    if (cass_write_pchar(map->name, out) != 0) return CASS_ERR_IO;
    if (cass_write_uint32(&map->flag, 1, out) != 1) return CASS_ERR_IO;
    if (cass_write_size(&map->table_ids.len, 1, out) != 1) return CASS_ERR_IO;
    if (cass_write_int32(map->table_ids.data, map->table_ids.len, out) != map->table_ids.len) return CASS_ERR_IO;

    if (map->loaded && map->dirty) {
	cass_map_dump_private(map);
    }
    return 0;
}

int cass_map_restore (cass_map_t **_map, cass_env_t *env, CASS_FILE *in)
{
    int ret;
    uint32_t n;

    ret = CASS_ERR_OUTOFMEM;
    cass_map_t *map = type_calloc(cass_map_t, 1);
    if (map == NULL) goto err;

    map->env = env;
    map->refcnt++;
    if (cass_read_int32(&map->magic, 1, in) != 1) goto err;
    assert(map->magic == MAP_MAGIC_SIG);

    map->name = cass_read_pchar(in);
    if (map->name == NULL) goto err;
    
    ret = CASS_ERR_IO;

    if (cass_read_uint32(&map->flag, 1, in) != 1) goto err;
    if (cass_read_size(&n, 1, in) != 1) goto err;

    ret = CASS_ERR_OUTOFMEM;
    ARRAY_INIT_SIZE(map->table_ids, n);
    if (n != 0 && map->table_ids.data == NULL) goto err;

    ret = CASS_ERR_IO;
    if (n > 0)
    {
    	if (cass_read_int32((int32_t *)map->table_ids.data, n, in) != n) goto err;
    	map->table_ids.len = n;
    }

    *_map = map;
    return 0;
err:
    if (map != NULL) cass_map_free(map);
    return ret;
}

int cass_map_describe(cass_map_t *map, CASS_FILE *out)
{
    char *str;
    
    cass_printf(out, "NAME:\t%s\n", map->name);
    if (map->flag & CASS_MAPPING)
	str = "direct";
    else
	str = "indirect";
    
    cass_printf(out, "MAPPING:\t%s\n", str);

    cass_printf(out, "TABLES: TBD");

    ARRAY_BEGIN_FOREACH(map->table_ids, uint32_t p)
    {
	cass_table_t *t = (cass_table_t *)cass_reg_get(&map->env->table, p);
	cass_printf(out, "\t%s", t->name);
    } ARRAY_END_FOREACH;
    cass_printf(out, "\n");
    
    return 0;
}

int cass_map_drop(cass_map_t *map) // destroy on-disk version.
{
    // XXX
    return -1;
}

int cass_map_free (cass_map_t *map) // release map struct.
{
    assert(map != NULL);
    map->refcnt--;
    if (map->refcnt > 0) return 0;
    if (map->name != NULL) free(map->name);
    
    assert(!map->dirty);
    ARRAY_CLEANUP(map->table_ids);

    if (map->loaded) cass_map_release(map);

    free(map);
    return 0;
}

