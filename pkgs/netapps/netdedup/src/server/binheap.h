/* Based on Data Structures and Algorithm Analysis in C (Second Edition)
 * by Mark Allen Weiss.
 *
 * Modified by Christian Bienia, Minlan Yu.
 */
#ifndef _BINHEAP_H
#define _BINHEAP_H

#include "dedupdef.h"


typedef chunk_t * HeapElementType;

/* Type of a priority queue. HeapStruct is private and should not be used directly */
struct HeapStruct {
  int Capacity;
  int Size;
  HeapElementType * Elements;
};

typedef struct HeapStruct * PriorityQueue;

/* Create an empty priority queue with initial capacity 'InitCapacity' */
PriorityQueue Initialize( int InitCapacity );

/* Free all data structures of PriorityQueue */
void Destroy( PriorityQueue H );

/* Delete contents of priority queue */
void MakeEmpty( PriorityQueue H );

/* Add element X to priority queue H, automatically increases capacity if queue is full */
void Insert( HeapElementType X, PriorityQueue H );

/* Return the smallest element in the priority queue (if not empty) */
HeapElementType FindMin( PriorityQueue H );

/* Delete the smallest element in the priority queue (if not empty) */
HeapElementType DeleteMin( PriorityQueue H );

/* Check whether priority queue is empty */
int IsEmpty( PriorityQueue H );

/* Check whether priority queue is full and calling Insert would result in memory reallocation */
int IsFull( PriorityQueue H );

/* Return number of elements in priority queue */
int NumberElements( PriorityQueue H );

#endif /* _BINHEAP_H */

