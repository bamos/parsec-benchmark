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
/* the raw data table */

#include <cass.h>


static int raw_init_private(cass_table_t *table, const char *param)
{
	int ret;
	struct raw_private *priv = type_calloc(struct raw_private, 1);
	if (priv == NULL) return CASS_ERR_OUTOFMEM;
	ret = cass_dataset_init(&priv->dataset, table->cfg->vec_size, table->cfg->vec_dim, CASS_DATASET_BOTH);
	if (ret != 0) { free(priv); priv = NULL; }
	table->__private = priv;
	return ret;
}

int raw_dump(cass_table_t *table)
{
	int ret;
	FILE *fout;
	struct raw_private *priv = (struct raw_private *)table->__private;
	assert(table->loaded);
	fout = fopen(table->filename, "w");
	if (fout == NULL) return CASS_ERR_IO;
	ret = cass_dataset_dump(&priv->dataset, fout);
	fclose(fout);
	if (ret == 0) table->dirty = 0;
	return ret;
}

static int raw_checkpoint_private(cass_table_t *table, CASS_FILE *out)
{
	int ret;
	struct raw_private *priv = (struct raw_private *)table->__private;
	if (table->loaded && table->dirty)
	{
		ret = raw_dump(table);
		if (ret != 0) return ret;
	}
	return cass_dataset_checkpoint(&priv->dataset, out);
}

static int raw_restore_private(cass_table_t *table, CASS_FILE *in)
{
	int ret;
	struct raw_private *priv = type_calloc(struct raw_private, 1);
	if (priv == NULL) return CASS_ERR_OUTOFMEM;
	ret = cass_dataset_restore(&priv->dataset, in);
	if (ret != 0)
	{
		free(priv);
		priv = NULL;
	}
	table->__private = priv;
	return ret;
}

int raw_load(cass_table_t *table)
{
	int err;
	FILE *fin;
	struct raw_private *priv = (struct raw_private *)table->__private;
	assert(!table->loaded);

	if (priv->dataset.num_vecset == 0)
	{
		priv->dataset.loaded = 1;
		return 0;
	}

	fin = fopen(table->filename, "r");
	if (fin == NULL) return CASS_ERR_IO;
	err = cass_dataset_load(&priv->dataset, fin, table->cfg->vec_type);
	fclose(fin);
	return err;
}

int raw_release(cass_table_t *table)
{
	int err = 0;
	struct raw_private *priv = (struct raw_private *)table->__private;
	if (table->loaded && table->dirty) err = raw_dump(table);
	if (err != 0) return err;
	return cass_dataset_release(&priv->dataset);
}

static int raw_batch_insert(cass_table_t *table, cass_dataset_t * parent, cass_vecset_id_t start, cass_vecset_id_t end)
{
	struct raw_private *priv = (struct raw_private *)table->__private;
	return cass_dataset_merge(&priv->dataset, parent, start, end - start + 1, NULL, NULL);
}

#define MAX_PROB	100
static int raw_query(cass_table_t *table, cass_query_t *query, cass_result_t *result)
{
	struct raw_private *priv = (struct raw_private *)table->__private;
	cass_dataset_t *ds = &priv->dataset;
	cass_vec_dist_t *vec_dist;
	cass_vecset_dist_t *vecset_dist;
	int r_threshold = param_get_int(query->extra_params, "-R", MAX_PROB);

	cass_size_t orig_size;
	cass_id_t i;

	assert((query->flags & CASS_RESULT_BITMAPS) == 0);
	assert((query->flags & CASS_RESULT_LISTS) == 0);

	vec_dist = cass_reg_get(&table->env->vec_dist, query->vec_dist_id);
	assert(vec_dist != NULL);
	vecset_dist = cass_reg_get(&table->env->vecset_dist, query->vecset_dist_id);
	assert(vecset_dist != NULL);

	result->flags = CASS_RESULT_LIST | CASS_RESULT_DIST;

	if (query->flags & CASS_RESULT_USERMEM)
	{
		if ((query->flags & CASS_RESULT_BITMAP) ||
		    ((query->flags & CASS_RESULT_LIST) == 0)) debug("User allocated bitmap not supported.\n");
		orig_size = result->u.list.size;
		if (query->topk > result->u.list.size)
		{
			ARRAY_EXPAND(result->u.list, query->topk);
		}
	}
	else
	{
		orig_size = 0;
		if (query->topk > 0)
		{
			ARRAY_INIT_SIZE(result->u.list, query->topk);
		}
		else
		{
			ARRAY_INIT(result->u.list);
		}
	}


	if (query->topk > 0)
	{
		assert(result->u.list.size >= query->topk);
		/* set the result array length */
		result->u.list.len = query->topk;
		TOPK_INIT(result->u.list.data, dist, query->topk, CASS_DIST_MAX);
		if (query->candidate != NULL)
		{
			if (query->candidate->flags & CASS_RESULT_LIST)
			{
				ARRAY_BEGIN_FOREACH(query->candidate->u.list, cass_list_entry_t cand)
				{
					cass_list_entry_t entry;
					if (cand.id == CASS_ID_MAX) continue;
					entry.id = cand.id;
					entry.dist = vecset_dist->__class->dist(ds, entry.id, query->dataset, query->vecset_id, vec_dist, vecset_dist);
					TOPK_INSERT_MIN(result->u.list.data, dist, query->topk, entry);

				} ARRAY_END_FOREACH;
			}
			else
			if (query->candidate->flags & CASS_RESULT_BITMAP)
			{
				uint32_t id = bitmap_get_size(&query->candidate->u.bitmap);
				for (i = 0; i < bitmap_get_count(&query->candidate->u.bitmap); i++)
				{
					id = bitmap_getNext(&query->candidate->u.bitmap, id);

					cass_list_entry_t entry;
					entry.id = id;
					entry.dist = vecset_dist->__class->dist(ds, entry.id, query->dataset, query->vecset_id, vec_dist, vecset_dist);
					TOPK_INSERT_MIN(result->u.list.data, dist, query->topk, entry);
				}
			}
			else assert(0);
		}
		else
		{
			for (i = 0; i < ds->num_vecset; i++)
			{
				if (rand() % MAX_PROB > r_threshold) continue;
				cass_list_entry_t entry;
				entry.dist = vecset_dist->__class->dist(ds, i, query->dataset, query->vecset_id, vec_dist, vecset_dist);
				entry.id = i;
				TOPK_INSERT_MIN(result->u.list.data, dist, query->topk, entry);
			}
		}
		if (query->flags & CASS_RESULT_SORT)
		{
			TOPK_SORT_MIN(result->u.list.data, cass_list_entry_t, dist, query->topk);
			result->flags |= CASS_RESULT_SORT;
		}
	}
	else
	{
		ARRAY_TRUNC(result->u.list);
		if (query->candidate != NULL)
		{
			if (query->candidate->flags & CASS_RESULT_LIST)
			for (i = 0; i < query->candidate->u.list.len; i++)
			{
				cass_list_entry_t entry;
				entry.id = query->candidate->u.list.data[i].id;
				if (entry.id == CASS_ID_MAX) continue;
				entry.dist = vecset_dist->__class->dist(ds, entry.id, query->dataset, query->vecset_id, vec_dist, vecset_dist);
				if (entry.dist < query->range)
				{
					ARRAY_APPEND(result->u.list, entry);
				}

			}
			else if (query->candidate->flags & CASS_RESULT_BITMAP)
			{
				uint32_t id = bitmap_get_size(&query->candidate->u.bitmap);
				for (i = 0; i < bitmap_get_count(&query->candidate->u.bitmap); i++)
				{
					id = bitmap_getNext(&query->candidate->u.bitmap, id);

					cass_list_entry_t entry;
					entry.id = id;
					entry.dist = vecset_dist->__class->dist(ds, entry.id, query->dataset, query->vecset_id, vec_dist, vecset_dist);
					if (entry.dist < query->range)
					{
						ARRAY_APPEND(result->u.list, entry);
					}
				}
			}
		}
		else
		{
			for (i = 0; i < ds->num_vecset; i++)
			{
				if (rand() % MAX_PROB > r_threshold) continue;
				cass_list_entry_t entry;
				entry.dist = vecset_dist->__class->dist(ds, i, query->dataset, query->vecset_id, vec_dist, vecset_dist);
				entry.id = i;
				if (entry.dist < query->range)
				{
					ARRAY_APPEND(result->u.list, entry);
				}
			}
		}
	}

	if (orig_size == 0) result->flags |= CASS_RESULT_MALLOC;
	else if (result->u.list.size > orig_size) result->flags |= CASS_RESULT_REALLOC;

	return 0;
}

/* this function is used for multiple oprs, and is thus not static */
/*
int raw_drop(cass_table_t *table)
{
	char buf[BUFSIZ];
	mkpath(buf, table->env->base_dir, table->name, table->opr->name);
	return unlink(buf);
}
*/

static int raw_free_private(cass_table_t *table)
{
	if (table->loaded) raw_release(table);
	free(table->__private);
	return 0;
}

static char *raw_tune(cass_table_t *table, char *extra_input)
{
//	struct raw_private *priv = (struct raw_private *)table->__private;
	return NULL;
}


cass_table_opr_t opr_raw = {
	.name = "raw",
	.type = CASS_DATA | CASS_SEQUENTIAL,
	.vecset_type = CASS_VECSET_SET,
	.vec_type = CASS_ANY,
	.dist_vecset = CASS_ANY,
	.dist_vec = CASS_ANY,
	
	.tune = raw_tune,
	.init_private = raw_init_private,
	.batch_insert = raw_batch_insert,
	.query = raw_query,
	.batch_query = NULL,
	.load = raw_load,
	.release = raw_release,
	.checkpoint_private = raw_checkpoint_private,
	.restore_private = raw_restore_private,
//	.drop = raw_drop,
	.free_private = raw_free_private
};


