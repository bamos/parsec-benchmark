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
	cass_vec_dist_class_t *class;
	cass_vec_dist_t *dist;
	cass_env_t *env;
	int ret, i;

	if (argc != 5)
	{
		printf("Add a vec distance.\n"
				"usage:\n\t%s <path> <name> <class> <param>\n"
				"\t<path> -- base directory.\n"
				"\t<name> -- distance name.\n"
				"\t<class>\n"
				"\t<param>\n"
				, argv[0]);
		return 0;
	}

	cass_init();
	ret = cass_env_open(&env, argv[1], 0);
	if (ret != 0) { printf("ERROR: %s\n", cass_strerror(ret)); return 0; }


	i = cass_vec_dist_class_lookup(argv[3]);
	if (i < 0) fatal("Invalid vec dist class.\n");
	class = cass_vec_dist_class_get(i);

	
	ret = class->construct((void **)&dist, argv[4]);
	if (ret != 0) fatal("Error constructing cass_vec_dist.\n");

	dist->name = strdup(argv[2]);
	assert(dist->name != NULL);

	cass_reg_add(&env->vec_dist, dist->name, dist);

	ret = cass_env_checkpoint(env);
	if (ret != 0) { printf("ERROR: %s\n", cass_strerror(ret)); return 0; }
	ret = cass_env_close(env, 0);
	if (ret != 0) { printf("ERROR: %s\n", cass_strerror(ret)); return 0; }
	cass_cleanup();

	return 0;
}
