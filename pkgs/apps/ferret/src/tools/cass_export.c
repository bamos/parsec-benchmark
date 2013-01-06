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
	int i, ret;
	if (argc < 4)
	{
		printf("Export a cass table to text file.\n"
				"usage:\n\t%s <path> <table> <file>\n"
				"\t<path> -- base directory.\n", argv[0]);
		return 0;
	}
	cass_init();

	ret = cass_env_open(&env, argv[1], 0);
	if (ret != 0) { printf("ERROR: %s\n", cass_strerror(ret)); return 0; }

	i = cass_reg_lookup(&env->table, argv[2]);
	if (i < 0) fatal("Cannot find table %s.\n", argv[2]);
	
	table = cass_reg_get(&env->table, i);

	cass_table_load(table);
	cass_map_load(table->map);

	ret = cass_table_export_data(table, argv[3]);

	if (ret < 0) fatal("Cannot export data: %s\n", cass_strerror(ret));

	ret = cass_env_close(env, 0);
	if (ret != 0) { printf("ERROR: %s\n", cass_strerror(ret)); return 0; }
	cass_cleanup();

	return 0;
}
