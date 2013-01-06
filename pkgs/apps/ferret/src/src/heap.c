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
#include <stdlib.h>
#include <string.h>
#include "heap.h"

typedef unsigned int u32int;
#define nil NULL
#define print printf


struct Heap {
	int	size;
	int	nheap;
	int	mheap;
	u32int	*heap;
	int	(*compare)(void*, void*);
};

#define swap(a, b, n)	{		\
	int __k;			\
	u32int __tmp;			\
	for(__k=n; __k>0; __k--){	\
		__tmp = *a;		\
		*a++ = *b;		\
		*b++ = __tmp;		\
	}				\
}

Heap*
mkheap(int (*compare)(void*, void*), int k, int m)
{
	Heap *heap;

	heap = malloc(sizeof *heap);
	if(heap == nil)
		return nil;
	heap->size = k/4;
	heap->nheap = 0;
	heap->mheap = m;
	heap->compare = compare;
	heap->heap = malloc(heap->mheap*4*heap->size);
	if(heap->heap == nil){
		free(heap);
		return nil;
	}
	memset(heap->heap, 0, heap->mheap*4*heap->size);
	return heap;
}

void
freeheap(Heap *heap)
{
	if(heap == nil)
		return;
	free(heap->heap);
	free(heap);
	return;
}

void
heapreset(Heap *heap)
{
	heap->nheap = 0;
	return;
}

int
heapsize(Heap *heap)
{
	return heap->nheap;
}

int
heapmaxsize(Heap *heap)
{
	return heap->mheap;
}

int
heapinsert(Heap *heap, void *v)
{
	int i;
	int k, p;
	u32int *h;
	u32int *a, *b;
	int (*compare)(void*, void*);

	if(heap->nheap == heap->mheap)
		return -1;

	k = heap->size;
	h = heap->heap;
	compare = heap->compare;

	i = heap->nheap++;
	a = h+i*k;
	memmove(a, v, k<<2);

	for(;;){
		if(i == 0)
			break;
		p = (i-1)/2;
		a = h+i*k;
		b = h+p*k;
		if(compare(a, b) > 0)
			break;
		swap(a, b, k);
		i = p;
	}

	return 0;
}

void*
heapmin(Heap *heap)
{
	if(heap->nheap == 0)
		return nil;
	return heap->heap;
}

int
heapextractmin(Heap *heap, void *v)
{
	int n, i;
	int j, k;
	u32int *h;
	u32int *a, *b;
	int (*compare)(void*, void*);

	if(heap->nheap == 0)
		return -1;

	h = heap->heap;
	k = heap->size;
	n = heap->nheap-1;
	compare = heap->compare;

	heap->nheap--;

	memmove(v, h, k<<2);
	memmove(h, h+n*k, k<<2);

	/* restore heap property */
	for(i=0, j=1; j<=n; i=j, j=2*i+1){
		if(j<n){
			a = h+j*k;
			if(compare(a, a+k) >= 0)
				j++;
		}
		a = h+i*k;
		b = h+j*k;
		if(compare(a, b) <= 0)
			break;
		swap(a, b, k);
	}
	return 0;
}

#define HEAPTEST
#ifdef HEAPTEST
enum {
	NHeap = 50,
};

static int
heapcmp(void *v1, void *v2)
{
	u32int *p1, *p2;

	p1 = (u32int*)v1;
	p2 = (u32int*)v2;

	if(*p1 < *p2)
		return +1;
	else if(*p1 > *p2)
		return -1;
	return 0;
}

static void
dumpheap(Heap *heap)
{
	int i;

	for(i=0; i<heap->nheap; i++)
		print("\t%d\t%d\n", i, heap->heap[i]);
	return;
}

int
heaptest(void)
{
	int i, x;
	Heap *heap;

	heap = mkheap(heapcmp, sizeof(u32int), NHeap);

	for(i=0; i<NHeap; i++){
		x = i+1;
		heapinsert(heap, &x);
	}
	for(i=0; i<NHeap; i++){
		heapextractmin(heap, &x);
		print("%d\n", x);
	}

	freeheap(heap);
	return 0;
}
#endif
