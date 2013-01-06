// Written by Christian Bienia
// The code in this file implements a memory pool of Cell structures.
// It serves three purposes:
//   1.) To minimize calls to malloc and free as much as possible
//   2.) To reuse cell structures as much as possible
//   3.) To eliminate unnecessary synchronization for memory allocation

#include <iostream>

#include <stdlib.h>
#include <assert.h>

#include "fluid.hpp"
#include "cellpool.hpp"



/* *** REMINDER ***
 The following asserts were added to the serial program:
 1. assert struct Cell aligned
 2. assert struct Cell and struct Cell_aux same size
*/



// Define ENABLE_MALLOC_FALLBACK to replace the more sophisticated cell poool
// implementation that uses its own memory management with a simple implementation
// that calls malloc & free directly every time a new cell is requested or
// released. This is only intended for debugging.
//#define ENABLE_MALLOC_FALLBACK

#ifndef ENABLE_MALLOC_FALLBACK
//Allocate and initialize a new data block for `cells' number of cells
//Data blocks will have the following format:
//
//   | struct datablockhdr | struct Cell | struct Cell | ..... |
//
//The cells inside the block will be connected to a NULL-terminated linked list
//with the cell at the lowest memory location being the first of its elements.
static struct datablockhdr *cellpool_allocblock(int cells) {
  struct datablockhdr *block = NULL;
  struct Cell *temp1, *temp2;
  int i;

  //allocate a full block
  assert(cells > 0);
#if defined(WIN32)
  block = (struct datablockhdr *)_aligned_malloc(sizeof(struct datablockhdr) + cells * sizeof(struct Cell), CACHELINE_SIZE);
  assert(block);
#elif defined(SPARC_SOLARIS)
  block = (struct datablockhdr *)memalign(CACHELINE_SIZE, sizeof(struct datablockhdr) + cells * sizeof(struct Cell));
#else
  int rv = posix_memalign((void **)(&block), CACHELINE_SIZE, sizeof(struct datablockhdr) + cells * sizeof(struct Cell));
  assert(!rv);
#endif

  //initialize header and cells
  block->next = NULL;
  temp1 = (struct Cell *)(block+1);
  for(i=0; i<cells; i++) {
    //If all structures are correctly padded then all pointers should also be correctly aligned,
    //but let's verify that nevertheless because the padding might change.
    assert((uint64_t)(temp1) % sizeof(void *) == 0);
    if(i != cells-1) {
      temp2 = temp1 + 1;
      temp1->next = temp2;
      temp1 = temp2;
    } else {
      //last Cell structure in block
      temp1->next = NULL;
    }
  }

  return block;
}

//Initialize the memory pool
//particles is used to determine the initial capacity and should correspond to the
//number of particles that the pool is expected to manage
void cellpool_init(cellpool *pool, int particles) {
  int ALLOC_MIN_CELLS = 1024;
  assert(sizeof(struct datablockhdr) % CACHELINE_SIZE == 0);
  assert(pool != NULL);
  assert(particles > 0);

  //Allocate the initial data, let's start with 4 times more cells than
  //best case (ignoring statically allocated Cells structures)
  pool->alloc = 4 * (particles/PARTICLES_PER_CELL); //PARTICLES_PER_CELL particles per cell structure
  pool->alloc = pool->alloc < ALLOC_MIN_CELLS ? ALLOC_MIN_CELLS : pool->alloc;
  pool->datablocks = cellpool_allocblock(pool->alloc);
  pool->cells = (struct Cell *)(pool->datablocks + 1);
}

//Get a Cell structure from the memory pool
Cell *cellpool_getcell(cellpool *pool) {
  struct Cell *temp;

  assert(pool != NULL);

  //If no more cells available then allocate more
  if(pool->cells == NULL) {
    //keep doubling the number of cells
    struct datablockhdr *block = cellpool_allocblock(pool->alloc);
    pool->alloc = 2 * pool->alloc;
    block->next = pool->datablocks;
    pool->datablocks = block;
    pool->cells = (struct Cell *)(pool->datablocks + 1);
  }

  //return first cell in list
  temp = pool->cells;
  pool->cells = temp->next;
  temp->next = NULL;
  return temp;
}

//Return a Cell structure to the memory pool
void cellpool_returncell(cellpool *pool, Cell *cell) {
  assert(pool != NULL);
  assert(cell != NULL);
  cell->next = pool->cells;
  pool->cells = cell;
}

//Destroy the memory pool
void cellpool_destroy(cellpool *pool) {
  assert(pool != NULL);

  //iterate through data blocks and free them all, this will also free all cells
  struct datablockhdr *ptr = pool->datablocks;
  struct datablockhdr *temp;
  while(ptr != NULL) {
    temp = ptr;
    ptr = ptr->next;
#if defined(WIN32)
    _aligned_free(temp);
#else
    free(temp);
#endif
  }
}

#else //ENABLE_MALLOC_FALLBACK

//Do nothing because there is no cell pool
void cellpool_init(cellpool *pool, int particles) {
  std::cout << "WARNING: Malloc fallback enabled for cell pool." << std::endl;
}

//Get a Cell structure
Cell *cellpool_getcell(cellpool *pool) {
  Cell *cell;

  cell = (struct Cell *)malloc(sizeof(struct Cell));
  assert(cell != NULL);
  return cell;
}

//Return a Cell structure
void cellpool_returncell(cellpool *pool, Cell *cell) {
  free(cell);
}

//Do nothing because there is no cell pool
void cellpool_destroy(cellpool *pool) {
  return;
}

#endif //ENABLE_MALLOC_FALLBACK



//Uncomment to turn this file into an independent program that runs a self-test
//#define ENABLE_TESTER

#ifdef ENABLE_TESTER

#include <stdio.h>

//constants to compute  contents of array elements (based on type of array)
const float p_vec_const = 0.0;
const float hv_vec_const = 1.0;
const float v_vec_const = 2.0;
const float a_vec_const = 3.0;
const float density_vec_const = 0.1;
const Vec3 coord_const(-1.1, -2.2, -3.3);

//fill a cell with constant, predetermined data
void write_cell(struct Cell *cell) {
  int i;

  //write to every element of cell to break as much as possible if pointers are wrong
  for(i=0; i<PARTICLES_PER_CELL; i++) {
    void *addr;

    //values depend only on memory location and constants
    addr = (void *)&(cell->p[i]);
    cell->p[i] = coord_const + p_vec_const + (uint64_t)addr;
    addr = (void *)&(cell->hv[i]);
    cell->hv[i] = coord_const + hv_vec_const + (uint64_t)addr;
    addr = (void *)&(cell->v[i]);
    cell->v[i] = coord_const + v_vec_const + (uint64_t)addr;
    addr = (void *)&(cell->a[i]);
    cell->a[i] = coord_const + a_vec_const + (uint64_t)addr;
    addr = (void *)&(cell->density[i]);
    cell->density[i] = density_vec_const + (uint64_t)addr;
  }
}

//check whether a cell filled with write_cell has still the same data
//returns true if the data has not changed
bool check_cell(struct Cell *cell) {
  Vec3 dummy;
  bool rv = true;
  int i;

  //check every element of cell
  for(i=0; i<PARTICLES_PER_CELL; i++) {
    void *addr;

    //values depend only on memory location and constants
    addr = (void *)&(cell->p[i]);
    dummy = coord_const + p_vec_const + (uint64_t)addr;
    rv = rv && (dummy == cell->p[i]);
    addr = (void *)&(cell->hv[i]);
    dummy = coord_const + hv_vec_const + (uint64_t)addr;
    rv = rv && (dummy == cell->hv[i]);
    addr = (void *)&(cell->v[i]);
    dummy = coord_const + v_vec_const + (uint64_t)addr;
    rv = rv && (dummy == cell->v[i]);
    addr = (void *)&(cell->a[i]);
    dummy = coord_const + a_vec_const + (uint64_t)addr;
    rv = rv && (dummy == cell->a[i]);
    addr = (void *)&(cell->density[i]);
    float fdummy = density_vec_const + (uint64_t)addr;
    rv = rv && (fdummy == cell->density[i]);
  }

  return rv;
}


//A simple self-test program that stresses the cell pool a little
//The basic strategy is to allocate lots of cells and write pre-determined values to
//the cell arrays. Then we read the values to make sure they haven't been overwritten.
int main() {
  int nCells = 2 * 1000 * 1000; //test with 2 million cells
  const int size_array = 389; //number of statically allocated cells (a prime number)
  struct Cell cells[size_array]; //array of dummy cells, serves as entry points for lists
  cellpool *pool;
  int i;

  printf("Initializing...\n");fflush(NULL);

   //init with low number of particles to cause lots of work
  cellpool_init(&pool, 1);

  //initialize statically allocated cells
  for(i=0; i<size_array; i++) {
    write_cell(&(cells[i]));
    cells[i].next = NULL;
  }

  printf("Testing (1st pass): ");fflush(NULL);

  //allocate all cells and initialize them with predetermined values
  for(i=0; i<nCells; i++) {
    struct Cell *ptr, *temp;

    //get a new cell and append it to lists in round-robin way
    temp = cellpool_getcell(pool);
    write_cell(temp);
    ptr = &(cells[i % size_array]);
    while(ptr->next != NULL) {
      ptr = ptr->next;
    }
    ptr->next = temp;

    //print a progress message every 1/10th of the work
    if((i+1) % (nCells/10) == 0) {
      printf("...%i%%", (i+1) / (nCells/100));fflush(NULL);
    }
  }
  printf("\n");fflush(NULL);

  //now make a 2nd pass and check that all values are still correct
  printf("Testing (2nd pass): ");fflush(NULL);
  int count = 0;
  for(i=0; i<size_array; i++) {
    struct Cell *ptr;

    ptr = &(cells[i]);
    do {
      if(!check_cell(ptr)) {
        printf("ERROR: Cell contents altered!\n");
        exit(1);
      }

      count++;
      //print a progress message every 1/10th of the work
      if((count+1) % (nCells/10) == 0) {
        printf("...%i%%", (count+1) / (nCells/100));fflush(NULL);
      }

      ptr = ptr->next;
    } while(ptr != NULL);
  }
  printf("\n");fflush(NULL);

  //cleanup & shutdown
  printf("Cleanup...\n");fflush(NULL);
  cellpool_destroy(&pool);
  printf("Terminating...\n");fflush(NULL);
  return 0;
}
#endif //ENABLE_TESTER
