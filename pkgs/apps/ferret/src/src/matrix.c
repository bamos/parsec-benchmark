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
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <cass.h>

//#define barrier() __asm__ __volatile__("": : :"memory")
#define barrier()


void  __matrix_check (void **matrix)
{
	char data;
	cass_size_t width, i, j;
	assert(matrix[1] != NULL);
	width = matrix[1] - matrix[0];
	i = 2;
	while (matrix[i] != NULL)
	{
		assert(matrix[i] - matrix[i-1] == width);
		i++;
	}
	i--;
	for (j = 0; j < width; j++)
	{
		data = ((char *)matrix[i])[j];
		barrier();
		((char *)matrix[i])[j] = data;
	}
}

cass_size_t __matrix_col (void **data, cass_size_t size)
{
	assert(data[1] != NULL);
	return (data[1] - data[0]) / size;
}

cass_size_t __matrix_row (void **data)
{
	cass_size_t i = 0;
	while (data[i] != NULL) i++;
	return i;
}

void **__array2matrix (cass_size_t row, cass_size_t col, cass_size_t size, void *data)
{
	cass_size_t i;
	void **idx = (void **)malloc((row + 1) * sizeof (void *));
	assert(idx != NULL);
	idx[0] = data;
	assert(idx[0] != NULL);
	for (i = 1; i < row; i++)
	{
		idx[i] = idx[i-1] + col * size;
	}
	idx[row] = NULL;
	return idx;
}

void *__matrix2array (void **data)
{
	void *ret = data[0];
	free(data);
	return ret;
}

void **__matrix_alloc (cass_size_t row, cass_size_t col, cass_size_t size)
{
	cass_size_t i;
	void **idx = (void **)malloc((row + 1)* sizeof (void *));
	assert(idx != NULL);
	idx[0] = calloc(row * col, size);
	assert(idx[0] != NULL);
	for (i = 1; i <= row; i++)
	{
		idx[i] = idx[i-1] + col * size;
	}
	assert(idx[row] - idx[0] == row * col * size);
	idx[row] = NULL;
	return idx;
}

void __matrix_free (void **data)
{
	free(data[0]);
	free(data);
}


int matrix_load_stream (FILE *fin, cass_size_t *size, cass_size_t *row, cass_size_t *col, void ***matrix)
{
	void **_matrix;
	int ret;
	cass_size_t nmemb;
	*row = 0;
	*col = 0;
	*size = 0;
	ret = cass_read_size(size, 1, fin);
	ret += cass_read_size(row, 1, fin);
	ret += cass_read_size(col, 1, fin);
	assert(ret == 3);
	_matrix = __matrix_alloc(*row, *col, *size);
	assert(_matrix != NULL);
	nmemb = (*row) * (*col);
	ret = cass_read(_matrix[0], *size, nmemb, fin);
	assert(ret == nmemb);
	*matrix = _matrix;
	return 0;
}


int matrix_dump_stream (FILE *fout, cass_size_t size, cass_size_t row, cass_size_t col, void **matrix)
{
	int ret;
	cass_size_t nmemb;
	ret = cass_write_size(&size, 1, fout);
	ret += cass_write_size(&row, 1, fout);
	ret += cass_write_size(&col, 1, fout);
	assert(ret == 3);
	nmemb = row * col;
	ret = cass_write(matrix[0], size, nmemb, fout);
	assert(ret == nmemb);
	return 0;
}

int matrix_load_file (const char *filename, cass_size_t *size, cass_size_t *row, cass_size_t *col, void ***matrix)
{
	FILE *fin = fopen(filename, "r");
	assert(fin != NULL);
	matrix_load_stream(fin, size, row, col, matrix);
	fclose(fin);
	return 0;
}

int matrix_dump_file (const char *filename, cass_size_t size, cass_size_t row, cass_size_t col, void **matrix)
{
	FILE *fout = fopen(filename, "w");
	assert(fout != NULL);
	matrix_dump_stream(fout, size, row, col, matrix);
	fclose(fout);
	return 0;
}

int matrix_load_file_head (const char *filename, cass_size_t *size, cass_size_t *row, cass_size_t *col)
{
	FILE *fin = fopen(filename, "rb");
	int ret;
	assert(fin != NULL);
	*row = *col = *size = 0;
	ret = cass_read_size(size, 1, fin);
	ret += cass_read_size(row, 1, fin);
	ret += cass_read_size(col, 1, fin);
	assert(ret == 3);
	fclose(fin);
	return 0;
}

void **__matrix_dup (cass_size_t row, cass_size_t col, cass_size_t size, void **data)
{
	void **ret = __matrix_alloc(row, col, size);
	memcpy(ret[0], data[0], row * col * size);
	return ret;
}

void ***__matrix3_alloc (cass_size_t num, cass_size_t row, cass_size_t col, cass_size_t size)
{
	void ***ret;
	int i;
	ret = (void ***)calloc(num+1, sizeof(void **));
	for (i = 0; i < num; i++)
	{
		ret[i] = __matrix_alloc(row, col, size);
		assert(ret[i] != NULL);
	}
	ret[num] = NULL;
	return ret;
}

void __matrix3_free (void ***data)
{
	int i = 0;
	for (;;)
	{
		if (data[i] == NULL) break;
		__matrix_free(data[i]);
		i++;
	}
	free(data);
}

