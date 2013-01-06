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

#include <math.h>
#include "matrix.h"

extern int *node;  /* ALL GLOBAL */
extern int postpass_partition_size;
extern int distribute;
BMatrix LB;
extern SMatrix L;
int P_dimi, P_dimj;

/* perform symbolic factorization of original matrix into block form */

CreateBlockedMatrix2(M, block_ub, T, firstchild, child,
	PERM, INVP, domain, partition)
SMatrix M;
int *T, *firstchild, *child, *PERM, *INVP, *domain, *partition;
{
  int i, j, k, p, which, super, n_nz;
  int *structure, *nz;
  Block *blocks;
  extern int P;
  extern int *domains, *proc_domains;
  extern double max_block_size;
  int num_partitions, piece_size, piece, current;

  LB.n = M.n;
  LB.domain = domain;
  LB.domains = domains;
  LB.proc_domains = proc_domains;
  LB.n_domains = proc_domains[P];
  LB.entries_allocated = block_ub+20;

  LB.proc_domain_storage = (double **) MyMalloc(LB.n_domains*sizeof(double *),
						 DISTRIBUTED);
  for (i=0; i<LB.n_domains; i++)
    LB.proc_domain_storage[i] = NULL;

  /* one dummy column for each domain */
  LB.partition_size = (int *) MyMalloc((LB.n+LB.n_domains+1)*sizeof(int),
				    DISTRIBUTED);
  LB.col = (int *) MyMalloc((LB.n+LB.n_domains+1)*sizeof(int), DISTRIBUTED);

  LB.entry = (Entry *) G_MALLOC(LB.entries_allocated*sizeof(Entry),0);
  MigrateMem(LB.entry, LB.entries_allocated*sizeof(Entry), DISTRIBUTED);
  LB.row = (int *) G_MALLOC(LB.entries_allocated*sizeof(int), 0);
  MigrateMem(LB.row, LB.entries_allocated*sizeof(int), DISTRIBUTED);

  FindMachineDimensions(P);

 /* merge partitions */

    for (j=0; j<M.n; j+=node[j]) {
      if (!LB.domain[j]) {
	num_partitions = FindNumPartitions(node[j], postpass_partition_size);
	current = j;
	for (piece=0; piece<num_partitions; piece++) {
	  piece_size = node[j]*(piece+1)/num_partitions -
	    node[j]*piece/num_partitions;
	  for (k=current; k<current+piece_size; k++) {
	    partition[k] = current;
	  }
	  current += piece_size;
	}
      }
    }

  /* determine block sizes */

  j = 0; LB.max_partition = 0; LB.n_partitions = 0;
  while (j < M.n) {
    if (LB.domain[j]) {
      LB.partition_size[j] = 1;
      j++;
    }
    else {
      k = j;
      while (partition[k] == j && k < M.n)
	k++;
      LB.partition_size[j] = k-j;
      for (i=j+1; i<k; i++)
	LB.partition_size[i] = j-i;

      if (LB.partition_size[j] > LB.max_partition)
        LB.max_partition = LB.partition_size[j];
      LB.n_partitions++;

      j = k;
    }
  }
  for (j=0; j<LB.n_domains; j++)
    LB.partition_size[LB.n+j] = 1;
  if (LB.n_partitions == 0)
    LB.n_partitions = 1;

  /* determine numbering based on partitions only */
  LB.renumbering = (int *) MyMalloc((LB.n+LB.n_domains)*sizeof(int),
				     DISTRIBUTED);
  ComputePartitionNumbering(LB.renumbering);
  for (j=0; j<LB.n_domains; j++)
    LB.renumbering[LB.n+j] = j%LB.n_partitions;

  /* determine mapping of rows/columns of blocks to rows/columns of procs */
  LB.mapI = (int *) MyMalloc(LB.n_partitions*sizeof(int), DISTRIBUTED);
  LB.mapJ = (int *) MyMalloc(LB.n_partitions*sizeof(int), DISTRIBUTED);
  for (i=0; i<LB.n_partitions; i++)
    LB.mapI[i] = LB.mapJ[i] = i;
  printf("No redistribution\n");

  printf("Supers: "); DumpSizes(LB, domain, node);
  printf("Blocks: "); DumpSizes(LB, domain, LB.partition_size);
  printf("%d partitions\n", LB.n_partitions);

  structure = (int *) malloc(M.n*sizeof(int));
  nz = (int *) malloc(M.n*sizeof(int));
  for (i=0; i<M.n; i++)
    structure[i] = 0;

  /* find the block structure of the factor */
  LB.col[0] = 0;
  /* first the domains... */
  for (super=0; super<LB.n; super+=node[super])
    if (domain[super]) {
      FindSuperStructure(M, super, PERM, INVP, firstchild, child,
			 structure, nz, &n_nz);
      FindDomStructure(super, nz, n_nz);
    }
    else {
      for (j=super; j<super+node[super]; j+=LB.partition_size[j]) {
	FindBlStructure(M, j, PERM, INVP, firstchild, child,
			structure, nz);

	/* fill in rest of partition */
	for (i=j+1; i<j+LB.partition_size[j]; i++)
	  LB.col[i+1] = LB.col[i];
      }
    }
  /* ...then the domain dummies */
  for (j=0; j<LB.n_domains; j++)
    FindDummyDomainStructure(j);

  LB.n_entries = LB.col[LB.n+LB.n_domains];
  free(structure); free(nz);

  /* place domain entries on owner clusters */
  PlaceDomains(P);

  /* count how many blocks are needed */
  LB.n_blocks = 0;
  for (j=0; j<LB.n; j+=LB.partition_size[j])
    if (!LB.domain[j])
      LB.n_blocks += (LB.col[j+1]-LB.col[j]);
  for (j=0; j<LB.n_domains; j++)
    LB.n_blocks += (LB.col[LB.n+j+1]-LB.col[LB.n+j]);

  printf("%d partitions, %d blocks\n", LB.n_partitions, LB.n_blocks);

  /* now allocate storage for blocks and fill in simple info */
  blocks = (Block *) MyMalloc(LB.n_blocks*sizeof(Block), DISTRIBUTED);
  which = 0;
  for (j=0; j<LB.n; j+=LB.partition_size[j]) {
    if (!LB.domain[j])
      for (i=LB.col[j]; i<LB.col[j+1]; i++) {
	BLOCK(i) = &blocks[which];
        BLOCK(i)->i = LB.row[i];
        BLOCK(i)->j = j;
	if (LB.renumbering[BLOCK(i)->i] < 0 ||
	    LB.renumbering[BLOCK(i)->j] < 0) {
	  printf("Block %d has bad structure\n");
	  exit(-1);
	}
	BLOCK(i)->done = 0;
	BLOCK(i)->pair = NULL;
	which++;
      }
  }
  /* and for domain dummies */
  for (p=0; p<P; p++)
    for (j=LB.proc_domains[p]; j<LB.proc_domains[p+1]; j++)
      for (i=LB.col[LB.n+j]; i<LB.col[LB.n+j+1]; i++) {
	BLOCK(i) = &blocks[which];
	BLOCK(i)->i = LB.row[i];
        BLOCK(i)->j = LB.n+j;
	BLOCK(i)->nz = NULL;
	BLOCK(i)->owner = p;
	BLOCK(i)->done = 0;
	BLOCK(i)->pair = NULL;
	which++;
      }

  ComputeBlockParents(T);

}

FindNumPartitions(set_size, piece_size)
{
  int num_partitions;

  if (set_size <= 4*piece_size/3)
    num_partitions = 1;
  else {
    num_partitions = (set_size+piece_size-1)/piece_size;
    if (piece_size - set_size/num_partitions >
	set_size/(num_partitions-1) - piece_size)
      num_partitions--;
  }

  return(num_partitions);
}


ComputeBlockParents(T)
int *T;
{
  int b, i, parent_col;

  /* compute block parents */

  for (b=0; b<LB.n; b+=LB.partition_size[b])
    if (!LB.domain[b]) {
      for (i=LB.col[b]; i<LB.col[b+1]; i++) {
	parent_col = T[b+LB.partition_size[b]-1];
	if (parent_col == LB.n)
	  BLOCK(i)->parent = -1;
	else if (BLOCK(i)->i <= BLOCK(i)->j)
	  BLOCK(i)->parent = -1; /* above diag */
	else {
	  BLOCK(i)->parent = FindBlock(BLOCK(i)->i,
						parent_col);
	  if (BLOCK(i)->parent == -1)
	    printf("Parent not found\n");
	}
      }
    }

  /* find parents for domain dummy blocks */
  for (b=0; b<LB.n_domains; b++)
    for (i=LB.col[LB.n+b]; i<LB.col[LB.n+b+1]; i++) {
      parent_col = T[LB.domains[b]];
      BLOCK(i)->parent = FindBlock(BLOCK(i)->i,
					    parent_col);
    }
}


/* find non-zero structure of individual blocks */

FillInStructure(M, firstchild, child, PERM, INVP)
SMatrix M;
int *firstchild, *child, *PERM, *INVP;
{
  int i, j, col, super;
  int *structure, *nz, n_nz;

  /* all procedures get structure=0, and return structure=0 */
  structure = (int *) malloc(M.n*sizeof(int));
  nz = (int *) malloc(M.n*sizeof(int));
  for (i=0; i<M.n; i++)
    structure[i] = 0;

  /* find the structure of dummy domain blocks */
  for (j=0; j<LB.n_domains; j++) {
    col = LB.domains[j];
    for (i=LB.col[col]+1; i<LB.col[col+1]; i++)
      nz[i-LB.col[col]-1] = LB.row[i];
    n_nz = LB.col[col+1]-LB.col[col]-1;
    FindDetailedStructure(LB.n+j, structure, nz, n_nz);
  }

  /* find the structure of the individual blocks */
  for (super=0; super<LB.n; super+=node[super]) {
    FindSuperStructure(M, super, PERM, INVP, firstchild, child,
		       structure, nz, &n_nz);
    if (!LB.domain[super])
      for (j=super; j<super+node[super]; j+=LB.partition_size[j]) {
	FindDetailedStructure(j, structure, nz, n_nz);
      }
  }

  free(structure); free(nz);

}

/* put original non-zero values into blocks */

FillInNZ(M, PERM, INVP)
SMatrix M;
int *PERM, *INVP;
{
  int i, j;
  double *scatter;

  scatter = (double *) malloc(M.n*sizeof(double));
  for (j=0; j<M.n; j++)
    scatter[j] = 0.0;

  /* fill in non-zeroes */  
  for (j=0; j<LB.n; j+=LB.partition_size[j])
    FillIn(M, j, PERM, INVP, scatter);

  free(scatter);
}


FindDomStructure(super, nz, n_nz)
int *nz;
{
  int col, i;

  for (col=super; col<super+node[super]; col++) {
    LB.col[col+1] = LB.col[col] + n_nz - (col-super);
    if (LB.col[col+1] > LB.entries_allocated) {
      printf("Overflow\n");
      exit(-1);
    }

    for (i=col-super; i<n_nz; i++)
      LB.row[LB.col[col]+i-(col-super)] = nz[i];
  }
}

FindDummyDomainStructure(which_domain)
{
  int col, row, current_block, current_block_last;

  col = LB.domains[which_domain];

  current_block = current_block_last = -1;
  row = LB.col[col]+1;

  /* column starts out empty */
  LB.col[LB.n+which_domain+1] = LB.col[LB.n+which_domain];

  while (row < LB.col[col+1]) {
    current_block = LB.row[row];
    if (LB.partition_size[current_block] < 0)
      current_block += LB.partition_size[current_block];
    current_block_last = current_block + LB.partition_size[current_block];
    LB.row[LB.col[LB.n+which_domain+1]] = current_block;
    LB.col[LB.n+which_domain+1]++;
    while (LB.row[row] >= current_block &&
	   LB.row[row] < current_block_last &&
	   row < LB.col[col+1])
      row++;
  }

  if (LB.col[LB.n+which_domain+1] > LB.entries_allocated) {
    printf("Overflow!!\n");
    exit(-1);
  }
}


CheckColLength(col, n_nz)
{
  extern int *nz;

  if (n_nz != nz[col])
    printf("Col %d: %d vs %d\n", col, n_nz, nz[col]);
}


FindBlStructure(M, super, PERM, INVP, firstchild, child, structure, nz)
SMatrix M;
int *PERM, *INVP, *firstchild, *child, *structure, *nz;
{
  int truecol, i, c, col, the_child, bl, n_nz;

  n_nz = 0;
  for (col=super; col<super+node[super]; col++) {
    truecol = PERM[col];
    for (i=M.col[truecol]; i<M.col[truecol+1]; i++) {
      bl = INVP[M.row[i]];
      if (LB.partition_size[bl] < 0)
	bl += LB.partition_size[bl];
      if (bl >= super && !structure[bl]) {
	structure[bl] = 1;
	nz[n_nz++] = bl;
      }
    }
  }

  for (c=firstchild[super]; c<firstchild[super+1]; c++) {
    the_child = child[c];
    if (LB.partition_size[the_child] < 0)
      the_child += LB.partition_size[the_child];
    for (i=LB.col[the_child]; i<LB.col[the_child+1]; i++) {
      bl = LB.row[i];
      if (LB.partition_size[bl] < 0)
	bl += LB.partition_size[bl];
      if (bl >= super && !structure[bl]) {
	structure[bl] = 1;
	nz[n_nz++] = bl;
      }
    }
  }

  /* reset structure[] to zero */
  for (i=0; i<n_nz; i++)
    structure[nz[i]] = 0;

  InsSort(nz, n_nz);

  LB.col[super+1] = LB.col[super] + n_nz;
  if (LB.col[super+1] > LB.entries_allocated) {
    printf("Overflow\n");
    exit(-1);
  }
  for (i=0; i<n_nz; i++)
    LB.row[LB.col[super]+i] = nz[i];

}


FindSuperStructure(M, super, PERM, INVP, firstchild, child,
		   structure, nz, n_nz)
SMatrix M;
int *PERM, *INVP, *firstchild, *child, *structure, *nz, *n_nz;
{
  int i, truecol, current, bl, c, the_child, row;

  *n_nz = 0;

  /* first add non-zeroes that come from columns within block */
  for (current=super; current<super+node[super]; current++) {
    truecol = PERM[current];
    for (i=M.col[truecol]; i<M.col[truecol+1]; i++) {
      row = INVP[M.row[i]];
      if (row >= super && !structure[row]) {
	structure[row] = 1;
	nz[(*n_nz)++] = row;
      }
    }
  }

  /* then add non-zeroes that come from children */
  for (c=firstchild[super]; c<firstchild[super+1]; c++) {
    the_child = child[c];
    if (LB.partition_size[the_child] < 0)
      the_child += LB.partition_size[the_child];
    if (LB.domain[the_child]) {
      for (i=LB.col[the_child]; i<LB.col[the_child+1]; i++) {
        row = LB.row[i];
        if (row >= super && !structure[row]) {
	  structure[row] = 1;
	  nz[(*n_nz)++] = row;
        }
      }
    }
    else {
      for (i=LB.col[the_child]; i<LB.col[the_child+1]; i++) {
          for (bl=0; bl<BLOCK(i)->length; bl++) {
	    if (BLOCK(i)->structure)
	      row = LB.row[i]+BLOCK(i)->structure[bl];
	    else
	      row = LB.row[i]+bl;
            if (row >= super && !structure[row]) {
	      structure[row] = 1;
	      nz[(*n_nz)++] = row;
            }
	  }
        }
    }
  }

  for (i=0; i<*n_nz; i++)
    structure[nz[i]] = 0;

  InsSort(nz, *n_nz);

  CheckColLength(super, *n_nz);

}


FindDetailedStructure(col, structure, nz, n_nz)
int *structure, *nz;
{
  int i, j, row, n, owner;

  for (i=0; i<n_nz; i++)
    structure[nz[i]] = 1;

  for (i=LB.col[col]; i<LB.col[col+1]; i++) {
    n = 0;
    row = LB.row[i];
    for (j=0; j<LB.partition_size[row]; j++)
      if (structure[row+j])
        n++;

    BLOCK(i)->length = n;
    if (n == LB.partition_size[row]) {
      BLOCK(i)->structure = NULL;
    }
    else {
      owner = EmbeddedOwner(i);
      if (owner < 0)
	printf("%d,%d: %d\n", BLOCKROW(i), BLOCKCOL(i), owner);
      BLOCK(i)->structure = (int *) MyMalloc(n*sizeof(int), owner);
      n = 0;
      for (j=0; j<LB.partition_size[row]; j++)
        if (structure[row+j])
          BLOCK(i)->structure[n++] = j;

    }
  }

  for (i=0; i<n_nz; i++)
    structure[nz[i]] = 0;
}


AllocateNZ()
{
  int i, j, b, size;

  for (j=0; j<LB.n; j+=LB.partition_size[j])
    if (!LB.domain[j]) {
      for (b=LB.col[j]; b<LB.col[j+1]; b++) {
	size = LB.partition_size[j]*BLOCK(b)->length;
	BLOCK(b)->nz = (double *) MyMalloc(size*sizeof(double),
					    BLOCK(b)->owner);
	for (i=0; i<size; i++)
	  BLOCK(b)->nz[i] = 0.0;
      }
    }
}



FillIn(M, col, PERM, INVP, scatter)
SMatrix M;
int *PERM, *INVP;
double *scatter;
{
  int i, b, j1, row, truecol;
  double Value();

  truecol = PERM[col];
  if (LB.domain[col]) {
    for (i=M.col[truecol]; i<M.col[truecol+1]; i++) {
      row = INVP[M.row[i]];
      if (row >= col) {
	if (M.nz)
	  scatter[row] = M.nz[i];
	else
	  scatter[row] = Value(M.row[i], truecol, M.n);
      }
	
    }
    for (i=LB.col[col]; i<LB.col[col+1]; i++) {
      LB.entry[i].nz = scatter[LB.row[i]];
      scatter[LB.row[i]] = 0.0;
      }
  }
  else {

    for (j1=0; j1<LB.partition_size[col]; j1++) {
      truecol = PERM[col+j1];
      for (i=M.col[truecol]; i<M.col[truecol+1]; i++) {
	row = INVP[M.row[i]];
	if (row >= col+j1) {
	  if (M.nz)
	    scatter[row] = M.nz[i];
	  else
	    scatter[row] = Value(M.row[i], truecol, M.n);
	}
      }
      for (b=LB.col[col]; b<LB.col[col+1]; b++) {
	for (i=0; i<BLOCK(b)->length; i++) {
	  if (BLOCK(b)->structure)
	    row = LB.row[b] + BLOCK(b)->structure[i];
	  else row = LB.row[b] + i;
	  BLOCK(b)->nz[i+j1*BLOCK(b)->length] =
		scatter[row];
	  scatter[row] = 0.0;
	}
      }
    }
  }
}


InsSort(nz, n)
int *nz;
{
  int i, j, tmp;

  for (i=1; i<n; i++) {
    j = i;
    while (j>0 && nz[j-1] > nz[j]) {
      tmp = nz[j]; nz[j] = nz[j-1]; nz[j-1] = tmp; j--;
    }
  }
}



/* determine relative indices for all blocks */

BlDepth(col)
{
  int current, depth;
  extern int *T;

  depth = 0;
  current = col;
  while (T[current] != current) {
    current = T[current];
    depth++;
  }

  return(depth);
}

/* must be stable, blocks in same column must remain in sorted order */
SortByKey(n, blocks, keys)
int *blocks, *keys;
{
  int i, j, blocki, keyi;

  for (i=0; i<n; i++) {
    blocki = blocks[i];
    keyi = keys[i];
    j = i;
    while ((j > 0) && (keys[j-1] > keyi)) {
      blocks[j] = blocks[j-1];
      keys[j] = keys[j-1];
      j--;
    }
    blocks[j] = blocki;
    keys[j] = keyi;
  }
}


/* must be stable, blocks in same column must remain in sorted order */

DumpSizes(LB, domain, sizes)
BMatrix LB;
int *domain, *sizes;
{
  int i, *buckets, maxm;

  maxm = 0;
  for (i=0; i<LB.n; i+=sizes[i])
    if (!domain[i] && sizes[i] > maxm)
      maxm = sizes[i];

  buckets = (int *) malloc((maxm+1)*sizeof(int));
  for (i=0; i<=maxm; i++)
    buckets[i] = 0;

  for (i=0; i<LB.n; i+=sizes[i])
    if (!domain[i])
      buckets[sizes[i]]++;

  for (i=0; i<=maxm; i++) {
    if (buckets[i] == 0)
      ;
    else
      printf("%d: %d  ", i, buckets[i]);
  }
  printf("\n");

  free(buckets);
}


ComputePartitionNumbering(numbering)
int *numbering;
{
  int j, which;

  for (j=0; j<LB.n; j++)
    numbering[j] = -1;

  which = 0;
  for (j=0; j<LB.n; j+=LB.partition_size[j])
    if (!LB.domain[j])
      numbering[j] = which++;
}


/* factor P */
FindMachineDimensions(P)
{
  int try, div;

  for (try=(int) sqrt((double) P); try>0; try--) {
    div = P/try;
    if (div*try == P)
      break;
  }

  P_dimi = div; P_dimj = try;
  printf("Processor array is %d by %d\n", P_dimi, P_dimj);
}


EmbeddedOwner(block)
{
  int row, col;

  row = LB.mapI[LB.renumbering[BLOCKROW(block)]] % P_dimi;
  col = LB.mapJ[LB.renumbering[BLOCKCOL(block)]] % P_dimj;

  return(row + col*P_dimi);
}

