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

#define SWAP(x, y) {int tmp; tmp=x; x=y; y=tmp;}

double max_block_size;
extern int *node;
int *boundary, *next_in_segment, *next_segment, *sets_affected, n_affected;
int *partition;
int *segment_perm;

ComputeTargetBlockSize(M, P)
SMatrix M;
{
  int max_ht;
  double total_ops;
  extern double *work_tree;

  max_ht = 0;
  FindMaxHeight(M, M.n, 0, &max_ht);

  total_ops = work_tree[M.n];

  max_block_size = sqrt(total_ops/(3*max_ht)/P);

  printf("%d max height, %.0f ops, %.2f conc, %.2f bl for %d P\n",
	 max_ht, total_ops, total_ops/(3*max_ht), max_block_size, P);

}

FindMaxHeight(L, root, height, maxm)
SMatrix L;
int *maxm;
{
  int i;
  extern int *firstchild, *child;

  if (height > *maxm)
    *maxm = height;

  for (i=firstchild[root]; i<firstchild[root+1]; i++)
    FindMaxHeight(L, child[i], height+1, maxm);
}


NoSegments(M)
SMatrix M;
{
  int i;

  partition = (int *) MyMalloc(M.n*sizeof(int), DISTRIBUTED);
  for (i=0; i<M.n; i++)
    partition[i] = node[i];
}


CreatePermutation(n, node, PERM, permutation_method)
int *node, *PERM;
{
  int j, k;
  int swap, tmp;
  extern int *domain;

  PERM[n] = n;
  if (permutation_method == NO_PERM) {
    for (j=0; j<n; j++)
      PERM[j] = j;
  }
}

