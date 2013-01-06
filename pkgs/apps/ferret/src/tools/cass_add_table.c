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
	cass_env_t *env;
	cass_table_t *table;
	int opr_id, cfg_id, map_id;
	int ret;

	if (argc != 5)
	{
		printf("Add a table.\n"
		       "usage:\n\t%s <path> <name> <cfg>\n"
		       "\t<path> -- base directory.\n"
		       "\t<name> -- table name.\n"
		       "\t<cfg> -- scheme name.\n"
		       "\t<map> -- map name.\n"
		       , argv[0]);
		return 0;
	}

	cass_init();
	ret = cass_env_open(&env, argv[1], 0);
	if (ret != 0) { printf("ERROR: %s\n", cass_strerror(ret)); return 0; }


	cfg_id = cass_reg_lookup(&env->cfg, argv[3]);
	if (cfg_id < 0) fatal("Invalid cfg.\n");

	opr_id = cass_table_opr_lookup("raw");
	assert(opr_id >= 0);

	map_id = cass_reg_lookup(&env->map, argv[4]);
	if (map_id < 0) fatal("Invalid map.\n");
	
	ret = cass_table_create(&table, env, argv[2], opr_id, cfg_id, -1, -1, map_id, "");

	if (ret != 0) fatal("Fail to create table: %s.\n", cass_strerror(ret));

	cass_reg_add(&env->table, table->name, table);

	ret = cass_env_checkpoint(env);
	if (ret != 0) { printf("ERROR: %s\n", cass_strerror(ret)); return 0; }
	ret = cass_env_close(env, 0);
	if (ret != 0) { printf("ERROR: %s\n", cass_strerror(ret)); return 0; }
	cass_cleanup();

	return 0;
}
