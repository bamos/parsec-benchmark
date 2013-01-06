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
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include "image.h"

#define MAX_PATH	1000

FILE *fout;
char *dirname;
int cnt;
int max_cnt;

char path[MAX_PATH];

int scan_dir (char *, char *head);

int dir_helper (char *dir, char *head)
{
	DIR *pd = NULL;
	struct dirent *ent = NULL;
	int result = 0;
	pd = opendir(dir);
	if (pd == NULL) goto except;
	for (;;)
	{
		ent = readdir(pd);
		if (ent == NULL) break;
		if (scan_dir(ent->d_name, head) != 0) return -1;
		if ((max_cnt > 0) && (cnt >= max_cnt)) break;
	}
	goto final;

except:
	result = -1;
	perror("Error:");
final:
	if (pd != NULL) closedir(pd);
	return result;
}

/* the whole path to the file */
int file_helper (const char *file)
{
	cass_vec_t *vec;
	cass_size_t i, j;
	cass_dataset_t ds;
	image_extract(file, &ds);

	fprintf(fout, "%s\t%d\n", file, ds.num_vec);
	vec = ds.vec;
	for (i = 0; i < ds.num_vec; i++)
	{
		fprintf(fout, "%g", vec->weight);
		for (j = 0; j < ds.vec_dim; j++)
		{
			fprintf(fout, "\t%g", vec->u.float_data[j]);
		}
		fprintf(fout, "\n");
		vec = (void *)vec + ds.vec_size;
	}
	cass_dataset_release(&ds);
	cnt++;
	return 0;
}

int scan_dir (char *dir, char *head)
{
	struct stat st;
	int ret;
	/* test for . and .. */
	if ((max_cnt > 0) && (cnt >= max_cnt)) return -1;
	if (dir[0] == '.')
	{
		if (dir[1] == 0) return 0;
		else if (dir[1] == '.')
		{
			if (dir[2] == 0) return 0;
		}
	}

	/* append the name to the path */
	strcat(head, dir);
	ret = stat(path, &st);
	if (ret != 0)
	{
		perror("Error:");
		return -1;
	}
	if (S_ISREG(st.st_mode)) file_helper(path);
	else if (S_ISDIR(st.st_mode))
	{
		strcat(head, "/");
		dir_helper(path, head + strlen(head));
	}
	/* removed the appended part */
	head[0] = 0;
	return 0;
}

int main (int argc, char *argv[])
{
	if (argc < 3)
	{
		fprintf(stderr, "usage:\n\t%s <output> <dir> [cnt]\n", argv[0]);
		fprintf(stderr, "\nIf <dir> == \".\", it's not prefixed to the output id.\n");
		return 0;
	}

	fout = fopen(argv[1], "w");
	assert(fout != NULL);
	dirname = argv[2];

	max_cnt = 0;
	if (argc > 3) max_cnt = atoi(argv[3]);


	image_init(argv[0]);

	cnt = 0;
	path[0] = 0;

	if (strcmp(dirname, ".") == 0)
	{
		dir_helper(".", path);
	}
	else
	{
		scan_dir(dirname, path);
	}

	fclose(fout);

	image_cleanup();

	return 0;
}

