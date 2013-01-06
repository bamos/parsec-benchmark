// Written by Christian Bienia
// The code in this file defines the interface for a memory pool of Cell structures.
// It serves three purposes:
//   1.) To minimize calls to malloc and free as much as possible
//   2.) To reuse cell structures as much as possible
//   3.) To eliminate unnecessary synchronization for memory allocation

#ifndef __CELLPOOL_HPP__
#define __CELLPOOL_HPP__ 1

#include "fluid.hpp"



//A simple list of data blocks headers, required for internal memory management of pool
//NOTE: Do not add additional member variables or the padding might be wrong.
struct datablockhdr {
  struct datablockhdr *next;
  //NOTE: This form of padding will break if additional variables are added to the structure
  //because it does not account for compiler-inserted padding between structure members.
  char padding[CACHELINE_SIZE - sizeof(datablockhdr *) % CACHELINE_SIZE];
};

//The memory pool data structure
//The allocated memory is accessible twice: As a linked list of cells and also as a linked
//list of data blocks. The data blocks preserve the original block structure as returned
//by malloc, the cell list breaks the blocks down into smaller cell structures which can
//be used by the program as needed.
typedef struct {
  //linked list of available cells
  struct Cell *cells;
  //number of cells allocated so far (NOT number of cells currently available in pool)
  int alloc;
  //linked list of allocated data blocks (required for free operation)
  struct datablockhdr *datablocks;
} cellpool;



//Initialize the memory pool
//particles is used to determine the initial capacity and should correspond to the
//number of particles that the pool is expected to manage
void cellpool_init(cellpool *pool, int particles);

//Get a Cell structure from the memory pool
Cell *cellpool_getcell(cellpool *pool);

//Return a Cell structure to the memory pool
void cellpool_returncell(cellpool *pool, Cell *cell);

//Destroy the memory pool
void cellpool_destroy(cellpool *pool);

#endif //__CELLPOOL_HPP__
