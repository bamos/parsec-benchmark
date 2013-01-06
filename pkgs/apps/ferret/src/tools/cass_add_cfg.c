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

int main (int argc, char *argv[])
{
	cass_vecset_cfg_t *cfg;
	cass_env_t *env;
	cass_vecset_dist_t *vecset_dist;
	cass_vec_dist_t *vec_dist;
	int i, ret;
	int dim;
	if (argc != 6)
	{
		printf("Add a table scheme.\n"
				"usage:\n\t%s <path> <cfg> <vecset_type> <vec_type> <dim>\n"
				"\t<path> -- base directory.\n"
				"\t<cfg> -- scheme name.\n"
				"\t<vecset_type> -- 1: single, 2: set.\n"
				"\t<vec_type> -- int | float | vit.\n"
				"\t<dim> -- dimension of feature vector.\n"
				, argv[0]);
		return 0;
	}

	cfg = type_calloc(cass_vecset_cfg_t, 1);
	cfg->refcnt++;
	cfg->name = strdup(argv[2]);

	i = atoi(argv[3]);
	if (i == 1) cfg->vecset_type = CASS_VECSET_SINGLE;
	else if (i == 2) cfg->vecset_type = CASS_VECSET_SET;
	else fatal("Invalid vecset type.\n");

	dim = atoi(argv[5]);
	
	cfg->vec_dim = dim;

	if (strcmp(argv[4], "int") == 0) { cfg->vec_type = CASS_VEC_INT; cfg->vec_size = CASS_VEC_HEAD_SIZE + dim * sizeof(int32_t); }
	else if (strcmp(argv[4], "float") == 0) { cfg->vec_type = CASS_VEC_FLOAT; cfg->vec_size = CASS_VEC_HEAD_SIZE + dim * sizeof(float); }
	else if (strcmp(argv[4], "bit") == 0) { cfg->vec_type = CASS_VEC_BIT; cfg->vec_size = CASS_VEC_HEAD_SIZE + (dim + CHUNK_BIT -1) / CHUNK_BIT * sizeof(chunk_t); }
	else fatal("Invalid vec type.\n");

	cfg->flag = 0;

	cass_init();
	ret = cass_env_open(&env, argv[1], 0);
	if (ret != 0) { printf("ERROR: %s\n", cass_strerror(ret)); return 0; }

	/*
	i = cass_reg_lookup(&env->vecset_dist, argv[6]);
	if (i < 0) fatal("Invalid vecset dist.\n");
	cfg->vecset_dist_id = i;

	vecset_dist = cass_reg_get(&env->vecset_dist, i);

	if ((vecset_dist->class->vecset_type & cfg->vecset_type) == 0)
	fatal("Vecset distance & vecset type do not match.\n");


	i = cass_reg_lookup(&env->vec_dist, argv[7]);
	if (i < 0) fatal("Invalid vec dist.\n");
	cfg->vec_dist_id = i;

	vec_dist = cass_reg_get(&env->vec_dist, i);

	if ((vec_dist->class->vec_type & cfg->vec_type) == 0)
	fatal("Vec distance & vecset type do not match.\n");
	*/

	cass_reg_add(&env->cfg, cfg->name, cfg);

	ret = cass_env_checkpoint(env);
	if (ret != 0) { printf("ERROR: %s\n", cass_strerror(ret)); return 0; }
	ret = cass_env_close(env, 0);
	if (ret != 0) { printf("ERROR: %s\n", cass_strerror(ret)); return 0; }
	cass_cleanup();

	return 0;
}
