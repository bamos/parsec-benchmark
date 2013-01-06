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

/* this version guarentees that all blocks are zeroed, and it expects
   that all returned blocks are zero */

EXTERN_ENV

#include "matrix.h"

#define ALIGN 8
#define MAXFAST 16
#define MAXFASTBL (1<<(MAXFAST-1))

#define SIZE(block) (block[-1]) /* in all blocks */
#define HOME(block) (block[-2]) /* in all blocks */
#define NEXTFREE(block) (*((unsigned **) block)) /* in free blocks */

struct MemPool {
	LOCKDEC(memoryLock)
	unsigned *volatile*freeBlock;
	int tally, touched, maxm;
	} *mem_pool;

int mallocP = 1, machineP = 1;

extern struct GlobalMemory *Global;


MallocInit(P)
{
  int p;
  
  mallocP = P;

  mem_pool = (struct MemPool *)
    G_MALLOC((mallocP+1)*sizeof(struct MemPool), 0);
  mem_pool++;
  /****** access to mem_pool[-1] is valid ******/

  for (p = -1; p<mallocP; p++)
    InitOneFreeList(p);
}


InitOneFreeList(p)
{
  int j;

  LOCKINIT(mem_pool[p].memoryLock);
  if (p > 0) {
    mem_pool[p].freeBlock = (unsigned **)
      G_MALLOC((MAXFAST+1)*sizeof(unsigned *), p);
    MigrateMem(mem_pool[p].freeBlock, (MAXFAST+1)*sizeof(unsigned *), p);
  }
  else {
    mem_pool[p].freeBlock = (unsigned **)
      G_MALLOC((MAXFAST+1)*sizeof(unsigned *), 0);
    MigrateMem(mem_pool[p].freeBlock, (MAXFAST+1)*sizeof(unsigned *),
	       DISTRIBUTED);
  }
  for (j=0; j<=MAXFAST; j++)
    mem_pool[p].freeBlock[j] = NULL;
  mem_pool[p].tally = mem_pool[p].maxm = mem_pool[p].touched = 0;
}


MallocStats()
{
  int i;

  printf("Malloc max: ");
  for (i=0; i<mallocP; i++)
    if (mem_pool[i].touched > 0)
      printf("%d* ", mem_pool[i].maxm);
    else
      printf("%d ", mem_pool[i].maxm);
  printf("\n");

}


/* returns first bucket where 2^bucket >= size */

FindBucket(size)
{
  int bucket;

  if (size > MAXFASTBL)
    bucket = MAXFAST;
  else {
    bucket = 1;
    while (1<<bucket < size)
      bucket++;
  }

  return(bucket);
}


/* size is in bytes */

char *MyMalloc(size, home)
int size, home;
{
  int i, bucket, leftover, alloc_size;
  unsigned *d, *result, *prev, *freespace, *freelast;
  unsigned int block_size;
  extern int MyNum;

  if (size < ALIGN)
    size = ALIGN;

  if (home == DISTRIBUTED)
    home = -1;

  bucket = FindBucket(size);

  if (bucket < MAXFAST)
    alloc_size = 1<<bucket;
  else
    alloc_size = ((size+ALIGN-1)/ALIGN)*ALIGN;

  result = NULL;

  if (bucket < MAXFAST) {
    if (mem_pool[home].freeBlock[bucket]) {
      LOCK(mem_pool[home].memoryLock)
      result = mem_pool[home].freeBlock[bucket];
      if (result)
	mem_pool[home].freeBlock[bucket] = NEXTFREE(result);
      UNLOCK(mem_pool[home].memoryLock)
    }
  }

  if (!result) {
    LOCK(mem_pool[home].memoryLock)
    prev = NULL;
    d = mem_pool[home].freeBlock[MAXFAST];
    while (d) {

      block_size = SIZE(d);

      if (block_size >= alloc_size) {  /* Found one! */

	leftover = block_size - alloc_size - 2*sizeof(unsigned);
        result = d + (leftover/sizeof(unsigned)) + 2;
	SIZE(result) = alloc_size;
	HOME(result) = home;

	if (leftover > MAXFASTBL) {
	  SIZE(d) = leftover;
	}
	else {
	  /* don't leave a block */

	  /* unlink 'd' */
	  if (prev)
	    NEXTFREE(prev) = NEXTFREE(d);
	  else
	    mem_pool[home].freeBlock[MAXFAST] = NEXTFREE(d);

	  if (leftover > 0) {
	    SIZE(d) = leftover;
	    MyFreeNow(d);
	  }
	}
        break;
      }

      prev = d;
      d = NEXTFREE(d);
    }

    UNLOCK(mem_pool[home].memoryLock)

  }

  if (result) {
    NEXTFREE(result) = 0;  /* in case it was set earlier */
  }
  else {
    /* grab a big block, free it, then retry request */
    block_size = max(alloc_size, 4*(1<<MAXFAST));
    LOCK(Global->memLock);
    freespace = (unsigned *) G_MALLOC(block_size+2*sizeof(unsigned),
					   home);
    MigrateMem(freespace, block_size+2*sizeof(unsigned), home);

    mem_pool[home].touched++;
    UNLOCK(Global->memLock);
    freespace+=2;
    SIZE(freespace) = block_size;
    HOME(freespace) = home;
    for (i=0; i<block_size/sizeof(unsigned); i++)
      freespace[i] = 0;
    if (block_size == alloc_size)
      result = freespace;
    else {
      mem_pool[home].tally += SIZE(freespace);
      if (mem_pool[home].tally > mem_pool[home].maxm) {
	mem_pool[home].maxm = mem_pool[home].tally;
      }
      MyFree(freespace);
      result = (unsigned *) MyMalloc(alloc_size, home);
    }
  }

  mem_pool[home].tally += SIZE(result);
  if (mem_pool[home].tally > mem_pool[home].maxm) {
    mem_pool[home].maxm = mem_pool[home].tally;
  }

  if (SIZE(result) < size)
    printf("*** Bad size from malloc %d, %d\n", size, SIZE(result));

  return((char *) result);

}


MigrateMem(start, length, home)
unsigned int *start;
int length, home;
{
  unsigned int *finish;
  unsigned int currpage, endpage;
  int i;
  int j;

/* POSSIBLE ENHANCEMENT:  Here is where one might distribute the memory
   pages across physically distributed memories as desired.

   One way to do this is as follows:

   finish = (unsigned int *) (((char *) start) + length);

   currpage = (unsigned int) start;
   endpage = (unsigned int) finish;
   if ((home == DISTRIBUTED) || (home < 0) || (home >= mallocP)) {
     j = 0;
     while (currpage < endpage) {

       Place all addresses x such that
         (currpage <= x < currpage + PAGE_SIZE) on node j

       j++;
       currpage += PAGE_SIZE;
     }
   } else {
     Place all addresses x such that
       (currpage <= x < endpage) on node home
   }  
*/
}
    

MyFree(block)
unsigned *block;
{
  int home;

  home = HOME(block);
  LOCK(mem_pool[home].memoryLock)
  MyFreeNow(block);
  UNLOCK(mem_pool[home].memoryLock)
}


MyFreeNow(block)
unsigned *block;
{
  int bucket, size, home;
  extern int MyNum;

  size = SIZE(block);
  home = HOME(block);

  if (size <= 0) {
    printf("Bad size %d\n", size);
    exit(-1);
  }
  if (home < -1 || home >= mallocP) {
    printf("Bad home %d\n", home);
    exit(-1);
  }

  if (size > MAXFASTBL)
    bucket = MAXFAST;
  else {
    bucket = FindBucket(size);
    if (size < 1<<bucket)
      bucket--;
  }

  /* throw away tiny blocks */
  if (bucket == 0)
    return;

  NEXTFREE(block) = (unsigned *) mem_pool[home].freeBlock[bucket];
  mem_pool[home].freeBlock[bucket] = block;
  mem_pool[home].tally -= size;

}

