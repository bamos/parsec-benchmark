#ifndef _stats_h_
#define _stats_h_

typedef struct stat_t {
	int	cnt; 
	float	min, max; 
	float	sum, sum2; 
	struct	stat_t *parallel;
} stat_t; 

void		stat_init (stat_t *s);
void		stat_cleanup (stat_t *s);

void		stat_pre_parallel (stat_t *s);
void		stat_post_parallel (stat_t *s);

void		stat_reset (stat_t *s);
void		stat_insert(stat_t *s, float v); 
float		stat_min(stat_t *s); 
float		stat_max(stat_t *s); 
float		stat_avg(stat_t *s); 
float		stat_std(stat_t *s); 

void		stat_print(stat_t *s, const char *msg);

#endif /* _stats_h_ */
