#ifndef _bench_h_
#define _bench_h_

typedef struct {
	int		qry; 
	int		knn; 
	int		*nabors;
	float		*dist;
	/*
	float	*dists; 
	*/
} queryT; 

typedef struct {
	int	cnt;
	float	recall;
	float	factor; 
} scoreT; 

typedef struct {
	int	Q; 
	queryT	*qrys; 
	scoreT	*score;
} benchT; 

int benchT_read(benchT *bench, const char *fname, int Q);

void benchT_free(benchT *bench); 

int benchT_qry(benchT *bench, int x); 

int *benchT_nabors(benchT *bench, int x);

void benchT_print(benchT *bench, int x); 

float benchT_score_topk(benchT *bench, int x, int K, int cnt, cass_list_entry_t *results); 

float benchT_prec_topk(benchT *bench, int x, int K, int cnt, ftopk_t *results, float *prec);

float benchT_score(benchT *bench, int x, int K, int cnt, int *results); 

float benchT_score_dist(benchT *bench, int x, int K, int cnt, float *dists); 

void benchT_done(benchT *bench); 
 
#endif /* _bench_h_ */

