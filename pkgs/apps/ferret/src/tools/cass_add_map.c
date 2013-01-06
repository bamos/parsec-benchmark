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

void
usage (char *name)
{
    printf("Add a mapping.\n"
	   "usage:\n\t%s <path> <map> <param>\n"
	   "\t<path> -- base directory.\n"
	   "\t<map> -- map name.\n"
	   "\t<param> -- direct | indirect\n"
	   , name);
}

int main (int argc, char *argv[])
{
    cass_map_t *map;
    cass_env_t *env;
    uint32_t flag;
    char *name;
    int ret, ix;
    
    if (argc != 4)
    {
	usage(argv[0]);
	return -1;
    }

    name = strdup(argv[2]);
    flag = 0;

    if (strcmp(argv[3], "direct") == 0) {
	flag = CASS_MAPPING;
    } else if (strcmp(argv[3], "indirect") != 0) {
	usage(argv[0]);
	fatal("Invalid param\n");
    }

    cass_init();
    ret = cass_env_open(&env, argv[1], 0);
    if (ret != 0) { printf("ERROR: %s\n", cass_strerror(ret)); return 0; }
    
    ix = cass_reg_lookup(&env->map, name);
    if (ix >= 0) {
	fatal("Mapping already exist in environment.\n");
    }

    ret = cass_map_create(&map, env, name, flag);
    if (ret != 0) { printf("ERROR: %s\n", cass_strerror(ret)); return 0; }

    cass_reg_add(&env->map, map->name, map);

    ret = cass_env_checkpoint(env);
    if (ret != 0) { printf("ERROR: %s\n", cass_strerror(ret)); return 0; }
    ret = cass_env_close(env, 0);
    if (ret != 0) { printf("ERROR: %s\n", cass_strerror(ret)); return 0; }
    cass_cleanup();
    
    return 0;
}
