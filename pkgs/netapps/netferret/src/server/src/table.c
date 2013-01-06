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
/* wdong */
#include <string.h>
#include <unistd.h>
#include <cass.h>

int cass_table_create (cass_table_t **_table,
		cass_env_t *env,  // See management section
                char *table_name,
                uint32_t opr_idx,
                int32_t cfg_id,
		int32_t parent_id,
                int32_t parent_cfg_id,
                int32_t map_id,
                char *param)
{
    char buf[BUFSIZ];
    int ret;
    ret = CASS_ERR_OUTOFMEM;
    cass_table_t *table = type_calloc(cass_table_t, 1);
    if (table == NULL) goto err;
    table->refcnt = 1;
    table->name = strdup(table_name);
    if (table->name == NULL) goto err;


    table->env = env;

    table->parent_id = parent_id;
    table->cfg_id = cfg_id;
    table->parent_cfg_id = parent_cfg_id;
    table->map_id = map_id;

    ARRAY_INIT(table->child_ids);

    table->opr = cass_table_opr_get(opr_idx);

    sprintf(buf, "%s/%s.%s", env->base_dir, table_name, table->opr->name);

    table->filename = strdup(buf);

    table->loaded = 0;
    table->dirty = 0;

    ret = __cass_table_ref(table); /* fill the pointers, create children list */
    if (ret != 0) goto err;

    ret = table->opr->init_private(table, param);
    if (ret != 0) goto err;

    *_table = table;
    return 0;
err:
    if (table != NULL) cass_table_free(table);
    return ret;
}

int cass_table_associate(cass_table_t *table, 
                               int32_t tbl2)
{
    cass_table_t *ptbl2 = cass_reg_get(&table->env->table, tbl2);
    assert(ptbl2 != NULL);
    ARRAY_APPEND(table->child_ids, tbl2);
    ARRAY_APPEND(table->children, ptbl2);
    assert(table->children.len == table->child_ids.len);
    ptbl2->parent_id = cass_reg_find(&table->env->table, table);
    ptbl2->parent_cfg_id = table->cfg_id;
    ptbl2->parent_cfg = table->cfg;
    ptbl2->parent_cfg->refcnt++;
    ptbl2->refcnt++;
    return 0;
}

int cass_table_disassociate(cass_table_t *table, 
                                  int32_t tbl2)
{
    int i;
    cass_table_t *ptbl2 = cass_reg_get(&table->env->table, tbl2);
    assert(ptbl2 != NULL);
    for (i = 0; i < table->children.len; i++) if (table->children.data[i] == ptbl2) break;
    for (; i < table->child_ids.len - 1; i++)
    {
        table->children.data[i] = table->children.data[i+1];
        table->child_ids.data[i] = table->child_ids.data[i+1];
    }
    table->child_ids.len--;
    table->children.len--;
    assert(table->children.len == table->child_ids.len);
    cass_table_free(ptbl2);
    cass_vecset_cfg_free(ptbl2->parent_cfg);
    return 0;
}

int cass_table_free(cass_table_t *table)
{
    assert(table != NULL);
    table->refcnt--;
    if (table->refcnt > 0) return 0;
    assert(!table->dirty);
    __cass_table_unref(table);
    if (table->__private != NULL) table->opr->free_private(table);
    ARRAY_CLEANUP(table->child_ids);
    if (table->name != NULL) free(table->name);
    if (table->filename != NULL) free(table->filename);
    free(table);
    return 0;
}

int cass_table_restore (cass_table_t **_table, cass_env_t *env, CASS_FILE *in)
{
    int ret;
    uint32_t n, c;
    ret = CASS_ERR_OUTOFMEM;
    cass_table_t *table = type_calloc(cass_table_t, 1);
    if (table == NULL) goto err;
    table->env = env;
    table->refcnt++;
    table->name = cass_read_pchar(in);
    if (table->name == NULL) goto err;
    char buf[BUFSIZ];

    ret = CASS_ERR_IO;

    //if (cass_read_uint32_t(&table->self_id, 1, in) != 1) goto err;
    if (cass_read_int32(&table->parent_id, 1, in) != 1) goto err;
    if (cass_read_int32(&table->cfg_id, 1, in) != 1) goto err;
    if (cass_read_int32(&table->parent_cfg_id, 1, in) != 1) goto err;
    if (cass_read_int32(&table->map_id, 1, in) != 1) goto err;

    if (cass_read_size(&n, 1, in) != 1) goto err;

    ret = CASS_ERR_OUTOFMEM;
    ARRAY_INIT_SIZE(table->child_ids, n);
    if (n != 0 && table->child_ids.data == NULL) goto err;

    ret = CASS_ERR_IO;
    if (n > 0)
    {
    	if (cass_read_int32((int32_t *)table->child_ids.data, n, in) != n) goto err;
    	table->child_ids.len = n;
    }

    if (cass_read_uint32(&c, 1, in) != 1) goto err;

    table->opr = cass_table_opr_get(c);
    assert(table->opr != NULL);
    sprintf(buf, "%s/%s.%s", env->base_dir, table->name, table->opr->name);

    table->filename = strdup(buf);

    table->loaded = table->dirty = 0;

    ret = table->opr->restore_private(table, in);
    if (ret != 0) goto err;

    *_table = table;
    return 0;
err:
    if (table != NULL) cass_table_free(table);
    return ret;
}

int cass_table_checkpoint (cass_table_t *table, CASS_FILE *out)
{
    uint32_t c;
    int ret;
    if (cass_write_pchar(table->name, out) != 0) return CASS_ERR_IO;
    //if (cass_write_uint32_t(&table->self_id, 1, out) != 1) return CASS_ERR_IO;
    if (cass_write_int32(&table->parent_id, 1, out) != 1) return CASS_ERR_IO;
    if (cass_write_int32(&table->cfg_id, 1, out) != 1) return CASS_ERR_IO;
    if (cass_write_int32(&table->parent_cfg_id, 1, out) != 1) return CASS_ERR_IO;
    if (cass_write_int32(&table->map_id, 1, out) != 1) return CASS_ERR_IO;

    if (cass_write_size(&table->child_ids.len, 1, out) != 1) return CASS_ERR_IO;
    if (cass_write_int32(table->child_ids.data, table->child_ids.len, out) != table->children.len) return CASS_ERR_IO;

    c = cass_table_opr_find(table->opr);
    if (cass_write_uint32(&c, 1, out) != 1) return CASS_ERR_IO;

    ret = table->opr->checkpoint_private(table, out);
    if (ret == 0) table->dirty = 0;
    return ret;
}


/*
int cass_table_insert (cass_table_t *table,
              cass_vecset_id_t id,
                      cass_vecset_t *vecset)
{
    ARRAY_BEGIN_FOREACH(table->children, p)
    {
        cass_table_insert(p, id, vecset);
    } ARRAY_END_FOREACH(table->children, p);
    return table->opr->insert(table, id, vecset);
}
*/

int cass_table_batch_insert(cass_table_t *table, cass_dataset_t * parent, cass_vecset_id_t start, cass_vecset_id_t end)
{
    int ret = 0;
    ARRAY_BEGIN_FOREACH(table->children, cass_table_t *p)
    {
        ret = cass_table_batch_insert(p, parent, start, end);
	if (ret != 0) return ret;
    } ARRAY_END_FOREACH;
    assert(table->opr->batch_insert != NULL);
    table->dirty = 1;
    return table->opr->batch_insert(table, parent, start, end);
}

int cass_table_describe (cass_table_t *table, CASS_FILE *out)
{
	cass_printf(out, "NAME:\t%s\n", table->name);
	cass_printf(out, "CLASS:\t%s\n", table->opr->name);

	if (table->cfg == NULL)
	{
		cass_printf(out, "CONFIG: (none, the table is sketch/index.)");
	}
	else
	{
		cass_printf(out, "CONFIG:\t%s\n", table->cfg->name);
	}

	/*
	{
		cass_vecset_dist_t *p = (cass_vecset_dist_t *)cass_reg_get(&table->env->vecset_dist, table->vecset_dist_id);
		assert(p != NULL);
		cass_printf(out, "VECSET DIST:\t%s\n", p->name);
	}

	{
		cass_vec_dist_t *p = (cass_vec_dist_t *)cass_reg_get(&table->env->vec_dist, table->vec_dist_id);
		assert(p != NULL);
		cass_printf(out, "VEC DIST:\t%s\n", p->name);
	}
	*/

	cass_printf(out, "MAP:\t");
	if (table->map_id == -1)
	{
		cass_printf(out, "(none)\n");
	}
	else
	{
		cass_map_t *p = (cass_map_t *)cass_reg_get(&table->env->map, table->map_id);
		assert(p != NULL);
		cass_printf(out, "%s\n", p->name);
	}

	cass_printf(out, "PARENT:\t");
	if (table->parent_id == -1)
	{
		cass_printf(out, "(none)\n");
	}
	else
	{
		cass_table_t *p = (cass_table_t *)cass_reg_get(&table->env->table, table->parent_id);
		assert(p != NULL);
		cass_printf(out, "%s\n", p->name);
	}

	cass_printf(out, "CHILDREN:");
	ARRAY_BEGIN_FOREACH(table->children, cass_table_t *p)
	{
		cass_printf(out, "\t%s", p->name);
	} ARRAY_END_FOREACH;
	cass_printf(out, "\n");

	return 0;
}

cass_table_t *cass_table_parent (cass_table_t *table)
{
	cass_env_t *env = table->env;
	assert(env != NULL);
	if (table->parent_id < 0) return NULL;
	return cass_reg_get(&env->table, table->parent_id);
}

int cass_table_drop (cass_table_t *table)
{
	return unlink(table->filename);
}

int cass_table_query (cass_table_t *table,
                     cass_query_t *query, cass_result_t *result)
{
	if (table->opr->query != NULL) return table->opr->query(table, query, result);
	else
	{
		assert(table->opr->batch_query != NULL);
		return table->opr->batch_query(table, 1, &query, &result);
	}
}


int cass_table_batch_query (cass_table_t *table, uint32_t count,
                           cass_query_t **queries,
                           cass_result_t **results)
{
	if (table->opr->batch_query != NULL) return table->opr->batch_query(table, count, queries, results);
	else
	{
		int i;
		assert(table->opr->query != NULL);
		for (i = 0; i < count; i++)
		{
			table->opr->query(table, queries[i], results[i]);
		}
	}
	return 0;
}

