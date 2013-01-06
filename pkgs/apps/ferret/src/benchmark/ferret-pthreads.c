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
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include <cass.h>
#include <cass_timer.h>
#include <../image/image.h>
#include "tpool.h"
#include "queue.h"

#ifdef ENABLE_PARSEC_HOOKS
#include <hooks.h>
#endif

#define DEFAULT_DEPTH	25
#define MAXR	100
#define IMAGE_DIM	14

const char *db_dir = NULL;
const char *table_name = NULL;
const char *query_dir = NULL;
const char *output_path = NULL;

FILE *fout;

int NTHREAD_LOAD = 1;
int NTHREAD_SEG	= 1;
int NTHREAD_EXTRACT = 1;
int NTHREAD_VEC	= 1;
int NTHREAD_RANK = 1;
int NTHREAD_OUT	= 1;
int DEPTH = DEFAULT_DEPTH;

int top_K = 10;

char *extra_params = "-L 8 - T 20";

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

struct queue q_load_seg;

struct seg_data
{
	int width, height, nrgn;
	char *name;
	unsigned char *mask;
	unsigned char *HSV;
};

struct queue q_seg_extract;

struct extract_data
{
	cass_dataset_t ds;
	char *name;
};

struct queue q_extract_vec;

struct vec_query_data
{
	char *name;
	cass_dataset_t *ds;
	cass_result_t result;
};

struct queue q_vec_rank;

struct rank_data
{
	char *name;
	cass_dataset_t *ds;
	cass_result_t result;
};

struct queue q_rank_out;


/* ------- The Helper Functions ------- */
int cnt_enqueue;
int cnt_dequeue;
char path[BUFSIZ];


int scan_dir (const char *, char *head);

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
	int r;
	struct load_data *data;

	data = (struct load_data *)malloc(sizeof(struct load_data));
	assert(data != NULL);

	data->name = strdup(file);

	r = image_read_rgb_hsv(file, &data->width, &data->height, &data->RGB, &data->HSV);
	assert(r == 0);

	/*
		r = image_read_rgb(file, &data->width, &data->height, &data->RGB);
		r = image_read_hsv(file, &data->width, &data->height, &data->HSV);
		*/

	cnt_enqueue++;
	enqueue(&q_load_seg, data);

	return 0;
}

int scan_dir (const char *dir, char *head)
{
	struct stat st;
	int ret;
	/* test for . and .. */
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



/* ------ The Stages ------ */
void *t_load (void *dummy)
{
	const char *dir = (const char *)dummy;

	path[0] = 0;

	if (strcmp(dir, ".") == 0)
	{
		dir_helper(".", path);
	}
	else
	{
		scan_dir(dir, path);
	}

	queue_signal_terminate(&q_load_seg);
	return NULL;
}

void *t_seg (void *dummy)
{
	struct seg_data *seg;
	struct load_data *load;

	while(1)
	{
		if(dequeue(&q_load_seg, &load) < 0)
		    break;
		
		assert(load != NULL);
		seg = (struct seg_data *)calloc(1, sizeof(struct seg_data));

		seg->name = load->name;

		seg->width = load->width;
		seg->height = load->height;
		seg->HSV = load->HSV;
		image_segment(&seg->mask, &seg->nrgn, load->RGB, load->width, load->height);

		free(load->RGB);
		free(load);

		enqueue(&q_seg_extract, seg);
	}

	queue_signal_terminate(&q_seg_extract);
	return NULL;

}

void *t_extract (void *dummy)
{
	struct seg_data *seg;
	struct extract_data *extract;

	while (1)
	{
		if(dequeue(&q_seg_extract, &seg) < 0)
		    break;
		
		assert(seg != NULL);
		extract = (struct extract_data *)calloc(1, sizeof(struct extract_data));

		extract->name = seg->name;

		image_extract_helper(seg->HSV, seg->mask, seg->width, seg->height, seg->nrgn, &extract->ds);

		free(seg->mask);
		free(seg->HSV);
		free(seg);

		enqueue(&q_extract_vec, extract);
	}

	queue_signal_terminate(&q_extract_vec);
	return NULL;
}

void *t_vec (void *dummy)
{
	struct extract_data *extract;
	struct vec_query_data *vec;
	cass_query_t query;
	while(1)
	{
		if(dequeue(&q_extract_vec, &extract) < 0)
		    break;
		
		assert(extract != NULL);
		vec = (struct vec_query_data *)calloc(1, sizeof(struct vec_query_data));
		vec->name = extract->name;

		memset(&query, 0, sizeof query);
		query.flags = CASS_RESULT_LISTS | CASS_RESULT_USERMEM;

		vec->ds = query.dataset = &extract->ds;
		query.vecset_id = 0;

		query.vec_dist_id = vec_dist_id;

		query.vecset_dist_id = vecset_dist_id;

		query.topk = 2*top_K;

		query.extra_params = extra_params;

	    	cass_result_alloc_list(&vec->result, vec->ds->vecset[0].num_regions, query.topk);

	//	cass_result_alloc_list(&result_m, 0, T1);
	//	cass_table_query(table, &query, &vec->result);
		cass_table_query(table, &query, &vec->result);

		enqueue(&q_vec_rank, vec);
	}

	queue_signal_terminate(&q_vec_rank);
	return NULL;
}

void *t_rank (void *dummy)
{
	struct vec_query_data *vec;
	struct rank_data *rank;
	cass_result_t *candidate;
	cass_query_t query;
	while (1)
	{
		if(dequeue(&q_vec_rank, &vec) < 0)
		    break;
		
		assert(vec != NULL);

		rank = (struct rank_data *)calloc(1, sizeof(struct rank_data));
		rank->name = vec->name;

		query.flags = CASS_RESULT_LIST | CASS_RESULT_USERMEM | CASS_RESULT_SORT;
		query.dataset = vec->ds;
		query.vecset_id = 0;

		query.vec_dist_id = vec_dist_id;

		query.vecset_dist_id = vecset_dist_id;

		query.topk = top_K;

		query.extra_params = NULL;

		candidate = cass_result_merge_lists(&vec->result, (cass_dataset_t *)query_table->__private, 0);
		query.candidate = candidate;


		cass_result_alloc_list(&rank->result, 0, top_K);
		cass_table_query(query_table, &query, &rank->result);

		cass_result_free(&vec->result);
		cass_result_free(candidate);
		free(candidate);
		cass_dataset_release(vec->ds);
		free(vec->ds);
		free(vec);
		enqueue(&q_rank_out, rank);
	}

	queue_signal_terminate(&q_rank_out);
	return NULL;
}

void *t_out (void *dummy)
{
	struct rank_data *rank;
	while (1)
	{
		if(dequeue(&q_rank_out, &rank) < 0)
		    break;
		
		assert(rank != NULL);

		fprintf(fout, "%s", rank->name);

		ARRAY_BEGIN_FOREACH(rank->result.u.list, cass_list_entry_t p)
		{
			char *obj = NULL;
			if (p.dist == HUGE) continue;
			cass_map_id_to_dataobj(query_table->map, p.id, &obj);
			assert(obj != NULL);
			fprintf(fout, "\t%s:%g", obj, p.dist);
		} ARRAY_END_FOREACH;

		fprintf(fout, "\n");

		cass_result_free(&rank->result);
		free(rank->name);
		free(rank);

		cnt_dequeue++;
		
		fprintf(stderr, "(%d,%d)\n", cnt_enqueue, cnt_dequeue);
	}

	assert(cnt_enqueue == cnt_dequeue);
	return NULL;
}

int main (int argc, char *argv[])
{
	stimer_t tmr;

	tdesc_t *t_load_desc;
	tdesc_t *t_seg_desc;
	tdesc_t *t_extract_desc;
	tdesc_t *t_vec_desc;
	tdesc_t *t_rank_desc;
	tdesc_t *t_out_desc;

	tpool_t *p_load;
	tpool_t *p_seg;
	tpool_t *p_extract;
	tpool_t *p_vec;
	tpool_t *p_rank;
	tpool_t *p_out;

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
		printf("%s <database> <table> <query dir> <top K> <depth> <n> <out>\n", argv[0]); 
		return 0;
	}

	db_dir = argv[1];
	table_name = argv[2];
	query_dir = argv[3];
	top_K = atoi(argv[4]);

	DEPTH = atoi(argv[5]);
	NTHREAD_SEG = atoi(argv[6]);
	NTHREAD_EXTRACT = atoi(argv[6]);
	NTHREAD_VEC = atoi(argv[6]);
	NTHREAD_RANK = atoi(argv[6]);

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
	queue_init(&q_load_seg,    DEPTH, NTHREAD_LOAD);
	queue_init(&q_seg_extract, DEPTH, NTHREAD_SEG);
	queue_init(&q_extract_vec, DEPTH, NTHREAD_EXTRACT);
	queue_init(&q_vec_rank,    DEPTH, NTHREAD_VEC);
	queue_init(&q_rank_out,    DEPTH, NTHREAD_RANK);

	t_load_desc = (tdesc_t *)calloc(NTHREAD_LOAD, sizeof(tdesc_t));
	t_seg_desc = (tdesc_t *)calloc(NTHREAD_SEG, sizeof(tdesc_t));
	t_extract_desc = (tdesc_t *)calloc(NTHREAD_EXTRACT, sizeof(tdesc_t));
	t_vec_desc = (tdesc_t *)calloc(NTHREAD_VEC, sizeof(tdesc_t));
	t_rank_desc = (tdesc_t *)calloc(NTHREAD_RANK, sizeof(tdesc_t));
	t_out_desc = (tdesc_t *)calloc(NTHREAD_OUT, sizeof(tdesc_t));

	t_load_desc[0].attr = NULL;
	t_load_desc[0].start_routine = t_load;
	t_load_desc[0].arg = query_dir;

	for (i = 1; i < NTHREAD_LOAD; i++) t_load_desc[i] = t_load_desc[0];

	t_seg_desc[0].attr = NULL;
	t_seg_desc[0].start_routine = t_seg;
	t_seg_desc[0].arg = NULL;

	for (i = 1; i < NTHREAD_SEG; i++) t_seg_desc[i] = t_seg_desc[0];

	t_extract_desc[0].attr = NULL;
	t_extract_desc[0].start_routine = t_extract;
	t_extract_desc[0].arg = NULL;

	for (i = 1; i < NTHREAD_EXTRACT; i++) t_extract_desc[i] = t_extract_desc[0];

	t_vec_desc[0].attr = NULL;
	t_vec_desc[0].start_routine = t_vec;
	t_vec_desc[0].arg = NULL;
	for (i = 1; i < NTHREAD_VEC; i++) t_vec_desc[i] = t_vec_desc[0];

	t_rank_desc[0].attr = NULL;
	t_rank_desc[0].start_routine = t_rank;
	t_rank_desc[0].arg = NULL;
	for (i = 1; i < NTHREAD_RANK; i++) t_rank_desc[i] = t_rank_desc[0];


	t_out_desc[0].attr = NULL;
	t_out_desc[0].start_routine = t_out;
	t_out_desc[0].arg = NULL;
	for (i = 1; i < NTHREAD_OUT; i++) t_out_desc[i] = t_out_desc[0];

	cnt_enqueue = cnt_dequeue = 0;

#ifdef ENABLE_PARSEC_HOOKS
	__parsec_roi_begin();
#endif
	p_load = tpool_create(t_load_desc, NTHREAD_LOAD);
	p_seg = tpool_create(t_seg_desc, NTHREAD_SEG);
	p_extract = tpool_create(t_extract_desc, NTHREAD_EXTRACT);
	p_vec = tpool_create(t_vec_desc, NTHREAD_VEC);
	p_rank = tpool_create(t_rank_desc, NTHREAD_RANK);
	p_out = tpool_create(t_out_desc, NTHREAD_OUT);

	tpool_join(p_out, NULL);
	tpool_join(p_rank, NULL);
	tpool_join(p_vec, NULL);
	tpool_join(p_extract, NULL);
	tpool_join(p_seg, NULL);
	tpool_join(p_load, NULL);

#ifdef ENABLE_PARSEC_HOOKS
	__parsec_roi_end();
#endif

	tpool_destroy(p_load);
	tpool_destroy(p_seg);
	tpool_destroy(p_extract);
	tpool_destroy(p_vec);
	tpool_destroy(p_rank);
	tpool_destroy(p_out);

	free(t_load_desc);
	free(t_seg_desc);
	free(t_extract_desc);
	free(t_vec_desc);
	free(t_rank_desc);
	free(t_out_desc);

	queue_destroy(&q_load_seg);
	queue_destroy(&q_seg_extract);
	queue_destroy(&q_extract_vec);
	queue_destroy(&q_vec_rank);
	queue_destroy(&q_rank_out);

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

