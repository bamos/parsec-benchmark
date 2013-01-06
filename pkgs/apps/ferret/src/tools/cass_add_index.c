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

#define BUFSIZE 0x4000

int main (int argc, char *argv[])
{
	char buf[BUFSIZE];

	char *index_name;
	char *index_cfg_name;

	cass_env_t *env;

	int32_t table_id, index_id;
	int32_t opr_id, cfg_id;

	cass_vecset_cfg_t *cfg;
	cass_table_t *table;
	cass_table_t *index;
	cass_dataset_t *ds;
	cass_table_opr_t *opr;

	FILE *fin;
	int ret;

	if (argc < 5)
	{
		printf("Add an index.\n"
				"usage:\n\t%s <path> <table> <index> <params> [name]\n"
				, argv[0]);
		return 0;
	}

	cass_init();

	ret = cass_env_open(&env, argv[1], 0);
	if (ret != 0) { printf("ERROR: %s\n", cass_strerror(ret)); return 0; }

	if (argc > 5)
	{
		sprintf(buf, "%s", argv[5]);
		index_name = strdup(buf);
		sprintf(buf, "%s@cfg", argv[5]);
		index_cfg_name = strdup(buf);
	}
	else
	{
		sprintf(buf, "%s@%s", argv[2], argv[3]);
		index_name = strdup(buf);
		sprintf(buf, "%s@%s@cfg", argv[2], argv[3]);
		index_cfg_name = strdup(buf);
	}


	if (cass_reg_lookup(&env->table, index_name) >= 0) fatal("Sketch already exists.\n");
	if (cass_reg_lookup(&env->cfg, index_cfg_name) >= 0) fatal("Sketch cfg already exists.\n");

	table_id = cass_reg_lookup(&env->table, argv[2]);
	if (table_id < 0) fatal("Table does not exist.\n");

	table = cass_reg_get(&env->table, table_id);

	assert(table->opr->type & CASS_DATA);

	opr_id = cass_table_opr_lookup(argv[3]);
	assert(opr_id >= 0);
	opr = cass_table_opr_get(opr_id);

	if (opr->cfg != NULL)
	{
		cfg = opr->cfg(argv[4]);
		assert(cfg != NULL);

		cfg->name = index_cfg_name;

	/*
	cfg->vecset_dist_id = -1;
	cfg->vec_dist_id = cass_reg_lookup(&env->vec_dist, "hamming");
	assert(cfg->vec_dist_id >= 0);
	*/

		cass_reg_add(&env->cfg, cfg->name, cfg);

		cfg_id = cass_reg_lookup(&env->cfg, cfg->name);
		assert(cfg_id >= 0);
	}
	else
	{
		cfg_id = CASS_ID_INV;
	}


	ret = cass_table_create(&index, env, index_name, opr_id, cfg_id, table_id, table->cfg_id, table->map_id, argv[4]);

	if (ret != 0) fatal("Fail to create index: %s.\n", cass_strerror(ret));

	cass_reg_add(&env->table, index->name, index);

	index_id = cass_reg_lookup(&env->table, index->name);
	assert(index_id >= 0);

	cass_table_associate(table, index_id);

	ds = &((struct raw_private *)table->__private)->dataset;
	if (ds->num_vecset > 0)
	{
		cass_table_load(table);
		cass_table_load(index);
		cass_table_batch_insert(index, ds, 0, ds->num_vecset - 1);
	}

	ret = cass_env_checkpoint(env);
	if (ret != 0) { printf("ERROR: %s\n", cass_strerror(ret)); return 0; }
	ret = cass_env_close(env, 0);
	if (ret != 0) { printf("ERROR: %s\n", cass_strerror(ret)); return 0; }
	cass_cleanup();

	return 0;
}
