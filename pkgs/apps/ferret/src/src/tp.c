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
/* Transportation problem solver */
#include <assert.h>
#include <stdlib.h>
#include <values.h>
#include <cass.h>

struct sol
{
	int i, j;
	float value;
	int flow, dir;
	float sigma;
	struct sol *next, *prev;
};

static void tp_init_russell (int nrow, float *row, int ncol, float *col, float **cost, struct sol ***_sol)
{
	float *u = type_calloc(float, nrow);
	float *v = type_calloc(float, ncol);
	int i, j, cnt;

	float max, t;
	int mrow, mcol;

	struct sol **sol;
	
	sol = type_matrix_alloc(struct sol, nrow, ncol);
	for (i = 0; i < nrow; i++)
		for (j = 0; j < ncol; j++)
		{
			sol[i][j].i = i;
			sol[i][j].j = j;
			sol[i][j].flow = 0;
		}

	cnt = 0;

	for (;;)
	{
		for (i = 0; i < nrow; i++) if (u[i] >= 0) u[i] = 0;
		for (j = 0; j < ncol; j++) if (v[j] >= 0) v[j] = 0;

		for (i = 0; i < nrow; i++) if (u[i] >= 0)
			for (j = 0; j < ncol; j++) if (v[j] >= 0)
			{
				if (cost[i][j] > u[i]) u[i] = cost[i][j];
				if (cost[i][j] > v[j]) v[j] = cost[i][j];
			}

		mrow = mcol = -1;

		for (i = 0; i < nrow; i++) if (u[i] >= 0)
			for (j = 0; j < ncol; j++) if (v[j] >= 0)
			{
				t = u[i] + v[j] - cost[i][j];
				if (mrow < 0 || t > max)
				{
					max = t;
					mrow = i;
					mcol = j;
				}

			}

		if (mrow < 0 || mcol < 0) break;

		assert(u[mrow] >= 0);
		assert(v[mcol] >= 0);


		cnt++;

		if (row[mrow] < col[mcol])
		{
			sol[mrow][mcol].flow = 1;
			sol[mrow][mcol].value = row[mrow];
			col[mcol] -= row[mrow];
			row[mrow] = 0.0;
			u[mrow] = -1;
		}
		else
		{
			sol[mrow][mcol].flow = 1;
			sol[mrow][mcol].value = col[mcol];
			row[mrow] -= col[mcol];
			col[mcol] = 0.0;
			v[mcol] = -1;
		}

	}

//	printf("%d, %d, %d\n", cnt, nrow, ncol);
	assert(cnt == ncol + nrow - 1);

	free(u);
	free(v);

	*_sol = sol;
}

//#define BINSET

#ifdef BINSET
#define SET_TYPE		unsigned long long
#define SET_INIT(set, size)	do { set = 0; assert(size <= 8 * sizeof(unsigned long long));} while (0)
#define SET_TEST(set,mem)	((set) & ((unsigned long long)1 << (mem)))
#define SET_ADD(set,mem)	do { set |= (unsigned long long )1 << (mem); } while (0)
#define SET_CLEANUP(set)	
#else
#define SET_TYPE		char *
#define SET_INIT(set, size)	do { set = (char *)type_calloc(char, size); } while(0)
#define SET_TEST(set,mem)	(set[mem])
#define SET_ADD(set,mem)	do { set[mem] = 1; } while (0)
#define SET_CLEANUP(set)	free(set)

#endif

/* find  initial solution using Vogel approximation */
static void tp_init_vogel (int nrow, float *row, int ncol, float *col, float **cost, struct sol ***_sol)
{
	SET_TYPE r_del;
	SET_TYPE c_del;

	int i, j, cnt;
	int lr, lc;

	float max;
	int mrow, mcol;

	struct sol **sol;

	SET_INIT(r_del, nrow);
	SET_INIT(c_del, ncol);
	
	sol = type_matrix_alloc(struct sol, nrow, ncol);
	for (i = 0; i < nrow; i++)
		for (j = 0; j < ncol; j++)
		{
			sol[i][j].i = i;
			sol[i][j].j = j;
			sol[i][j].flow = 0;
		}

	lr = nrow;
	lc = ncol;
	

	while (lr + lc > 2)
	{
		max = 0.0;
		mrow = mcol = -1;

		for (i = 0; i < nrow; i++) if (!SET_TEST(r_del, i))
		{
			float m1, m2;	/* m1 smallest, m2 2nd smallest */
			int m1_idx, m2_idx;
			m1 = m2 = 0.0;
			m1_idx = m2_idx = -1;
			for (j = 0; j < ncol; j++) if (!SET_TEST(c_del, j))
			{
				if ((m2_idx < 0) || (cost[i][j] < m2))
				{
					if ((m1_idx < 0) || cost[i][j] < m1)
					{
						m2 = m1;
						m2_idx = m1_idx;
						m1 = cost[i][j];
						m1_idx = j;
					}
					else
					{
						m2 = cost[i][j];
						m2_idx = j;
					}
				}
			}

			assert(m1_idx >= 0);
			if (m2_idx < 0) continue;

			if ((mrow < 0) || (m2 - m1 > max))
			{
				max = m2 - m1;
				mrow = i;
				mcol = m1_idx;
			}
		}

		for (i = 0; i < ncol; i++) if (!SET_TEST(c_del, i))
		{
			float m1, m2;	/* m1 smallest, m2 2nd smallest */
			int m1_idx, m2_idx;
			m1 = m2 = 0;
			m1_idx = m2_idx = -1;
			for (j = 0; j < nrow; j++) if (!SET_TEST(r_del, j))
			{
				if ((m2_idx < 0) || (cost[j][i] < m2))
				{
					if ((m1_idx < 0) || (cost[j][i] < m1))
					{
						m2 = m1;
						m2_idx = m1_idx;
						m1 = cost[j][i];
						m1_idx = j;
					}
					else
					{
						m2 = cost[j][i];
						m2_idx = j;
					}
				}
			}

			assert(m1_idx >= 0);
			if (m2_idx < 0) continue;

			if ((mrow < 0) || (m2 - m1 > max))
			{
				max = m2 - m1;
				mrow = m1_idx;
				mcol = i;
			}
		}

		assert(mrow >= 0);
		assert(mcol >= 0);
		/*
		assert(mrow >= 0);
		assert(mrow < nrow);
		assert(mcol >= 0);
		assert(mcol < ncol);
		*/
		assert(!SET_TEST(r_del, mrow));
		assert(!SET_TEST(c_del, mcol));

		if ((lr > 1) && ((row[mrow] <= col[mcol]) || (lc <= 1)))
		{
			sol[mrow][mcol].flow = 1;
			sol[mrow][mcol].value = row[mrow];
			col[mcol] -= row[mrow];
			row[mrow] = 0.0;
			if (col[mcol] < 0) col[mcol] = 0.0;
			SET_ADD(r_del, mrow);
			lr--;
		}
		else
		{
			assert(lc > 1);
			sol[mrow][mcol].flow = 1;
			sol[mrow][mcol].value = col[mcol];
			row[mrow] -= col[mcol];
			col[mcol] = 0.0;
			SET_ADD(c_del, mcol);
			lc--;
		}

	}

	assert(lc == 1);
	assert(lr == 1);

	for (;;)
	{
		mrow = -1;
		for (i = 0; i < nrow; i++) if (!SET_TEST(r_del, i))
		{
			mcol = -1;
			for (j = 0; j < ncol; j++) if (!SET_TEST(c_del, j))
			{
				mcol = j;
				break;
			}
			if (mcol >= 0)
			{
				mrow = i;
				break;
			}
		}
		if (mrow < 0 || mcol < 0) break;
		assert(!SET_TEST(r_del, mrow));
		assert(!SET_TEST(c_del, mcol));

		cnt++;

		if (row[mrow] < col[mcol])
		{
			sol[mrow][mcol].flow = 1;
			sol[mrow][mcol].value = row[mrow];
			col[mcol] -= row[mrow];
			row[mrow] = 0.0;
			SET_ADD(r_del, mrow);
		}
		else
		{
			sol[mrow][mcol].flow = 1;
			sol[mrow][mcol].value = col[mcol];
			row[mrow] -= col[mcol];
			col[mcol] = 0.0;
			SET_ADD(c_del, mcol);
		}
	}


	SET_CLEANUP(r_del);
	SET_CLEANUP(c_del);

	*_sol = sol;
}

#define U_FLAG_DONE	1
#define U_FLAG_U	2
#define U_FLAG_V	4

struct U
{
	int flags;
	float value;
	struct U *next;
};


static inline int tp_update (int nrow, int ncol, float **cost, struct sol **sol, struct U *u, struct U *v)
{
	struct U q, *qt, *qc;

	struct sol *head, *min, *tail, *cur, queue;

	int i, j, ii, jj, cnt;

	float min_flow;

	cnt = 1;

	for (i = 0; i < nrow; i++) u[i].flags = U_FLAG_U;
	for (j = 0; j < ncol; j++) v[j].flags = U_FLAG_V;

	u[0].value = 0.0;
	u[0].flags |= U_FLAG_DONE;

	qc = &u[0];
	q.next = NULL;
	qt = &q;

	for (;;)
	{
		if (qc->flags & U_FLAG_U)
		{
			i = qc - u;
			for (j = 0; j < ncol; j++)
				if (sol[i][j].flow && !(v[j].flags & U_FLAG_DONE))
				{
					cnt++;
					v[j].value = cost[i][j] - u[i].value;
					v[j].flags |= U_FLAG_DONE;

					qt->next = &v[j];
					qt = qt->next;
					qt->next = NULL;
				}
		}
		else
		{
			j = qc - v;
			for (i = 0; i < nrow; i++)
				if (sol[i][j].flow && !(u[i].flags & U_FLAG_DONE))
				{
					cnt++;
					u[i].value = cost[i][j] - v[j].value;
					u[i].flags |= U_FLAG_DONE;

					qt->next = &u[i];
					qt = qt->next;
					qt->next = NULL;
				}
		}
		if (cnt == nrow + ncol) break;

		if (qt == &q) break;
		qc = q.next;
		q.next = qc->next;
		if (q.next == NULL) qt = &q;
	}

	assert(cnt == nrow + ncol);

	head = NULL;
	for (i = 0; i < nrow; i++)
		for (j = 0; j < ncol; j++)
		{
			sol[i][j].next = sol[i][j].prev = NULL;
			if (sol[i][j].flow) continue;
			sol[i][j].sigma = cost[i][j] - (u[i].value + v[j].value);
			if (head == NULL || sol[i][j].sigma < head->sigma)
			{
				head = &sol[i][j];
			}
		}

	if (head == NULL) return 1;
	if (head->sigma >= 0) return 1;

	/* search for a loop */

	head->dir = 0;
	head->flow = 1;
	//assert(head->value == 0);
	queue.next = NULL;
	tail = &queue;

	queue.i = head->i;
	queue.j = head->j;

	cur = head;

	for (;;)
	{
		/* find '-' in the col */
		if (cur->dir == 0)
		{
			j = cur->j;
			for (i = 0; i < nrow; i++)
			{
				if (i == cur->i) continue;
				if (!sol[i][j].flow) continue;
				if (sol[i][j].prev != NULL) continue;

				sol[i][j].prev = cur;
				sol[i][j].dir = 1;

				tail->next = &sol[i][j];
				tail = tail->next;
			}
		}
		else
		/* find '+' in the row */
		{

			i = cur->i;
			for (j = 0; j < ncol; j++)
			{
				if (j == cur->j) continue;
				if (sol[i][j].flow == 0) continue;
				if (sol[i][j].prev != NULL) continue;
				
				sol[i][j].prev = cur;
				sol[i][j].dir = 0;

				tail->next = &sol[i][j];
				tail = tail->next;
			}
		}

		cur = queue.next;
		queue.next = cur->next;
		if (cur == tail) tail = &queue;

		if (cur == head) break;
	}

	cur = head;
	min = NULL;

	//printf("(%d,%d)->", cur->i, cur->j);
	for (;;)
	{
		cur = cur->prev;
//		printf("(%d,%d)->", cur->i, cur->j);
		if (min == NULL || cur->value < min->value) min = cur;
		cur = cur->prev;
//		printf("(%d,%d)", cur->i, cur->j);
		if (cur == head) break;
//		printf("->");
	}
//	printf("\n");


	min_flow = min->value;
	assert(min->flow);
	min->flow = 0;

	cur = head;
	for (;;)
	{
		cur->value += min_flow;
		cur = cur->prev;
		cur->value -= min_flow;
		cur = cur->prev;
		if (cur == head) break;
	}

	return 0;
}

void print_sol (int nrow, int ncol, float **cost, struct sol **sol)
{
	float c;
	float sum;
	int i, j;
	printf("SOL:\n");
	c = 0;
	for (i = 0; i < nrow; i++)
	{
		sum = 0;
		for (j = 0; j < ncol; j++)
		{
			if (sol[i][j].flow)
			printf("%.4f\t", sol[i][j].value);
			else
			printf("*.----\t");

			sum += sol[i][j].value;
			c += sol[i][j].value * cost[i][j];
		}
		printf("\t%.4f\n\n", sum);
	}
	printf("\n");
	for (j = 0; j < ncol; j++)
	{
		sum = 0;
		for (i = 0; i < nrow; i++)
		{
			sum += sol[i][j].value;
		}
		printf("%.4f\t", sum);
	}
	printf("\n");
	printf("COST: %.4f\n", c);
}

float tp_cost (int nrow, int ncol, float **cost, struct sol **sol)
{
	int i, j;
	float c = 0;
	for (i = 0; i < nrow; i++)
		for (j = 0; j < ncol; j++)
			if (sol[i][j].flow)	c += cost[i][j] * sol[i][j].value;
	return c;
}


float tp_solve (int nrow, float *row, int ncol, float *col, float **cost)
{
	struct U *u, *v;
	struct sol **sol;
	float c;

	tp_init_vogel(nrow, row, ncol, col, cost, &sol);

//	print_sol(nrow, ncol, cost, sol);
//
	u = type_calloc(struct U, nrow);
	v = type_calloc(struct U, ncol);

	for (;;)
	{
		if (tp_update(nrow, ncol, cost, sol, u, v)) break;
//		if (tp_update(nrow, ncol, cost, sol)) break;
//		print_sol(nrow, ncol, cost, sol);
	}

	free(u);
	free(v);

	c = tp_cost(nrow, ncol, cost, sol);

	matrix_free(sol);

	return c;
}

