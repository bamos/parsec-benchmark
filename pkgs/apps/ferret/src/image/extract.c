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
#include <math.h>
#include "image.h"

#define MAXR	256

#define IMAGE_DIM	14

static float   dw[IMAGE_DIM] =  {6.0, 3.0, 1.5, 4.0, 2.0, 1.0, 4.0, 2.0, 1.0, 0.2, 0.4, 0.04, 0.007, 0.007};


void apply_weight (cass_dataset_t *ds)
{
	uint32_t i, j;
	cass_vec_t *vec = ds->vec;
	for (i = 0; i < ds->num_vec; i++)
	{
		for (j = 0; j < ds->vec_dim; j++)
		{
			vec->u.float_data[j] *= dw[j];
		}
		vec = (void *)vec + ds->vec_size;	
	}
}

box_t	
*box_new(int x, int y)
{ 
	box_t	*box = type_calloc(box_t, 1); 

	box->a1 = x; 
	box->b1 = y; 
	box->a2 = x + 1; 
	box->b2 = y + 1; 
	box->cx = x; 
	box->cy = y; 
	box->p1 = 1; 
	box->p2 = 0; 

	return box; 
} 

void 
box_combine(box_t *box)
{
	if (box->p2) { 
		if (box->a1 < box->u1)
			box->u1 = box->a1;
		if (box->b1 < box->v1)
			box->v1 = box->b1; 
		if (box->a2 > box->u2)
			box->u2 = box->a2; 
		if (box->b2 > box->v2)
			box->v2 = box->b2; 

		box->p2 += box->p1;
	}
	else {
		box->u1 = box->a1;
		box->v1 = box->b1;
		box->u2 = box->a2;
		box->v2 = box->b2; 

		box->p2 = box->p1; 
	}

	box->p1 = 0; 
}

void 
box_insert_pxl(box_t *box, int x, int y, int clear)
{	
	int		sz = (box->a2 - box->a1) * (box->b2 - box->b1);
	float	az = (float)(box->p1) / sz; 

	box->cx += x; 
	box->cy += y; 		

	if (!clear || (az > 0.15)) { 
		if (x < box->a1)
			box->a1 = x; 
		else if (x >= box->a2)
			box->a2 = x + 1; 

		if (y < box->b1)
			box->b1 = y; 
		else if (y >= box->b2)
			box->b2 = y + 1; 

		box->p1++; 
	} 
	else { 
		box_combine(box); 

		box->a1 = x; 
		box->b1 = y;
		box->a2 = x + 1; 
		box->b2 = y + 1; 

		box->p1 = 1; 
	} 
}

void 
box_to_vec(box_t *box, float *vec, int offset)
{
	int		np = box->p1 + box->p2; 
	float	lr = (float)(box->p2) / np; 
	int		rp, dx, dy, sz; 

	np = box->p1 + box->p2; 
	box->cx /= np;
	box->cy /= np;

	if (lr < 0.1) {
		dx = box->a2 - box->a1;
		dy = box->b2 - box->b1; 
		rp = box->p1; 
	}
	else {
		box_combine(box);
		dx = box->u2 - box->u1; 
		dy = box->v2 - box->v1; 
		rp = np; 
	}

	sz = dx * dy; 
	vec[offset] = log((float)dy / dx);  // log of aspect ratio
	vec[offset+1] = (float)rp / sz; 	// area / size 
	vec[offset+2] = log(sz);			// log of box size 
	vec[offset+3] = box->cx;			// region centroid x 
	vec[offset+4] = box->cy; 			// region centroid y
} 

box_set_t *
box_set_new(int hint)
{
	box_set_t	*box_set = type_calloc(box_set_t, 1);

	box_set->boxes = type_calloc(box_t *, hint); 
	box_set->nbox = 0; 
	box_set->size = hint; 

	return box_set; 
}

void
box_set_free(box_set_t **box_set)
{
	int		i; 

	for(i=0; i<(*box_set)->nbox; i++) 
		free((*box_set)->boxes[i]);

	free((*box_set)->boxes); 
	free(*box_set);
	*box_set = NULL; 
}

void 
box_set_insert_pxl(box_set_t *box_set, int r, int x, int y)
{
	int		i, size; 
	box_t	**new_boxes; 

	if (r >= box_set->size) { 
		size = 2 * box_set->size; 
		new_boxes = (box_t **) calloc (size, sizeof(box_t *)); 
		for (i=0; i<box_set->size; i++) 
			new_boxes[i] = box_set->boxes[i]; 
		free(box_set->boxes); 

		box_set->boxes = new_boxes; 
		box_set->size = size; 

	} 

	if (box_set->boxes[r])
		box_insert_pxl(box_set->boxes[r], x, y, 1); 
	else { 
		box_set->boxes[r] = box_new(x, y); 
		box_set->nbox++;
	}
}

int img_map_to_seg(img_map_t *map, cass_dataset_t *ds)
{
	cass_size_t vec_size;
	cass_size_t vec_dim;
	cass_vec_t *vec;
	int		r, c, m, k; 
	float	w; 
	
	vec_dim = IMAGE_DIM;
	vec_size = sizeof(cass_vec_t ) + sizeof(float) * vec_dim;
	cass_dataset_init(ds, vec_size, vec_dim, CASS_DATASET_BOTH);
	cass_dataset_grow(ds, 1, map->nrgn);
	
	w = 0.0;
	for (r=0; r<map->nrgn; r++) {
		//seg->weights[r] = sqrt((float)(map.rgn_sz[r])); 
		w += sqrt((float)(map->rgn_sz[r])); 
	}
	w = 1.0 / w; 

	vec = ds->vec;
	for (r=0; r<map->nrgn; r++) {

		vec->parent = 0;
		vec->weight = sqrt((float)(map->rgn_sz[r])) * w; 

		k = 0; 	
		for (c=0; c<CHAN; c++)
		for (m=0; m<MNTS; m++) {
			vec->u.float_data[k] = map->rgn[c][m][r];
			k++;
		}
		box_to_vec(map->box_set->boxes[r], vec->u.float_data, k);

		/*
		if (use_pca) {
		    // shift to make mean = 0

		    for (ix = 0; ix < IMG_ND; ix++) {
			seg->features[r][ix] -= img_mean[ix];
		    }
		    feature_weight_adjust(seg->features[r], img_dw, IMG_ND);
		    feature_pca_translate(seg->features[r], img_pca_matrix, IMG_ND);
		}
		*/
		vec = (void *)vec + ds->vec_size;
	}

	ds->loaded = 1;
	ds->num_vecset = 1;
	ds->num_vec = map->nrgn;
	ds->vecset[0].num_regions = map->nrgn;
	ds->vecset[0].start_vecid = 0;

	return 0;
}

int image_extract_helper (unsigned char *HSV, unsigned char *mask, int width, int height, int nrgn, cass_dataset_t *ds)
{
	img_map_t map;
	unsigned char *b;
	int i, j, c, m, r;
	float v;

	map.ncol = width;
	map.nrow = height;
	map.nrgn = nrgn;

	map.map = type_array2matrix(unsigned char, map.nrow, map.ncol, mask);

	map.size = map.ncol * map.nrow;
	
	map.rgn_sz = (int *)calloc(map.nrgn, sizeof(int)); 

	/* extract boxes */
	map.box_set = box_set_new(MAXR); 

	for (i=0; i<map.nrow; i++) 
	for (j=0; j<map.ncol; j++) {
		r = map.map[i][j]; 
		box_set_insert_pxl(map.box_set, r, i, j); 
	} 

	/* extract color moments */


	for (c=0; c<CHAN; c++)
	for (m=0; m<MNTS; m++)
		map.rgn[c][m] = (float *)calloc(map.nrgn, sizeof(float)); 

	/* extract color & rgn size */
	b = HSV; 
	for (i=0; i< map.nrow; i++) {
	//	b = a;
		for (j=0; j< map.ncol; j++) {
			r = map.map[i][j]; 
			map.rgn_sz[r]++;
		
			for (c=0; c<CHAN; c++)
				map.rgn[c][0][r] += b[c]; 

			b += CHAN;
		}
	//	a += columns * CHAN; 
	}

	for (c=0; c<CHAN; c++)
	for (r=0; r<map.nrgn; r++) 
		map.rgn[c][0][r] /= map.rgn_sz[r]; 

	b = (uchar *)HSV;//a = (uchar *)HSV; 
	for (i=0; i<map.nrow; i++) { 
		//b = a; 
		for (j=0; j<map.ncol; j++) {
			r = map.map[i][j]; 

			for (c=0; c<CHAN; c++) { 
				v = b[c] - map.rgn[c][0][r]; 
				map.rgn[c][1][r] += v * v; 
				map.rgn[c][2][r] += v * v * v; 
			}
			b += CHAN; 
		}
		//a += columns * CHAN;
	}

	for (c=0; c<CHAN; c++) 
	for (r=0; r<map.nrgn; r++) {
		map.rgn[c][0][r] /= 255.0;
		map.rgn[c][1][r] = sqrt(map.rgn[c][1][r] 
			/ map.rgn_sz[r]) / 255.0;
		map.rgn[c][2][r] = cbrt(map.rgn[c][2][r] 
			/ map.rgn_sz[r]) / 255.0;
	}

	/* fill the dataset */
	img_map_to_seg(&map, ds);

	/* apply the weight */
	apply_weight(ds);


	matrix_free_index(map.map);
	if (map.box_set)
	    box_set_free(&map.box_set); 
	free(map.rgn_sz);
	if (map.rgn) {
	    for (c=0; c<CHAN; c++)
		for (m=0; m<MNTS; m++)
		    free(map.rgn[c][m]); 
	}

	return 0;
}

//NOTE: Disable image_extract b/c not needed for ferret benchmark
#if 0 
int image_extract (const char *fname, cass_dataset_t *ds)
{
	unsigned char *HSV, *RGB;
	unsigned char *mask;
	int width, height, nrgn;
	int r;

	r = image_read_hsv(fname, &width, &height, &HSV);
	r = image_read_rgb(fname, &width, &height, &RGB);
	assert(r == 0);

	image_segment(&mask, &nrgn, RGB, width, height);

	image_extract_helper(HSV, mask, width, height, nrgn, ds);

	/* free image & map */
	free(HSV);
	free(RGB);
	free(mask);

	return 0;
}
#endif
