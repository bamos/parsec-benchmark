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
#include <cass_hash.h>
#include <cass_file.h>

int ohash_init_with_file (ohash_t *ohash, const char *filename)
{
	CASS_FILE *fin;
	fin = cass_open(filename, "rb");
	if (fin == NULL) return -1;
	ohash_init_with_stream(ohash, fin);
	fclose(fin);
	return 0;
}

int ohash_init_with_stream (ohash_t *ohash, CASS_FILE *fin)
{
	cass_size_t size, bsize;
	int i, ret;
	size = bsize = 0;
	ret = cass_read_size(&size, 1, fin);
	assert(ret == 1);
	ohash->size = size;
	ohash->bucket = malloc(size * sizeof(*ohash->bucket));
	for (i = 0; i < size; i++)
	{
		void *data;
		cass_size_t len;
		bsize = 0;
		ret = cass_read_size(&bsize, 1, fin);
		assert(ret == 1);
		ARRAY_INIT_SIZE(ohash->bucket[i], bsize);
		if (bsize == 0) continue;
		ARRAY_BEGIN_WRITE_RAW(ohash->bucket[i], data, len);
		ret = cass_read_int32(data, bsize, fin);
		assert(ret == bsize);
		ARRAY_END_WRITE_RAW(ohash->bucket[i], bsize);
	}
	return 0;
}

int ohash_dump_file (ohash_t *ohash, const char *filename)
{
	CASS_FILE *fout;
	fout = cass_open(filename, "wb");
	if (fout == NULL) return -1;
	ohash_dump_stream(ohash, fout);
	fclose(fout);
	return 0;
}


int ohash_dump_stream (ohash_t *ohash, CASS_FILE *fout)
{
	int i, ret;
	ret = cass_write_size(&ohash->size, 1, fout);
	assert(ret == 1);
	for (i = 0; i < ohash->size; i++)
	{
		void *data;
		cass_size_t len;
		ARRAY_BEGIN_READ_RAW(ohash->bucket[i], data, len);
		ret = cass_write_size(&len, 1, fout);
		assert(ret == 1);
		ret = cass_write_int32(data, len, fout);
		assert(ret == len);
		ARRAY_END_READ_RAW(ohash->bucket[i]);
	}
	return 0;
}

int ohash_init_with_txt (ohash_t *ohash, const char *filename)
{
	FILE *fin;
	fin = fopen(filename, "r");
	if (fin == NULL) return -1;
	ohash_init_with_txt_stream(ohash, fin);
	fclose(fin);
	return 0;
}

int ohash_init_with_txt_stream (ohash_t *ohash, FILE *fin)
{
	cass_size_t size, bsize;
	int i, j, k, ret;
	size = 0;
	bsize = 0;
	ret = fscanf(fin, "%u", &size);
	assert(ret == 1);
	ohash->size = size;
	ohash->bucket = malloc(size * sizeof(typeof(*ohash->bucket)));
	for (i = 0; i < size; i++)
	{
		int *data;
		cass_size_t len;
		ret = fscanf(fin, "%u", &bsize);
		assert(ret == 1);
		ARRAY_INIT_SIZE(ohash->bucket[i], bsize);
		ARRAY_BEGIN_WRITE_RAW(ohash->bucket[i], data, len);
		for (j = 0; j < bsize; j++)
		{
			ret = fscanf(fin, "%d", &k);
			assert(ret == 2);
			data[j] = k;
		}
		ARRAY_END_WRITE_RAW(ohash->bucket[i], bsize);
	}
	return 0;
}

int ohash_dump_txt (ohash_t *ohash, const char *filename)
{
	FILE *fout;
	fout = fopen(filename, "w");
	if (fout == NULL) return -1;
	ohash_dump_txt_stream(ohash, fout);
	fclose(fout);
	return 0;
}

int ohash_dump_txt_stream (ohash_t *ohash, FILE *fout)
{
	int i, j, ret;
	ret = fprintf(fout, "%u\n", ohash->size);
	assert(ret == 1);
	for (i = 0; i < ohash->size; i++)
	{
		int *data;
		cass_size_t len;
		ARRAY_BEGIN_READ_RAW(ohash->bucket[i], data, len);
		ret = fprintf(fout, "%u", len);
		for (j = 0; j < len; j++)
		{
			ret = fprintf(fout, "\t%d", data[j]);
		}
		fprintf(fout, "\n");
		ARRAY_END_READ_RAW(ohash->bucket[i]);
	}
	return 0;
}

void ohash_stat (ohash_t *ohash)
{
	int i;
	for (i = 0; i < ohash->size; i++)
	{
		printf("%d\t", ARRAY_LEN(ohash->bucket[i]));
	}
	printf("\n");
}

