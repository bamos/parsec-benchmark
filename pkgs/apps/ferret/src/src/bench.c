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
#include <cass_bench.h>

extern size_t D, N;
/* extern float *weight; */
extern float **points;

int
benchT_read(benchT *bench, const char *fname, int Q)
{
	int		i, j; 
	FILE	*fp = fopen(fname, "r");
	queryT	*query;  

	if (fp == NULL) return -1;
	bench->qrys = (queryT *)calloc(Q, sizeof(queryT)); 
	for (i=0; i<Q; i++) { 
		query = &bench->qrys[i];
		if (fscanf(fp, "%d", &(query->qry)) != 1) break;
		fscanf(fp, "%d", &(query->knn));
 
		query->nabors = (int *)calloc(query->knn, sizeof(int)); 
		query->dist = (float *)calloc(query->knn, sizeof(float));
		/*
		query->dists = (float *)calloc(query->knn, sizeof(float)); 
		*/

		for (j=0; j<query->knn; j++) {
			fscanf(fp, "%d", &(query->nabors[j])); 
			fscanf(fp, "%g", &(query->dist[j])); 
			/*
			query->dists[j] = dist_l2(D, points[query->qry], points[query->nabors[j]], weight); 
			*/
		}
	}
	fclose(fp); 

	bench->Q = Q; 
	bench->score = NULL;

	return 0;
}

void
benchT_free(benchT *bench)
{
	int		i; 

	for (i=0; i<bench->Q; i++) {
		free(bench->qrys[i].nabors); 
		free(bench->qrys[i].dist); 
	}
	free(bench->qrys); 
}
 		
int 
benchT_qry(benchT *bench, int x)
{
	if (x < bench->Q)
		return bench->qrys[x].qry; 
	else
		return -1; 
}

int *
benchT_nabors(benchT *bench, int x)
{
	if (x < bench->Q)
		return bench->qrys[x].nabors; 
	else
		return NULL; 
}

void
benchT_print(benchT *bench, int x)
{
	int		i; 
	queryT	*query = &bench->qrys[x]; 

	printf("qry %d:\n", query->qry); 
	for (i=0; i<query->knn; i++)
		printf(" %d", query->nabors[i]); 
	printf("\n"); 
}

float 
benchT_score_topk(benchT *bench, int x, int K, int cnt, cass_list_entry_t *results)
{
	int		i, j;
	int		matched = 0; 
	float		recall; 
	queryT		*query = &bench->qrys[x]; 

//	benchT_print(bench, x); 

	for (i=0; i<K; i++) { 
		j = 0; 
		while ((j < cnt) && (results[j].id != query->nabors[i]))
			j++; 

		if (j < cnt) {
			matched++; 

			if (matched == K)
				break; 
		}
	} 

	recall = (float)matched / K;

	return recall; 
} 

float 
benchT_prec_topk(benchT *bench, int x, int K, int cnt, ftopk_t *results, float *prec)
{
	int		i, j;
	float		p; 
	queryT		*query = &bench->qrys[x]; 

//	benchT_print(bench, x); 
	p = 0;

	for (i=0; i<K; i++) { 
		j = 0; 
		while ((j < cnt) && (results[j].index != query->nabors[i]))
			j++; 

		if (j < cnt) {
			prec[i] = (float) (i + 1) / (float) (j + 1);
		}
		else
		{
			prec[i] = 0.0;
		}
		p += prec[i];
	} 

	p /= K;

	return p; 
} 

float 
benchT_score(benchT *bench, int x, int K, int cnt, int *results)
{
	int		i, j;
	int		matched = 0; 
	float		recall; 
	queryT		*query = &bench->qrys[x]; 

//	benchT_print(bench, x); 

	for (i=0; i<K; i++) { 
		j = 0; 
		while ((j < cnt) && (results[j] != query->nabors[i]))
			j++; 

		if (j < cnt) {
			matched++; 

			if (matched == K)
				break; 
		}
	} 

	recall = (float)matched / K;

	return recall; 
} 

/*
float
benchT_score_dist(benchT *bench, int x, int K, int cnt, float *dists)
{
	int	i; 
	float	error;
	queryT	*query = &bench->qrys[x]; 

	error = 0.0; 
	for (i=0; i<cnt; i++) {
		if (query->dists[i] > 0)
			error += dists[i] / query->dists[i]; 
		else if (dists[i] == 0)
			error += 1.0;
		else
			printf("ERROR:\t%d\t%.3f\t%.3f\n", i, dists[i], query->dists[i]);

		//printf("%d\t%.3f\t%.3f\t%.3f\n", i, dists[i], query->dists[i], error);
	} 

	return error/cnt; 
}
*/
