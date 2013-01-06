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

#include <stdio.h>

#define FitsInCache 2048

#define MAX_PROC 128
#define PAGE_SIZE 4096

typedef struct {
	int n, m, *col, *startrow, *row;
	double *nz;
	} SMatrix;

struct Pair {
	int block_num;
	struct Pair *next;
	};

typedef struct {
        int i, j, owner, remaining, nmod;
	int length; /* number of full rows in block */
	int parent; /* block number of parent block */
	double checksum;
	volatile unsigned int done;
	int *structure, *relative;
	double *nz;
	struct Pair *pair;
      } Block;

typedef union {
        Block *block;
	double nz;
      } Entry;
       
typedef struct {
        int n, *col, *row, n_blocks, n_entries, entries_allocated;
	int *partition_size, *renumbering, *mapI, *mapJ;
	int *domain, *domains, n_domains, *proc_domains;
	int n_partitions, max_partition;
	double **proc_domain_storage;
	Entry *entry;
      } BMatrix;

#define BLOCK(bl_no) (LB.entry[bl_no].block)
#define BLOCKROW(bl_no) (BLOCK(bl_no)->i)
#define BLOCKCOL(bl_no) (BLOCK(bl_no)->j)
#define OWNER(bl_no) (BLOCK(bl_no)->owner)

struct Update {
	int i, j, src, remaining;
	int dimi, dimj, *structi, *structj;
	double *update;
	struct Update *next;
      };

struct Task {
  int block_num, desti, destj, src;
  struct Update *update;
  struct Task *next;
  };

struct GlobalMemory {
	BARDEC(start)
	LOCKDEC(waitLock)
	LOCKDEC(memLock)
	unsigned long runtime[MAX_PROC];
	};

struct BlockList {
	int theBlock, row, col, length;
	int *structure;
	double *nz;
	struct BlockList *next;
	};

struct LocalCopies {
       double *blktmp;
       int max_panel;
       int *link;
       int *first;
       double *storage;
       double *updatetmp;
       int *relative;
       struct Update *freeUpdate;
       struct Task *freeTask;
       unsigned long rs;
       unsigned long rf;
       unsigned long us;
       unsigned long uf;
       unsigned long ss;
       unsigned long sf;
       unsigned long runtime;
       unsigned long runs;
       };

SMatrix ReadSparse(), NewMatrix();
SMatrix SymbolicFactor();
double *NewVector();
char *MyMalloc();

#define DISTRIBUTED 888

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

#define LB_DOMAINS 2

#define EMBED 1

#define NO_PERM 1

#define FAN_OUT 2

int PreAllocate(int,struct LocalCopies *);
int BNumericSolve(int,struct LocalCopies *);
int DriveParallel(int,struct LocalCopies *);
int DiagReady(int, int, int,struct LocalCopies *);
int LowerReady(int, int, int, int,struct LocalCopies *);
int BFac(int, int,struct LocalCopies *);
int BDiv(int, int, int, int, double *, double *, int,struct LocalCopies *);
int BMod(int, int, int, int, int, double *, double *, double *, int,struct LocalCopies *);
int CopyBlock(double *, double *, int, int, int, int, int, int,struct LocalCopies *);
int BLMod(int, int, int, double *, double *, int,struct LocalCopies *);
int PreProcessFO(int,struct LocalCopies *);
int PreAllocateFO(int,struct LocalCopies *);
int BNumericSolveFO(int,struct LocalCopies *);
int DriveParallelFO(int,struct LocalCopies *);
int DriveSequentialFO(int,struct LocalCopies *);
int HandleTaskFO(int,struct LocalCopies *);
int DiagReceived(int, int,struct LocalCopies *);
int BlockReceived(int, int,struct LocalCopies *);
struct BlockList *CopyOneBlock(int, int,struct LocalCopies *);
int DecrementRemaining(int, int,struct LocalCopies *);
int PerformUpdate(struct BlockList *, struct BlockList *, int,struct LocalCopies *);
int HandleUpdateFO(int, int, struct Update *, int,struct LocalCopies *);
int HandleUpdate2FO(int, int, int, int,struct LocalCopies *);
int BlockReadyFO(int, int,struct LocalCopies *);
int BlockDoneFO(int, int,struct LocalCopies *);
int InitRemainingFO(int,struct LocalCopies *);
int InitReceivedFO(int,struct LocalCopies *);
int RemoveUpdate(int, int, int, struct Update **, int,struct LocalCopies *);
int FindUpdate(int, int, int, struct Update *, int *, struct Update **, int,struct LocalCopies *);
int PrimeQueue(int,struct LocalCopies *);
int Send(int, int, int, int, struct Update *, int, int,struct LocalCopies *);
int TaskWaiting(int,struct LocalCopies *);
int ActiveSpin(volatile unsigned int *, int,struct LocalCopies *);
int GetBlock(int *, int *, int *, struct Update **, int,struct LocalCopies *);
int RemoveUpdate(int, int, int, struct Update **, int,struct LocalCopies *);
int FactorLLDomain(int, int,struct LocalCopies *);
int HandleUpdate(int, int, int, int, double *,struct LocalCopies *);
int HandleDiag(int, int, double *,struct LocalCopies *);
int HandleBlock(int, int, double *,struct LocalCopies *);
int DecrementRemainingSimFO(int, int, double *,struct LocalCopies *);
int BlockReadySimFO(int, int, double *,struct LocalCopies *);
int BlockDoneSimFO(int, int, double *,struct LocalCopies *);
int SendProbe(int, int, int,struct LocalCopies *);
int ReceiveProbe(int, int, int,struct LocalCopies *);
int SendPing(int, int, int,struct LocalCopies *);
int ReceivePing(int, int, int,struct LocalCopies *);
int HandleTask(int,struct LocalCopies *);
int DoOneMod(int, int, int,struct LocalCopies *);
int DoOneLMod(int, int,struct LocalCopies *);
int UpdateFinished(int, int, int, int,struct LocalCopies *);
int PropOne(int, int, int, struct Update *, int,struct LocalCopies *);
int DistributeUpdateFO(int, int,struct LocalCopies *);
int DistributeUpdate(int, int, int,struct LocalCopies *);
int DistributeUpdateOld(double *, int, int, int,struct LocalCopies *);
int FactorMFDomain(int, int,struct LocalCopies *);
int DumpUpdate(int, struct Update *, int,struct LocalCopies *);
char *MyMalloc(int, int);
int SimBlockFO(int, int, double *, double *, double *, struct LocalCopies *);
int AssignBlocks3(int, int, int, struct LocalCopies *);
int FireOffLeafs(double *, struct LocalCopies *);

