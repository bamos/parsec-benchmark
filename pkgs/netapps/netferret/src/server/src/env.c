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

int cass_env_errmsg(cass_env_t *env, int error, const char *fmt, ...)
{
#warning to be elaborated.
    return 0;
}

int cass_env_panic(cass_env_t *env, const char *msg)
{
    puts(msg);
    exit(-1);
    return 0;
}

static int cass_env_add_default (cass_env_t *env)
{
	return 0;
}

// cass_env functions
int cass_env_open(cass_env_t **_env, char *base_dir, uint32_t flags)
{
    int ret = 0;
    CASS_FILE *in = NULL;
    char buf[BUFSIZ];
    cass_env_t *env = NULL;
    cass_size_t i, n;

    if (base_dir == NULL) return CASS_ERR_PARAMETER;

    if (!dexist(base_dir)) return CASS_ERR_PARAMETER;

    if (mkpath(buf, base_dir, "__cass", "env") != 0) goto err;

    /* check if the files are already there
     * if not, create an empty enviroment */
    if (!fexist(buf))
    {
            ret = CASS_ERR_OUTOFMEM;
	    env = type_calloc(cass_env_t, 1);
	    if (env == NULL) goto err;
            env->base_dir = strdup(base_dir);
            if (env->base_dir == NULL) goto err;
	    cass_reg_init(&env->table);
	    cass_reg_init(&env->cfg);
	    cass_reg_init(&env->map);
	    cass_reg_init(&env->vecset_dist);
	    cass_reg_init(&env->vec_dist);
	    cass_env_add_default(env);
	    *_env = env;
	    return 0;
    }

    if (flags && CASS_EXCL)
    {
	    return CASS_ERR_PARAMETER;
    }

    ret = CASS_ERR_OUTOFMEM;

    env = type_calloc(cass_env_t, 1);
    
    if (env == NULL) goto err;

    env->base_dir = strdup(base_dir);
    if (env->base_dir == NULL) goto err;
    
    env->mode = flags & (CASS_EXCL | CASS_READONLY);

    ret = CASS_ERR_IO;
    in = cass_open(buf, "r");

    if (in == NULL) goto err;

    /* table */
    if (cass_read_size(&n, 1, in) != 1) goto err;
    ret = CASS_ERR_OUTOFMEM;
    cass_reg_init_size(&env->table, n);
    if (n != 0 && env->table.data == NULL) goto err;

    ret = CASS_ERR_IO;
    for (i = 0; i < n; i++)
    {
	    cass_table_t *table;
            ret = cass_table_restore(&table, env, in);
            if (ret != 0) goto err;
	    cass_reg_add(&env->table, table->name, table);
    }

    /* cfg */
    if (cass_read_size(&n, 1, in) != 1) goto err;
    ret = CASS_ERR_OUTOFMEM;
    cass_reg_init_size(&env->cfg, n);
    if (n != 0 && env->cfg.data == NULL) goto err;

    ret = CASS_ERR_IO;
    for (i = 0; i < n; i++)
    {
	    cass_vecset_cfg_t *cfg;
            ret = cass_vecset_cfg_restore(&cfg, in);
            if (ret != 0) goto err;
	    cass_reg_add(&env->cfg, cfg->name, cfg);
    }

    /* map */
    if (cass_read_size(&n, 1, in) != 1) goto err;
    ret = CASS_ERR_OUTOFMEM;
    cass_reg_init_size(&env->map, n);
    if (n != 0 && env->map.data == NULL) goto err;

    ret = CASS_ERR_IO;
    for (i = 0; i < n; i++)
    {
	    cass_map_t *map;
            ret = cass_map_restore(&map, env, in);
            if (ret != 0) goto err;
	    cass_reg_add(&env->map, map->name, map);
    }
    /* vecset_dist */
    if (cass_read_size(&n, 1, in) != 1) goto err;
    ret = CASS_ERR_OUTOFMEM;
    cass_reg_init_size(&env->vecset_dist, n);
    if (n != 0 && env->vecset_dist.data == NULL) goto err;

    ret = CASS_ERR_IO;
    for (i = 0; i < n; i++)
    {
            const cass_vecset_dist_class_t *t;
	    cass_vecset_dist_t *dist;
            uint32_t j;
            ret = CASS_ERR_IO;
            if (cass_read_uint32(&j, 1, in) != 1) goto err;
            t = cass_vecset_dist_class_get(j);
	    assert(t != NULL);
	    ret = t->restore((void **)&dist, in);
            if (ret != 0) goto err;
	    cass_reg_add(&env->vecset_dist, dist->name, dist);
    }

    /* vec_dist */
    if (cass_read_size(&n, 1, in) != 1) goto err;
    ret = CASS_ERR_OUTOFMEM;
    cass_reg_init_size(&env->vec_dist, n);
    if (n != 0 && env->vec_dist.data == NULL) goto err;

    ret = CASS_ERR_IO;
    for (i = 0; i < n; i++)
    {
            const cass_vec_dist_class_t *t;
	    cass_vec_dist_t *dist;
            uint32_t i;
            ret = CASS_ERR_IO;
            if (cass_read_uint32(&i, 1, in) != 1) goto err;
            t = cass_vec_dist_class_get(i);
	    ret = t->restore((void **)&dist, in);
            if (ret != 0) goto err;
	    cass_reg_add(&env->vec_dist, dist->name, dist);
    }

    cass_close(in);
    in = NULL;

    ARRAY_BEGIN_FOREACH(env->table, cass_reg_entry_t ref)
    {
        ret = __cass_table_ref(ref.p);
        if (ret != 0) goto err;
    } ARRAY_END_FOREACH;

    *_env = env;

    return 0;
err:    if (env != NULL) cass_env_close(env, 0);
    if (in != NULL) cass_close(in);
    return ret;
}

int cass_env_close(cass_env_t *env, uint32_t flags)
{
    ARRAY_BEGIN_FOREACH(env->table, cass_reg_entry_t reg)
    {
	    cass_table_free(reg.p);
    } ARRAY_END_FOREACH;
    cass_reg_cleanup(&env->table);

    ARRAY_BEGIN_FOREACH(env->cfg, cass_reg_entry_t reg)
    {
	    cass_vecset_cfg_free(reg.p);
    } ARRAY_END_FOREACH;
    cass_reg_cleanup(&env->cfg);

    ARRAY_BEGIN_FOREACH(env->map, cass_reg_entry_t reg)
    {
	    cass_map_free(reg.p);
    } ARRAY_END_FOREACH;
    cass_reg_cleanup(&env->map);

    ARRAY_BEGIN_FOREACH(env->vecset_dist, cass_reg_entry_t reg)
    {
	    ((cass_vecset_dist_t *)reg.p)->__class->free(reg.p);
    } ARRAY_END_FOREACH;
    cass_reg_cleanup(&env->vecset_dist);

    ARRAY_BEGIN_FOREACH(env->vec_dist, cass_reg_entry_t reg)
    {
	    ((cass_vec_dist_t *)reg.p)->__class->free(reg.p);
    } ARRAY_END_FOREACH;
    cass_reg_cleanup(&env->vec_dist);

    if (env->base_dir != NULL) free(env->base_dir);

    free(env);
    return 0;
}

int cass_env_checkpoint(cass_env_t *env)
{
    int ret;
    CASS_FILE *out = NULL;
    char buf[BUFSIZ];

    ret = CASS_ERR_PARAMETER;

    if (env->mode & CASS_READONLY) goto err;

#warning add file switch
    if (mkpath(buf, env->base_dir, "__cass", "env") != 0) goto err;
    
    ret = CASS_ERR_IO;
    out = cass_open(buf, "w");
    if (out == NULL) goto err;

    /* table */
    if (cass_write_size(&env->table.len, 1, out) != 1) goto err;

    ARRAY_BEGIN_FOREACH(env->table, cass_reg_entry_t reg)
    {
            ret = cass_table_checkpoint(reg.p, out);
            if (ret != 0) goto err;
    } ARRAY_END_FOREACH;

    /* cfg */
    if (cass_write_size(&env->cfg.len, 1, out) != 1) goto err;

    ARRAY_BEGIN_FOREACH(env->cfg, cass_reg_entry_t reg)
    {
            ret = cass_vecset_cfg_checkpoint(reg.p, out);
            if (ret != 0) goto err;
    } ARRAY_END_FOREACH;
    
    /* map */
    if (cass_write_size(&env->map.len, 1, out) != 1) goto err;

    ARRAY_BEGIN_FOREACH(env->map, cass_reg_entry_t reg)
    {
            ret = cass_map_checkpoint(reg.p, out);
            if (ret != 0) goto err;
    } ARRAY_END_FOREACH;
   
    /* vecset_dist */
    if (cass_write_size(&env->vecset_dist.len, 1, out) != 1) goto err;

    ARRAY_BEGIN_FOREACH(env->vecset_dist, cass_reg_entry_t reg)
    {
	    cass_vecset_dist_t *dist = reg.p;
	    int32_t id = cass_vecset_dist_class_find(dist->__class);
	    assert(id >= 0);
	    if (cass_write_int32(&id, 1, out) != 1) goto err;
	    ret = dist->__class->checkpoint(dist, out);
            if (ret != 0) goto err;
    } ARRAY_END_FOREACH;


    /* vec_dist */
    if (cass_write_size(&env->vec_dist.len, 1, out) != 1) goto err;

    ARRAY_BEGIN_FOREACH(env->vec_dist, cass_reg_entry_t reg)
    {
	    cass_vec_dist_t *dist = reg.p;
	    int32_t id = cass_vec_dist_class_find(dist->__class);
	    if (cass_write_int32(&id, 1, out) != 1) goto err;
	    ret = dist->__class->checkpoint(dist, out);
            if (ret != 0) goto err;
    } ARRAY_END_FOREACH;
    cass_close(out);

    return 0;
err:    if (out != NULL) cass_close(out);

    return ret;
}

#define SPREFIX "========  "
#define SPOSTFIX "  ========\n"

int cass_env_describe(cass_env_t *env, CASS_FILE *out)
{
    int ret;

    cass_printf(out, SPREFIX "BUILT-IN TABLE CLASS (%d)" SPOSTFIX, cass_table_opr_reg.len);

    ARRAY_BEGIN_FOREACH(cass_table_opr_reg, cass_reg_entry_t reg)
    {
	    cass_table_opr_t *p = reg.p;
	    cass_printf(out, "%s\n", p->name);
    } ARRAY_END_FOREACH;

    cass_printf(out, "\n");

    cass_printf(out, SPREFIX "BUILT-IN VECSET DIST CLASS (%d)" SPOSTFIX, cass_vecset_dist_class_reg.len);

    ARRAY_BEGIN_FOREACH(cass_vecset_dist_class_reg, cass_reg_entry_t reg)
    {
	    cass_vecset_dist_class_t *p = reg.p;
	    cass_printf(out, "%s\n", p->name);
    } ARRAY_END_FOREACH;

    cass_printf(out, "\n");

    cass_printf(out, SPREFIX "BUILT-IN VEC DIST CLASS (%d)" SPOSTFIX, cass_vec_dist_class_reg.len);

    ARRAY_BEGIN_FOREACH(cass_vec_dist_class_reg, cass_reg_entry_t reg)
    {
	    cass_vec_dist_class_t *p = reg.p;
	    cass_printf(out, "%s\n", p->name);
    } ARRAY_END_FOREACH;

    cass_printf(out, SPREFIX "TABLE(%d)" SPOSTFIX, env->table.len);

    ARRAY_BEGIN_FOREACH(env->table, cass_reg_entry_t reg)
    {
            ret = cass_table_describe(reg.p, out);
            if (ret != 0) goto err;
	    cass_printf(out, "\n");
    } ARRAY_END_FOREACH;

    cass_printf(out, SPREFIX "CFG(%d)" SPOSTFIX, env->cfg.len);
    /* cfg */

    ARRAY_BEGIN_FOREACH(env->cfg, cass_reg_entry_t reg)
    {
            ret = cass_vecset_cfg_describe(reg.p, env, out);
            if (ret != 0) goto err;
	    cass_printf(out, "\n");
    } ARRAY_END_FOREACH;
    
    /* cfg */
    cass_printf(out, SPREFIX "MAP(%d)" SPOSTFIX, env->map.len);

    ARRAY_BEGIN_FOREACH(env->map, cass_reg_entry_t reg)
    {
            ret = cass_map_describe(reg.p, out);
            if (ret != 0) goto err;
	    cass_printf(out, "\n");
    } ARRAY_END_FOREACH;
   
    /* vecset_dist */
    cass_printf(out, SPREFIX "VECSET_DIST(%d)" SPOSTFIX, env->vecset_dist.len);

    ARRAY_BEGIN_FOREACH(env->vecset_dist, cass_reg_entry_t reg)
    {
	    cass_vecset_dist_t *dist = reg.p;
	    ret = dist->__class->describe(dist, out);
            if (ret != 0) goto err;
	    cass_printf(out, "\n");
    } ARRAY_END_FOREACH;


    /* vec_dist */
    cass_printf(out, SPREFIX "VEC_DIST(%d)" SPOSTFIX, env->vec_dist.len);

    ARRAY_BEGIN_FOREACH(env->vec_dist, cass_reg_entry_t reg)
    {
	    cass_vec_dist_t *dist = reg.p;
	    ret = dist->__class->describe(dist, out);
            if (ret != 0) goto err;
	    cass_printf(out, "\n");
    } ARRAY_END_FOREACH;
    cass_close(out);

    return 0;
err:
    return ret;
}


int __cass_table_ref (cass_table_t *table)
{
    cass_env_t *env = table->env;

    if (table->cfg_id >= (signed)env->cfg.len) return CASS_ERR_CORRUPTED;
    if (table->cfg_id >= 0)
    {
    	table->cfg = env->cfg.data[table->cfg_id].p;
    	assert(table->cfg != NULL);
    	table->cfg->refcnt++;
    }

    if (table->parent_cfg_id >= (signed)env->cfg.len) return CASS_ERR_CORRUPTED;
    if (table->parent_cfg_id >= 0)
    {
    	table->parent_cfg = env->cfg.data[table->parent_cfg_id].p;
	assert(table->parent_cfg != NULL);
    	table->parent_cfg->refcnt++;
    }

    if (table->map_id >= (signed)env->map.len) return CASS_ERR_CORRUPTED;
    if (table->map_id >= 0)
    {
	table->map = env->map.data[table->map_id].p;
	assert(table->map != NULL);
	table->map->refcnt ++;
    }

    ARRAY_INIT_SIZE(table->children, table->child_ids.len);
    if (table->children.size > 0 && table->children.data == NULL) return CASS_ERR_OUTOFMEM;
    ARRAY_BEGIN_FOREACH(table->child_ids, cass_id_t id)
    {
        cass_table_t *t = (cass_table_t *)env->table.data[id].p;
        assert(t != NULL);
        t->refcnt++;
        ARRAY_APPEND(table->children, t);
    } ARRAY_END_FOREACH;
    return 0;
}

/* decrement the reference count. */
int __cass_table_unref (cass_table_t *table)
{
    if (table->cfg != NULL) cass_vecset_cfg_free(table->cfg);
    if (table->parent_cfg != NULL) cass_vecset_cfg_free(table->parent_cfg);
    if (table->map != NULL) cass_map_free(table->map);

    ARRAY_BEGIN_FOREACH(table->children, cass_table_t *p)
    {
        cass_table_free(p);
    } ARRAY_END_FOREACH;
    ARRAY_CLEANUP(table->children);
    return 0;

}


int cass_vecset_cfg_checkpoint (cass_vecset_cfg_t *cfg, CASS_FILE *out)
{
    cass_write_pchar(cfg->name, out);
    if (cass_write_uint32(&cfg->vecset_type, 1, out) != 1) return CASS_ERR_IO;
    if (cass_write_uint32(&cfg->vec_type, 1, out) != 1) return CASS_ERR_IO;
    if (cass_write_size(&cfg->vec_size, 1, out) != 1) return CASS_ERR_IO;
    if (cass_write_size(&cfg->vec_dim, 1, out) != 1) return CASS_ERR_IO;
    if (cass_write_uint32(&cfg->flag, 1, out) != 1) return CASS_ERR_IO;
//    if (cass_write_int32(&cfg->vec_dist_id, 1, out) != 1) return CASS_ERR_IO;
//    if (cass_write_int32(&cfg->vecset_dist_id, 1, out) != 1) return CASS_ERR_IO;
    return 0;
}


int cass_vecset_cfg_restore (cass_vecset_cfg_t **_cfg, CASS_FILE *out)
{
    cass_vecset_cfg_t *cfg = type_calloc(cass_vecset_cfg_t, 1);
    if (cfg == NULL) return CASS_ERR_OUTOFMEM;
    cfg->refcnt++;
    cfg->name = cass_read_pchar(out);
    if (cfg->name == NULL) return CASS_ERR_IO;
    if (cass_read_uint32(&cfg->vecset_type, 1, out) != 1) return CASS_ERR_IO;
    if (cass_read_uint32(&cfg->vec_type, 1, out) != 1) return CASS_ERR_IO;
    if (cass_read_size(&cfg->vec_size, 1, out) != 1) return CASS_ERR_IO;
    if (cass_read_size(&cfg->vec_dim, 1, out) != 1) return CASS_ERR_IO;
    if (cass_read_uint32(&cfg->flag, 1, out) != 1) return CASS_ERR_IO;
//    if (cass_read_int32(&cfg->vec_dist_id, 1, out) != 1) return CASS_ERR_IO;
//    if (cass_read_int32(&cfg->vecset_dist_id, 1, out) != 1) return CASS_ERR_IO;
    *_cfg = cfg;
    return 0;
}

int cass_vecset_cfg_free (cass_vecset_cfg_t *cfg)
{
    cfg->refcnt--;
    if (cfg->refcnt > 0) return 0;
    free(cfg->name);
    free(cfg);
    return 0;
}

int cass_vecset_cfg_describe (cass_vecset_cfg_t *cfg, cass_env_t *env, CASS_FILE *out)
{
	cass_printf(out, "NAME:\t%s\n", cfg->name);

	cass_printf(out, "VECSET TYPE:\t%s\n", cass_vecset_type_name(cfg->vecset_type));

	cass_printf(out, "VEC TYPE:\t%s\n", cass_vec_type_name(cfg->vec_type));

	cass_printf(out, "VEC SIZE:\t%d\n", cfg->vec_size);
	cass_printf(out, "VEC DIM:\t%d\n", cfg->vec_dim);


	/*
	{
		cass_vecset_dist_t *p = (cass_vecset_dist_t *)cass_reg_get(&env->vecset_dist, cfg->vecset_dist_id);
		cass_printf(out, "VECSET DIST:\t%s\n", p != NULL ? p->name : "(none)");
	}

	{
		cass_vec_dist_t *p = (cass_vec_dist_t *)cass_reg_get(&env->vec_dist, cfg->vec_dist_id);
		assert(p != NULL);
		cass_printf(out, "VEC DIST:\t%s\n", p != NULL ? p->name : "(none)");
	}
	*/

	return 0;
}
