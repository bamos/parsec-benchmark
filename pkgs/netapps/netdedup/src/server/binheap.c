/* Based on Data Structures and Algorithm Analysis in C (Second Edition)
 * by Mark Allen Weiss.
 *
 * Modified by Christian Bienia, Minlan Yu.
 */

#include <stdlib.h>
#include <stdio.h>
#include "binheap.h"

/* Macros for error handling */
#define Error( Str ) FatalError( Str )
#define FatalError( Str ) fprintf( stderr, "Binary Heap: %s\n", Str ), exit( 1 )


#define MinPQSize (16)

PriorityQueue Initialize( int InitCapacity ) {
  PriorityQueue H;

  if( InitCapacity < MinPQSize ) {
    Error( "Priority queue size is too small" );
  }

  H = malloc( sizeof( struct HeapStruct ) );
  if( H ==NULL ) {
    FatalError( "Out of space!!!" );
  }

  /* Allocate the array plus one extra for sentinel */
  H->Elements = malloc( ( InitCapacity + 1 ) * sizeof( HeapElementType ) );
  if( H->Elements == NULL ) {
    FatalError( "Out of space!!!" );
  }

  H->Capacity = InitCapacity;
  H->Size = 0;
  H->Elements[0] = NULL;

  return H;
}

void MakeEmpty( PriorityQueue H ) {
  H->Size = 0;
}

void Insert( HeapElementType X, PriorityQueue H ) {
  int i;

  if( IsFull( H ) ) {
    /* double capacity of heap */
    H->Capacity = 2 * H->Capacity;
    H->Elements = realloc( H->Elements, ( H->Capacity + 1 ) * sizeof( HeapElementType ) );
    if( H->Elements == NULL ) {
      FatalError( "Out of space!!!" );
    }
  }

  /* NOTE: H->Element[ 0 ] is a sentinel */

  if (H->Size == 0) {
    H->Elements[1] = X;
    H->Size = 1;
    return;
  }

  H->Size++;
  for( i = H->Size; H->Elements[i/2]!= NULL && H->Elements[ i / 2 ]->sequence.l2num > X->sequence.l2num; i /= 2 ) {
    H->Elements[ i ] = H->Elements[ i / 2 ];
  }
  H->Elements[ i ] = X;
}

HeapElementType DeleteMin( PriorityQueue H ) {
  HeapElementType MinElement, LastElement;
  int i, Child;

  if( IsEmpty( H ) ) {
    Error( "Priority queue is empty" );
    return H->Elements[ 0 ];
  }
  MinElement = H->Elements[ 1 ];
  LastElement = H->Elements[ H->Size-- ];

  for( i = 1; i * 2 <= H->Size; i = Child ) {
    /* Find smaller child */
    Child = i * 2;
    if( Child != H->Size && H->Elements[ Child + 1 ]->sequence.l2num < H->Elements[ Child ]->sequence.l2num ) {
      Child++;
    }

    /* Percolate one level */
    if( LastElement->sequence.l2num > H->Elements[ Child ]->sequence.l2num ) {
      H->Elements[ i ] = H->Elements[ Child ];
    } else {
      break;
    }
  }
  H->Elements[ i ] = LastElement;
  return MinElement;
}

HeapElementType FindMin( PriorityQueue H ) {
  if( !IsEmpty( H ) ) {
    return H->Elements[ 1 ];
  } else {
    //FatalError( "Priority Queue is Empty" );
    return NULL;
  }
}

int IsEmpty( PriorityQueue H ) {
  return (H->Size == 0);
}

int IsFull( PriorityQueue H ) {
  return H->Size == H->Capacity;
}

int NumberElements( PriorityQueue H ) {
  return H->Size;
}

void Destroy( PriorityQueue H ) {
  free( H->Elements );
  free( H );
}

