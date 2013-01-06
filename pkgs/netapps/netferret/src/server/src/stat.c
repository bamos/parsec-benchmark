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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h> 
#include <math.h> 
#include <values.h>
#include <cass_stat.h>

#ifdef _OPENMP
#include <omp.h>
#endif
	
#define MAX_THREAD	32

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

void stat_init (stat_t *s)
{
	stat_reset(s);
	s->parallel = NULL;
}

void stat_cleanup (stat_t *s)
{
	if (s->parallel != NULL) free(s->parallel);
}

void stat_pre_parallel (stat_t *s)
{
#ifdef _OPENMP
	int i;
	if (s->parallel == NULL)
	{
		s->parallel = (stat_t *)malloc(MAX_THREAD * sizeof(stat_t));
		assert(s->parallel != NULL);
	}

	for (i = 0; i < MAX_THREAD; i++)
	{
		stat_reset(&s->parallel[i]);
	}
#endif
}

void stat_post_parallel (stat_t *s)
{
#ifdef _OPENMP
	int i;
	for (i = 0; i < MAX_THREAD; i++)
	{
		if (s->parallel[i].cnt <= 0) continue;
		s->cnt += s->parallel[i].cnt;
		s->sum += s->parallel[i].sum;
		s->sum2 += s->parallel[i].sum2;
		s->max = MAX(s->max, s->parallel[i].max);
		s->min = MIN(s->min, s->parallel[i].min);
	}
#endif
}

void
stat_reset(stat_t *ret)
{
	ret->sum = 0; 
	ret->sum2 = 0; 
	ret->cnt = 0; 
	ret->max = MINFLOAT;
	ret->min = MAXFLOAT;
} 

static inline void
_stat_insert(stat_t *s, float v)
{
	s->min = MIN(s->min, v);
	s->max = MAX(s->max, v);
	s->sum += v; 
	s->sum2 += v * v; 
	s->cnt++; 
} 

void stat_insert(stat_t *s, float v)
{
#ifdef _OPENMP
	if (omp_in_parallel())
	{
		int id = omp_get_thread_num();
		assert(id < MAX_THREAD);
		_stat_insert(&s->parallel[id], v);
	}
	else
#endif
	{
		_stat_insert(s, v);
	}
}

float
stat_min(stat_t *s)
{
	return s->min; 
} 

float
stat_max(stat_t *s)
{
	return s->max; 
}

float
stat_avg(stat_t *s)
{
	if (s->cnt > 0)
		return s->sum / s->cnt; 
	else
		return 0; 
} 

float
stat_std(stat_t *s)
{
	if (s->cnt > 1) 
		return sqrt((s->sum2 - (s->sum/s->cnt) * s->sum)/(s->cnt - 1)); 
	else
		return 0; 
} 

void
stat_print(stat_t *s, const char *msg)
{
	if (msg)
		printf("%s: ", msg); 

	printf("cnt=%d, min=%.3f, max=%.3f, avg=%.3f, std=%.3f\n", 
		s->cnt, s->min, s->max, stat_avg(s), stat_std(s)); 
} 

