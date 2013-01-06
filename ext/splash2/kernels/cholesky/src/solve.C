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

/*************************************************************************/
/*                                                                       */
/*  Sparse Cholesky Factorization (Fan-Out with no block copy-across)    */
/*                                                                       */
/*  Command line options:                                                */
/*                                                                       */
/*  -pP : P = number of processors.                                      */
/*  -Bb : Use a postpass partition size of b.                            */
/*  -Cc : Cache size in bytes.                                           */
/*  -s  : Print individual processor timing statistics.                  */
/*  -t  : Test output.                                                   */
/*  -h  : Print out command line options.                                */
/*                                                                       */
/*  Note: This version works under both the FORK and SPROC models        */
/*                                                                       */
/*************************************************************************/

MAIN_ENV

#include <math.h>
#include "matrix.h"

#define SH_MEM_AMT   100000000
#define DEFAULT_PPS         32
#define DEFAULT_CS       16384
#define DEFAULT_P            1

double CacheSize = DEFAULT_CS;
double CS;
int BS = 45;
void Go();

struct GlobalMemory *Global;

int *T, *nz, *node, *domain, *domains, *proc_domains;

int *PERM, *INVP;

int solution_method = FAN_OUT*10+0;

int distribute = -1;

int target_partition_size = 0;
int postpass_partition_size = DEFAULT_PPS;
int permutation_method = 1;
int join = 1; /* attempt to amalgamate supernodes */
int scatter_decomposition = 0;

int P=DEFAULT_P;
int iters = 1;
SMatrix M;      /* input matrix */

char probname[80];

extern struct Update *freeUpdate[MAX_PROC];
extern struct Task *freeTask[MAX_PROC];

struct gpid {
  int pid;
  unsigned long initdone;
  unsigned long finish;
} *gp;

int do_test = 0;
int do_stats = 0;

main(argc, argv)
char *argv[];
{
  SMatrix L;
  double *b, *x, *CreateVector(), *TriangularSolve(), *TriBSolve();
  double norm, ComputeNorm();
  unsigned int elapsed;
  extern BMatrix LB;
  int i;
  int c;
  int *assigned_ops, num_nz, num_domain, num_alloc, ps;
  int *PERM2;
  extern char *optarg;
  extern int *firstchild, *child, *nz, *node;
  extern double *work_tree;
  extern int *domain, *partition;
  extern int *block_start, *all_blocks;
  unsigned long start;
  double mint, maxt, avgt;

  CLOCK(start)

  while ((c = getopt(argc, argv, "B:C:p:D:sth")) != -1) {
    switch(c) {
    case 'B': postpass_partition_size = atoi(optarg); break;  
    case 'C': CacheSize = (double) atoi(optarg); break;  
    case 'p': P = atoi(optarg); break;  
    case 's': do_stats = 1; break;  
    case 't': do_test = 1; break;  
    case 'h': printf("Usage: SCHOL <options> file\n\n");
              printf("options:\n");
              printf("  -Bb : Use a postpass partition size of b.\n");
              printf("  -Cc : Cache size in bytes.\n");
              printf("  -pP : P = number of processors.\n");
              printf("  -s  : Print individual processor timing statistics.\n");
              printf("  -t  : Test output.\n");
              printf("  -h  : Print out command line options.\n\n");
              printf("Default: SCHOL -p%1d -B%1d -C%1d\n",
                     DEFAULT_P,DEFAULT_PPS,DEFAULT_CS);
              exit(0);
              break;
    }
  }

  CS = CacheSize / 8.0;
  CS = sqrt(CS);
  BS = (int) floor(CS+0.5);

  MAIN_INITENV(, SH_MEM_AMT)

  gp = (struct gpid *) G_MALLOC(sizeof(struct gpid),0);
  gp->pid = 0;
  Global = (struct GlobalMemory *)
    G_MALLOC(sizeof(struct GlobalMemory), 0);
  BARINIT(Global->start)
  LOCKINIT(Global->waitLock)
  LOCKINIT(Global->memLock)

  MallocInit(P);  

  i = 0;
  while (++i < argc && argv[i][0] == '-')
    ;
  M = ReadSparse(argv[i], probname);

  distribute = LB_DOMAINS*10 + EMBED;

  printf("\n");
  printf("Sparse Cholesky Factorization\n");
  printf("     Problem: %s\n",probname);
  printf("     %d Processors\n",P);
  printf("     Postpass partition size: %d\n",postpass_partition_size);
  printf("     %0.0f byte cache\n",CacheSize);
  printf("\n");
  printf("\n");

  printf("true partitions\n");

  printf("Fan-out, ");
  printf("no block copy-across\n");

  printf("LB domain, "); 
  printf("embedded ");
  printf("distribution\n");

  printf("No ordering\n");

  PERM = (int *) MyMalloc((M.n+1)*sizeof(int), DISTRIBUTED);
  INVP = (int *) MyMalloc((M.n+1)*sizeof(int), DISTRIBUTED);

  CreatePermutation(M.n, (int *) NULL, PERM, NO_PERM);

  InvertPerm(M.n, PERM, INVP);

  T = (int *) MyMalloc((M.n+1)*sizeof(int), DISTRIBUTED);
  EliminationTreeFromA(M, T, PERM, INVP);

  firstchild = (int *) MyMalloc((M.n+2)*sizeof(int), DISTRIBUTED);
  child = (int *) MyMalloc((M.n+1)*sizeof(int), DISTRIBUTED);
  ParentToChild(T, M.n, firstchild, child);

  nz = (int *) MyMalloc((M.n+1)*sizeof(int), DISTRIBUTED);
  ComputeNZ(M, T, nz, PERM, INVP);

  work_tree = (double *) MyMalloc((M.n+1)*sizeof(double), DISTRIBUTED);
  ComputeWorkTree(M, nz, work_tree);

  node = (int *) MyMalloc((M.n+1)*sizeof(int), DISTRIBUTED);
  FindSupernodes(M, T, nz, node, PERM, INVP);

  Amalgamate2(1, M, T, nz, node, (int *) NULL, 1);


  assigned_ops = (int *) malloc(P*sizeof(int));
  domain = (int *) MyMalloc(M.n*sizeof(int), DISTRIBUTED);
  domains = (int *) MyMalloc(M.n*sizeof(int), DISTRIBUTED);
  proc_domains = (int *) MyMalloc((P+1)*sizeof(int), DISTRIBUTED);
  printf("before partition\n");
  fflush(stdout);
  Partition(M, P, T, assigned_ops, domain, domains, proc_domains, distribute);
  free(assigned_ops);

  {
    int i, tot_domain_updates, tail_length;

    tot_domain_updates = 0;
    for (i=0; i<proc_domains[P]; i++) {
      tail_length = nz[domains[i]]-1;
      tot_domain_updates += tail_length*(tail_length+1)/2;
    }
    printf("%d total domain updates\n", tot_domain_updates);
  }

  num_nz = num_domain = 0;
  for (i=0; i<M.n; i++) {
    num_nz += nz[i];
    if (domain[i])
      num_domain += nz[i];
  }
  
  ComputeTargetBlockSize(M, P);

  printf("Target partition size %d, postpass size %d\n",
	 target_partition_size, postpass_partition_size);

  NoSegments(M);

  PERM2 = (int *) malloc((M.n+1)*sizeof(int));
  CreatePermutation(M.n, node, PERM2, permutation_method);
  ComposePerm(PERM, PERM2, M.n);
  free(PERM2);

  InvertPerm(M.n, PERM, INVP);

  b = CreateVector(M);

  ps = postpass_partition_size;
  num_alloc = num_domain + (num_nz-num_domain)*10/ps/ps;
  CreateBlockedMatrix2(M, num_alloc, T, firstchild, child, PERM, INVP,
		       domain, partition);

  FillInStructure(M, firstchild, child, PERM, INVP);

  AssignBlocksNow(distribute);  /* distribute == 21 */

  AllocateNZ();

  FillInNZ(M, PERM, INVP);
  FreeMatrix(M);

  InitTaskQueues(P);

  PreAllocate1FO(0);
  ComputeRemainingFO();
  ComputeReceivedFO();

  //for (i=1; i<P; i++) {
  CREATE(Go, P)
  //}

  //Go();

  WAIT_FOR_END(P)

  printf("%.0f operations for factorization\n", work_tree[M.n]);

  printf("\n");
  printf("                            PROCESS STATISTICS\n");
  printf("              Total\n");
  printf(" Proc         Time \n");
  printf("    0    %10.0d\n", Global->runtime[0]);
  if (do_stats) {
    maxt = avgt = mint = Global->runtime[0];
    for (i=1; i<P; i++) {
      if (Global->runtime[i] > maxt) {
        maxt = Global->runtime[i];
      }
      if (Global->runtime[i] < mint) {
        mint = Global->runtime[i];
      }
      avgt += Global->runtime[i];
    }
    avgt = avgt / P;
    for (i=1; i<P; i++) {
      printf("  %3d    %10ld\n",i,Global->runtime[i]);
    }
    printf("  Avg    %10.0f\n",avgt);
    printf("  Min    %10.0f\n",mint);
    printf("  Max    %10.0f\n",maxt);
    printf("\n");
  }

  printf("                            TIMING INFORMATION\n");
  printf("Start time                        : %16ld\n",
          start);
  printf("Initialization finish time        : %16ld\n",
          gp->initdone);
  printf("Overall finish time               : %16ld\n",
          gp->finish);
  printf("Total time with initialization    : %16ld\n",
          gp->finish-start);
  printf("Total time without initialization : %16ld\n",
          gp->finish-gp->initdone);
  printf("\n");

  if (do_test) {
    printf("                             TESTING RESULTS\n");
    x = TriBSolve(LB, b, PERM, INVP);
    norm = ComputeNorm(x, LB.n);
    if (norm >= 0.0001) {
      printf("Max error is %10.9f\n", norm);
    } else {
      printf("PASSED\n");
    }
  }

  MAIN_END
}


void Go()
{
  int iter;
  int MyNum;
  struct LocalCopies *lc;

  LOCK(Global->waitLock)
    MyNum = gp->pid;
    gp->pid++;
  UNLOCK(Global->waitLock)

/* POSSIBLE ENHANCEMENT:  Here is where one might pin processes to
   processors to avoid migration */

  lc =(struct LocalCopies *) G_MALLOC(sizeof(struct LocalCopies)+2*PAGE_SIZE,
              			       MyNum)
  lc->freeUpdate = NULL;
  lc->freeTask = NULL;
  lc->runtime = 0;

  PreAllocateFO(MyNum,lc);

    /* initialize - put original non-zeroes in L */

  PreProcessFO(MyNum,lc);

  BARRIER(Global->start, P);

/* POSSIBLE ENHANCEMENT:  Here is where one might reset the
   statistics that one is measuring about the parallel execution */

  if ((MyNum == 0) || (do_stats)) {
    CLOCK(lc->rs);
  }

  BNumericSolveFO(MyNum,lc);

  BARRIER(Global->start, P);

  if ((MyNum == 0) || (do_stats)) {
    CLOCK(lc->rf);
    lc->runtime += (lc->rf-lc->rs);
  }

  if (MyNum == 0) {
    CheckRemaining();
    CheckReceived();
  }

  BARRIER(Global->start, P);

  if ((MyNum == 0) || (do_stats)) {
    Global->runtime[MyNum] = lc->runtime;
  }
  if (MyNum == 0) {
    gp->initdone = lc->rs;
    gp->finish = lc->rf;
  } 

  BARRIER(Global->start, P);

}


PlaceDomains(P)
{
  int p, d, first;
  char *range_start, *range_end;
  int page;
  extern int *firstchild, *child;
  extern BMatrix LB;

  for (p=P-1; p>=0; p--)
    for (d=LB.proc_domains[p]; d<LB.proc_domains[p+1]; d++) {
      first = LB.domains[d];
      while (firstchild[first] != firstchild[first+1])
        first = child[firstchild[first]];

      /* place indices */
      range_start = (char *) &LB.row[LB.col[first]];
      range_end = (char *) &LB.row[LB.col[LB.domains[d]+1]];
      MigrateMem(&LB.row[LB.col[first]],
		 (LB.col[LB.domains[d]+1]-LB.col[first])*sizeof(int),
		 p);

      /* place non-zeroes */
      range_start = (char *) &BLOCK(LB.col[first]);
      range_end = (char *) &BLOCK(LB.col[LB.domains[d]+1]);
      MigrateMem(&BLOCK(LB.col[first]),
		 (LB.col[LB.domains[d]+1]-LB.col[first])*sizeof(double),
		 p);
    }
}



/* Compute result of first doing PERM1, then PERM2 (placed back in PERM1) */

ComposePerm(PERM1, PERM2, n)
int *PERM1, *PERM2, n;
{
  int i, *PERM3;

  PERM3 = (int *) malloc((n+1)*sizeof(int));

  for (i=0; i<n; i++)
    PERM3[i] = PERM1[PERM2[i]];

  for (i=0; i<n; i++)
    PERM1[i] = PERM3[i];

  free(PERM3);
}

