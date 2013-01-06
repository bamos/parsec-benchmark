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

/* file format
 *
 * LINE 1:	<# vecset>	<vecset type:	1 | 2>
 * LINE 2:	<type : int | float>	<dim>
 * LINE 3:	<id 1>	<# vec>
 * LINE 4:	<weight> <vec 1>
 * ...
 * 		<id 2> <# vec>
 * 		<weight> <vec 1>
 * 		...
 */

#warning to be extended to allow external id

static void eat_comment (FILE *fin)
{
	int c;
	for (;;)
	{
		c = fgetc(fin);
		if (c != '#') break;
		for (;;)
		{
			c = fgetc(fin);
			if (c == '\n') break;
		}
	}
	ungetc(c, fin);
}

static int _import_vec_int (int32_t *vec, cass_size_t dim, FILE *fin)
{
	cass_size_t i;
	for (i = 0; i < dim; i++)
	{
		if (fscanf(fin, "%d", &vec[i]) != 1) return CASS_ERR_IO;
	}
	return 0;
}

static int _import_vec_float (float *vec, cass_size_t dim, FILE *fin)
{
	cass_size_t i;
	for (i = 0; i < dim; i++)
	{
		if (fscanf(fin, "%f", &vec[i]) != 1) return CASS_ERR_IO;
	}
	return 0;
}

static int _import_vec_bit (chunk_t *vec, cass_size_t dim, FILE *fin)
{
	int c;
	cass_size_t i, j, k;
	chunk_t chunk = 0;
	for (;;)
	{
		c = fgetc(fin);
		if (c == EOF) return CASS_ERR_IO;
		if (c == '0' || c == '1') break;
	}
	j = k = 0;
	ungetc(c, fin);
	for (i = 0; i < dim; i++)
	{
		c = fgetc(fin);
		if (c != '0' && c != '1') return CASS_ERR_IO;
		chunk = chunk << 1;
		chunk |= c - '0';
		j++;
		if (j == CHUNK_BIT)
		{
			vec[k++] = chunk;
			j = 0;
		}
	}
	return 0;
}


static int _export_vec_int (int32_t *vec, cass_size_t dim, FILE *fout)
{
	cass_size_t i;
	for (i = 0; i < dim; i++)
	{
		if (i > 0) fprintf(fout, "\t");
		if (fprintf(fout, "%d", vec[i]) <= 0) return CASS_ERR_IO;
	}
	fprintf(fout, "\n");
	return 0;
}

static int _export_vec_float (float *vec, cass_size_t dim, FILE *fout)
{
	cass_size_t i;
	for (i = 0; i < dim; i++)
	{
		if (i > 0) fprintf(fout, "\t");
		if (fprintf(fout, "%f", vec[i]) <= 0) return CASS_ERR_IO;
	}
	fprintf(fout, "\n");
	return 0;
}

static int _export_vec_bit (chunk_t *vec, cass_size_t dim, FILE *fout)
{
	int c, i, j, k;
	chunk_t chunk;
	j = k = 0;
	chunk = vec[k];
	for (i = 0; i < dim; i++)
	{
		c = chunk & 1;
		chunk = chunk >> 1;
		fprintf(fout, "%c", '0'+c);
		j++;
		if (j == CHUNK_BIT)
		{
			k++;
			chunk = vec[k];
			j = 0;
		}
	}
	fprintf(fout, "\n");
	return 0;
}

int cass_table_import_data(cass_table_t *table, char *fname)
{
	int ret;
	FILE *fin;
	cass_size_t dim;
	char objname[BUFSIZ];
	uint32_t j, k, l;
	cass_dataset_t *ds;
	cass_vec_t *vec;

	cass_vecset_id_t vecset_start;

	assert(table->opr->type & CASS_DATA);

	ds = &((struct raw_private *)table->__private)->dataset;

	vecset_start = ds->num_vecset;

	assert(table->loaded);

	fin = fopen(fname, "r");
	if (fin == NULL) return CASS_ERR_IO;

	ret = CASS_ERR_IO;

	dim = table->cfg->vec_dim;

	for (;;)
	{	
	    eat_comment(fin);
	    
	    if (fscanf(fin, "%s%u", objname, &l) != 2) break;

	    ret = cass_map_dataobj_to_id(table->map, objname, &k);
	    if (ret < 0) {
		// new obj.
		ret = cass_map_insert(table->map, &k, objname);
		if (ret < 0)
		    goto err;
	    }
	    if (ds->num_vecset < k)
		ds->num_vecset = k;

	    if (ds->num_vecset + 1 > ds->max_vecset)
	    {
		ds->max_vecset = grow(ds->max_vecset, ds->num_vecset + 1);
		ds->vecset = (cass_vecset_t *)realloc(ds->vecset, sizeof (cass_vecset_t) * ds->max_vecset);
		ret = CASS_ERR_OUTOFMEM;
		if (ds->vecset == NULL) return ret; 
	    }

	    ret = CASS_ERR_IO;
	    
	    ds->vecset[k].num_regions = l;
	    if (table->cfg->vecset_type == CASS_VECSET_SINGLE && l != 1) goto err;
	    ds->vecset[k].start_vecid = ds->num_vec;

	    if (ds->num_vec + l > ds->max_vec)
	    {
		ds->max_vec = grow(ds->max_vec, ds->num_vec + l);
		ds->vec = (cass_vec_t *)realloc(ds->vec,  ds->vec_size * ds->max_vec);
		ret = CASS_ERR_OUTOFMEM;
		if (ds->vec == NULL) return ret; 
		ret = CASS_ERR_IO;
	    }

	    vec = (cass_vec_t *)((void *)ds->vec + (ds->num_vec * ds->vec_size));
	    for (j = 0; j < l; j++)
	    {
		ret = CASS_ERR_IO;
		vec->parent = ds->num_vecset;
		eat_comment(fin);
		if (fscanf(fin, "%f", &vec->weight) != 1) goto err;
		switch (table->cfg->vec_type)
		{
			case CASS_VEC_INT: ret = _import_vec_int(vec->u.int_data, dim, fin); break;
			case CASS_VEC_FLOAT: ret = _import_vec_float(vec->u.float_data, dim, fin); break;
			case CASS_VEC_BIT: ret = _import_vec_bit(vec->u.bit_data, dim, fin); break;
			default: assert(0);
		}
		if (ret != 0) goto err;
		vec = (cass_vec_t *)((void *)vec + ds->vec_size);
		ds->num_vec++;
	    }
	    ds->num_vecset++;
	}

	table->dirty = 1;

	fclose(fin);
	fin = NULL;

	/* cascade insert */
    	ARRAY_BEGIN_FOREACH(table->children, cass_table_t *p)
    	{
       		ret = cass_table_batch_insert(p, ds, vecset_start, ds->num_vecset -1);
		if (ret != 0) goto err;
    	} ARRAY_END_FOREACH;


	return 0;

err:
	if (fin != NULL) fclose(fin);
	return ret;
}

int cass_table_import_direct_data(cass_table_t *table, char *fname)
{
	int ret;
	FILE *fin;
	cass_size_t dim;
	uint32_t j, k, l;
	cass_dataset_t *ds;
	cass_vec_t *vec;

	cass_vecset_id_t vecset_start;

	assert(table->opr->type & CASS_DATA);

	ds = &((struct raw_private *)table->__private)->dataset;

	vecset_start = ds->num_vecset;

	assert(table->loaded);

	fin = fopen(fname, "r");
	if (fin == NULL) return CASS_ERR_IO;

	ret = CASS_ERR_IO;

	dim = table->cfg->vec_dim;

	for (;;)
	{

	
	    if (ds->num_vecset + 1 > ds->max_vecset)
	    {
		ds->max_vecset = grow(ds->max_vecset, ds->num_vecset + 1);
		ds->vecset = (cass_vecset_t *)realloc(ds->vecset, sizeof (cass_vecset_t) * ds->max_vecset);
		ret = CASS_ERR_OUTOFMEM;
		if (ds->vecset == NULL) return ret; 
	    }

	    ret = CASS_ERR_IO;

	    eat_comment(fin);
	    
	    if (fscanf(fin, "%u%u", &k, &l) != 2) break;
	    if (k != ds->num_vecset) goto err;
	    ds->vecset[k].num_regions = l;
	    if (table->cfg->vecset_type == CASS_VECSET_SINGLE && l != 1) goto err;
	    ds->vecset[k].start_vecid = ds->num_vec;

	    if (ds->num_vec + l > ds->max_vec)
	    {
		ds->max_vec = grow(ds->max_vec, ds->num_vec + l);
		ds->vec = (cass_vec_t *)realloc(ds->vec,  ds->vec_size * ds->max_vec);
		ret = CASS_ERR_OUTOFMEM;
		if (ds->vec == NULL) return ret; 
		ret = CASS_ERR_IO;
	    }

	    vec = (cass_vec_t *)((void *)ds->vec + (ds->num_vec * ds->vec_size));
	    for (j = 0; j < l; j++)
	    {
		ret = CASS_ERR_IO;
		vec->parent = ds->num_vecset;
		eat_comment(fin);
		if (fscanf(fin, "%f", &vec->weight) != 1) goto err;
		switch (table->cfg->vec_type)
		{
			case CASS_VEC_INT: ret = _import_vec_int(vec->u.int_data, dim, fin); break;
			case CASS_VEC_FLOAT: ret = _import_vec_float(vec->u.float_data, dim, fin); break;
			case CASS_VEC_BIT: ret = _import_vec_bit(vec->u.bit_data, dim, fin); break;
			default: assert(0);
		}
		if (ret != 0) goto err;
		vec = (cass_vec_t *)((void *)vec + ds->vec_size);
		ds->num_vec++;
	    }
	    ds->num_vecset++;
	}

	table->dirty = 1;

	fclose(fin);
	fin = NULL;

	/* cascade insert */
    	ARRAY_BEGIN_FOREACH(table->children, cass_table_t *p)
    	{
       		ret = cass_table_batch_insert(p, ds, vecset_start, ds->num_vecset -1);
		if (ret != 0) goto err;
    	} ARRAY_END_FOREACH;


	return 0;

err:
	if (fin != NULL) fclose(fin);
	return ret;
}

int cass_table_export_data(cass_table_t *table, char *fname)
{
	FILE *fout;
	cass_dataset_t *ds;
	cass_vec_t *vec;
	uint32_t i, j, k, dim;
	char *p;

	assert(table->opr->type & CASS_DATA ||
		table->opr->type & CASS_VEC_SKETCH);

	ds = &((struct raw_private *)table->__private)->dataset;

	assert(ds->loaded);

	fout = fopen(fname, "w");
	if (fout == NULL) return CASS_ERR_IO;

	dim = table->cfg->vec_dim;

	if (table->cfg->vecset_type == CASS_VECSET_NONE) k = 0;
	else if (table->cfg->vecset_type == CASS_VECSET_SINGLE) k = 1;
	else if (table->cfg->vecset_type == CASS_VECSET_SET) k = 2;
	else assert(0);

	fprintf(fout, "# %u\t%u\n", ds->num_vecset, k);


	if (table->cfg->vec_type == CASS_VEC_INT)
	{
		p = "int";
	}
	else if (table->cfg->vec_type == CASS_VEC_FLOAT)
	{
		p = "float";
	}
	else if (table->cfg->vec_type == CASS_VEC_BIT)
	{
		p = "bit";
	}

	fprintf(fout, "# %s\t%u\n", p, dim);

	if ((ds->flags & CASS_DATASET_BOTH) == CASS_DATASET_BOTH)
	{
		vec = ds->vec;
		for (i = 0; i < ds->num_vecset; i++)
		{
			char *name;
			cass_map_id_to_dataobj(table->map, i, &name);
			fprintf(fout, "%s\t%u\n", name, ds->vecset[i].num_regions);
			k = ds->vecset[i].start_vecid;
			assert((void *)ds->vec + k * ds->vec_size == vec);
			for (j = 0; j < ds->vecset[i].num_regions; j++)
			{
				fprintf(fout, "%f\t", vec->weight);
				switch (table->cfg->vec_type)
				{
					case CASS_VEC_INT: _export_vec_int(vec->u.int_data, dim, fout); break;
					case CASS_VEC_FLOAT: _export_vec_float(vec->u.float_data, dim, fout); break;
					case CASS_VEC_BIT: _export_vec_bit(vec->u.bit_data, dim, fout); break;
					default: assert(0);
				}
				vec = (cass_vec_t *)((void *)vec + ds->vec_size);
			}
		}
	}
	else if (ds->flags & CASS_DATASET_VEC)
	{
		vec = ds->vec;
		fprintf(fout, "#weight\tparent\tvector\n");
		for (i = 0; i < ds->num_vec; i++)
		{
			fprintf(fout, "%f\t", vec->weight);
			fprintf(fout, "%u\t", vec->parent);
			switch (table->cfg->vec_type)
			{
				case CASS_VEC_INT: _export_vec_int(vec->u.int_data, dim, fout); break;
				case CASS_VEC_FLOAT: _export_vec_float(vec->u.float_data, dim, fout); break;
				case CASS_VEC_BIT: _export_vec_bit(vec->u.bit_data, dim, fout); break;
				default: assert(0);
			}
			vec = (cass_vec_t *)((void *)vec + ds->vec_size);
		}
	}

	fclose(fout);
	return 0;


}

