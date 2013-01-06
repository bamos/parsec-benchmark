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
#include <values.h>
#include "chunk_cnt.inc"

#include "emd.h"


cass_dist_t dist_trivial_dist (cass_size_t n, void *p1, void *p2, void *p3)
{
	return 0.0;
}

#define DIST_SIMPLE_METHODS(type,_class)	\
static int dist_##type##_construct (void **_dist, const char *param) \
{ \
	cass_vec_dist_t *dist = type_calloc(cass_vec_dist_t, 1); \
	dist->refcnt++; \
	dist->__class = &_class; \
	*_dist = dist; \
	return 0; \
} \
\
static int dist_##type##_restore (void **_dist, CASS_FILE *fin) \
{ \
	cass_vec_dist_t *dist; \
	dist_##type##_construct(_dist, NULL); \
	dist = *_dist; \
	dist->name = cass_read_pchar(fin); \
	if (dist->name == NULL) \
	{ \
		free(dist); \
		return CASS_ERR_IO; \
	} \
	return 0; \
}

static int dist_simple_describe (void *_dist, CASS_FILE *fout)
{ 
	cass_vec_dist_t *dist = _dist; \
	cass_printf(fout, "NAME:\t%s\n", dist->name);
	cass_printf(fout, "CLASS:\t%s\n", dist->__class->name);
	return 0;
}

static int dist_simple_checkpoint (void *_dist, CASS_FILE *fout)
{
	cass_vec_dist_t *dist = _dist;
	return cass_write_pchar(dist->name, fout);
}


static void dist_simple_free (void *_dist)
{
	cass_vec_dist_t *dist = _dist;
	dist->refcnt--;
	if (dist->refcnt == 0)
	{
		if (dist->name != NULL) free(dist->name);
		free(dist);
	}
}

DIST_SIMPLE_METHODS(trivial, vec_dist_trivial)

cass_vec_dist_class_t vec_dist_trivial =
{
	.name = "trivial",
	.vec_type = CASS_ANY,
	.type = CASS_VEC_DIST_TYPE_TRIVIAL,
	.dist = dist_trivial_dist,
	.describe = dist_simple_describe,
	.construct = dist_trivial_construct,
	.checkpoint = dist_simple_checkpoint,
	.restore = dist_trivial_restore,
	.free = dist_simple_free,
};


DIST_SIMPLE_METHODS(L1_int, vec_dist_L1_int)

cass_dist_t __dist_L1_int32 (cass_size_t D, const int32_t *v1, const int32_t *v2)
{
	return (cass_dist_t) dist_L1_int32_t(D, v1, v2);
}

cass_vec_dist_class_t vec_dist_L1_int =
{
	.name = "L1_int",
	.vec_type = CASS_VEC_INT,
	.type = CASS_VEC_DIST_TYPE_L1,
	.dist = __dist_L1_int32,
	.describe = dist_simple_describe,
	.construct = dist_L1_int_construct,
	.checkpoint = dist_simple_checkpoint,
	.restore = dist_L1_int_restore,
	.free = dist_simple_free,
};

DIST_SIMPLE_METHODS(L2_int, vec_dist_L2_int)

cass_dist_t __dist_L2_int32 (cass_size_t D, const int32_t *v1, const int32_t *v2)
{
	return (cass_dist_t) dist_L2_int32_t(D, v1, v2);
}

cass_vec_dist_class_t vec_dist_L2_int =
{
	.name = "L2_int",
	.vec_type = CASS_VEC_INT,
	.type = CASS_VEC_DIST_TYPE_L2,
	.dist = __dist_L2_int32,
	.describe = dist_simple_describe,
	.construct = dist_L2_int_construct,
	.checkpoint = dist_simple_checkpoint,
	.restore = dist_L2_int_restore,
	.free = dist_simple_free,
};

DIST_SIMPLE_METHODS(L1_float, vec_dist_L1_float)

cass_vec_dist_class_t vec_dist_L1_float =
{
	.name = "L1_float",
	.vec_type = CASS_VEC_FLOAT,
	.type = CASS_VEC_DIST_TYPE_L1,
	.dist = dist_L1_float,
	.describe = dist_simple_describe,
	.construct = dist_L1_float_construct,
	.checkpoint = dist_simple_checkpoint,
	.restore = dist_L1_float_restore,
	.free = dist_simple_free,
};

DIST_SIMPLE_METHODS(L2_float, vec_dist_L2_float)

cass_vec_dist_class_t vec_dist_L2_float =
{
	.name = "L2_float",
	.vec_type = CASS_VEC_FLOAT,
	.type = CASS_VEC_DIST_TYPE_L1,
	.dist = dist_L2_float,
	.describe = dist_simple_describe,
	.construct = dist_L2_float_construct,
	.checkpoint = dist_simple_checkpoint,
	.restore = dist_L2_float_restore,
	.free = dist_simple_free,
};

DIST_SIMPLE_METHODS(cos_float, vec_dist_cos_float)

cass_vec_dist_class_t vec_dist_cos_float =
{
	.name = "cosine",
	.vec_type = CASS_VEC_FLOAT,
	.type = CASS_VEC_DIST_TYPE_COS,
	.dist = dist_cos_float,
	.describe = dist_simple_describe,
	.construct = dist_cos_float_construct,
	.checkpoint = dist_simple_checkpoint,
	.restore = dist_cos_float_restore,
	.free = dist_simple_free,
};

DIST_SIMPLE_METHODS(hamming, vec_dist_hamming)

cass_dist_t __dist_hamming (cass_size_t n, const chunk_t *c1, const chunk_t *c2)
{
	return (cass_dist_t) dist_hamming(n, c1, c2);
}

cass_vec_dist_class_t vec_dist_hamming =
{
	.name = "hamming",
	.vec_type = CASS_VEC_BIT,
	.type = CASS_VEC_DIST_TYPE_HAMMING,
	.dist = __dist_hamming,
	.describe = dist_simple_describe,
	.construct = dist_hamming_construct,
	.checkpoint = dist_simple_checkpoint,
	.restore = dist_hamming_restore,
	.free = dist_simple_free,
};


#define SDIST_SIMPLE_METHODS(type,_class)	\
static int sdist_##type##_construct (void **_dist, const char *param) \
{ \
	cass_vecset_dist_t *dist = type_calloc(cass_vecset_dist_t, 1); \
	dist->refcnt++; \
	dist->__class = &_class; \
	*_dist = dist; \
	return 0; \
} \
\
static int sdist_##type##_restore (void **_dist, CASS_FILE *fin) \
{ \
	cass_vecset_dist_t *dist; \
	sdist_##type##_construct(_dist, NULL); \
	dist = *_dist; \
	dist->name = cass_read_pchar(fin); \
	if (dist->name == NULL) \
	{ \
		free(dist); \
		return CASS_ERR_IO; \
	} \
	return 0; \
}

static int sdist_simple_describe (void *_dist, CASS_FILE *fout)
{
	cass_vecset_dist_t *dist = _dist;
	cass_printf(fout, "NAME:\t%s\n", dist->name);
	cass_printf(fout, "CLASS:\ttrivial\n");
	return 0;
}

static int sdist_simple_checkpoint (void *_dist, CASS_FILE *fout)
{
	cass_vecset_dist_t *dist = _dist;
	return cass_write_pchar(dist->name, fout);
}

static void sdist_simple_free (void *_dist)
{
	cass_vecset_dist_t *dist = _dist;
	dist->refcnt--;
	if (dist->refcnt == 0)
	{
		free(dist->name);
		free(dist);
	}
}

SDIST_SIMPLE_METHODS(trivial, vecset_dist_trivial);

cass_dist_t sdist_trivial (cass_dataset_t *ds1, cass_vecset_id_t p1, cass_dataset_t *ds2, cass_vecset_id_t p2, cass_vec_dist_t *vec_dist, void *p)
{
	return 0.0;
}

cass_vecset_dist_class_t vecset_dist_trivial =
{
	.name = "trivial",
	.vecset_type = CASS_ANY,
	.type = CASS_VECSET_DIST_TYPE_TRIVIAL,
	.dist = sdist_trivial,
	.describe = sdist_simple_describe,
	.construct = sdist_trivial_construct,
	.checkpoint = sdist_simple_checkpoint,
	.restore = sdist_trivial_restore,
	.free = sdist_simple_free,
};

SDIST_SIMPLE_METHODS(single, vecset_dist_single);

cass_dist_t sdist_single (cass_dataset_t *ds1, cass_vecset_id_t p1, cass_dataset_t *ds2, cass_vecset_id_t p2, cass_vec_dist_t *vec_dist, void *p)
{
	cass_vec_t *v1, *v2;
	v1 = ds1->vec + ds1->vec_size * ds1->vecset[p1].start_vecid;
	v2 = ds2->vec + ds2->vec_size * ds2->vecset[p2].start_vecid;

	return vec_dist->__class->dist(ds1->vec_dim, v1->u.data, v2->u.data, vec_dist);
}

cass_vecset_dist_class_t vecset_dist_single =
{
	.name = "single",
	.vecset_type = CASS_ANY,
	.type = CASS_VECSET_DIST_TYPE_SINGLE,
	.dist = sdist_single,
	.describe = sdist_simple_describe,
	.construct = sdist_single_construct,
	.checkpoint = sdist_simple_checkpoint,
	.restore = sdist_single_restore,
	.free = sdist_simple_free,
};

SDIST_SIMPLE_METHODS(emd, vecset_dist_emd);

cass_dist_t sdist_emd (cass_dataset_t *ds1, cass_vecset_id_t p1, cass_dataset_t *ds2, cass_vecset_id_t p2, cass_vec_dist_t *vec_dist, void *p)
{
	cass_vecset_t *vecset1;
	cass_vecset_t *vecset2;
	signature_t sig1;
	signature_t sig2;
	cass_vec_t *vec;
	int i;

	assert(ds1->vec_dim == ds2->vec_dim);

	vecset1 = &ds1->vecset[p1];
	vecset2 = &ds2->vecset[p2];

	sig1.n = vecset1->num_regions;
	sig2.n = vecset2->num_regions;

	sig1.Features = alloca(sig1.n * sizeof *sig1.Features);
	sig1.Weights = alloca(sig1.n * sizeof *sig1.Weights);
	sig2.Features = alloca(sig2.n * sizeof *sig2.Features);
	sig2.Weights = alloca(sig2.n * sizeof *sig2.Weights);

	vec = (void *)ds1->vec + ds1->vec_size * vecset1->start_vecid;
	for (i = 0; i < sig1.n; i++)
	{
		sig1.Features[i] = vec->u.float_data;
		sig1.Weights[i] = vec->weight;
		vec = (void *)vec + ds1->vec_size;
	}

	vec = (void *)ds2->vec + ds2->vec_size * vecset2->start_vecid;
	for (i = 0; i < sig2.n; i++)
	{
		sig2.Features[i] = vec->u.float_data;
		sig2.Weights[i] = vec->weight;
		vec = (void *)vec + ds2->vec_size;
	}

	return emd(&sig1, &sig2, vec_dist->__class->dist, ds1->vec_dim, vec_dist, NULL, NULL);
}

cass_vecset_dist_class_t vecset_dist_emd =
{
	.name = "emd",
	.vecset_type = CASS_ANY,
	.type = CASS_VECSET_DIST_TYPE_EMD,
	.dist = sdist_emd,
	.describe = sdist_simple_describe,
	.construct = sdist_emd_construct,
	.checkpoint = sdist_simple_checkpoint,
	.restore = sdist_emd_restore,
	.free = sdist_simple_free,
};

SDIST_SIMPLE_METHODS(myemd, vecset_dist_myemd);

extern float tp_solve (int nrow, float *row, int ncol, float *col, float **cost);

cass_dist_t sdist_myemd (cass_dataset_t *ds1, cass_vecset_id_t p1, cass_dataset_t *ds2, cass_vecset_id_t p2, cass_vec_dist_t *vec_dist, void *p)
{
	cass_vecset_t *vecset1, *vecset2;
	cass_vec_t *vec1, *vec2;
	float *row, *col;
	float **cost;
	float d, srow, scol, ss;
	int nrow, ncol;
	int i, j;

	assert(ds1->vec_dim == ds2->vec_dim);

	vecset1 = &ds1->vecset[p1];
	vecset2 = &ds2->vecset[p2];

	nrow = vecset1->num_regions;
	ncol = vecset2->num_regions;

	row = (float *)alloca((nrow + 1) * sizeof (float));
	col = (float *)alloca((ncol + 1) * sizeof (float));

	srow = scol = 0;

	vec1 = (void *)ds1->vec + ds1->vec_size * vecset1->start_vecid;
	for (i = 0; i < nrow; i++)
	{
		row[i] = vec1->weight;
		srow += row[i];
		vec1 = (void *)vec1 + ds1->vec_size;
	}

	vec2 = (void *)ds2->vec + ds2->vec_size * vecset2->start_vecid;
	for (i = 0; i < ncol; i++)
	{
		col[i] = vec2->weight;
		scol += col[i];
		vec2 = (void *)vec2 + ds2->vec_size;
	}

	cost = type_matrix_alloc(float, nrow + 1, ncol + 1);
	
	vec1 = (void *)ds1->vec + ds1->vec_size * vecset1->start_vecid;
	for (i = 0; i < nrow; i++)
	{
		vec2 = (void *)ds2->vec + ds2->vec_size * vecset2->start_vecid;
		for (j = 0; j < ncol; j++)
		{
			cost[i][j] = vec_dist->__class->dist(ds1->vec_dim, vec1->u.float_data, vec2->u.float_data, vec_dist);
			vec2 = (void *)vec2 + ds2->vec_size;
		}

		vec1 = (void *)vec1 + ds1->vec_size;
	}

#define ABS_ERR	0.0

	if (srow > scol)
	{
		ss = scol;
		col[ncol] = srow - scol;
		if (col[ncol] > ABS_ERR) ncol++;
	}
	else
	{
		ss = srow;
		row[nrow] = scol - srow;
		if (row[nrow] > ABS_ERR) nrow++;
	}

	d = tp_solve(nrow, row, ncol, col, cost);

	matrix_free(cost);

	return d / ss;
}

cass_vecset_dist_class_t vecset_dist_myemd =
{
	.name = "myemd",
	.vecset_type = CASS_ANY,
	.type = CASS_VECSET_DIST_TYPE_EMD,
	.dist = sdist_myemd,
	.describe = sdist_simple_describe,
	.construct = sdist_myemd_construct,
	.checkpoint = sdist_simple_checkpoint,
	.restore = sdist_myemd_restore,
	.free = sdist_simple_free,
};

