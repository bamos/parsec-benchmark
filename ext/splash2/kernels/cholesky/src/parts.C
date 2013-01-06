/*************************************************************************/
/*                                                                       */
/*  Copyright (c) 1994 Stanford University                               */
/*                                                                       */
/*  All rights reserved.                                                 */
/*                                                                       */
/*  Permission is given to use, copy, and modify this software for any   */
/*  non-commercial purpose as long as this copyright notice is not       */
/*  removed.  All other uses, including redistribution in whole or in    */
/*  part, are forbidden without prior written permission.                */
/*                                                                       */
/*  This software is provided with absolutely no warranty and no         */
/*  support.                                                             */
/*                                                                       */
/*************************************************************************/

EXTERN_ENV

#include "matrix.h"
#include <math.h>


struct Chunk {
	int first, last, assign;
	struct Chunk *next;
	} *chunks_head = NULL, *chunks_tail = NULL;

int tolerance = 20;

double domain_ops;

double *divide_lo=NULL, *divide_hi;

extern double *work_tree;
extern int *firstchild, *child;


Partition(M, parts, T, assigned_ops, domain, domains, proc_domains, distribute)
SMatrix M;
int *T, *assigned_ops, *domain, *domains, *proc_domains;
{
  int i, p, start, minm, maxm, ops, change;
  int which=0;
  int *depth;
  double ave, maxo=0.0, maxd;
  struct Chunk *t, *GetChunk(), *NewChunk();

  start = 0;
  for (i=0; i<M.n; i++)
    if (T[i] == M.n) {
      t = NewChunk();
      t->first = start; t->last = i+1;
      AddInOrder(t);
      start = i+1;
    }

    NumberPartition(parts, assigned_ops, 0);

    for (;;) {
      minm = assigned_ops[MinBucket(assigned_ops, parts)];
      maxm = assigned_ops[MaxBucket(assigned_ops, parts)];

      if (maxm == 0 ||
	  (100.0*(maxm - minm)/(double) maxm < tolerance) ||
	  (1.0*parts*maxm/work_tree[M.n] < 0.05))
	break;

      t = GetChunk();

      change = Divide(t);

      if (!change)
	break;

      NumberPartition(parts, assigned_ops, 0);
    }

    NumberPartition(parts, assigned_ops, 1);

  which = 0;
  for (i=0; i<parts; i++) {
    proc_domains[i] = which;

    t = chunks_head;
    while (t) {
      if (t->assign == i) {
        domains[which++] = t->last-1;
      }
      t = t->next;
    }
  }
  proc_domains[parts] = which;

  /* free chunk list */
  while (chunks_head) {
    t = chunks_head;
    chunks_head = chunks_head->next;
    free(t);
  }

  depth = (int *) malloc(proc_domains[parts]*sizeof(int));
  for (p=0; p<parts; p++)
    for (i=proc_domains[p]; i<proc_domains[p+1]; i++)
      depth[i] = 1000000*p-BlDepth(domains[i]);

  SortByKey(proc_domains[p], domains, depth);

  free(depth);

  for (i=0; i<M.n; i++)
    domain[i] = 0;
  for (i=0; i<proc_domains[parts]; i++)
    MarkSubtreeAsDomain(domain, domains[i]);

  domain_ops = 0;
  for (i=0; i<proc_domains[parts]; i++)
    domain_ops += work_tree[domains[i]];

  maxd = 0;
  for (p=0; p<parts; p++) {
    ops = 0;
    for (i=proc_domains[p]; i<proc_domains[p+1]; i++)
      ops += work_tree[domains[i]];
    if (ops > maxd)
      maxd = ops;
  }
  ave = domain_ops/(double) parts;

  printf("Divide for %d P, %d domains, ", parts, proc_domains[parts]);
  printf("%.2f of work static, ", domain_ops/work_tree[M.n]);
  printf("%.2f eff, (%.2f overall)\n", ave/maxd, work_tree[M.n]/maxo/parts);
}


MarkSubtreeAsDomain(domain, root)
int *domain;
{
  int i, first, root_super;
  extern int *node;

  first = root;
  while (firstchild[first] != firstchild[first+1])
    first = child[firstchild[first]];

  /* all nodes not at root have domain[i] = 1 */
  for (i=first; i<=root; i++)
    domain[i] = 1;

  /* root super has domain[i] = 2 */
  root_super = root;
  if (node[root_super] < 0)
    root_super += node[root_super];
  for (i=root_super; i<root_super+node[root_super]; i++)
    domain[i] = 2;
}


NumberPartition(parts, assigned_ops, distribute)
int *assigned_ops;
{
  int i, minm;
  struct Chunk *t, *old_t;

  for (i=0; i<parts; i++)
    assigned_ops[i] = 0;

  t = chunks_head;
  while (t) {
    minm = MinBucket(assigned_ops, parts);
    assigned_ops[minm] += work_tree[t->last-1];
    old_t = t;
    t = t->next;
    if (distribute) {
      old_t->assign = minm;
    }
  }
}

Divide(root)
struct Chunk *root;
{
  int i, first, first_in_super;
  int change = 1;
  struct Chunk *t2, *NewChunk();

  first_in_super = root->last-1;
  while (firstchild[first_in_super]+1 ==
	 firstchild[first_in_super+1]) {
    first_in_super--;
  }

    first = root->first;
    for (i=firstchild[first_in_super];i<firstchild[first_in_super+1]; i++){
      t2 = NewChunk();
      t2->last = child[i]+1;
      t2->first = first;
      AddInOrder(t2);
      first = t2->last;
    }
    free(root);

  return(change);
}


MaxBucket(assigned_ops, parts)
int *assigned_ops;
{
	int i, maxm, ind;

	maxm = assigned_ops[0]; ind = 0;
	for (i=1; i<parts; i++)
		if (assigned_ops[i] > maxm) {
			ind = i; maxm = assigned_ops[i];
			}

	return(ind);
}

MinBucket(assigned_ops, parts)
int *assigned_ops;
{
	int i, minm, ind;

	minm = assigned_ops[0]; ind = 0;
	for (i=1; i<parts; i++)
		if (assigned_ops[i] < minm) {
			ind = i; minm = assigned_ops[i];
			}

	return(ind);
}


struct Chunk *NewChunk()
{
	struct Chunk *t;

	t = (struct Chunk *) malloc(sizeof(struct Chunk));

	return(t);
}



AddInOrder(t)
struct Chunk *t;
{
	struct Chunk *current;
	int work;

	work = work_tree[t->last-1];
	current = chunks_head;
	if (!current) {
		t->next = NULL;
		chunks_head = t;
		}
	else if (work >= work_tree[current->last-1]) {
		t->next = chunks_head;
		chunks_head = t;
		}
	else {
		while (current->next && work<work_tree[current->next->last-1])
			current = current->next;
		t->next = current->next;
		current->next = t;
		}
		
	if (t->next == NULL)
		chunks_tail = t;
}


struct Chunk *GetChunk()
{
	struct Chunk *t;

	t = chunks_head;
	if (t) {
	  chunks_head = t->next;
	  if (t == chunks_tail)
	    chunks_tail = NULL;
	  }

	return(t);
}

