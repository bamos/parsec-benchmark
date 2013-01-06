/* AUTORIGHTS
Copyright (C) 2007 Princeton University
Copyright (C) 2010 Christian Fensch

TBB version of ferret written by Christian Fensch.

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
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include "../include/cass.h"
#include "../include/cass_timer.h"
#include "../image/image.h"
#include "ferret-tbb.h"


#ifdef ENABLE_PARSEC_HOOKS
#include <hooks.h>
#endif

#define DEFAULT_DEPTH	25
#define MAXR	100
#define IMAGE_DIM	14

/* Backwards compatability hack for TBB 2.1 (which ships with Parsec)
   TBB 2.2 changes the modes for filter in a pipeline.
*/

#ifndef TBB_INTERFACE_VERSION
#error TBB_INTERFACE_VERSION is not defined. Cannot determine correct interface
#endif

#if TBB_INTERFACE_VERSION < 4001
/* earlier version than TBB 2.2 */
#define SERIAL_IN_ORDER_FILTER serial
#define SERIAL_OUT_OF_ORDER_FILTER serial
#include <tbb/task_scheduler_init.h>
#else
/* TBB 2.2 or later */
#define SERIAL_IN_ORDER_FILTER serial_in_order
#define SERIAL_OUT_OF_ORDER_FILTER serial_out_of_order
#endif

const char *db_dir = NULL;
const char *table_name = NULL;
const char *query_dir = NULL;
const char *output_path = NULL;

FILE *fout;

int TOKEN_IN_FLIGHT = 1;

int top_K = 10;

const char *extra_params = "-L 8 - T 20";

int input_end, output_end;
pthread_cond_t done;
pthread_mutex_t done_mutex;

cass_env_t *env;
cass_table_t *table;
cass_table_t *query_table;

int vec_dist_id = 0;
int vecset_dist_id = 0;

struct load_data
{
	int width, height;
	char *name;
	unsigned char *HSV, *RGB;
};


struct seg_data
{
	int width, height, nrgn;
	char *name;
	unsigned char *mask;
	unsigned char *HSV;
};


struct extract_data
{
	cass_dataset_t ds;
	char *name;
};


struct vec_query_data
{
	char *name;
	cass_dataset_t *ds;
	cass_result_t result;
};


struct rank_data
{
	char *name;
	cass_dataset_t *ds;
	cass_result_t result;
};



struct all_data {
	union {
		struct load_data      load;
		struct rank_data      rank;
	} first;
	union {
		struct seg_data       seg;
		struct vec_query_data vec;
	} second;
	struct extract_data extract;
};


/* ------- The Helper Functions ------- */
int cnt_enqueue;
int cnt_dequeue;

/* the whole path to the file */
struct all_data *file_helper (const char *file)
{
	int r;
	struct all_data *data;

	data = (struct all_data *)malloc(sizeof(struct all_data));
	assert(data != NULL);

	data->first.load.name = strdup(file);

	r = image_read_rgb_hsv(file,
			       &data->first.load.width,
			       &data->first.load.height,
			       &data->first.load.RGB,
			       &data->first.load.HSV);
	assert(r == 0);

	cnt_enqueue++;

	return data;
}

/* ------ The Stages ------ */


filter_load::filter_load(const char * dir) :
	tbb::filter(SERIAL_IN_ORDER_FILTER)
{
	m_path[0] = 0;
	
	if (strcmp(dir, ".") == 0) {
		m_single_file = NULL;
		push_dir(".");
	}
	else if (strcmp(dir, "..") == 0) {
		m_single_file = NULL;
	}
	else {
		int ret;
		struct stat st;
		
		ret = stat(dir, &st);
		if (ret != 0)
		{
			perror("Error:");
			m_single_file = NULL;
		}
		if (S_ISREG(st.st_mode))
			m_single_file = dir;
		else if (S_ISDIR(st.st_mode)) {
			m_single_file = NULL;
			push_dir(dir);
		}
	}
}

void filter_load::push_dir(const char * dir) {
	int path_len = strlen(m_path);
	DIR *pd = NULL;
	
	strcat(m_path, dir);
	pd = opendir(m_path);
	if (pd != NULL) {
		strcat(m_path, "/");
		m_dir_stack.push(pd);
		m_path_stack.push(path_len);
	} else {
		m_path[path_len] = 0;
	}
}

void *filter_load::operator()( void* item ){
	if(m_single_file) {
		struct all_data *ret;
		ret = file_helper(m_single_file);
		m_single_file = NULL;
		return ret;
	}

	if(m_dir_stack.empty())
		return NULL;

	for(;;) {
		DIR *pd = m_dir_stack.top();
		struct dirent *ent = NULL;
		int res = 0;
		struct stat st;
		int path_len = strlen(m_path);

		ent = readdir(pd);
		if (ent == NULL) {
			closedir(pd);
			m_path[m_path_stack.top()] = 0;
			m_path_stack.pop();
			m_dir_stack.pop();
			if(m_dir_stack.empty())
				return NULL;
		}
		
		if((ent->d_name[0] == '.') &&
		   ((ent->d_name[1] == 0) || ((ent->d_name[1] == '.') && 
					      (ent->d_name[2] == 0)) ) )
			continue;
		
		strcat(m_path, ent->d_name);
		res = stat(m_path, &st);
		if (res != 0)
		{
			perror("Error:");
			return NULL;
		}
		if (S_ISREG(st.st_mode)) {
			struct all_data *ret;
			ret = file_helper(m_path);
			m_path[path_len]=0;
			return ret;
		} else if (S_ISDIR(st.st_mode)) {
			m_path[path_len]=0;
			push_dir(ent->d_name);
		} else
			m_path[path_len]=0;
	}
}


filter_seg::filter_seg() :
	tbb::filter(parallel)
	{}

void *filter_seg::operator()( void* item ) {
	struct all_data *data = (struct all_data*)item;
	
	data->second.seg.name = data->first.load.name;

	data->second.seg.width = data->first.load.width;
	data->second.seg.height = data->first.load.height;
	data->second.seg.HSV = data->first.load.HSV;
	image_segment(&data->second.seg.mask,
		      &data->second.seg.nrgn,
		      data->first.load.RGB,
		      data->first.load.width,
		      data->first.load.height);

	free(data->first.load.RGB);
	return item;
}


filter_extract::filter_extract() :
	tbb::filter(parallel)
	{}


void *filter_extract::operator()( void* item ) {
	struct all_data *data = (struct all_data *)item;

	data->extract.name = data->second.seg.name;

	image_extract_helper(data->second.seg.HSV,
			     data->second.seg.mask,
			     data->second.seg.width,
			     data->second.seg.height,
			     data->second.seg.nrgn,
			     &data->extract.ds);

	free(data->second.seg.mask);
	free(data->second.seg.HSV);


	return item;
}


filter_vec::filter_vec() :
	tbb::filter(parallel)
	{}


void *filter_vec::operator()(void* item) {
	struct all_data *data = (struct all_data *) item;
	cass_query_t query;

	data->second.vec.name = data->extract.name;

	memset(&query, 0, sizeof query);
	query.flags = CASS_RESULT_LISTS | CASS_RESULT_USERMEM;

	data->second.vec.ds = query.dataset = &data->extract.ds;
	query.vecset_id = 0;

	query.vec_dist_id = vec_dist_id;

	query.vecset_dist_id = vecset_dist_id;

	query.topk = 2*top_K;

	query.extra_params = extra_params;

	cass_result_alloc_list(&data->second.vec.result,
			       data->second.vec.ds->vecset[0].num_regions,
			       query.topk);

	cass_table_query(table, &query, &data->second.vec.result);

	return item;
}


filter_rank::filter_rank() :
	tbb::filter(parallel)
	{}

void *filter_rank::operator()(void* item) {
	struct all_data *data = (struct all_data*) item;
	
	cass_result_t *candidate;
	cass_query_t query;

	data->first.rank.name = data->second.vec.name;

	query.flags = CASS_RESULT_LIST | CASS_RESULT_USERMEM | CASS_RESULT_SORT;
	query.dataset = data->second.vec.ds;
	query.vecset_id = 0;

	query.vec_dist_id = vec_dist_id;

	query.vecset_dist_id = vecset_dist_id;

	query.topk = top_K;

	query.extra_params = NULL;

	candidate = cass_result_merge_lists(&data->second.vec.result,
					    (cass_dataset_t *)query_table->__private,
					    0);
	query.candidate = candidate;

	cass_result_alloc_list(&data->first.rank.result,
			       0, top_K);
	cass_table_query(query_table, &query,
			 &data->first.rank.result);

	cass_result_free(&data->second.vec.result);
	cass_result_free(candidate);
	free(candidate);
	cass_dataset_release(data->second.vec.ds);

	return item;
}


filter_out::filter_out() :
	tbb::filter(SERIAL_OUT_OF_ORDER_FILTER)
	{}

void *filter_out::operator()(void* item) {
	struct all_data *data = (struct all_data *) item;
	
	fprintf(fout, "%s", data->first.rank.name);

	ARRAY_BEGIN_FOREACH(data->first.rank.result.u.list, cass_list_entry_t p)
	{
		char *obj = NULL;
		if (p.dist == HUGE) continue;
		cass_map_id_to_dataobj(query_table->map, p.id, &obj);
		assert(obj != NULL);
		fprintf(fout, "\t%s:%g", obj, p.dist);
	} ARRAY_END_FOREACH;

	fprintf(fout, "\n");

	cass_result_free(&data->first.rank.result);
	free(data->first.rank.name);
	free(data);

	cnt_dequeue++;

	fprintf(stderr, "(%d,%d)\n", cnt_enqueue, cnt_dequeue);

	return NULL;
}


int main (int argc, char *argv[])
{
	stimer_t tmr;
	tbb::pipeline ferret_pipeline;

	int ret, i;

#ifdef PARSEC_VERSION
#define __PARSEC_STRING(x) #x
#define __PARSEC_XSTRING(x) __PARSEC_STRING(x)
        printf("PARSEC Benchmark Suite Version "__PARSEC_XSTRING(PARSEC_VERSION)"\n");
        fflush(NULL);
#else
        printf("PARSEC Benchmark Suite\n");
        fflush(NULL);
#endif //PARSEC_VERSION
#ifdef ENABLE_PARSEC_HOOKS
	__parsec_bench_begin(__parsec_ferret);
#endif

	if (argc < 8)
	{
		printf("%s <database> <table> <query dir> <top K> <ignored> <n> <out>\n", argv[0]); 
		return 0;
	}

	db_dir = argv[1];
	table_name = argv[2];
	query_dir = argv[3];
	top_K = atoi(argv[4]);

	TOKEN_IN_FLIGHT = atoi(argv[6]);

	output_path = argv[7];

	fout = fopen(output_path, "w");
	assert(fout != NULL);

	cass_init();

	ret = cass_env_open(&env, db_dir, 0);
	if (ret != 0) { printf("ERROR: %s\n", cass_strerror(ret)); return 0; }

	vec_dist_id = cass_reg_lookup(&env->vec_dist, "L2_float");
	assert(vec_dist_id >= 0);

	vecset_dist_id = cass_reg_lookup(&env->vecset_dist, "emd");
	assert(vecset_dist_id >= 0);

	i = cass_reg_lookup(&env->table, table_name);


	table = query_table = cass_reg_get(&env->table, i);

	i = table->parent_id;

	if (i >= 0)
	{
		query_table = cass_reg_get(&env->table, i);
	}

	if (query_table != table) cass_table_load(query_table);

	cass_map_load(query_table->map);

	cass_table_load(table);

	image_init(argv[0]);

	stimer_tick(&tmr);

	filter_load    my_load_filter(query_dir);
	filter_seg     my_seg_filter;
	filter_extract my_extract_filter;
	filter_vec     my_vec_filter;
	filter_rank    my_rank_filter;
	filter_out     my_out_filter;

	ferret_pipeline.add_filter(my_load_filter);
	ferret_pipeline.add_filter(my_seg_filter);
	ferret_pipeline.add_filter(my_extract_filter);
	ferret_pipeline.add_filter(my_vec_filter);
	ferret_pipeline.add_filter(my_rank_filter);
	ferret_pipeline.add_filter(my_out_filter);

	input_end = output_end = 0;
	cnt_enqueue = cnt_dequeue = 0;


#ifdef ENABLE_PARSEC_HOOKS
	__parsec_roi_begin();
#endif

#if TBB_INTERFACE_VERSION < 4001
	/* in TBB versions before 2.2 this call is mandatory */
	tbb::task_scheduler_init init(TOKEN_IN_FLIGHT);
#endif

	ferret_pipeline.run(TOKEN_IN_FLIGHT);

#ifdef ENABLE_PARSEC_HOOKS
	__parsec_roi_end();
#endif

	stimer_tuck(&tmr, "QUERY TIME");

	ret = cass_env_close(env, 0);
	if (ret != 0) { printf("ERROR: %s\n", cass_strerror(ret)); return 0; }

	cass_cleanup();

	image_cleanup();

	fclose(fout);

#ifdef ENABLE_PARSEC_HOOKS
	__parsec_bench_end();
#endif
	return 0;
}

