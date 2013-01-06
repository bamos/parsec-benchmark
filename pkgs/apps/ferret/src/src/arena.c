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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <cass.h>

#include <arena.h>

#undef MIN
#undef MAX

#define MIN(a, b)  (((a)<(b))?(a):(b))
#define MAX(a, b)  (((a)>(b))?(a):(b))

#define ARENASTATS	1

typedef unsigned int u32int; 
typedef unsigned long long u64int;
/* typedef unsigned char uchar;  */
typedef uintptr_t uintptr;

enum {
	StructAlign = sizeof(union {
				u64int vl;
				double d;
				u32int p;
				void *v;
				struct { u64int v; } vs;
				struct { double d; } ds;
				struct { u32int p; } ss;
				struct { void *v; } xs; })
};

struct MemArena {
	u64int	blksize;
	uchar	*curblk;
	uchar	*curend;

	void	*(*alloc)(size_t);
	void	*(*realloc)(void*, size_t);
	void	(*free)(void*);

	u64int	nalloc;			/* total allocations */
	u64int	totalloc;		/* total memory allocated */
	u64int	totfragb;		/* non-alignment fragmentation */
	u64int	minallocb;		/* minimum allocation size */
	u64int	maxallocb;		/* maximum allocation size */

	uint	nblks;
	uint	nblkalloc;
	uchar	**blktab;
};

MemArena*
mkmemarena(void *(*memalloc)(size_t), void *(*memrealloc)(void*, size_t), void (*memfree)(void*), unsigned long n)
{
	MemArena *a;

	if(n == 0)
		n = 1<<22;
	if(n < 4096 || n > 1<<24)
		return NULL;

	if(memalloc == NULL)
		memalloc = malloc;
	if(memrealloc == NULL)
		memrealloc = realloc;
	if(memfree == NULL)
		memfree = free;

	a = (*memalloc)(sizeof *a);
	if(a == NULL)
		return NULL;
	memset(a, 0, sizeof *a);
	a->blksize = n;
	a->curblk = NULL;
	a->curend = NULL;
	a->alloc = memalloc;
	a->realloc = memrealloc;
	a->free = memfree;
	a->minallocb = ~0;		/* count down */
	a->nblks = 0;
	a->nblkalloc = 1<<3;
	a->blktab = a->alloc(a->nblkalloc*sizeof(a->blktab[0]));
	return a;
}

void
freememarena(MemArena *a)
{
	int i, n;

	if(a == NULL)
		return;

	n = a->nblks;
	for(i=0; i<n; i++)
		a->free(a->blktab[i]);
	a->free(a->blktab);
	a->free(a);
	return;
}

void
memarenastats(MemArena *a)
{
	if(ARENASTATS){
		fprintf(stdout, "%-12s\t%10llu\n", "nalloc", a->nalloc);
		fprintf(stdout, "%-12s\t%10llu\t", "totalloc", a->totalloc);
		fprintf(stdout, "%-12s\t%10llu\n", "totfragb", a->totfragb);
		fprintf(stdout, "%-12s\t%10llu\t", "maxallocb", a->maxallocb);
		fprintf(stdout, "%-12s\t%10llu\n", "minallocb", a->minallocb);
	}
	fprintf(stdout, "%-12s\t%10llu\n", "blksize", a->blksize);
	fprintf(stdout, "%-12s\t%10u\t", "nblks", a->nblks);
	fprintf(stdout, "%-12s\t%10u\n", "nblkalloc", a->nblkalloc);
	return;
}

void*
memarenamalloc(MemArena *a, unsigned long n)
{
	uchar *b, *p;
	uchar **tab;

	if(ARENASTATS){
		a->nalloc++;
		a->totalloc += n;
		a->minallocb = MIN(a->minallocb, n);
		a->maxallocb = MAX(a->maxallocb, n);
#ifdef NOTDEF
		if(ARENASTATS > 1)
			fprintf(stderr, "memarenamalloc %lu 0x%p\n",
				n, ((uintptr*)&a)[-1]);
#endif
	}

	b = a->curblk;
	p = a->curend;
	b = (uchar*)((((uintptr)b) + (StructAlign-1))&~(StructAlign - 1));
	if(b+n > p){
		if(ARENASTATS)
			a->totfragb += p - b;
		if(n > a->blksize)
			goto Fail;
		if(a->nblks == a->nblkalloc){
			a->nblkalloc += 64;
			tab = a->realloc(a->blktab, a->nblkalloc*sizeof(tab[0]));
			if(tab == NULL)
				goto Fail;
			a->blktab = tab;
		}
		b = a->alloc(a->blksize);
		if(b == NULL)
			goto Fail;
		p = b+a->blksize;
		a->curblk = b;
		a->curend = p;
	}
	a->curblk = b+n;
	return b;

  Fail:
	return NULL;
}
